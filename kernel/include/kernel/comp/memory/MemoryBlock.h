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
 * Date: 2020-11-06 23:44:21
 * Author: Eric Yonng
 * Description: 一块完整的内存将其切分成n块MemoryBlock
*/


#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_MEMORY_BLOCK_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_MEMORY_BLOCK_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>

KERNEL_BEGIN

class MemoryBuffer;

// 每个block内存对齐
struct KERNEL_EXPORT MemoryBlock
{
    // 引用计数 动态参数
    Int64 _ref;
    // 所在buffer
    MemoryBuffer *_buffer;
    // 下一块内存块 动态参数
    MemoryBlock *_next;
    // 实际对象的使用空间
    UInt64 _realUseBytes;
    // 是否在分配器内
    bool _isInAlloctor;
};

#undef WRITE_MEMORY_BLOCK
#define WRITE_MEMORY_BLOCK(memBlock, W_REF, W_BUFFER, W_NEXT, W_REAL_USE_BYTES, W_IS_IN_ALLOCTOR)   \
memBlock->_ref = W_REF; \
memBlock->_buffer = W_BUFFER; \
memBlock->_next = W_NEXT; \
memBlock->_realUseBytes = W_REAL_USE_BYTES; \
memBlock->_isInAlloctor = W_IS_IN_ALLOCTOR;


KERNEL_END

#endif
