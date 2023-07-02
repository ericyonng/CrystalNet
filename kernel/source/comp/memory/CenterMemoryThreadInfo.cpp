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

KERNEL_BEGIN

CenterMemoryThreadInfo::CenterMemoryThreadInfo(UInt64 threadId)
:_threadId(threadId)
,_head(NULL)
,_headSwap(NULL)
,_pendingBlockCount{0}
,_historyBlockCount{0}
{

}

CenterMemoryThreadInfo::~CenterMemoryThreadInfo()
{
    ContainerUtil::DelContainer(_memoryAlloctorRefBlockCount);
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


    std::map<MemoryAlloctor *, UInt64> profileCache;
    for(;_head;)
    {
        auto alloctor = _head->_buffer->_alloctor;
        CenterMemoryProfileInfo *profileInfo = NULL;
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
        auto memoryBlock = _head;
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

        _head = _head->_next;
    }

    for(auto iter = profileCache.begin(); iter != profileCache.end();)
    {
        _profileLck.Lock();
        auto iterProfile = _memoryAlloctorRefBlockCount.find(iter->first);
        if(iterProfile == _memoryAlloctorRefBlockCount.end())
            _memoryAlloctorRefBlockCount.insert(std::make_pair(iter->first, new CenterMemoryProfileInfo));

        iterProfile->second->_historyBlockCount += iter->second;
        iterProfile->second->_currentPendingCount += iter->second;
        _profileLck.Unlock();

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

KERNEL_END