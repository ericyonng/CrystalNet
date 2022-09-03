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

#include<kernel/kernel_inc.h>
#include<kernel/comp/LibStack.h>
#include<kernel/comp/memory/MemoryBlock.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

// PRE DECLEAR
class MemoryBuffer;
class MemoryAlloctor;


// 缓存空闲比较大小
class KERNEL_EXPORT MemoryIdleComp
{
public:
    bool operator()(const MemoryBuffer *l, const MemoryBuffer *r) const;
};


class KERNEL_EXPORT MemoryBuffer
{
public:
    // 内存大小 = blockSize * blockCnt
    MemoryBuffer(UInt64 blockSize,  UInt64 blockCnt, UInt64 usableBytesPerBlock, MemoryAlloctor *alloctor);
    virtual ~MemoryBuffer();

    // 惰性初始化，不完全初始化所有节点（避免瞬间的性能下降）
    void Init();
    // 分配新block节点
    MemoryBlock *AllocNewBlock();
    // buffer信息
    LibString ToString();

public:
    /* 内存 */ 
    // 可用的内存优先从dead_area区取
    // 没有的再从_head取
    // 若buffer全部被释放那么buffer会被放到Alloctor的空闲区等待被gc
    Byte8                   *_buffer;                       // 内存[DEAD_AREA + MemoryBlockArea]
    Byte8                   *_usableBuffer;                 // 可用内存区起始地址
    MemoryBlock             *_head;                         // 内存块头
    LibStack<MemoryBlock *, MemoryBuffer> *_deadArea;   // 释放指针的存放区域
    

    /* 固定参数 */
    // TODO:这里buffersize值依赖_deadAreaSizeAfterAlign 所以需要确认初始化列表的初始化顺序是否先初始化_deadAreaSizeAfterAlign
    const UInt64                  _stackAlignSize;          // 死亡区栈对象的大小（经过内存对齐后）
    const UInt64                  _deadAreaSizeAfterAlign;  // 内存对齐后的死亡区内存大小
    const UInt64                  _bufferSize;              // 内存区大小
    const UInt64                  _blockSize;               // 单个内存单元大小
    const UInt64                  _blockCnt;                // 内存块个数
    const UInt64                  _usableBytesPerBlock;     // 每个内存块可用内存大小
    UInt64                        _curInitBlockIdx;         // 当前初始化buffer位置

    /* 动态参数 */
    UInt64                  _usedBlockCnt;      // 已使用
    UInt64                  _idleBlockCount;    // 空闲块个数
    UInt64                  _historyOpNum;      // 历史分配或者释放计数

    MemoryAlloctor          *_alloctor;         // 分配器
    MemoryBuffer            *_pre;              // 上一个buffer
    MemoryBuffer            *_next;             // 下一个buffer
    bool                    _isInList;          // 是否在队列中
};


inline MemoryBuffer::MemoryBuffer(UInt64 blockSize, UInt64 blockCnt, UInt64 usableBytesPerBlock, MemoryAlloctor *alloctor)
    :_buffer(NULL)
    ,_usableBuffer(NULL)
    ,_head(NULL)
    ,_deadArea(NULL)

    // LibStack 大小 + MemoryBlock * blockCnt(都是内存对齐后的)
    ,_stackAlignSize(__MEMORY_ALIGN__(sizeof(LibStack<MemoryBlock *, MemoryBuffer>)))
    ,_deadAreaSizeAfterAlign(_stackAlignSize + __MEMORY_ALIGN__(sizeof(MemoryBlock *)) * blockCnt)
    ,_bufferSize(blockSize * blockCnt + _deadAreaSizeAfterAlign)
    ,_blockSize(blockSize)
    ,_blockCnt(blockCnt)
    // 内存块最大可用的空间
    ,_usableBytesPerBlock(usableBytesPerBlock)
    ,_curInitBlockIdx(0)

    ,_usedBlockCnt(0)
    ,_idleBlockCount(blockCnt)
    , _historyOpNum(0)

    ,_alloctor(alloctor)
    ,_pre(NULL)
    ,_next(NULL)
    ,_isInList(false)
{
    _buffer = CRYSTAL_NEW_MULTI(Byte8, _bufferSize);
}

inline MemoryBuffer::~MemoryBuffer()
{
    CRYSTAL_DELETE_SAFE(_buffer);
}


inline void MemoryBuffer::Init()
{
    /**
    *    死亡区
    */
    _deadArea = reinterpret_cast<LibStack<MemoryBlock *, MemoryBuffer> *>(_buffer);
    MemoryBlock **deadStackPtr = reinterpret_cast<MemoryBlock **>(_buffer + _stackAlignSize);
    _deadArea->Init(this, deadStackPtr, _blockCnt);

    /**
    *     可用内存区
    */
   _usableBuffer = _buffer + _deadAreaSizeAfterAlign;

    /**
    *   链表头和尾部指向同一位置
    */
    _head = reinterpret_cast<MemoryBlock*>(_usableBuffer);
    _head->_blockSize = _blockSize;
    _head->_usableBytes = _usableBytesPerBlock;
    _head->_objBytes = 0;
    _head->_ref = 0;
    _head->_buffer = this;
    _head->_isInAlloctor = true;
    _head->_next = NULL;
    _curInitBlockIdx = 0;
    
    // 出于性能考虑使用惰性初始化
    // MemoryBlock *temp = _head;

    // // 构建内存块链表
    // for(size_t i = 1; i < _blockCnt; ++i)
    // {
    //     Byte8 *cache = (_usableBuffer + _blockSize * i);
    //     MemoryBlock *block = reinterpret_cast<MemoryBlock*>(cache);
    //     block->_blockSize = _blockSize;
    //     block->_usableBytes = _usableBytesPerBlock;
    //     block->_objBytes = 0;
    //     block->_ref = 0;
    //     block->_buffer = this;
    //     block->_isInAlloctor = true;
    //     block->_next = NULL;
        
    //     temp->_next = block;
    //     temp = block;
    // }
}


// 分配新block节点
inline MemoryBlock *MemoryBuffer::AllocNewBlock()
{
    MemoryBlock *toAlloc = _head;

    if (UNLIKELY(_curInitBlockIdx + 1 == _blockCnt))
    {
        _head = NULL;
        return toAlloc;
    }

    // 指向下一个可分配的block
    ++_curInitBlockIdx;
    Byte8 *cache = (_usableBuffer + _blockSize * _curInitBlockIdx);
    MemoryBlock *block = reinterpret_cast<MemoryBlock*>(cache);
    block->_blockSize = _blockSize;
    block->_usableBytes = _usableBytesPerBlock;
    block->_objBytes = 0;
    block->_ref = 0;
    block->_buffer = this;
    block->_isInAlloctor = true;
    block->_next = NULL;
    _head = block;

    return toAlloc;
}

inline LibString MemoryBuffer::ToString()
{
    LibString str;
    str.AppendFormat("const info bufferAddress = %p, stackAlignSize = %llu, deadAreaSizeAfterAlign = %llu,"
    " bufferSize = %llu, blockSize = %llu, blockCnt = %llu, usableBytesPerBlock = %llu \n "
    "dynamic info: usedBlockCnt = %llu, idleBlockCount = %llu,"
    " historyOpNum = %llu, _curInitBlockIdx = %llu "
    , _buffer, _stackAlignSize, _deadAreaSizeAfterAlign
    , _bufferSize, _blockSize, _blockCnt, _usableBytesPerBlock
    , _usedBlockCnt, _idleBlockCount, _historyOpNum, _curInitBlockIdx );

    return str;
}

inline bool MemoryIdleComp::operator()(const MemoryBuffer *l, const MemoryBuffer *r) const
{
    // 查询或者移除
    if(l == r)
        return l < r;
    
    // 空闲个数一样则按照地址的大小排列
    if (l->_idleBlockCount == r->_idleBlockCount)
        return l < r;

    return l->_idleBlockCount > r->_idleBlockCount;
}

KERNEL_END

#endif
