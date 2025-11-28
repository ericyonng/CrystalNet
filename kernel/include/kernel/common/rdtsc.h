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
 * Author: Eric Yonng
 * Date: 2023-12-02 20:02:19
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_RDTSC_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_RDTSC_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/common/BaseType.h>


KERNEL_BEGIN

/**
 * The rdtscp support flag.
 */
#if CRYSTAL_SUPPORT_RDTSC
    #ifndef CRYSTAL_NET_STATIC_KERNEL_LIB
        KERNEL_EXPORT bool IsSurportRdtscp;
    #else
     extern bool IsSurportRdtscp;
    #endif
#endif // IsSurportRdtscp

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
 KERNEL_EXPORT UInt64 WindowsCrystalRdTsc();
 KERNEL_EXPORT UInt64 WindowsCrystalGetCpuCounterFrequancy();
#endif

// rdtsc方法
#ifndef CRYSTAL_NET_STATIC_KERNEL_LIB
 KERNEL_EXPORT UInt64 (*RdtscFunction)();
#else
 extern UInt64 (*RdtscFunction)();
#endif

KERNEL_EXPORT void InitTSCSupportFlags();

// 当前环境rdtsc是否可靠时钟源
KERNEL_EXPORT bool IsCurrentEnvSupportInvariantRdtsc();


#if CRYSTAL_TARGET_PLATFORM_LINUX

KERNEL_EXPORT CRYSTAL_FORCE_INLINE UInt64 LinuxRdtscp()
{
    UInt32 lo = 0, hi = 0;
    __asm__ volatile ("rdtscp" : "=a" (lo), "=d" (hi) :: "rcx");
    return (static_cast<UInt64>(hi) << 32) | lo;
}

KERNEL_EXPORT CRYSTAL_FORCE_INLINE UInt64 LinuxRdtsc()
{
    UInt32 lo = 0, hi = 0;
    __asm__ volatile ("lfence\n\t"
                      "rdtsc\n\t"
                      "mov %%edx, %1;"
                      "mov %%eax, %0;"
                      "lfence\n\t":"=r"(lo), "=r"(hi)
                      ::"%eax", "%edx");
    return (static_cast<UInt64>(hi) << 32) | lo;
}
#endif


// 序列化的rdtsc
KERNEL_EXPORT CRYSTAL_FORCE_INLINE UInt64 CrystalRdTsc()
{
// #if CRYSTAL_TARGET_PLATFORM_LINUX
//     UInt64 var;
//     __asm__ volatile ("lfence\n\t"
//                     "rdtsc\n\t"  
//                     "shl $32,%%rdx;"
//                     "or %%rdx,%%rax;"
//                     "mov %%rax, %0;"
//                     "lfence\n\t":"=r"(var)
//                     ::"%rax", "%rbx", "%rcx", "%rdx");
//
//     return var;
// #endif
//
// #if CRYSTAL_TARGET_PLATFORM_WINDOWS
//     return WindowsCrystalRdTsc();
// #endif
//
    return RdtscFunction();
//     
// // #if CRYSTAL_SUPPORT_RDTSC
// #if CRYSTAL_TARGET_PLATFORM_WINDOWS
//     return WindowsCrystalRdTsc();
// #else // Non-win32
//     uint32 lo = 0, hi = 0;
//     if (__LLBC_supportedRdtscp)
//         __asm__ volatile ("rdtscp" : "=a" (lo), "=d" (hi) :: "rcx");
//     else
//         __asm__ volatile ("lfence\n\t"
//                           "rdtsc\n\t"
//                           "mov %%edx, %1;"
//                           "mov %%eax, %0;"
//                           "lfence\n\t":"=r"(lo), "=r"(hi)
//                           ::"%eax", "%ebx", "%ecx", "%edx");
//     return (static_cast<uint64>(hi) << 32) | lo;
// #endif // LLBC_TARGET_PLATFORM_WIN32
// #else
//     return LLBC_GetMicroseconds();
// #endif // LLBC_SUPPORT_RDTSC
}

// 序列化的rdtsc 10ns级别
KERNEL_EXPORT CRYSTAL_FORCE_INLINE UInt64 CrystalNativeRdTsc()
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    UInt32 low, high;
    __asm__ volatile ("rdtsc" : "=a"(low), "=d"(high));

    return (low) | ((UInt64)(high) << 32);
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    return WindowsCrystalRdTsc();
#endif
}

// 获取rdtsc frequancy 一个程序只需要get一次即可
KERNEL_EXPORT UInt64 CrystalGetCpuCounterFrequancy();

KERNEL_END

#endif
