/*!
 * MIT License
 *  
 * Copyright (c) 2020 Eric Yonng<120453674@qq.com>
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *  
 * 
 * Date: 2020-10-06 18:57:50
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_INT128_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_INT128_H__

#pragma once

#include <kernel/common/compile.h>
#include <kernel/common/BaseType.h>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
 typedef struct alignas(16) {
     UInt64 low;
     UInt64 high;
 } UInt128; 
 typedef  struct alignas(16) {
     Int64 low;
     Int64 high;
 } Int128; 

 // 宽字节字符
 typedef __wchar_t wchar; 

#elif CRYSTAL_TARGET_PLATFORM_LINUX 

 #undef MEM_ALIGNED_16BYTE
 #define MEM_ALIGNED_16BYTE __attribute__(( __aligned__(16) )) 
 typedef __uint128_t UInt128;
 typedef __int128_t Int128; 
 typedef unsigned short wchar;

#endif

#endif
