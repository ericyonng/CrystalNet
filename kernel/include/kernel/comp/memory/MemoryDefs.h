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
 * Date: 2020-11-09 02:34:06
 * Author: Eric Yonng
 * Description: 可分配的内存大小限制 超过限制的使用系统分配
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_MEMORY_DEFS_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_MEMORY_DEFS_H__

#pragma once

// 最小16Bytes
#undef MIN_MEMORY_UNIT
#define MIN_MEMORY_UNIT         16
// 最大64MB
#undef MAX_MEMORY_UNIT          
#define MAX_MEMORY_UNIT         67108864
// 触发gc条件 alloc 或者 free达到256
#undef MEMORY_GC_COND_PARAM
#define MEMORY_GC_COND_PARAM    256

// 初始化的buffer block个数 一个buffer合适的数量
#undef MEMORY_BUFFER_BLOCK_INIT
#define MEMORY_BUFFER_BLOCK_INIT 2llu

// 内存池最小内存块大小 MIN_MEMORY_UNIT 字节
#define MEMORY_POOL_MINI_BLOCK_BYTES    MIN_MEMORY_UNIT
// 初始化时候最大内存块 2MB
#define MEMORY_POOL_MAX_BLOCK_BYTES     2097152
// 每块大内存最大内存块数量
#define MAX_BLOCK_NUM_PER_BUFF_DEF      40960
// 最小内存块数量 一个buffer 最小的极限分配数量 
#define MIN_BLOCK_NUM_PER_BUFF_DEF      2
// 当内存块达到限制大小时统一统一指定最小内存块数量(MIN_BLOCK_NUM_PER_BUFF_DEF) 目前限制16MB
#define LOCK_WHEN_OVER_SPECIFY_BYTES 16777216
// 当内存块小于限制大小时统一统一指定最大内存块数量(MAX_BLOCK_NUM_PER_BUFF_DEF) MIN_MEMORY_UNIT 字节以下内存块都是最大值MAX_BLOCK_NUM_PER_BUFF_DEF
#define LOCK_WHEN_BELOW_SPECIFY_BYTES MIN_MEMORY_UNIT

// 内存块数量开始变化的内存块大小
// #define START_CHG_MEMROY_BLOCK_BYTES    1024   

// gc过渡区空内存 emptybuffer 限制 最小内存块的空内存数量/最大内存块空内存数量
#define MIN_EMPTY_BUFFER_NUM_LIMIT 2048
#define MAX_EMPTY_BUFFER_NUM_LIMIT 4

#endif
