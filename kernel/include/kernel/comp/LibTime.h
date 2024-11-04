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
 * Date: 2020-12-30 01:29:03
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LIB_TIME_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LIB_TIME_H__

#pragma once

#include <string.h>
#include <time.h>

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>
#include <kernel/comp/memory/ObjPoolMacro.h>

#include <kernel/comp/LibString.h>
#include <kernel/comp/Utils/TimeUtil.h>

KERNEL_BEGIN

class TimeSlice;

class KERNEL_EXPORT LibTime
{ 
    POOL_CREATE_OBJ_DEFAULT(LibTime);
    
public:
    LibTime();
    LibTime(const LibTime &other);
    virtual ~LibTime();

     
    static LibTime Now();
    // 耗时在400ns+
    static Int64 NowTimestamp();            // 
    static Int64 NowMilliTimestamp();       // 
    static Int64 NowMicroTimestamp();       // 
    static Int64 NowNanoTimestamp();       // 
    
    static LibTime FromSeconds(Int64 seconds);
    static LibTime FromMilliSeconds(Int64 milliSeconds);
    static LibTime FromMicroSeconds(Int64 microSeconds);
    static LibTime FromNanoSeconds(Int64 nanoSeconds);
    static LibTime FromFmtString(const LibString &fmt);    // fmt：1970-07-01 12:12:12.000000055 精度到纳秒 
    static LibTime FromTimeMoment(int year, int month, int day, int hour, int minute, int second, int milliSecond = 0, int microSecond = 0, int nanoSecond = 0);
    static LibTime FromTimeStruct(const tm &timeStruct, int milliSecond = 0, int microSecond = 0, int nanoSecond = 0);
     
    Int64 GetSecTimestamp() const;
    Int64 GetMilliTimestamp() const;
    Int64 GetMicroTimestamp() const;
    Int64 GetNanoTimestamp() const;
     
    const LibTime &UpdateTime();
    const LibTime &UpdateTime(Int64 nanoSecTime);
    const LibTime &UpdateAppendTime(const TimeSlice &addSliceBaseOnNowTime);
    const LibTime &UpdateAppendTime(Int64 addNanoSecBaseOnNowTime);

    bool operator ==(const LibTime &time) const;
    bool operator ==(const Int64 &microSecondTimestamp) const;
    bool operator !=(const LibTime &time) const;
    bool operator <(const LibTime &time) const;
    bool operator >(const LibTime &time) const;
    bool operator <=(const LibTime &time) const;
    bool operator >=(const LibTime &time) const;

    LibTime &operator =(const LibTime &other);
    
    /**
     * Get current time of day.
     * @return TimeSlice - the current time of day.
     */
    TimeSlice GetTimeOfDay() const;
    /**
     * Get remaining seconds to nearest day special monent.
     * @param[in] slice        - slice value.
     * @param[in] hour        - hour.
     * @param[in] minute      - minute.
     * @param[in] second      - second.
     * @param[in] milliSecond - milli-second.
     * @param[in] microSecond - micro-second.
     * @param[in] from        - from time.
     * @return TimeSlice - timeslice value.
     */
    TimeSlice GetIntervalTo(const TimeSlice &slice) const;    // slice是当天的时刻如：10:10:10.100000的微妙数
    TimeSlice GetIntervalTo(Int64 hour, Int64 minute, Int64 second, Int64 milliSecond = 0, Int64 microSecond = 0, Int64 nanoSecond = 0) const;
    static TimeSlice GetIntervalTo(const LibTime &from, const TimeSlice &slice);
    static TimeSlice GetIntervalTo(const LibTime &from, Int64 hour, Int64 minute, Int64 second, Int64 milliSecond = 0, Int64 microSecond = 0, Int64 nanoSecond = 0);

    /**
     * Time slice operations.
     */
    TimeSlice operator -(const LibTime &time) const;

    LibTime operator +(const TimeSlice &slice) const;
    LibTime operator -(const TimeSlice &slice) const;
     
    /**
     * Add specified time parts values.
     * Notes: These operations are thread-safe, all add parts added to new LLBC_Time object.
     * @param[in] <time parts> - the all time parts(year, month, day, ...).
     * @return LLBC_Time - the new time object.
     */
    LibTime AddYears(int years) const;
    LibTime AddMonths(int months) const;
    LibTime AddDays(int days) const;
    LibTime AddHours(int hours) const;
    LibTime AddMinutes(int minutes) const;
    LibTime AddSeconds(int seconds) const;
    LibTime AddMilliSeconds(int milliSeconds) const;
    LibTime AddMicroSeconds(int microSeconds) const;
    LibTime AddNanoSeconds(int nanoSeconds) const;
    
    /**
     * Get GMT time struct.
     * @param[out] timeStruct - time struct object reference.
     * @return const tm & - time struct object.
     */
    const tm &GetGmtTime() const;
    void GetGmtTime(tm &timeStruct) const;

    /**
     * Get local time struct.
     * @param[out] timeStruct - time struct reference.
     * @return const tm & - time struct object.
     */
    const tm &GetLocalTime() const;
    void GetLocalTime(tm &timeStruct) const;

    Int32 GetLocalYear() const; // since 1900
    Int32 GetLocalMonth() const;    // start by 1
    Int32 GetLocalDay() const;
    Int32 GetLocalDayOfWeek() const;    // start by 0
    Int32 GetLocalDayOfYear() const;    // start by 1
    Int32 GetLocalHour() const;
    Int32 GetLocalMinute() const;
    Int32 GetLocalSecond() const;
    Int32 GetLocalMilliSecond() const;
    Int32 GetLocalMicroSecond() const;
    Int32 GetLocalNanoSecond() const;

    LibTime GetZeroTime() const;

    LibString Format(const Byte8 *outFmt = NULL) const;
    static LibString Format(time_t timestamp, const Byte8 *outFmt);

    // 格林威治时间
    LibString FormatAsGmt(const char *outFmt = NULL) const;
    static LibString FormatAsGmt(time_t timestamp, const char *outFmt);

    // UTC时间(和格林威治时间指同一个东西都是0时区时间)
    LibString FormatAsUtc(const char *outFmt = NULL) const;
    static LibString FormatAsUtc(time_t timestamp, const char *outFmt);

    /**
     * Get the time object string representation.
     * @return FS_String - the object string representation.
     */
    LibString ToString() const;
    LibString ToStringOfMillSecondPrecision() const;

    operator bool () const;
    
private:
    explicit LibTime(Int64 microSecTimestamp);
    // explicit LibTime(const std::chrono::system_clock::time_point &now);
    void _UpdateTimeStructs();
    

private:
    Int64 _rawTime{0};          // microsecond ()
    tm _gmtTimeStruct{0};       // 
    tm _localTimeStruct{0};     // 
};

KERNEL_END

#include <kernel/comp/LibTimeImpl.h>

#endif
