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
 * Date: 2020-12-21 01:56:43
 * Author: Eric Yonng
 * Description: 
 * 模板函数已经显示实例化在kernel静态库中，只要其他程序使用kernel的静态库就会找到实例化的模板而不必重新实例化
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_FUNC_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_FUNC_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>
#include <kernel/common/LibObject.h>

KERNEL_BEGIN

class MemoryPool;
class SpinLock;
class LibString;

extern KERNEL_EXPORT MemoryPool *KernelGetTlsMemoryPool();
extern KERNEL_EXPORT MemoryPool *KernelGetDefaultMemoryPool();
extern KERNEL_EXPORT UInt64 KernelGetCurrentThreadId();

// 内存池适配
template<typename BuildType>
extern KERNEL_EXPORT MemoryPool *KernelMemoryPoolAdapter();
// 多线程版本使用默认的内存池
template<>
KERNEL_EXPORT CRYSTAL_FORCE_INLINE MemoryPool *KernelMemoryPoolAdapter<_Build::MT>()
{
    return KernelGetDefaultMemoryPool();
}
// 单线程版本使用本地线程内存池
template<>
KERNEL_EXPORT CRYSTAL_FORCE_INLINE MemoryPool *KernelMemoryPoolAdapter<_Build::TL>()
{
    return KernelGetTlsMemoryPool();
}

// 分配内存
template<typename BuildType>
extern void *KernelAllocMemory(UInt64 memSize);

// 释放内存
template<typename BuildType>
extern void KernelFreeMemory(void *ptr);

template<typename BuildType>
extern void *KernelAllocMemoryBy(void *pool, UInt64 memSize);
template<typename BuildType>
extern void KernelFreeMemoryBy(void *pool, void *ptr);

// 内存分配宏
#ifndef KERNEL_ALLOC_MEMORY_TL 
 #define KERNEL_ALLOC_MEMORY_TL(Sz) KERNEL_NS::KernelAllocMemory<KERNEL_NS::_Build::TL>(Sz)
#endif
#ifndef KERNEL_ALLOC_MEMORY_MT 
 #define KERNEL_ALLOC_MEMORY_MT(Sz) KERNEL_NS::KernelAllocMemory<KERNEL_NS::_Build::MT>(Sz)
#endif
#ifndef KERNEL_FREE_MEMORY_TL
 #define KERNEL_FREE_MEMORY_TL(Ptr) KERNEL_NS::KernelFreeMemory<KERNEL_NS::_Build::TL>(Ptr)
#endif
#ifndef KERNEL_FREE_MEMORY_MT
 #define KERNEL_FREE_MEMORY_MT(Ptr) KERNEL_NS::KernelFreeMemory<KERNEL_NS::_Build::MT>(Ptr)
#endif

// 类型转换
template<typename T>
ALWAYS_INLINE T *KernelCastTo(void *p)
{
    return reinterpret_cast<T *>(p);
}

// 类型转换
template<typename T>
ALWAYS_INLINE T *KernelCastTo(const void *p)
{
    return reinterpret_cast<T *>(const_cast<void *>(p));
}

// 类型转空类型
template<typename T>
ALWAYS_INLINE void *KernelToVoid(T *ptr)
{
    return ptr;
}

// 类型转空类型
template<typename T>
ALWAYS_INLINE void *KernelToVoid(const T *ptr)
{
    return const_cast<T *>(ptr);
}

KERNEL_EXPORT SpinLock &GetConsoleLocker();
KERNEL_EXPORT void LockConsole();
KERNEL_EXPORT void UnlockConsole();

KERNEL_END

#endif
