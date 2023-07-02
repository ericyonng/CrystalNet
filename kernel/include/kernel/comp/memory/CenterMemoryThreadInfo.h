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
 * Date: 2023-07-02 00:11:12
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_CENTER_MEMORY_THREAD_INFO_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_CENTER_MEMORY_THREAD_INFO_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Lock/Lock.h>
#include <kernel/comp/memory/MemoryBlock.h>

KERNEL_BEGIN

class MemoryAlloctor;
struct MergeMemoryBufferInfo;
class MemoryBuffer;

struct KERNEL_EXPORT MergeMemoryAlloctorInfo
{
    UInt64 _alloctorCreateThreadId; // 分配器创建时所在线程id

    // 合并的buffer信息
    MergeMemoryBufferInfo *_head;
    MergeMemoryBufferInfo *_tail;
    UInt64 _count;
    std::map<MemoryBuffer *, MergeMemoryBufferInfo *> _memoryBufferRefMergeInfo;
};

class KERNEL_EXPORT CenterMemoryThreadInfo
{
public:
    CenterMemoryThreadInfo(UInt64 threadId);
    ~CenterMemoryThreadInfo();

    LibString ToString() const;

    // block添加
    void PushBlock(MemoryBlock *block);

    // 规整block
    UInt64 MergeBlocks();

    // 合并到Alloctor
    void MergeToAlloctor();

    // 是否清空了
    bool IsEmpty() const;

private:
    const UInt64 _threadId;

    // 交换区
    SpinLock _lck;                              // 线程安全
    MemoryBlock *_head;                         // 最终合并到该链表
    MemoryBlock *_headSwap;                     // 临时放置的链表
    std::atomic<UInt64> _pendingBlockCount;     // 待处理的block总数量

    // 整理后每个MemoryAlloctor的信息
    std::map<MemoryAlloctor *, MergeMemoryAlloctorInfo> _memroyAlloctorRefInfo;
};

ALWAYS_INLINE void CenterMemoryThreadInfo::PushBlock(MemoryBlock *block)
{
    _lck.Lock();
    block->_next = _headSwap;
    _headSwap = block;
    ++_pendingBlockCount;
    _lck.Unlock();
}

ALWAYS_INLINE bool CenterMemoryThreadInfo::IsEmpty() const
{
    auto ret = _pendingBlockCount == 0;

    return ret || _memroyAlloctorRefInfo.empty();
}

KERNEL_END

#endif
