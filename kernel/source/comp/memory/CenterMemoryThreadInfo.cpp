/*!
 *  MIT License
 *  
 *  Copyright (c) 2020 ericyonng<120453674@qq.com>
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 * 
 * Date: 2023-07-02 00:11:36
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/memory/CenterMemoryThreadInfo.h>
#include <kernel/comp/memory/MemoryAlloctor.h>
#include <kernel/comp/memory/MergeMemoryBufferInfo.h>
#include <kernel/comp/memory/CenterMemoryProfileInfo.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/memory/CenterMemoryCollector.h>
#include <kernel/comp/Tls/TlsStack.h>
#include <kernel/comp/memory/MemoryBlock.h>
#include <kernel/comp/memory/MemoryBuffer.h>
#include <kernel/comp/Tls/Tls.h>

KERNEL_BEGIN

CenterMemoryThreadInfo::CenterMemoryThreadInfo(UInt64 threadId, CenterMemoryCollector *collector)
:_threadId(threadId)
,_collector(collector)
,_head(NULL)
,_headSwap(NULL)
,_pendingBlockCount{0}
,_historyBlockCount{0}
,_tlsStack(NULL)
,_isQuit{false}
{

}

CenterMemoryThreadInfo::~CenterMemoryThreadInfo()
{

}

LibString CenterMemoryThreadInfo::ToString() const
{
    const UInt64 pendingCount = _pendingBlockCount.load();
    const UInt64 historyCount = _historyBlockCount.load();
    LibString info;

    info.AppendFormat("thread id:%llu, history count:%llu pendingCount:%llu\n", _threadId, historyCount, pendingCount);

    // 性能信息
    _profileLck.Lock();
    const auto profileInfo = _memoryAlloctorRefBlockCount;
    _profileLck.Unlock();

    for(auto iter : profileInfo)
    {
        auto alloctor = iter.first;
        info.AppendFormat("memoryalloctor adddr:%p, create source:%s, unit block bytes:%llu, \nhistory center colloctor merge block count:%llu, current pending merge count:%llu"
                    , alloctor, alloctor->GetCreateSource().c_str(), alloctor->GetUnitSize(), iter.second->_historyBlockCount, iter.second->_currentPendingCount);
    }

    return info;
}

UInt64 CenterMemoryThreadInfo::MergeBlocks()
{
    if(LIKELY(_pendingBlockCount == 0))
        return 0;

    UInt64 blockCount = 0;
    _lck.Lock();
    auto toSwap = _head;
    _head = _headSwap;
    _headSwap = toSwap;
    blockCount = _pendingBlockCount;
    _pendingBlockCount = 0;
    _lck.Unlock();

    // 丢进来的memoryalloctor 需要按照线程区分并丢给对应的线程TODO:
    std::map<MemoryAlloctor *, UInt64> profileCache;
    for(;_head;)
    {
        auto memoryBlock = _head;
        _head = _head->_next;

        auto alloctor = memoryBlock->_buffer->_alloctor;

        if(!alloctor->IsThreadLocalCreate())
        {
            alloctor->Lock();
            alloctor->Free(memoryBlock);
            alloctor->Unlock();
            continue;
        }

        if(alloctor->GetOwnerThreadId() != _threadId)
        {
            auto threadInfo = _collector->GetThreadInfo(alloctor->GetOwnerThreadId());
            threadInfo->_MergeBlocks(alloctor, memoryBlock, profileCache);
        }
        else
        {
            _MergeBlocks(alloctor, memoryBlock, profileCache);
        }
    }

    for(auto iter = profileCache.begin(); iter != profileCache.end();)
    {
        if(iter->first->GetOwnerThreadId() != _threadId)
        {
            auto threadInfo = _collector->GetThreadInfo(iter->first->GetOwnerThreadId());
            threadInfo->_MergeProfileInfo(iter->first, iter->second);
        }
        else
        {
            _MergeProfileInfo(iter->first, iter->second);
        }

        iter = profileCache.erase(iter);
    }

    return blockCount;
}

void CenterMemoryThreadInfo::MergeToAlloctor()
{
    for(auto iter = _memroyAlloctorRefInfo.begin(); iter != _memroyAlloctorRefInfo.end();)
    {
        auto alloctor = iter->first;
        auto &alloctorInfo = iter->second;
        alloctor->PushMergeList(alloctorInfo._count, alloctorInfo._head, alloctorInfo._tail);
        iter = _memroyAlloctorRefInfo.erase(iter);
    }

    // 性能信息
    for(auto iter : _memoryAlloctorRefBlockCount)
    {
        _profileLck.Lock();
        auto profileInfo = iter.second;
        profileInfo->_currentPendingCount = 0;
        _profileLck.Unlock();
    }
}

void CenterMemoryThreadInfo::OnThreadWillQuit()
{
    _isQuit = true;

    // tls资源由CenterCollector释放
}

void CenterMemoryThreadInfo::OnCollectorThreadDown()
{
    _Clear();
}

void CenterMemoryThreadInfo::SetForceFreeIdleBuffer(bool force)
{
    _tlsStack->GetDef()->_isForceFreeIdleBuffer = force;
}

UInt64 CenterMemoryThreadInfo::GetAllocBytes() const
{
    return _tlsStack->GetDef()->_alloctorTotalBytes;
}

void CenterMemoryThreadInfo::_Clear()
{
    _profileLck.Lock();
    ContainerUtil::DelContainer(_memoryAlloctorRefBlockCount);
    _profileLck.Unlock();

    // 不是收集器所在线程的tls都清理掉
    if(_collector->GetWorkerThreadId() != _threadId)
        TlsUtil::DestroyTlsStack(_tlsStack);

    _tlsStack = NULL;
}

void CenterMemoryThreadInfo::_MergeBlocks(MemoryAlloctor *alloctor, MemoryBlock *memoryBlock, std::map<MemoryAlloctor *, UInt64> &profileCache)
{
    auto iter = _memroyAlloctorRefInfo.find(alloctor);
    if(iter == _memroyAlloctorRefInfo.end())
    {
        iter = _memroyAlloctorRefInfo.insert(std::make_pair(alloctor, MergeMemoryAlloctorInfo())).first;
        auto &info = iter->second;
        info._alloctorCreateThreadId = alloctor->GetOwnerThreadId();
        info._head = NULL;
        info._tail = NULL;
        info._count = 0;

    }
    auto &info = iter->second;

    // 性能信息
    auto iterProfile = profileCache.find(alloctor);
    if(iterProfile == profileCache.end())
        iterProfile = profileCache.insert(std::make_pair(alloctor, 0)).first;

    iterProfile->second += 1;

    // buffer链表
    auto iterBufferInfo = info._memoryBufferRefMergeInfo.find(memoryBlock->_buffer);
    if(iterBufferInfo == info._memoryBufferRefMergeInfo.end())
    {
        auto newBufferInfo = new MergeMemoryBufferInfo;
        newBufferInfo->_buffer = memoryBlock->_buffer;

        if(info._head == NULL)
            info._tail = newBufferInfo;

        newBufferInfo->_next = info._head;
        info._head = newBufferInfo;
        info._count += 1;
        iterBufferInfo = info._memoryBufferRefMergeInfo.insert(std::make_pair(memoryBlock->_buffer, newBufferInfo)).first;
    }

    auto newBufferInfo = iterBufferInfo->second;

    // 设置链表尾
    if(newBufferInfo->_head == NULL)
        newBufferInfo->_tail = memoryBlock;

    // 更新链表头
    memoryBlock->_next = newBufferInfo->_head;
    newBufferInfo->_head = memoryBlock;

    // 更新数量
    newBufferInfo->_count += 1;
}

void CenterMemoryThreadInfo::_MergeProfileInfo(MemoryAlloctor *alloctor, UInt64 addCount)
{
    _profileLck.Lock();
    auto iterProfile = _memoryAlloctorRefBlockCount.find(alloctor);
    if(iterProfile == _memoryAlloctorRefBlockCount.end())
        iterProfile = _memoryAlloctorRefBlockCount.insert(std::make_pair(alloctor, new CenterMemoryProfileInfo)).first;

    iterProfile->second->_historyBlockCount += addCount;
    iterProfile->second->_currentPendingCount += addCount;
    _profileLck.Unlock();
}



KERNEL_END