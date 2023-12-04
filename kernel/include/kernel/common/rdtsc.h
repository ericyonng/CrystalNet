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

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
 KERNEL_EXPORT UInt64 WindowsCrystalRdTsc();
 KERNEL_EXPORT UInt64 WindowsCrystalGetCpuCounterFrequancy();
#endif

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
    return WindowsCrystalRdTsc();
#endif
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
KERNEL_EXPORT CRYSTAL_FORCE_INLINE UInt64 CrystalGetCpuCounterFrequancy()
{
// 只有在x86下才有rdtsc
#if CRYSTAL_TARGET_PROCESSOR_X86_64 ||  CRYSTAL_TARGET_PROCESSOR_X86
    // windows下只需要使用api不需要使用tsc
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        return WindowsCrystalGetCpuCounterFrequancy();
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

KERNEL_END

#endif
