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
 * Date: 2021-01-01 23:09:49
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Utils/TimeUtil.h>
#include <atomic>
#include <chrono>

#if CRYSTAL_TARGET_PLATFORM_LINUX
#include <sys/syscall.h>
 #include <time.h>
 #include <sys/time.h>
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    #include "sysinfoapi.h"
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
 #include <time.h>
#endif

KERNEL_BEGIN


std::atomic<Int32> __g_timezone{0};
Int64 TimeUtil::_systemTimeBegin = 0;
UInt64 TimeUtil::_cpuBegin = 0;

void TimeUtil::SetTimeZone()
{
    // 设置成服务器所在时区
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    tzset();
    time_t now = time(NULL);
    // __g_timezone = -1 * static_cast<Int32>(localtime(&now)->tm_gmtoff);         // linux下localtime(&now)->tm_gmtoff是个整数，但是本框架需要取得相反的数，如+8时区，这里需要输出-28800

    __g_timezone.store(-1 * static_cast<Int32>(localtime(&now)->tm_gmtoff), std::memory_order_release);
#else // WIN32
    _tzset();
    __g_timezone.store(_timezone, std::memory_order_release);
#endif // Non-WIN32
}

// TODO需要验证接口
Int32 TimeUtil::GetTimeZone()
{
    return __g_timezone.load(std::memory_order_acquire);
}

Int32 TimeUtil::GetMonthMaxDays(Int32 year, Int32 month)
{
    if(month >= 1 && month <= 7)
    {
        if(month % 2 == 1)
            return 31;
        else if(month == 2)
            return IsLeapYear(year) ? 29 : 28;
        else
            return 30;
    }
    else if(month >= 8 && month <= 12)
    {
        return month % 2 == 0 ? 31 : 30;
    }
    else
    {
        return -1;
    }
}

Int64 TimeUtil::GetNanoTimestamp()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    return GetMicroTimestamp() * TimeDefs::NANO_SECOND_PER_MICRO_SECOND;
#else
    struct timespec tp;
    ::clock_gettime(CLOCK_REALTIME, &tp);

    return (Int64)tp.tv_sec * TimeDefs::NANO_SECOND_PER_SECOND + tp.tv_nsec;
#endif
}

Int64 TimeUtil::GetChronoMicroTimestamp()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock().now().time_since_epoch()).count() / TimeDefs::RESOLUTION_PER_MICROSECOND;
}

Int64 TimeUtil::GetSystemElapseNanoseconds()
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

Int64 TimeUtil::GetProcessElapseNanoseconds()
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


Int64 TimeUtil::GetHandwareSysRunTime()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    return GetMicroTimestamp() * TimeDefs::NANO_SECOND_PER_MICRO_SECOND;
#else
    struct timespec tp;
    ::syscall(SYS_clock_gettime, CLOCK_MONOTONIC_RAW, &tp);

    return (Int64)tp.tv_sec * TimeDefs::NANO_SECOND_PER_SECOND + tp.tv_nsec;
#endif
}

Int64 TimeUtil::GetClockRealTime()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    return GetMicroTimestamp() * TimeDefs::NANO_SECOND_PER_MICRO_SECOND;
#else
    struct timespec tp;
    ::clock_gettime(CLOCK_REALTIME, &tp);

    return (Int64)tp.tv_sec * TimeDefs::NANO_SECOND_PER_SECOND + tp.tv_nsec;
#endif
}

Int64 TimeUtil::GetClockRealTimeCoarse()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    return GetMicroTimestamp() * TimeDefs::NANO_SECOND_PER_MICRO_SECOND;
#else
    struct timespec tp;
    ::clock_gettime(CLOCK_REALTIME_COARSE, &tp);

    return (Int64)tp.tv_sec * TimeDefs::NANO_SECOND_PER_SECOND + tp.tv_nsec;
#endif
}

Int64 TimeUtil::GetClockMonotonicSysRunTime()
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
Int64 TimeUtil::GetMicroTimestamp()
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

Int64 TimeUtil::GetThreadElapseNanoseconds()
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

KERNEL_END
