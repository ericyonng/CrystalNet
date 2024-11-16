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
 * Date: 2021-01-01 23:50:14
 * Author: Eric Yonng
 * Description: 最小单位微妙
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TIME_SLICE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TIME_SLICE_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>
#include <kernel/common/timedefs.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class KERNEL_EXPORT TimeSlice
{
    POOL_CREATE_OBJ_DEFAULT(TimeSlice);
    
public:
    TimeSlice();

    /**
     * Construct by slice, in seconds.
     * @param[in] seconds      - the slice seconds part.
     * @param[in] milliSeconds - the slice milli-seconds part.
     * @param[in] microSeconds - the slice micro-seconds part.
     */
    explicit TimeSlice(int seconds, Int64 milliSeconds = 0, Int64 microSeconds = 0, Int64 nanoSeconds = 0);

    /**
     * Construct by slice string representation(fmt: 00:00:00.xxxxxx).
     * @param[in] slice - the slice value string representation, fmt: 00:00:00.xxxxx, the micro-seconds is optional.
     */
    TimeSlice(const LibString &fmtSlice);

    /**
     * Copy constructor.
     */
    TimeSlice(const TimeSlice &slice);

    /**
     * Time slice parts constructor.
     * @param[in] days         - the days part.
     * @param[in] hours        - the hours part.
     * @param[in] minutes      - the minutes part.
     * @param[in] seconds      - the seconds part.
     * @param[in] milliSeconds - the milli-seconds part, default is 0.
     * @param[in] microSeconds - the micro-seconds part, default is 0.
     */
    TimeSlice(Int64 days, Int64 hours, Int64 minutes, Int64 seconds, Int64 milliSeconds = 0, Int64 microSeconds = 0, Int64 nanoSeconds = 0);

    /**
     * Destructor.
     */
    ~TimeSlice();

    /**
     * Get days/hours/minutes/seconds/milli-seconds/micro-seconds.
     * @return int - the time slice parts value. slice 的一部分
     */
    Int64 GetDays() const;
    Int64 GetHours() const;
    Int64 GetMinutes() const;
    Int64 GetSeconds() const;
    Int64 GetMilliSeconds() const;
    Int64 GetMicroSeconds() const;
    Int64 GetNanoSeconds() const;

    Int64 GetTotalDays() const;
    Int64 GetTotalHours() const;
    Int64 GetTotalMinutes() const;
    Int64 GetTotalSeconds() const;
    Int64 GetTotalMilliSeconds() const;
    const Int64 GetTotalMicroSeconds() const;
    const Int64 &GetTotalNanoSeconds() const;

    TimeSlice operator +(const TimeSlice &slice) const;
    TimeSlice operator -(const TimeSlice &slice) const;

    TimeSlice &operator +=(const TimeSlice &slice);
    TimeSlice &operator -=(const TimeSlice &slice);

    bool operator ==(const TimeSlice &slice) const;
    bool operator !=(const TimeSlice &slice) const;
    bool operator <(const TimeSlice &slice)const;
    bool operator >(const TimeSlice &slice) const;
    bool operator <=(const TimeSlice &slice) const;
    bool operator >=(const TimeSlice &slice) const;

    TimeSlice &operator =(const TimeSlice &slice);
    TimeSlice &operator =(Int64 nanoSecSlice);

    operator bool() const;

    LibString ToString() const;

    // From接口
    static TimeSlice FromSeconds(Int64 seconds);
    static TimeSlice FromMilliSeconds(Int64 milliseconds);
    static TimeSlice FromMicroSeconds(Int64 microseconds);
    static TimeSlice FromNanoSeconds(Int64 nanoseconds);

    static const TimeSlice &ZeroSlice()
    {
       static TimeSlice s_zero;
       return s_zero;
    }

private:
    friend class LibTime;

    TimeSlice(const Int64 &slice);

private:
    Int64 _slice;   // 纳秒级时间
};

KERNEL_END

#include <kernel/comp/TimeSliceImpl.h>

#endif
