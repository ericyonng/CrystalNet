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
 * Date: 2021-01-14 21:57:20
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_DEFS_MEMORY_ALLOCTOR_CONFIG_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_DEFS_MEMORY_ALLOCTOR_CONFIG_H__

#pragma once

#include <kernel/kernel_inc.h>

KERNEL_BEGIN

struct KERNEL_EXPORT MemoryAlloctorConfig
{
    MemoryAlloctorConfig(UInt64 unitBytes, UInt64 initMemoryBufferNum = 0);

    UInt64 _unitBytes;              // 分配大小
    UInt64 _bufferBlockNumLimit;    // 大内存最大的block数量（大内存有起始block数量，期间动态上涨）,大内存其实分配block数量由Init时候输入
    UInt64 _initMemoryBufferNum;    // 初始化 memory buffer个数
};

struct KERNEL_EXPORT MemoryAlloctorConfigs
{
    // key:unitBytes, value:unitBytes 分配器的配置
    std::map<UInt64, MemoryAlloctorConfig> _cfgs;
};

KERNEL_END

#endif
