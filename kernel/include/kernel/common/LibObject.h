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
 * Date: 2021-08-22 21:19:07
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_LIB_OBJECT_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_LIB_OBJECT_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>

KERNEL_BEGIN

// 为了避免频繁归并内存, 跨线程的时候请使用跨线程版本的分配内存_Build::MT, 生命周期只在单线程的, 使用线程本地版本的分配内存
// 跨线程版本的是共用的内存池或者对象池, 线程本地版本的则是每个线程独立, 线程本地的性能更好
struct KERNEL_EXPORT _Build
{
    // 多线程版本
    struct KERNEL_EXPORT MT
    {
        enum Type
        {
            V = 0,
        };
    };

    // thread local 版本
    struct KERNEL_EXPORT TL
    {
        enum Type
        {
            V = 1,
        };
    };

    // 未知
    struct KERNEL_EXPORT UNKNOWN
    {
    };
};

KERNEL_END

using KernelBuildMT = KERNEL_NS::_Build::MT;
using KernelBuildTL = KERNEL_NS::_Build::TL;

#endif
