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
 * Date: 2020-11-07 01:45:25
 * Author: Eric Yonng
 * Description: 当middle_area超过数组上限时候淘汰掉最后一个内存,转移到gc区等待下次被释放内存 ，超过一个周期且gc游标转过一圈则执行一次gc
                newBlock是野指针可能的原因：1.被gc掉了，stack返回一个-1
                分配器分 
                1.活跃区内存块链表 _activeBuffersHead
                2.释放区栈链表（buffer中的元素没有完全被释放）
                3.待gc区_deadBufferHead

                分配算法：
                1.优先从释放区栈链表取内存，为了重复利用被释放的内存
                2.释放去栈链表没有可用内存，则优先使用活跃区内存
                3.活跃区没有内存，则向gc区取待gc的内存
                4.若gc区没有内存则创建一个新的大内存块MemoryBuffer，并加入到活跃区，并获取一个newBlock
                newBlock+_memoryBlockHeadSize
                5.更新gc状态
                6.执行gc

                gc算法
                当达到限制的一半的时候把末尾节点塞到gc区，新的emptybuffer插到节点头
                另一个线程swap下gc去的内存块，并释放内存（cpu不繁忙，时候清理内存）
                当达到emptylimit的一半时候记下emptybuffer在双向链表中的节点，当达到gc条件时候从中间断开链表再把要gc的emptybuffer链表转移到gc器
                待gc区要有一个节点计数器，用于计算断开时的节点

                为了避免频繁归并内存, 跨线程的时候请使用跨线程版本的分配内存, 生命周期只在单线程的, 使用线程本地版本的分配内存
                合并内存块MemoryBuffer13w个每次
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_MEMORY_ALLOCTOR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_MEMORY_ALLOCTOR_H__

#pragma once

#include <atomic>

#include <kernel/kernel_export.h>
#include <kernel/comp/memory/MemoryBlock.h>
#include <kernel/comp/Lock/Impl/SpinLock.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/memory/MemoryDefs.h>

KERNEL_BEGIN

struct MemoryAlloctorConfig;
class CenterMemoryCollector;
class GarbageCollector;
struct MergeMemoryBufferInfo;

class TlsDefaultObj;

// 线程不安全
class KERNEL_EXPORT MemoryAlloctor
{
    MemoryAlloctor(const MemoryAlloctor&) = delete;
    MemoryAlloctor(MemoryAlloctor&&) = delete;
    MemoryAlloctor &operator = (const MemoryAlloctor &) = delete;

public:
    MemoryAlloctor(const MemoryAlloctorConfig &config);
    virtual ~MemoryAlloctor();

    void *Alloc(UInt64 objBytes);

    // TODO:需要解决重复释放的问题,重复释放ref<0 需要asset掉 如果block不属于本分配器但是属于同一线程下其他分配器分配的那么会返回其他分配器Alloctor
    void Free(void *ptr);
    void Free(MemoryBlock *block);
    MemoryBlock *GetMemoryBlockBy(void *ptr);
    void AddRef(void *ptr);
    LibString ToString() const;
    LibString UsingInfo() const;

    const std::string &GetCreateSource() const;

    // 初始化与销毁
    void Init(bool isThreadLocalCreate, UInt64 initBlockNumPerBuffer = MEMORY_BUFFER_BLOCK_INIT, const std::string &source = "");
    void Destroy();

    void Lock();
    void Unlock();

    UInt64 GetTotalBytes() const;
    UInt64 GetBytesInUse() const;
    UInt64 GetBlockNumInUse() const;
    UInt64 GetTotalBlockNum() const;
    UInt64 GetBlockSize() const;
    // 要分配的大小,不包含头,以及对齐
    UInt64 GetUnitSize() const;
    UInt64 GetOwnerThreadId() const;

    bool CheckOwner(const MemoryBlock *block) const;

    void PushMergeList(UInt64 memoryBuffMergeNum, MergeMemoryBufferInfo *head, MergeMemoryBufferInfo *tail);

    void ForceMergeBlocks();

    bool IsThreadLocalCreate() const;

private:
    MemoryBuffer *_NewBuffer();
    void _GcBuffer(MemoryBuffer *buffer);

    void _AddToList(MemoryBuffer *&listHead, MemoryBuffer *buffer);
    void _RemoveFromList(MemoryBuffer *&listHead, MemoryBuffer *buffer);
    void _JudgeBufferRecycle(MemoryBuffer *buffer);

    void _DoFreeBlock(MemoryBlock *block);

    void _MergeBlocks();

    void _MergeBlocks(MergeMemoryBufferInfo *memoryBufferInfo);
    void _Recycle(UInt64 recycleNum, MergeMemoryBufferInfo *bufferInfoListHead, MergeMemoryBufferInfo *bufferInfoListTail);

    // memoryAlloctor链表 用于合并内存
    // 线程安全接口
    void _RemoveSelfFromDurtyList();
    void _AddSelfToDurtyListIfNotIn();

protected:
    // 内存分配收集器
    friend class AlloctorInfoCollector;

    // 属性
    const UInt64 _unitBytesToAlloc;              // 要分配的内存大小
    const UInt64 _blockSizeAfterAlign;           // 内存对齐后的blocksize
    const UInt64 _bufferBlockNumLimit;           // 每个buffer block数量的上限
    const UInt64 _memoryBlockHeadSize;           // 内存块头内存对齐后的大小
    const UInt64 _initMemoryBufferNum;           // 初始化memory buffer个数
    UInt64 _initMinBlockCntPerBuffer;            // 初始化时最小内存块数量
    UInt64 _curBlockCntPerBuffer;                // 当前每个buffer拥有的block个数 开始初始化时候是 MEMORY_BUFFER_BLOCK_INIT 个
    UInt64 _blockCountInUsed;                    // 当前内存块使用情况
    UInt64 _bytesInUsed;                         // 当前占用的字节数
    UInt64 _totalBytes;                          // 总内存占用
    UInt64 _totalBlockAmount;                    // 内存块总量
    UInt64 _trigerNewBufferWhenAlloc;            // 在分配内存时触发NewBuffer总次数,这个指标比较影响性能,频繁的触发new buffer会导致系统性能下降
    std::string _createSource;                   // 创建alloctor来源
    UInt64 _threadId;                            // alloctor 所在线程id 原则是在哪个线程创建分配器就在哪个线程分配和释放内存
    std::atomic_bool _isInit;
    bool _isThreadLocalCreate;                  // 分配器是否由thread_local创建

    /* 新版本简单实现 原则, 新操作的放到队列头,以便 cache 命中 */
    MemoryBuffer *_activeHead;                  // 活跃队列
    MemoryBuffer *_busyHead;                    // buffer内的block都分配完了则插入到队列头
    Int64 _curActiveBufferNum;
    
    // 锁
    SpinLock _lck;
    GarbageCollector *_gc;

    // 其他线程归并过来的内存 key:buffer, value:UInt64:block的数量, 第一个MemoryBlock:block链表头, 第二个MemoryBlock链表尾
    SpinLock _toMergeLck;
    std::atomic_bool _needMerge;
    std::atomic<UInt64> _bufferToMergeCount;
    MergeMemoryBufferInfo *_mergeBufferList;
    MergeMemoryBufferInfo *_mergeBufferListSwap;

    CenterMemoryCollector *_centerMemroyCollector;

public:
    TlsDefaultObj *_tlsDefaultObj;
};

ALWAYS_INLINE void MemoryAlloctor::Free(void *ptr)
{
    MemoryBlock *block = GetMemoryBlockBy(ptr);
    Free(block);
}

ALWAYS_INLINE MemoryBlock *MemoryAlloctor::GetMemoryBlockBy(void *ptr)
{
    return reinterpret_cast<MemoryBlock *>(reinterpret_cast<Byte8 *>(ptr) - _memoryBlockHeadSize);
}

ALWAYS_INLINE void MemoryAlloctor::AddRef(void *ptr)
{
    MemoryBlock *block = GetMemoryBlockBy(ptr);
    ++block->_ref;
}

ALWAYS_INLINE LibString MemoryAlloctor::ToString() const
{
    return UsingInfo();
}

ALWAYS_INLINE const std::string &MemoryAlloctor::GetCreateSource() const
{
    return _createSource;
}

ALWAYS_INLINE void MemoryAlloctor::Lock()
{
    _lck.Lock();
}

ALWAYS_INLINE void MemoryAlloctor::Unlock()
{
    _lck.Unlock();
}

ALWAYS_INLINE UInt64 MemoryAlloctor::GetTotalBytes() const
{
    return _totalBytes;
}

ALWAYS_INLINE UInt64 MemoryAlloctor::GetBytesInUse() const
{
    return _bytesInUsed;
}

ALWAYS_INLINE UInt64 MemoryAlloctor::GetBlockNumInUse() const
{
    return _blockCountInUsed;
}

ALWAYS_INLINE UInt64 MemoryAlloctor::GetTotalBlockNum() const
{
    return _totalBlockAmount;
}

ALWAYS_INLINE UInt64 MemoryAlloctor::GetBlockSize() const
{
    return _blockSizeAfterAlign;
}

ALWAYS_INLINE UInt64 MemoryAlloctor::GetUnitSize() const
{
    return _unitBytesToAlloc;
}

ALWAYS_INLINE UInt64 MemoryAlloctor::GetOwnerThreadId() const
{
    return _threadId;
}

ALWAYS_INLINE bool MemoryAlloctor::IsThreadLocalCreate() const
{
    return _isThreadLocalCreate;
}

KERNEL_END

#endif
