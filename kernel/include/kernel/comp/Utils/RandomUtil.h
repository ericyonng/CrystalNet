// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2026-07-15 00:07:54
// Author: Eric Yonng
// Description:


#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_RANDOM_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_RANDOM_UTIL_H__

#pragma once

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>

KERNEL_BEGIN

// 均匀的随机数(MT19937_64 高质量随机数, 线程安全(都是线程本地的随机数引擎))
class KERNEL_EXPORT RandomUtil
{
public:
    static Int64 RandomInt64();
    // [start, endValue]:左闭, 右闭
    static Int64 RandomInt64(Int64 start, Int64 endValue);
    
    // 整个Int32整数域
    static Int32 RandomInt32();
    // [start, endValue]:左闭, 右闭
    static Int32 RandomInt32(Int32 start, Int32 endValue);

    static UInt64 RandomUInt64();
    // [start, endValue]:左闭, 右闭
    static UInt64 RandomUInt64(UInt64 start, UInt64 endValue);
    
    // 整个Int32整数域
    static UInt32 RandomUInt32();
    // [start, endValue]:左闭, 右闭
    static UInt32 RandomUInt32(UInt32 start, UInt32 endValue);
};

KERNEL_END

#endif