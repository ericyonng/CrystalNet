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
 * Date: 2020-11-28 21:58:13
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_BASE_MACRO_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_BASE_MACRO_H__

#pragma once

// windows 加快编译
#ifdef _WIN32
 #ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
 #endif
#endif

// 命名空间
#undef CRYSTAL_NET_BEGIN
#define CRYSTAL_NET_BEGIN namespace CRYSTAL_NET {

#undef CRYSTAL_NET_END
#define CRYSTAL_NET_END }

#undef KERNEL_BEGIN
#define KERNEL_BEGIN                                    \
CRYSTAL_NET_BEGIN                                       \
    namespace kernel {

#undef KERNEL_END
#define KERNEL_END } CRYSTAL_NET_END

// CRYSTAL_NET命名空间
#undef CRYSTAL_NET_NS
#define CRYSTAL_NET_NS ::CRYSTAL_NET

// kernel命名空间
#undef KERNEL_NS
#define KERNEL_NS   CRYSTAL_NET_NS::kernel

// Force inline macro define. __forceinline
#undef CRYSTAL_FORCE_INLINE
#if defined(_MSC_VER)
 #define CRYSTAL_FORCE_INLINE inline
#elif defined(__GNUC__) || defined(__clang__)
 #define CRYSTAL_FORCE_INLINE __inline__ __attribute__((always_inline))
#else
 #define CRYSTAL_FORCE_INLINE inline
#endif

// 内联属性
#undef ALWAYS_INLINE
#define ALWAYS_INLINE CRYSTAL_FORCE_INLINE

#if defined(_MSC_VER)
    #ifndef LIKELY
        #define LIKELY(x) (x)
    #endif
    #ifndef UNLIKELY
        #define UNLIKELY(x) (x)
    #endif
#else
    #ifndef LIKELY
        #define LIKELY(x) __builtin_expect(!!(x), 1)
    #endif
    #ifndef UNLIKELY
        #define UNLIKELY(x) __builtin_expect(!!(x), 0)
    #endif
#endif

#endif
