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

KERNEL_BEGIN

CenterMemoryThreadInfo::CenterMemoryThreadInfo(UInt64 threadId)
:_threadId(threadId)
,_head(NULL)
,_headSwap(NULL)
{

}

CenterMemoryThreadInfo::~CenterMemoryThreadInfo()
{

}

LibString CenterMemoryThreadInfo::ToString() const
{
    return LibString().AppendFormat("thread id:%llu", _threadId);
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

    for(;_head;)
    {
        auto alloctor = _head->_buffer->_alloctor;
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
}

KERNEL_END