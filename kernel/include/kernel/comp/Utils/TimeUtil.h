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
 * Date: 2021-01-01 23:06:50
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_TIME_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_TIME_UTIL_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/Cpu/LibCpuCounter.h>

KERNEL_BEGIN

class KERNEL_EXPORT TimeUtil
{
public:
    // 设置成服务器所在时区 TODO:需要在不同平台下验证
    static void SetTimeZone();
    // 获取服务器时区单位(s)且有符号，如+8区需要输出 +28800s，时区是相对于格林尼治时间的偏移秒数
    static Int32 GetTimeZone();
    static bool IsLeapYear(Int32 year);

    /**
     * Get specific month max days.
     * @param[in] year  - the year.
     * @param[in] month - the month.
     * @return int - the specific month max days, if failed, return 0.
     */
    static Int32 GetMonthMaxDays(Int32 year, Int32 month);

    // 获取不随调时变化的单调递增硬件时钟(系统启动运行的时间),产生系统调用额外开销 只支持linux版本,windows下会直接调用GetMicroTimestamp 单位微妙
    static Int64 GetHandwareSysRunTime();
    // clocktime 会随用户调时而变化,系统调用开销 29ns级别（在vdso加持下）
    static Int64 GetClockRealTime();
    // clocktime 随用户调时而变化 更快精度相对较低的时钟 高性能时钟 千万次调用只话费85ms 精度最差 毫秒级精度
    static Int64 GetClockRealTimeCoarse();
    // clocktime 不会随用户调时而变化的单调递增时钟,有系统调用开销,但会受ntp影响 系统启动运行的时间 单调递增
    static Int64 GetClockMonotonicSysRunTime();
    // 通用时间 除GetClockRealTimeCoarse外最高性能 但调用一次仍然将近400+ns 30ns级别(在vdso加持下)
    static Int64 GetMicroTimestamp();
    // 通用时间 Linux下使用CLOCK_REALTIME 纳秒级精度, windows下使用 GetMicroTimestamp * 1000 微妙级精度
    static Int64 GetNanoTimestamp();
    // chrono时间
    static Int64 GetChronoMicroTimestamp();

    // 限定只用于测试性能
    static Int64 GetSystemElapseNanoseconds();
    static Int64 GetProcessElapseNanoseconds();
    static Int64 GetThreadElapseNanoseconds();

    // 最高性能时间间隔 
    // 每次tsc时钟中断都会给tsc寄存器加1, 并且新的cpu tsc寄存器是同步的 
    // rdtsc可以通过添加序列化指令来达到阻止指令重排, rdtscp指令是序列化的,不会被指令重排，但是性能较于rdtsc稍差
    // (end tick - begin tick) / frequancy 就可以取得时间间隔
    static UInt64 RdTscTickNum();   // 消除cpuid调用的影响,再测量起始的时候使用
    static UInt64 GetCpuCounterFrequancy(); // 内含初始化函数(若没有初始化则初始化tsc_hz)(先执行__cpuid指令, 再执行rdtsc, 再执行clock_gettime, 执行1000000的循环后再执行clock_gettime计算时间, 再执行一次rdtsc, 再执行__cpuid再用时间戳差计算tsc_hz)

    // 基于rdtsc
    static void InitFastTime();
    static void ResetFastTime(Int64 nowTimeNanoseconds);
    static Int64 GetFastMicroTimestamp();
    static Int64 GetFastNanoTimestamp();
    
private:
    static Int64 _systemTimeBegin;      // 系统启动时间戳 纳秒
    static UInt64 _cpuBegin;    // 系统启动时的tsc
};

ALWAYS_INLINE bool TimeUtil::IsLeapYear(Int32 year)
{
    return ( (((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0));
}

ALWAYS_INLINE Int64 TimeUtil::GetHandwareSysRunTime()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    return GetMicroTimestamp() * TimeDefs::NANO_SECOND_PER_MICRO_SECOND;
#else
    struct timespec tp;
    ::syscall(SYS_clock_gettime, CLOCK_MONOTONIC_RAW, &tp);

    return (Int64)tp.tv_sec * TimeDefs::NANO_SECOND_PER_SECOND + tp.tv_nsec;
#endif
}

ALWAYS_INLINE Int64 TimeUtil::GetClockRealTime()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    return GetMicroTimestamp() * TimeDefs::NANO_SECOND_PER_MICRO_SECOND;
#else
    struct timespec tp;
    ::clock_gettime(CLOCK_REALTIME, &tp);

    return (Int64)tp.tv_sec * TimeDefs::NANO_SECOND_PER_SECOND + tp.tv_nsec;
#endif
}

ALWAYS_INLINE Int64 TimeUtil::GetClockRealTimeCoarse()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    return GetMicroTimestamp() * TimeDefs::NANO_SECOND_PER_MICRO_SECOND;
#else
    struct timespec tp;
    ::clock_gettime(CLOCK_REALTIME_COARSE, &tp);

    return (Int64)tp.tv_sec * TimeDefs::NANO_SECOND_PER_SECOND + tp.tv_nsec;
#endif
}

ALWAYS_INLINE Int64 TimeUtil::GetClockMonotonicSysRunTime()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    return GetMicroTimestamp() * TimeDefs::NANO_SECOND_PER_MICRO_SECOND;
#else
    struct timespec tp;
    ::clock_gettime(CLOCK_MONOTONIC, &tp);

    return (Int64)tp.tv_sec * TimeDefs::NANO_SECOND_PER_SECOND + tp.tv_nsec;
#endif
}

// 通用时间,高性能
ALWAYS_INLINE Int64 TimeUtil::GetMicroTimestamp()
{
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    // 第二个参数返回时区
    struct timeval timeVal;
    ::gettimeofday(&timeVal, NULL);

    return (Int64)timeVal.tv_sec * TimeDefs::MICRO_SECOND_PER_SECOND + timeVal.tv_usec;

#else
    // Get time.
    FILETIME ft;
    ::GetSystemTimeAsFileTime(&ft);

    Int64 timeInMicroSec = ((Int64)ft.dwHighDateTime) << 32;
    timeInMicroSec |= ft.dwLowDateTime;
    timeInMicroSec /= 10;

    return timeInMicroSec - CRYSTAL_EPOCH_IN_USEC;
#endif
}

ALWAYS_INLINE Int64 TimeUtil::GetNanoTimestamp()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    return GetMicroTimestamp() * TimeDefs::NANO_SECOND_PER_MICRO_SECOND;
#else
    struct timespec tp;
    ::clock_gettime(CLOCK_REALTIME, &tp);

    return (Int64)tp.tv_sec * TimeDefs::NANO_SECOND_PER_SECOND + tp.tv_nsec;
#endif
}

ALWAYS_INLINE Int64 TimeUtil::GetChronoMicroTimestamp()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock().now().time_since_epoch()).count() / TimeDefs::RESOLUTION_PER_MICROSECOND;
}

ALWAYS_INLINE Int64 TimeUtil::GetSystemElapseNanoseconds()
{
    #if CRYSTAL_TARGET_PLATFORM_LINUX
        struct timespec tp;
        ::clock_gettime(CLOCK_MONOTONIC, &tp);

        return (Int64)tp.tv_sec * TimeDefs::NANO_SECOND_PER_SECOND + tp.tv_nsec;   
    #endif

    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
		return GetMicroTimestamp() * TimeDefs::NANO_SECOND_PER_MICRO_SECOND;
        // static_assert(false, "windows not support GetSystemElapseNanoseconds");
        // return GetMicroTimestamp() * TimeDefs::NANO_SECOND_PER_MICRO_SECOND;
    #endif
}

ALWAYS_INLINE Int64 TimeUtil::GetProcessElapseNanoseconds()
{
    #if CRYSTAL_TARGET_PLATFORM_LINUX
        struct timespec tp;
        ::clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tp);

        return (Int64)tp.tv_sec * TimeDefs::NANO_SECOND_PER_SECOND + tp.tv_nsec;
    #endif

    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
		return GetMicroTimestamp() * TimeDefs::NANO_SECOND_PER_MICRO_SECOND;
		// static_assert(false, "windows not support GetProcessElapseNanoseconds");
        // return GetMicroTimestamp() * TimeDefs::NANO_SECOND_PER_MICRO_SECOND;
    #endif
}

ALWAYS_INLINE Int64 TimeUtil::GetThreadElapseNanoseconds()
{
    #if CRYSTAL_TARGET_PLATFORM_LINUX
        struct timespec tp;
        ::clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tp);

        return (Int64)tp.tv_sec * TimeDefs::NANO_SECOND_PER_SECOND + tp.tv_nsec;
    #endif

    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
		return GetMicroTimestamp() * TimeDefs::NANO_SECOND_PER_MICRO_SECOND;
		// static_assert(false, "windows not support GetThreadElapseNanoseconds");
        // return GetMicroTimestamp() * TimeDefs::NANO_SECOND_PER_MICRO_SECOND;
    #endif
}

ALWAYS_INLINE UInt64 TimeUtil::RdTscTickNum()
{
    return KERNEL_NS::CrystalNativeRdTsc();
}

ALWAYS_INLINE UInt64 TimeUtil::GetCpuCounterFrequancy()
{
    return KERNEL_NS::CrystalGetCpuCounterFrequancy();
}

ALWAYS_INLINE void TimeUtil::InitFastTime()
{
    _cpuBegin = KERNEL_NS::CrystalNativeRdTsc();
    _systemTimeBegin = TimeUtil::GetNanoTimestamp();
}

ALWAYS_INLINE void TimeUtil::ResetFastTime(Int64 nowTimeNanoseconds)
{
    _cpuBegin = KERNEL_NS::CrystalNativeRdTsc();
    _systemTimeBegin = nowTimeNanoseconds;
}

ALWAYS_INLINE Int64 TimeUtil::GetFastMicroTimestamp()
{
// #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
//     const auto nowCpu = KERNEL_NS::CrystalNativeRdTsc();
//     const auto cpuSlice = static_cast<Int64>((nowCpu - _cpuBegin) / LibCpuFrequency::_countPerMicroSecond);
//     return _systemTimeBegin + cpuSlice;
// #else
//     return GetMicroTimestamp();
// #endif

    return GetFastNanoTimestamp() / TimeDefs::NANO_SECOND_PER_MICRO_SECOND;
}

ALWAYS_INLINE Int64 TimeUtil::GetFastNanoTimestamp()
{
// #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
//     const auto nowCpu = KERNEL_NS::CrystalNativeRdTsc();
//     const auto cpuSlice = static_cast<Int64>((nowCpu - _cpuBegin) / LibCpuFrequency::_countPerNanoSecond);
//     return _systemTimeBegin + cpuSlice;
// #else
//     return GetMicroTimestamp() * TimeDefs::NANO_SECOND_PER_MICRO_SECOND;
// #endif

    // TODO:临时处理linux 下rdtsc与真实时间差距太大,跑太快飘到几十秒开外,需要后续处理,暂时用GetMicroTimestamp来处理
    return GetClockRealTime();
    // #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    //     // linux精度到nanosecond
    //     const auto nowCpu = KERNEL_NS::CrystalNativeRdTsc();
    //     const auto cpuSlice = static_cast<Int64>((nowCpu - _cpuBegin) / LibCpuFrequency::_countPerNanoSecond);
    //     return _systemTimeBegin + cpuSlice;
    // #else
    //     // windows下的cpu只能支持微妙计数
    //     const auto nowCpu = KERNEL_NS::CrystalNativeRdTsc();
    //     const auto cpuSlice = static_cast<Int64>((nowCpu - _cpuBegin) / LibCpuFrequency::_countPerMicroSecond);
    //     return _systemTimeBegin + cpuSlice * TimeDefs::NANO_SECOND_PER_MICRO_SECOND;
    // #endif

}


KERNEL_END

#endif
