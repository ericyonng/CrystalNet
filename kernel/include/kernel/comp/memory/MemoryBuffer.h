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
 * Date: 2020-11-07 00:55:53
 * Author: Eric Yonng
 * Description: 一块完整的内存块
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_MEMORY_BUFFER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_MEMORY_BUFFER_H__

#pragma once

#include <kernel/kernel_export.h>

#include<kernel/comp/memory/MemoryBlock.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Utils/BackTraceUtil.h>

KERNEL_BEGIN

// PRE DECLEAR
class MemoryBuffer;
class MemoryAlloctor;

class  MemoryBuffer
{
public:
    // 内存大小 = blockSize * blockCnt
    MemoryBuffer(UInt64 blockSize,  UInt64 blockCnt, UInt64 usableBytesPerBlock, MemoryAlloctor *alloctor);
    ~MemoryBuffer();

    // 惰性初始化，不完全初始化所有节点（避免瞬间的性能下降）
    void Init();
    // 分配新block节点
    MemoryBlock *AllocNewBlock()
    {
        // // 用光一次，向左移位一位
        if(UNLIKELY(++_usedBlockCnt == _blockCnt))
            _notEnableGcFlag <<= _shiftBitNum;

        // 优先从释放链表中拿
        if(LIKELY(_freeHead))
        {// 
            MemoryBlock *toAlloc = _freeHead;
            // 说明正在被其他对象引用需要
            if(UNLIKELY(toAlloc->_ref > 0))
            {
                const auto blockHeaderSize = __MEMORY_ALIGN__(sizeof(MemoryBlock));

                throw std::logic_error(KERNEL_NS::LibString().AppendFormat("block toAlloc:[%p], will alloc obj addr:[%p], size:%llu, is in using please check %s !!!"
                , toAlloc, reinterpret_cast<Byte8 *>(toAlloc) + blockHeaderSize, toAlloc->_realUseBytes
                , BackTraceUtil::CrystalCaptureStackBackTrace().c_str()).GetRaw());
            }

            _freeHead = _freeHead->_next;
            toAlloc->_next = NULL;
            return toAlloc;
        }

        // 不需要考虑_head 是_tail 的情况, 因为用尽的时候再调用AllocNewBlock 此时block一定在_freeHead中, 其他情况一定是异常
        MemoryBlock *toAlloc = _head;
        toAlloc->_buffer = this;
        toAlloc->_next = NULL;
        toAlloc->_isInAlloctor = true;
        _head = reinterpret_cast<MemoryBlock *>(_currentBufferPos += _blockSize);

        return toAlloc;
    }
    // 回收
    void FreeBlock(MemoryBlock *block);
    void FreeBlockList(MemoryBlock *blockHead, MemoryBlock *blockEnd, UInt64 mergeBlockCount);
    // buffer信息
    LibString ToString();

    static UInt64 GetShiftNum(UInt64 memBytes);

    // 重置gcflag 会进入下次gc规则循环
    void ResetGcFlag();

public:
    /* 内存 */ 
    // 可用的内存优先从dead_area区取
    // 没有的再从_head取
    // 若buffer全部被释放那么buffer会被放到Alloctor的空闲区等待被gc
    Byte8                   *_buffer;                       // 内存[DEAD_AREA + MemoryBlockArea]
    Byte8                   *_currentBufferPos;             // 内存[DEAD_AREA + MemoryBlockArea]
    MemoryBlock             *_head;                         // 内存块头
    MemoryBlock             *_tail;                         // 内存块末尾
    MemoryBlock             *_freeHead;                     // free 链表
    

    /* 固定参数 */
    // TODO:这里buffersize值依赖_deadAreaSizeAfterAlign 所以需要确认初始化列表的初始化顺序是否先初始化_deadAreaSizeAfterAlign
    const UInt64                  _bufferSize;              // 内存区大小
    const UInt64                  _blockSize;               // 单个内存单元大小
    const UInt64                  _blockCnt;                // 内存块个数
    const UInt64                  _usableBytesPerBlock;     // 每块内存块实际有效空间

    /* 动态参数 */
    UInt64                   _usedBlockCnt;      // 已使用

    /* gc重要标记
    *  1. 初始值是1
       2. 调用Alloc 当 _usedBlockCnt 为 _blockCnt 时向左移位一次（表示缓存用光）
       3. 当 _notEnableGcFlag 为0时，表示下次当 _usedBlockCnt 为0时会把MemoryBuffer gc掉，也就是说，当缓存被用光64次后，缓存中没有对象时候就会被gc掉
       4.若不希望buffer被gc掉，可以每次分配的时候手动将flag|=1，这样可以保证buffer永不会被gc掉
    */
    UInt64                  _notEnableGcFlag;
    const UInt64            _shiftBitNum;       // 移位位数

    MemoryAlloctor          *_alloctor;         // 分配器
    MemoryBuffer            *_pre;              // 上一个buffer
    MemoryBuffer            *_next;             // 下一个buffer
    bool                    _isInBusy;          // 是否在忙链表中
};


ALWAYS_INLINE MemoryBuffer::MemoryBuffer(UInt64 blockSize, UInt64 blockCnt, UInt64 usableBytesPerBlock, MemoryAlloctor *alloctor)
    :_buffer(NULL)
    ,_currentBufferPos(NULL)
    ,_head(NULL)
    ,_tail(NULL)
    ,_freeHead(NULL)

    ,_bufferSize(blockSize * blockCnt)
    ,_blockSize(blockSize)
    ,_blockCnt(blockCnt)
    // 内存块最大可用的空间
    ,_usableBytesPerBlock(usableBytesPerBlock)

    ,_usedBlockCnt(0)
    ,_notEnableGcFlag(1)
    ,_shiftBitNum(GetShiftNum(_usableBytesPerBlock))

    ,_alloctor(alloctor)
    ,_pre(NULL)
    ,_next(NULL)
    ,_isInBusy(false)
{
    _buffer = reinterpret_cast<Byte8 *>(::malloc(_bufferSize));
    if (UNLIKELY(!_buffer))
        throw std::logic_error("have no enough memory to alloc.");
}

ALWAYS_INLINE MemoryBuffer::~MemoryBuffer()
{
    ::free(_buffer);
}

ALWAYS_INLINE void MemoryBuffer::Init()
{
    // 出于性能考虑使用惰性初始化
    _head = reinterpret_cast<MemoryBlock *>(_buffer);
    _currentBufferPos = _buffer;
    _tail = reinterpret_cast<MemoryBlock *>(_buffer + _bufferSize);
}

ALWAYS_INLINE void MemoryBuffer::FreeBlock(MemoryBlock *block)
{
    ASSERT(block->_ref == 0);

    // _notEnableGcFlag 一方面是为了内存不会那么快被释放, 另一方面也会考虑内存占用问题, 需要做平衡
    // 不适合在Free的时候通过判0来改变_notEnableGcFlag, 因为如果有个bug, 导致某个block长时间不free就会导致MemoryBuffer长时间不会被GC掉,导致内存资源占用过大
    --_usedBlockCnt;
    block->_next = _freeHead;
    _freeHead = block;
}

ALWAYS_INLINE void MemoryBuffer::FreeBlockList(MemoryBlock *blockHead, MemoryBlock *blockEnd, UInt64 mergeBlockCount)
{
    _usedBlockCnt -= mergeBlockCount;
    blockEnd->_next = _freeHead;
    _freeHead = blockHead;
}


ALWAYS_INLINE UInt64 MemoryBuffer::GetShiftNum(UInt64 memBytes)
{
    // 1024 被定义为小内存 有64次被用光的机会
    if(LIKELY(memBytes <= 1024))
        return 1;

    // 4096内的 被定义为中等内存 有16次被用光的机会
    if(LIKELY(memBytes <= 4096))
        return 4;

    // 1MB 内的 被定义为中等内存 有4次被用光的机会
    if(LIKELY(memBytes <= 1048576))
        return 16;

    // 1MB以上的内存只有 1 次被用光的机会,用光后会被gc掉
    return 64;
}

ALWAYS_INLINE void MemoryBuffer::ResetGcFlag()
{
    _notEnableGcFlag = 1LLU;
}

KERNEL_END

#endif
