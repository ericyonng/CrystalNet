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
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_MEMORY_ALLOCTOR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_MEMORY_ALLOCTOR_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/MemoryBlock.h>
#include <kernel/comp/memory/MemoryBuffer.h>
#include <kernel/comp/Lock/Lock.h>
#include <kernel/comp/memory/MemoryDefs.h>
#include <kernel/comp/memory/GarbageCollector.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

struct MemoryAlloctorConfig;

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
    // TODO:需要解决重复释放的问题,重复释放ref<0 需要asset掉
    void Free(void *ptr);
    void AddRef(void *ptr);
    LibString ToString() const;
    LibString UsingInfo() const;

    // 初始化与销毁
    void Init(UInt64 initBlockNumPerBuffer = MEMORY_BUFFER_BLOCK_INIT, const std::string &source = "");
    void Destroy();

    void Lock();
    void Unlock();

    UInt64 GetTotalBytes() const;
    UInt64 GetBytesInUse() const;
    UInt64 GetBlockNumInUse() const;
    UInt64 GetTotalBlockNum() const;
    UInt64 GetBlockSize() const;
    UInt64 GetIdleEmptyBufferNum() const;
    UInt64 GetIdleEmptyBufferLimit() const;
    // 要分配的大小,不包含头,以及对齐
    UInt64 GetUnitSize() const;

    bool CheckOwner(const MemoryBlock *block) const;

private:
    void _GC();
    void _UpdateGCStatus();
    inline MemoryBuffer *_NewBuffer();
    inline void _AddToActiveHead(MemoryBuffer *buffer);
    inline void _RemoveFromActive(MemoryBuffer *buffer);
    inline void _AddToDeadBlockStackList(LibStack<MemoryBlock *, MemoryBuffer> *deadBlockStack);
    inline void _RemoveFromDeadBlockStackList(LibStack<MemoryBlock *, MemoryBuffer> *deadBlockStack);
    inline void _AddToDeadBufferList(MemoryBuffer *buffer);
    // inline void _RemoveFromDeadBufferList(MemoryBuffer *buffer);
    inline bool _IsMemoryInOccupying(MemoryBuffer *buffer);
    // void _AddDeadEmptyBufferCount();
    // void _ReduceDeadEmptyBufferCount();

protected:
    // 内存分配收集器
    friend class AlloctorInfoCollector;

    // 属性
    const UInt64 _unitBytesToAlloc;              // 要分配的内存大小
    const UInt64 _blockSizeAfterAlign;           // 内存对齐后的blocksize
    const UInt64 _bufferBlockNumLimit;           // 每个buffer block数量的上限
    const UInt64 _memoryBlockHeadSize;           // 内存块头内存对齐后的大小
    UInt64 _initMinBlockCntPerBuffer;           // 初始化时最小内存块数量
    UInt64 _curBlockCntPerBuffer;   // 当前每个buffer拥有的block个数 开始初始化时候是 MEMORY_BUFFER_BLOCK_INIT 个
    UInt64 _blockCountInUsed;                    // 当前内存块使用情况
    UInt64 _bytesInUsed;                         // 当前占用的字节数
    UInt64 _totalBytes;                          // 总内存占用
    UInt64 _totalBlockAmount;                    // 内存块总量
    std::string _createSource;                   // 创建alloctor来源
    UInt64 _threadId;                            // alloctor 所在线程id
    std::atomic_bool _isInit;

    // 活跃区域 双向链表
    MemoryBuffer *_activeBuffersHead;
    UInt64 _curActiveBufferNum;

    /* 被释放的指针栈区 双向链表 */ 
    // 当有指针被释放时先找到stack 再push到stack里面去, 
    // 并把该stack放到deadHead的位置以便下次访问的时候被cache命中
    // 当buffer中最后一个元素被释放后从stack中移除 并放到emptybuffer区域等待被gc
    // 若Stack被分配完之后也从deadHead中移除
    LibStack<MemoryBlock *, MemoryBuffer> *_deadBlockStackHead;

    // 空闲区域 当其他区域没有内存时可以将dead区内存调整到deadBlockStack 并从deadBuffer区移除
    MemoryBuffer *_deadBufferHead;     // 空的buffer队列,当buffer个数达到限制时候必须要把空的buffer也gc掉
    MemoryBuffer *_deadBufferTail;     // 空buffer队列 末节点 从末节点开始gc（最不活跃的结点）
    MemoryBuffer *_gcBeginNode;        // gc 起始节点
    const UInt64 _deadBufferLimit;     // 空缓存限制   TODO:   // 16MB->0 8MB->
    const UInt64 _halfDeadBufferLimit;  // 半数空缓存
    const bool _isCreateBufferWhenInit; // 初始化时创建空内存,一般情况下不需要,避免对象不需要池情况下把内存吃尽
    UInt64 _deadBufferCount;           // 当前空缓存个数

    // gc gc到活跃buffer的一半以下，且是emptyBufferLimit的一半以下
    const UInt64 _trigerGcCond;         // 触发gc条件:alloc 或者 free 次数累计达到 200次 TODO:随内存动态变化
    UInt64 _curOpCount;                 // 分配与释放内存次数
    bool _isDuringGc;                  // 触发gc时机:alloc 或者 free 次数累计达到
    GarbageCollector _gc;

    // 锁
    SpinLock _lck;
};

inline void MemoryAlloctor::AddRef(void *ptr)
{
    MemoryBlock *block = reinterpret_cast<MemoryBlock *>(reinterpret_cast<Byte8 *>(ptr) - __MEMORY_ALIGN__(sizeof(MemoryBlock)));
    ++block->_ref;
}

inline LibString MemoryAlloctor::ToString() const
{
    return UsingInfo();
}

inline LibString MemoryAlloctor::UsingInfo() const
{
    // 总的使用大小,总的大小,使用的block块数,总块数,总的buffer数量,使用的buffer数量,block大小,创建来源,总的使用字节数,总的字节数,当前死亡buffer数量,死亡buffer限制数量,分配与释放操作次数,是否初始化创建buffer,半数空缓存限制,当前活跃buffer数量
    LibString str;

    str << "alloctor init thread id:" << _threadId << ";\n"
        << "block size:" << _blockSizeAfterAlign << ", create source:" << _createSource << "is create buffer when init:" << (_isCreateBufferWhenInit ? "true" : "false") <<  ";\n"
        << "current alloctor buffer total bytes:" << _totalBytes << ", current all using bytes:" << _bytesInUsed << ";\n"
        << "total block amount:" << _totalBlockAmount << ", using block:" << _blockCountInUsed << ";\n"
        << "current block count per buffer for next time:" << _curBlockCntPerBuffer << ", max block count limit per buffer:" << _bufferBlockNumLimit << ";\n"
        << "current free/alloc operate num:" << _curOpCount << ", trigger gc free/alloc operate num limit:" << _trigerGcCond << ", is during gc:" << (_isDuringGc ? "true" : "false") << ";\n"
        << "active buffer num:" << _curActiveBufferNum << ", dead buffer num:" << _deadBufferCount << ";\n"
        << "dead buffer limit:" << _deadBufferLimit << ";\n"
        << "gc begin node:" << _gcBeginNode << ";\n"
        ;
        
    return str;
}

inline void MemoryAlloctor::Lock()
{
    _lck.Lock();
}

inline void MemoryAlloctor::Unlock()
{
    _lck.Unlock();
}

inline UInt64 MemoryAlloctor::GetTotalBytes() const
{
    return _totalBytes;
}

inline UInt64 MemoryAlloctor::GetBytesInUse() const
{
    return _bytesInUsed;
}

inline UInt64 MemoryAlloctor::GetBlockNumInUse() const
{
    return _blockCountInUsed;
}

inline UInt64 MemoryAlloctor::GetTotalBlockNum() const
{
    return _totalBlockAmount;
}

inline UInt64 MemoryAlloctor::GetBlockSize() const
{
    return _blockSizeAfterAlign;
}

inline UInt64 MemoryAlloctor::GetIdleEmptyBufferNum() const
{
    return _deadBufferCount;
}

inline UInt64 MemoryAlloctor::GetIdleEmptyBufferLimit() const
{
    return _deadBufferLimit;
}

inline UInt64 MemoryAlloctor::GetUnitSize() const
{
    return _unitBytesToAlloc;
}

inline bool MemoryAlloctor::CheckOwner(const MemoryBlock *block) const
{
    return block->_buffer && (block->_buffer->_alloctor == this);
}

inline void MemoryAlloctor::_GC()
{
    if(LIKELY(!_isDuringGc))
        return;

    // gc掉半数
    _deadBufferCount = _halfDeadBufferLimit;
    _curOpCount = 0;
    _deadBufferTail = _gcBeginNode->_pre;
    _deadBufferTail->_next = NULL;
    if(_deadBufferCount == 0)
    {
        _deadBufferTail = NULL;
        _deadBufferHead = NULL;
    }

    _gc.push(_gcBeginNode);

    _UpdateGCStatus();
}

inline void MemoryAlloctor::_UpdateGCStatus()
{
    // gc到活跃buffer的一半以下，且是emptyBufferLimit的一半以下
    // const UInt64 target_cnt = _deadBufferLimit >> 1;
    if (_deadBufferCount > _halfDeadBufferLimit && !_gcBeginNode)
    {
        _totalBytes-=_deadBufferTail->_bufferSize;
        _totalBlockAmount -= _deadBufferTail->_blockCnt;

        // gc一次降低一倍
        _curBlockCntPerBuffer >>= 1;
        _curBlockCntPerBuffer = _curBlockCntPerBuffer<_initMinBlockCntPerBuffer?_initMinBlockCntPerBuffer:_curBlockCntPerBuffer;
        _gcBeginNode = _deadBufferTail;
    }

    _isDuringGc = (_deadBufferCount > _halfDeadBufferLimit) && (_curOpCount >= _trigerGcCond);
}

inline MemoryBuffer *MemoryAlloctor::_NewBuffer()
{
    MemoryBuffer *new_buffer = new MemoryBuffer(this->_blockSizeAfterAlign, this->_curBlockCntPerBuffer, this->_unitBytesToAlloc, this);
    new_buffer->Init();

    // 下一次block cnt是*2
    UInt64 blockNum = _curBlockCntPerBuffer << 1;
    _curBlockCntPerBuffer = blockNum > _bufferBlockNumLimit ? _bufferBlockNumLimit : blockNum;
    _totalBytes += new_buffer->_bufferSize;
    _totalBlockAmount += new_buffer->_blockCnt;

    return new_buffer;
}

inline void MemoryAlloctor::_AddToActiveHead(MemoryBuffer *buffer)
{
    buffer->_next = _activeBuffersHead;
    if (LIKELY(_activeBuffersHead))
        _activeBuffersHead->_pre = buffer;
    _activeBuffersHead = buffer;
    ++_curActiveBufferNum;
    buffer->_isInList = true;
}

inline void MemoryAlloctor::_RemoveFromActive(MemoryBuffer *buffer)
{
    MemoryBuffer *preBuffer = buffer->_pre;
    MemoryBuffer *nextBuffer = buffer->_next;

    buffer->_isInList = false;

    if(LIKELY(preBuffer))
        preBuffer->_next = nextBuffer;
    if(LIKELY(nextBuffer))
        nextBuffer->_pre = preBuffer;

    if (UNLIKELY(_activeBuffersHead == buffer))
        _activeBuffersHead = nextBuffer;
    
    buffer->_pre = NULL;
    buffer->_next = NULL;
    --_curActiveBufferNum;
}

inline void MemoryAlloctor::_AddToDeadBlockStackList(LibStack<MemoryBlock *, MemoryBuffer> *deadBlockStack)
{
    deadBlockStack->_next = _deadBlockStackHead;
    if (LIKELY(_deadBlockStackHead))
        _deadBlockStackHead->_pre = deadBlockStack;
    _deadBlockStackHead = deadBlockStack;
    deadBlockStack->_isInStackList = true;
}

inline void MemoryAlloctor::_RemoveFromDeadBlockStackList(LibStack<MemoryBlock *, MemoryBuffer> *deadBlockStack)
{
    LibStack<MemoryBlock *, MemoryBuffer> *preBuffer = deadBlockStack->_pre;
    LibStack<MemoryBlock *, MemoryBuffer> *nextBuffer = deadBlockStack->_next;

    if (LIKELY(preBuffer))
        preBuffer->_next = nextBuffer;
    if (LIKELY(nextBuffer))
        nextBuffer->_pre = preBuffer;

    if (UNLIKELY(_deadBlockStackHead == deadBlockStack))
        _deadBlockStackHead = nextBuffer;

    deadBlockStack->_pre = NULL;
    deadBlockStack->_next = NULL;
    deadBlockStack->_isInStackList = false;
}

inline void MemoryAlloctor::_AddToDeadBufferList(MemoryBuffer *buffer)
{
    buffer->_next = _deadBufferHead;
    if (LIKELY(_deadBufferHead))
        _deadBufferHead->_pre = buffer;
    else
        _deadBufferTail = buffer;
    _deadBufferHead = buffer;

    // _AddDeadEmptyBufferCount();
    // _UpdateGCStatus();
}

// inline void MemoryAlloctor::_RemoveFromDeadBufferList(MemoryBuffer *buffer)
// {
//     MemoryBuffer *preBuffer = buffer->_pre;
//     MemoryBuffer *nextBuffer = buffer->_next;

//     if (LIKELY(preBuffer))
//         preBuffer->_next = nextBuffer;
//     if (LIKELY(nextBuffer))
//         nextBuffer->_pre = preBuffer;

//     if (LIKELY(_deadBufferHead == buffer))
//         _deadBufferHead = nextBuffer;
//     if (UNLIKELY(_deadBufferTail == buffer))
//         _deadBufferTail = buffer->_pre;

//     buffer->_pre = NULL;
//     buffer->_next = NULL;

//     _ReduceDeadEmptyBufferCount();
//     _UpdateGCStatus();
// }

inline bool MemoryAlloctor::_IsMemoryInOccupying(MemoryBuffer *buffer)
{
    return buffer->_head || buffer->_usedBlockCnt;
}

// inline void MemoryAlloctor::_AddDeadEmptyBufferCount()
// {
//     ++_deadBufferCount;
//     if(_deadBufferCount < _halfDeadBufferLimit)
//         return;

//     // 前面缓冲多了要往前移
//     if(LIKELY(_gcBeginNode))
//     {
//         _gcBeginNode = _gcBeginNode->_pre;
//     }
// }

// inline void MemoryAlloctor::_ReduceDeadEmptyBufferCount()
// {
//     --_deadBufferCount;
//     if(LIKELY(_gcBeginNode))
//     {
//         _gcBeginNode = _gcBeginNode->_pre;
//     }

//     // 低于gc条件则不需要gc
//     if(_deadBufferCount < _halfDeadBufferLimit)
//         _gcBeginNode = NULL;
// }

KERNEL_END

#endif
