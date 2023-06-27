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
 * Date: 2020-12-30 01:29:21
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/LibTime.h>
#include <kernel/comp/TimeSlice.h>
#include <kernel/comp/Utils/StringUtil.h>
#include <kernel/comp/Utils/TimeUtil.h>

static inline const KERNEL_NS::LibString &GetZeroTimeString()
{
    static const KERNEL_NS::LibString __g_ZeroTimeString = "1970-01-01 00:00:00.000000";
    return __g_ZeroTimeString;
}

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(LibTime);


LibTime::LibTime()
    :_rawTime(0)
    ,_gmtTimeStruct{0}
    ,_localTimeStruct{0}
{

}

LibTime::LibTime(const LibTime &other)
{
    _rawTime = other._rawTime;
    _gmtTimeStruct = other._gmtTimeStruct;
    _localTimeStruct = other._localTimeStruct;
}

LibTime::~LibTime()
{
}

LibTime LibTime::FromSeconds(Int64 seconds)
{
    return LibTime(seconds * TimeDefs::NANO_SECOND_PER_SECOND);
}

LibTime LibTime::FromMilliSeconds(Int64 milliSeconds)
{
    return LibTime(milliSeconds * TimeDefs::NANO_SECOND_PER_MILLI_SECOND);
}

LibTime LibTime::FromMicroSeconds(Int64 microSeconds)
{
    return LibTime(microSeconds * TimeDefs::NANO_SECOND_PER_MICRO_SECOND);
}

LibTime LibTime::FromNanoSeconds(Int64 nanoSeconds)
{
    return LibTime(nanoSeconds);
}

LibTime LibTime::FromFmtString(const LibString &fmt)
{
    if(UNLIKELY(fmt.empty()))
        return LibTime(0);

    // Split date, time
    auto dateTimes = fmt.Split(' ', 1);
    if(dateTimes.size() == 1) // Only has date part or time part(try guess).
    {
        if(dateTimes[0].GetRaw().find('-') != std::string::npos) // Is date part, append default time part.
            dateTimes.push_back("0:0:0.000");
        else // Is time part, insert default date part.
            dateTimes.insert(dateTimes.begin(), "1970-1-1");
    }

    const auto &datePart = dateTimes[0];
    const auto &timePart = dateTimes[1];

    // Split year,month,day
    auto dateParts = datePart.Split('-', 2);
    if(dateParts.size() == 1) // Only has day part.
    {
        dateParts.insert(dateParts.begin(), "1");
        dateParts.insert(dateParts.begin(), "1970");
    }
    else if(dateParts.size() == 2) // Only has day and month parts.
    {
        dateParts.insert(dateParts.begin(), "1970");
    }

    // Split hour,minute,second
    auto timeParts = timePart.Split(':', 2);
    if(timeParts.size() == 1) // Only has second part.
    {
        timeParts.insert(timeParts.begin(), "0");
        timeParts.insert(timeParts.begin(), "0");
    }
    else if(timeParts.size() == 2) // Only has second and minute parts.
    {
        timeParts.insert(timeParts.begin(), "0");
    }

    // Split time, nanoseconds 
    auto secondParts = timeParts[2].Split('.', 1);
    if(secondParts.size() == 1) // Only has second part.
        secondParts.push_back("0");

    // Convert it
    Int32 year = StringUtil::StringToInt32(dateParts[0].c_str());
    Int32 month =  StringUtil::StringToInt32(dateParts[1].c_str());
    Int32 day =  StringUtil::StringToInt32(dateParts[2].c_str());

    Int32 hour =  StringUtil::StringToInt32(timeParts[0].c_str());
    Int32 minute = StringUtil::StringToInt32(timeParts[1].c_str());
    Int32 second =  StringUtil::StringToInt32(timeParts[2].c_str());
    Int32 nanoSecond = StringUtil::StringToInt32(secondParts[1].c_str());

    return FromTimeMoment(year,
                         month,
                         day,
                         hour,
                         minute,
                         second,
                         static_cast<Int32>(nanoSecond / TimeDefs::NANO_SECOND_PER_MILLI_SECOND),
                         static_cast<Int32>(nanoSecond % TimeDefs::NANO_SECOND_PER_MILLI_SECOND / TimeDefs::NANO_SECOND_PER_MICRO_SECOND), 
                         static_cast<Int32>(nanoSecond % TimeDefs::NANO_SECOND_PER_MICRO_SECOND));
}

LibTime LibTime::FromTimeMoment(int year, int month, int day, int hour, int minute, int second, int milliSecond, int microSecond, int nanoSecond)
{
    tm timeStruct;
    timeStruct.tm_year = year - 1900;
    timeStruct.tm_mon = month - 1;
    timeStruct.tm_mday = day;

    if(year == 1970 && month == 1 && day == 1)
    {
        int tz = TimeUtil::GetTimeZone();
        int totalSeconds = hour * TimeDefs::SECOND_PER_HOUR +
            minute * TimeDefs::SECOND_PER_MINUTE + second;
        if(tz < 0 && totalSeconds < -tz)
        {
            hour = -tz / TimeDefs::SECOND_PER_HOUR;
            minute = (-tz % TimeDefs::SECOND_PER_HOUR) / TimeDefs::SECOND_PER_MINUTE;
            second = -tz % TimeDefs::SECOND_PER_MINUTE;
        }
    }

    timeStruct.tm_hour = hour;
    timeStruct.tm_min = minute;
    timeStruct.tm_sec = second;

    return FromTimeStruct(timeStruct, milliSecond, microSecond, nanoSecond);
}

LibTime LibTime::FromTimeStruct(const tm &timeStruct, int milliSecond, int microSecond, int nanoSecond)
{
    time_t clanderTimeInSecs = ::mktime(const_cast<tm *>(&timeStruct));
    return LibTime(clanderTimeInSecs*TimeDefs::NANO_SECOND_PER_SECOND + 
                milliSecond * TimeDefs::NANO_SECOND_PER_MILLI_SECOND + microSecond * TimeDefs::NANO_SECOND_PER_MICRO_SECOND + nanoSecond);
}

const LibTime &LibTime::UpdateAppendTime(const TimeSlice &addSliceBaseOnNowTime)
{
    _rawTime = TimeUtil::GetFastNanoTimestamp() + addSliceBaseOnNowTime.GetTotalNanoSeconds();
    // _rawTime = std::chrono::system_clock::now().time_since_epoch().count() / LibTime::_resolutionPerMicroSecond
    //     + addSliceBaseOnNowTime.GetTotalMicroSeconds();
    _UpdateTimeStructs();
    return *this;
}

const LibTime &LibTime::UpdateAppendTime(Int64 addNanoSecBaseOnNowTime)
{
    _rawTime = TimeUtil::GetFastNanoTimestamp() + addNanoSecBaseOnNowTime;
    // _rawTime = std::chrono::system_clock::now().time_since_epoch().count() / LibTime::_resolutionPerMicroSecond
    //     + addMicroSecBaseOnNowTime;
    _UpdateTimeStructs();
    return *this;
}

LibTime &LibTime::operator =(const LibTime &other)
{
    if(this == &other)
        return *this;
    else if(*this == other)
        return *this;

    _rawTime = other._rawTime;
    memcpy(&_localTimeStruct, &other._localTimeStruct, sizeof(tm));
    memcpy(&_gmtTimeStruct, &other._gmtTimeStruct, sizeof(tm));

    return *this;
}

TimeSlice LibTime::GetTimeOfDay() const
{
    Int64 timeZone = TimeUtil::GetTimeZone() * TimeDefs::NANO_SECOND_PER_SECOND;

    Int64 localTime = _rawTime - timeZone;
    return localTime % TimeDefs::NANO_SECOND_PER_DAY;
}

TimeSlice LibTime::GetIntervalTo(const TimeSlice &slice) const
{
    // Get past time(local time zone).
    Int64 localTime = _rawTime - TimeUtil::GetTimeZone() * TimeDefs::NANO_SECOND_PER_SECOND;
    Int64 todayElapsed = localTime % TimeDefs::NANO_SECOND_PER_DAY;

    // Calculate slice value.
    Int64 sliceVal = slice.GetTotalNanoSeconds() - todayElapsed;
    if(sliceVal < 0)
        sliceVal = TimeDefs::NANO_SECOND_PER_DAY + sliceVal;    // 

    return TimeSlice(sliceVal);
}

TimeSlice LibTime::GetIntervalTo(int hour, int minute, int second, int milliSecond /*= 0*/, int microSecond /*= 0*/, int nanoSecond /*= 0*/) const
{
    return GetIntervalTo(TimeSlice(0, hour, minute, second, milliSecond, microSecond, nanoSecond));
}

TimeSlice LibTime::GetIntervalTo(const LibTime &from, const TimeSlice &slice)
{
    return from.GetIntervalTo(slice);
}

TimeSlice LibTime::GetIntervalTo(const LibTime &from, int hour, int minute, int second, int milliSecond /*= 0*/, int microSecond /*= 0*/, int nanoSecond /*= 0*/)
{
    return from.GetIntervalTo(hour, minute, second, milliSecond, microSecond, nanoSecond);
}

TimeSlice LibTime::operator -(const LibTime &time) const
{
    return TimeSlice(_rawTime - time._rawTime);
}

LibTime LibTime::operator +(const TimeSlice &slice) const
{
    return LibTime(_rawTime + slice.GetTotalNanoSeconds());
}

LibTime LibTime::operator -(const TimeSlice &slice) const
{
    return LibTime(_rawTime - slice.GetTotalNanoSeconds());
}

LibTime LibTime::AddYears(int years) const
{
    if(years == 0)
        return *this;

    tm newTimeStruct;
    GetLocalTime(newTimeStruct);

    newTimeStruct.tm_year += years;
    bool isLeap = TimeUtil::IsLeapYear(GetLocalYear());
    if(isLeap &&
       GetLocalMonth() == 2 && GetLocalDay() == 29)
    {
        if(!TimeUtil::IsLeapYear(GetLocalYear() + years))
            newTimeStruct.tm_mday -= 1;
    }

    return FromTimeStruct(newTimeStruct, GetLocalMilliSecond(), GetLocalMicroSecond(), GetLocalNanoSecond());
}

LibTime LibTime::AddMonths(int months) const
{
    LibTime yearAddedTime = AddYears(months / 12);

    months %= 12;
    tm newTimeStruct;
    yearAddedTime.GetLocalTime(newTimeStruct);

    if(months >= 0)
    {
        int remainingMonths = 11 - newTimeStruct.tm_mon;
        if(months > remainingMonths)
        {
            newTimeStruct.tm_year += 1;
            newTimeStruct.tm_mon = months - (12 - newTimeStruct.tm_mon);
        }
        else
        {
            newTimeStruct.tm_mon += months;
        }
    }
    else
    {
        months = -months;
        int elapsedMonths = newTimeStruct.tm_mon + 1;
        if(months >= elapsedMonths)
        {
            newTimeStruct.tm_year -= 1;
            newTimeStruct.tm_mon = 12 - (months - elapsedMonths) - 1;
        }
        else
        {
            newTimeStruct.tm_mon -= months;
        }
    }

    newTimeStruct.tm_mday = std::min<Int32>(newTimeStruct.tm_mday,
                                            TimeUtil::GetMonthMaxDays(yearAddedTime.GetLocalYear(), newTimeStruct.tm_mon + 1));

    return FromTimeStruct(newTimeStruct, GetLocalMilliSecond(), GetLocalMicroSecond(), GetLocalNanoSecond());
}

LibTime LibTime::AddDays(int days) const
{
    return *this + TimeSlice(static_cast<Int64>(days * TimeDefs::NANO_SECOND_PER_DAY));
}

LibTime LibTime::AddHours(int hours) const
{
    return *this + TimeSlice(static_cast<Int64>(hours * TimeDefs::NANO_SECOND_PER_HOUR));
}

LibTime LibTime::AddMinutes(int minutes) const
{
    return *this + TimeSlice(static_cast<Int64>(minutes * TimeDefs::NANO_SECOND_PER_MINUTE));
}

LibTime LibTime::AddSeconds(int seconds) const
{
    return *this + TimeSlice(static_cast<Int64>(seconds * TimeDefs::NANO_SECOND_PER_SECOND));
}

LibTime LibTime::AddMilliSeconds(int milliSeconds) const
{
    return *this + TimeSlice(static_cast<Int64>(milliSeconds * TimeDefs::NANO_SECOND_PER_MILLI_SECOND));
}

LibTime LibTime::AddMicroSeconds(int microSeconds) const
{
    return *this + TimeSlice(static_cast<Int64>(microSeconds * TimeDefs::NANO_SECOND_PER_MICRO_SECOND));
}

LibTime LibTime::AddNanoSeconds(int nanoSeconds) const
{
    return *this + TimeSlice(static_cast<Int64>(nanoSeconds));
}

LibTime LibTime::GetZeroTime() const
{
    // 转换成本地时间
    Int64 zoneTime = static_cast<Int64>(TimeUtil::GetTimeZone()*TimeDefs::NANO_SECOND_PER_SECOND);
    Int64 localTime = _rawTime - zoneTime;
    Int64 zeroTime = localTime / TimeDefs::NANO_SECOND_PER_DAY * TimeDefs::NANO_SECOND_PER_DAY;
    zeroTime += zoneTime;
    return LibTime(zeroTime);
}

LibString LibTime::Format(const Byte8 *outFmt) const
{
    // 1999-02-02 22:22:22
    BUFFER128 buf = {0};
    if(outFmt)
        strftime(buf, sizeof(buf), outFmt, &_localTimeStruct);
    else
        sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d"
                , _localTimeStruct.tm_year + 1900
                , _localTimeStruct.tm_mon + 1
                , _localTimeStruct.tm_mday
                , _localTimeStruct.tm_hour
                , _localTimeStruct.tm_min
                , _localTimeStruct.tm_sec);

    return buf;
}

LibString LibTime::FormatAsGmt(const char *outFmt) const
{
    BUFFER32 buf = {0};
    if(outFmt)
        strftime(buf, sizeof(buf), outFmt, &_gmtTimeStruct);
    else
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &_gmtTimeStruct);

    return buf;
}

LibString LibTime::ToString() const
{
    if(UNLIKELY(!_rawTime))
        return GetZeroTimeString();

    auto localTime = _rawTime - static_cast<Int64>(TimeUtil::GetTimeZone()*TimeDefs::NANO_SECOND_PER_SECOND);
    LibString repr;   
    return repr.AppendFormat("%s.%09lld", Format().c_str(), localTime % TimeDefs::NANO_SECOND_PER_SECOND);
}

LibString LibTime::ToStringOfMillSecondPrecision() const
{
    if(UNLIKELY(!_rawTime))
        return GetZeroTimeString();

    auto localTime = _rawTime - static_cast<Int64>(TimeUtil::GetTimeZone()*TimeDefs::NANO_SECOND_PER_SECOND);
    LibString repr;   
    return repr.AppendFormat("%s.%03lld", Format().c_str(), localTime%TimeDefs::NANO_SECOND_PER_SECOND / TimeDefs::NANO_SECOND_PER_MILLI_SECOND);
}

LibTime::LibTime(Int64 microSecTimestamp)
    :_rawTime(microSecTimestamp)
{
    _UpdateTimeStructs();
}

// LibTime::LibTime(const std::chrono::system_clock::time_point &now)
// {
//     _rawTime = now.time_since_epoch().count() / LibTime::_resolutionPerMicroSecond;    // 
//     _UpdateTimeStructs();
// }

void LibTime::_UpdateTimeStructs()
{
    time_t calendarTime = static_cast<time_t>(_rawTime / TimeDefs::NANO_SECOND_PER_SECOND);
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    ::localtime_s(&_localTimeStruct, &calendarTime);
    ::gmtime_s(&_gmtTimeStruct, &calendarTime);
#else
    ::localtime_r(&calendarTime, &_localTimeStruct);
    ::gmtime_r(&calendarTime, &_gmtTimeStruct);
#endif
}

KERNEL_END
