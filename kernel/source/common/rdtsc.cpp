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
 * Date: 2023-12-04 13:24:15
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/common/rdtsc.h>
#include <kernel/comp/Utils/TimeUtil.h>
#include <kernel/common/timedefs.h>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    #include <WinSock2.h>
    #include <profileapi.h> // cpucounter
#endif

// Include cpu info fetch support header file.
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
#include <intrin.h>
#elif CRYSTAL_TARGET_PLATFORM_LINUX
// #include <cpuid.h>
#endif

KERNEL_BEGIN

bool IsSurportRdtscp = false;

UInt64 (*RdtscFunction)() = NULL;

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
UInt64 WindowsCrystalRdTsc()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return static_cast<UInt64>(li.QuadPart);
}

UInt64 WindowsCrystalGetCpuCounterFrequancy()
{
    LARGE_INTEGER li;
    QueryPerformanceFrequency(&li);
    return static_cast<UInt64>(li.QuadPart);
}
#endif

bool IsCurrentEnvSupportInvariantRdtsc()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    return true;
#else
    // 检查是否支持invariant TSC
    UInt32 eax, ebx, ecx, edx;
    __get_cpuid(0x1, &eax, &ebx, &ecx, &edx);
    if (ecx & (1 << 8))
    { // 检查CPUID 0x1的ECX[8]位
        printf("TSC is invariant!\n");
        return true;
    }

    return false;

#endif
}


void InitTSCSupportFlags()
{
    // RDTSCP flag.
#if CRYSTAL_SUPPORT_RDTSC
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        int cpuInfo[4];
        __cpuid(&cpuInfo[0], 0x80000001);
        IsSurportRdtscp = ((cpuInfo[3] & (1 << 27)) != 0);
    #elif CRYSTAL_TARGET_PLATFORM_LINUX
        UInt32 a = 0, b = 0, c = 0, d = 0;
        __get_cpuid(0x80000001, &a, &b, &c, &d);
        IsSurportRdtscp = ((d & (1 << 27)) != 0);
    #else // default: set to true.
        IsSurportRdtscp = true;
    #endif
#endif

#if CRYSTAL_SUPPORT_RDTSC
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        RdtscFunction = &WindowsCrystalRdTsc;
    #else
    if (IsSurportRdtscp)
    {
        RdtscFunction = &LinuxRdtscp;
    }
    else
    {
        RdtscFunction = &LinuxRdtsc;
    }
    #endif
    
#else
    
#endif
}


// 获取rdtsc frequancy 一个程序只需要get一次即可
UInt64 CrystalGetCpuCounterFrequancy()
{
    // 只有在x86下才有rdtsc
#if CRYSTAL_TARGET_PROCESSOR_X86_64 ||  CRYSTAL_TARGET_PROCESSOR_X86
    // windows下只需要使用api不需要使用tsc
#if CRYSTAL_TARGET_PLATFORM_WINDOWS    
    return WindowsCrystalGetCpuCounterFrequancy();
#else

    // 返回MHz
    // UInt32 eax = 0, ebx = 0, ecx = 0, edx = 0;
    // __get_cpuid(0x15, &eax, &ebx, &ecx, &edx);
    // return (UInt64)(eax) * 1000000;
    
    
    // params
    // struct timespec tpStart, tpEnd;
    UInt64 tscStart, tscEnd;
    auto startTime = KERNEL_NS::TimeUtil::GetMicroTimestamp();
    
    // start calculate using clock_gettime CLOCK_MONOTONIC nanoseconds since system boot
    // UInt64 startNano = ::clock_gettime(CLOCK_MONOTONIC, &tpStart);    // start time
    tscStart = CrystalRdTsc();   
    ::sleep(2);
    tscEnd = CrystalRdTsc();

    auto endTime = KERNEL_NS::TimeUtil::GetMicroTimestamp();

    //::clock_gettime(CLOCK_MONOTONIC, &tpEnd);
    
    // calculate elapsed
    // const UInt64 nanoEndTime = ((tpEnd.tv_sec * TimeDefs::NANO_SECOND_PER_SECOND) + tpEnd.tv_nsec);
    // const UInt64 nanoStartTime = (tpStart.tv_sec * TimeDefs::NANO_SECOND_PER_SECOND + tpStart.tv_nsec);
    // const UInt64 nanosecElapsed = nanoEndTime - nanoStartTime;
    // const UInt64 tscElapsed = tscEnd - tscStart;
    
    // calculate tsc frequancy = elapsedTsc * nanoSecondPerSecond / elapsedNanoSecond;
    // return tscElapsed * TimeDefs::NANO_SECOND_PER_SECOND / nanosecElapsed;
    return (tscEnd - tscStart) * KERNEL_NS::TimeDefs::MICRO_SECOND_PER_SECOND / (endTime - startTime);
#endif
#else
    return CRYSTAL_INFINITE;
#endif
}

KERNEL_END