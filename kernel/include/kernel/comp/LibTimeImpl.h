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
 * Date: 2021-01-02 02:47:38
 * Author: Eric Yonng
 * Description: 
*/

#ifdef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LIB_TIME_H__

KERNEL_BEGIN

// Inline
ALWAYS_INLINE LibTime LibTime::Now()
{
    return LibTime(TimeUtil::GetFastNanoTimestamp());
}

ALWAYS_INLINE Int64 LibTime::NowTimestamp()
{
    return TimeUtil::GetFastNanoTimestamp() / TimeDefs::NANO_SECOND_PER_SECOND;
    // return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

ALWAYS_INLINE Int64 LibTime::NowMilliTimestamp()
{
    return TimeUtil::GetFastNanoTimestamp() / TimeDefs::NANO_SECOND_PER_MILLI_SECOND;
    // return (std::chrono::system_clock().now().time_since_epoch().count() / LibTime::_resolutionPerMicroSecond) / LibTime::_microSecondPerMilliSecond;
}

ALWAYS_INLINE Int64 LibTime::NowMicroTimestamp()
{
    return TimeUtil::GetFastNanoTimestamp() / TimeDefs::NANO_SECOND_PER_MICRO_SECOND;
    // syscall(SYS_clock_gettime, CLOCK_MONOTONIC_RAW, &monotonic_time)
    // return (std::chrono::system_clock().now().time_since_epoch().count() / LibTime::_resolutionPerMicroSecond);
}

ALWAYS_INLINE Int64 LibTime::NowNanoTimestamp()
{
    return TimeUtil::GetFastNanoTimestamp();
}

ALWAYS_INLINE Int64 LibTime::GetSecTimestamp() const
{
    return _rawTime / TimeDefs::NANO_SECOND_PER_SECOND;
}

ALWAYS_INLINE Int64 LibTime::GetMilliTimestamp() const
{
    return _rawTime / TimeDefs::NANO_SECOND_PER_MILLI_SECOND;
}

ALWAYS_INLINE Int64 LibTime::GetMicroTimestamp() const
{
    return _rawTime / TimeDefs::NANO_SECOND_PER_MICRO_SECOND;
}

ALWAYS_INLINE Int64 LibTime::GetNanoTimestamp() const
{
    return _rawTime;
}

ALWAYS_INLINE const LibTime &LibTime::UpdateTime()
{
    _rawTime = TimeUtil::GetFastNanoTimestamp();
    // _rawTime = std::chrono::system_clock::now().time_since_epoch().count() / LibTime::_resolutionPerMicroSecond;
    _UpdateTimeStructs();
    return *this;
}

ALWAYS_INLINE const LibTime &LibTime::UpdateTime(Int64 nanoSecTime)
{
    _rawTime = nanoSecTime;
    _UpdateTimeStructs();
    return *this;
}

ALWAYS_INLINE bool LibTime::operator ==(const LibTime &time) const
{
    return _rawTime == time._rawTime;
}

ALWAYS_INLINE bool LibTime::operator ==(const Int64 &nanoSecondTimestamp) const
{
    return _rawTime == nanoSecondTimestamp;
}

ALWAYS_INLINE bool LibTime::operator !=(const LibTime &time) const
{
    return _rawTime != time._rawTime;
}

ALWAYS_INLINE bool LibTime::operator <(const LibTime &time) const
{
    return _rawTime < time._rawTime;
}

ALWAYS_INLINE bool LibTime::operator >(const LibTime &time) const
{
    return _rawTime > time._rawTime;
}

ALWAYS_INLINE bool LibTime::operator >=(const LibTime &time) const
{
    return _rawTime >= time._rawTime;
}

ALWAYS_INLINE bool LibTime::operator <=(const LibTime &time) const
{
    return _rawTime <= time._rawTime;
}

ALWAYS_INLINE const tm &LibTime::GetGmtTime() const
{
    return _gmtTimeStruct;
}

ALWAYS_INLINE void LibTime::GetGmtTime(tm &timeStruct) const
{
     ::memcpy(&timeStruct, &_gmtTimeStruct, sizeof(tm));
}

ALWAYS_INLINE const tm &LibTime::GetLocalTime() const
{
    return _localTimeStruct;
}

ALWAYS_INLINE void LibTime::GetLocalTime(tm &timeStruct) const
{
    memcpy(&timeStruct, &_localTimeStruct, sizeof(tm));
}

ALWAYS_INLINE Int32 LibTime::GetLocalYear() const
{
    return _localTimeStruct.tm_year + 1900;     // since 1900
}

ALWAYS_INLINE Int32 LibTime::GetLocalMonth() const
{
    return _localTimeStruct.tm_mon + 1;     // start by 1
}

ALWAYS_INLINE Int32 LibTime::GetLocalDay() const
{
    return _localTimeStruct.tm_mday;
}

ALWAYS_INLINE Int32 LibTime::GetLocalDayOfWeek() const
{
    return _localTimeStruct.tm_wday;        // start by 0
}

ALWAYS_INLINE Int32 LibTime::GetLocalDayOfYear() const
{
    return _localTimeStruct.tm_yday + 1;    // start by 1
}

ALWAYS_INLINE Int32 LibTime::GetLocalHour() const
{
    return _localTimeStruct.tm_hour;
}

ALWAYS_INLINE Int32 LibTime::GetLocalMinute() const
{
    return _localTimeStruct.tm_min;
}

ALWAYS_INLINE Int32 LibTime::GetLocalSecond() const
{
    return _localTimeStruct.tm_sec;
}

ALWAYS_INLINE Int32 LibTime::GetLocalMilliSecond() const
{
    auto localTime = _rawTime - static_cast<Int64>(TimeUtil::GetTimeZone() * TimeDefs::NANO_SECOND_PER_SECOND);
    return static_cast<Int32>((localTime % TimeDefs::NANO_SECOND_PER_SECOND) / TimeDefs::NANO_SECOND_PER_MILLI_SECOND);
}

ALWAYS_INLINE Int32 LibTime::GetLocalMicroSecond() const
{
    auto localTime = _rawTime - static_cast<Int64>(TimeUtil::GetTimeZone() * TimeDefs::NANO_SECOND_PER_SECOND);
    return static_cast<Int32>((localTime % TimeDefs::NANO_SECOND_PER_MILLI_SECOND) / TimeDefs::NANO_SECOND_PER_MICRO_SECOND);
}

ALWAYS_INLINE Int32 LibTime::GetLocalNanoSecond() const
{
    auto localTime = _rawTime - static_cast<Int64>(TimeUtil::GetTimeZone() * TimeDefs::NANO_SECOND_PER_SECOND);
    return static_cast<Int32>(localTime % TimeDefs::NANO_SECOND_PER_MICRO_SECOND);
}

ALWAYS_INLINE LibString LibTime::Format(time_t timestamp, const Byte8 *outFmt)
{
    return FromSeconds(timestamp).Format(outFmt);
}

ALWAYS_INLINE LibString LibTime::FormatAsGmt(time_t timestamp, const char *outFmt)
{
    return FromSeconds(timestamp).FormatAsGmt(outFmt);
}

ALWAYS_INLINE LibString LibTime::FormatAsUtc(const char *outFmt) const
{
    return FormatAsGmt(outFmt);
}

ALWAYS_INLINE LibString LibTime::FormatAsUtc(time_t timestamp, const char *outFmt)
{
    return FromSeconds(timestamp).FormatAsGmt(outFmt);
}

ALWAYS_INLINE LibTime::operator bool () const
{
    return _rawTime != 0;
}

KERNEL_END

#endif
