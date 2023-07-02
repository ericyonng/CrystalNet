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
 * Date: 2023-07-01 23:57:37
 * Author: Eric Yonng
 * Description: 
 * 1.用于跨线程MemoryAlloctor释放转到重要内存收集器
 * 2.每个线程对应一个数据区用于存放在该线程释放时的内存块信息
 * 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_CENTER_MEMORY_COLLECTOR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_CENTER_MEMORY_COLLECTOR_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/Lock/Lock.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Tls/TlsStack.h>
#include <kernel/comp/BinaryArray.h>
#include <kernel/comp/memory/CenterMemoryTopnThreadInfo.h>
#include <kernel/comp/SmartPtr.h>

KERNEL_BEGIN

class CenterMemoryThreadInfo;
class LibThread;

struct MemoryBlock;
struct MergeMemoryBufferInfo;


class KERNEL_EXPORT CenterMemoryCollector
{
public:
    CenterMemoryCollector();
    ~CenterMemoryCollector();

    static CenterMemoryCollector *GetInstance();

    void Start();
    void WillClose();
    void Close();
    void WaitClose();

    // 每个线程都需要注册到收集器
    void RegisterThreadInfo(UInt64 threadId, TlsStack<TlsStackSize::SIZE_1MB> *tlsStack);

    // 要释放的内存块 只有当Collector结束的时候才会失败, 此时系统已经正在清理, 不需要归还block
    void PushBlock(UInt64 freeThreadId, MemoryBlock *block);

    // 回收内存块信息
    void Recycle(UInt64 recycleNum, MergeMemoryBufferInfo *bufferInfoListHead, MergeMemoryBufferInfo *bufferInfoListTail);

    // 所有分配器都要等Collector结束才能释放
    bool IsWorking() const;

    // 设置扫描时间间隔
    void SetWorkerIntervalMs(Int64 ms);

    // 设置超过该数量时唤醒工作线程
    void SetBlockNumForPurgeLimit(UInt64 blockNumForPurgeLimit);

    // 设置超过该数量时候对内存buffer信息进行回收释放
    void SetRecycleForPurgeLimit(UInt64 recycleForPurgeLimit);

    // 设置所有线程内存分配上限, 超过了有空闲buffer就立即释放 默认4GB
    void SetAllThreadMemoryAllocUpperLimit(UInt64 bytes);

    LibString ToString() const;

private:
    void _OnWorker(LibThread *thread);
    void _DoWorker();

private:
    std::atomic_bool _isDestroy;
    std::atomic_bool _isWorking;
    std::atomic<Int32> _willQuitThreadCount;
    std::atomic<Int32> _totalRegisterThreadCount;   // 排除自己的全部注册的线程数量
    
    SpinLock _registerGuard;                             
    std::unordered_map<UInt64, CenterMemoryThreadInfo *> _threadIdRefThreadInfo;
    LibThread *_worker;

    std::atomic<UInt64> _currentPendingBlockTotalNum;   // 当前总数量
    std::atomic<UInt64> _historyPendingBlockTotalNum;   // 历史总数量
    UInt64 _blockNumForPurgeLimit;                      // 当数量超过时候需要唤醒工作线程 默认 128 * 1024
    Int64 _workIntervalMs;                             // 扫描时间间隔 默认100ms扫描一次

    ConditionLocker _guard;                             

    std::atomic<UInt64> _recycleMemoryBufferInfoCount;      // 回收内存块信息
    std::atomic<UInt64> _historyRecycleMemoryBufferInfoCount;      // 历史回收内存块信息
    UInt64 _recycleForPurgeLimit;                           // 当数量超过时候需要唤醒工作线程 默认 128 * 1024
    MergeMemoryBufferInfo *_head;                           // 内存块信息链表头
    MergeMemoryBufferInfo *_headToSwap;                     // 内存块信息链表头
    SpinLock _recycleGuard;                             

    UInt64 _allThreadMemoryAllocUpperLimit;                 // 所有内存分配上限
    UInt64 _lastScanTotalAllocBytes;                        // 上次扫描时所有线程内存分配情况

    mutable SpinLock _topnGuard;
    BinaryArray<SmartPtr<CenterMemoryTopnThreadInfo>, CenterMemoryTopnThreadInfoComp> *_threadAllocTopn; // 内存分配排行榜
    mutable BinaryArray<SmartPtr<CenterMemoryTopnThreadInfo>, CenterMemoryTopnThreadInfoComp> *_threadAllocTopnSwap; // 内存分配排行榜副本 外部只读取副本
};

ALWAYS_INLINE bool CenterMemoryCollector::IsWorking() const
{
    return _isWorking;
}

ALWAYS_INLINE void CenterMemoryCollector::SetWorkerIntervalMs(Int64 ms)
{
    _workIntervalMs = ms;
}

ALWAYS_INLINE void CenterMemoryCollector::SetBlockNumForPurgeLimit(UInt64 blockNumForPurgeLimit)
{
    _blockNumForPurgeLimit = blockNumForPurgeLimit;
}

ALWAYS_INLINE void CenterMemoryCollector::SetRecycleForPurgeLimit(UInt64 recycleForPurgeLimit)
{
    _recycleForPurgeLimit = recycleForPurgeLimit;
}

ALWAYS_INLINE void CenterMemoryCollector::SetAllThreadMemoryAllocUpperLimit(UInt64 bytes)
{
    _allThreadMemoryAllocUpperLimit = bytes;
}

KERNEL_END

#endif
