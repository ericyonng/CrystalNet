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
#include <kernel/common/type.h>
#include <kernel/common/libs.h>
#include <kernel/common/macro.h>
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
inline T *KernelCastTo(void *p)
{
    return reinterpret_cast<T *>(p);
}

// 类型转换
template<typename T>
inline T *KernelCastTo(const void *p)
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

// 序列化的rdtsc
KERNEL_EXPORT CRYSTAL_FORCE_INLINE UInt64 CrystalRdTsc()
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    UInt64 var;
    __asm__ volatile ("lfence\n\t"
                    "rdtsc\n\t"  
                    "shl $32,%%rdx;"
                    "or %%rdx,%%rax;"
                    "mov %%rax, %0;"
                    "lfence\n\t":"=r"(var)
                    ::"%rax", "%rbx", "%rcx", "%rdx");

    return var;
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    LARGE_INTEGER li;
    ::QueryPerformanceCounter(&li);
    return static_cast<UInt64>(li.QuadPart);
#endif
}

// 序列化的rdtsc
KERNEL_EXPORT CRYSTAL_FORCE_INLINE UInt64 CrystalNativeRdTsc()
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    UInt32 low, high;
    __asm__ volatile ("rdtsc" : "=a"(low), "=d"(high));

    return (low) | ((UInt64)(high) << 32);
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    LARGE_INTEGER li;
    ::QueryPerformanceCounter(&li);
    return static_cast<UInt64>(li.QuadPart);
#endif
}

// 获取rdtsc frequancy 一个程序只需要get一次即可
KERNEL_EXPORT CRYSTAL_FORCE_INLINE UInt64 CrystalGetCpuCounterFrequancy()
{
// 只有在x86下才有rdtsc
#if CRYSTAL_TARGET_PROCESSOR_X86_64 ||  CRYSTAL_TARGET_PROCESSOR_X86
    // windows下只需要使用api不需要使用tsc
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        LARGE_INTEGER li;
        ::QueryPerformanceFrequency(&li);
        return static_cast<UInt64>(li.QuadPart);
    #else
        // params
        // struct timespec tpStart, tpEnd;
        UInt64 tscStart, tscEnd;

        // start calculate using clock_gettime CLOCK_MONOTONIC nanoseconds since system boot
        // UInt64 startNano = ::clock_gettime(CLOCK_MONOTONIC, &tpStart);    // start time
        tscStart = CrystalRdTsc();   
        ::sleep(1);
        tscEnd = CrystalRdTsc();
        //::clock_gettime(CLOCK_MONOTONIC, &tpEnd);

        // calculate elapsed
        // const UInt64 nanoEndTime = ((tpEnd.tv_sec * TimeDefs::NANO_SECOND_PER_SECOND) + tpEnd.tv_nsec);
        // const UInt64 nanoStartTime = (tpStart.tv_sec * TimeDefs::NANO_SECOND_PER_SECOND + tpStart.tv_nsec);
        // const UInt64 nanosecElapsed = nanoEndTime - nanoStartTime;
        // const UInt64 tscElapsed = tscEnd - tscStart;

        // calculate tsc frequancy = elapsedTsc * nanoSecondPerSecond / elapsedNanoSecond;
        // return tscElapsed * TimeDefs::NANO_SECOND_PER_SECOND / nanosecElapsed;
        return tscEnd - tscStart;
    #endif
#else
    return CRYSTAL_INFINITE;
#endif
}

KERNEL_EXPORT SpinLock &GetConsoleLocker();
KERNEL_EXPORT void LockConsole();
KERNEL_EXPORT void UnlockConsole();

// KERNEL_EXPORT SpinLock& GetBackTraceLock();
// KERNEL_EXPORT void LockBackTrace();
// KERNEL_EXPORT void UnlockBackTrace();

KERNEL_EXPORT LibString &KernelAppendFormat(LibString &o, const Byte8 *fmt, ...) LIB_KERNEL_FORMAT_CHECK(2, 3);

KERNEL_END

// extern KERNEL_EXPORT std::atomic<Int64> g_TotalBytes;
// extern KERNEL_EXPORT std::string g_MemleakBackTrace;

// extern void *operator new(size_t bytes);
// extern void *operator new[](size_t bytes);

#endif
