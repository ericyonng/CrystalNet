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
 * Date: 2021-01-02 02:48:05
 * Author: Eric Yonng
 * Description: 
*/

#ifdef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TIME_SLICE_H__

KERNEL_BEGIN

ALWAYS_INLINE TimeSlice::TimeSlice()
    :_slice(0)
{

}

ALWAYS_INLINE TimeSlice::TimeSlice(int seconds, Int64 milliSeconds /*= 0*/, Int64 microSeconds /*= 0*/, Int64 nanoSeconds /*= 0*/)
{
    _slice = seconds * TimeDefs::NANO_SECOND_PER_SECOND +
        milliSeconds * TimeDefs::NANO_SECOND_PER_MILLI_SECOND + microSeconds * TimeDefs::NANO_SECOND_PER_MICRO_SECOND + nanoSeconds;
}

ALWAYS_INLINE TimeSlice::TimeSlice(const TimeSlice &slice)
    :_slice(slice._slice)
{
}

ALWAYS_INLINE TimeSlice::TimeSlice(int days, int hours, int minutes, Int64 seconds, Int64 milliSeconds /* = 0 */, Int64 microSeconds /* = 0 */, Int64 nanoSeconds)
{
    _slice = static_cast<Int64>((((((days * 24) + hours) * 60) + minutes) * 60) + seconds) *
        TimeDefs::NANO_SECOND_PER_SECOND + milliSeconds * TimeDefs::NANO_SECOND_PER_MILLI_SECOND + microSeconds * TimeDefs::NANO_SECOND_PER_MICRO_SECOND + nanoSeconds;
}

ALWAYS_INLINE TimeSlice::~TimeSlice()
{
}

ALWAYS_INLINE int TimeSlice::GetDays() const
{
    return static_cast<int>(_slice / TimeDefs::NANO_SECOND_PER_DAY);
}

ALWAYS_INLINE int TimeSlice::GetHours() const
{
    return static_cast<int>((_slice % TimeDefs::NANO_SECOND_PER_DAY) /
                            TimeDefs::NANO_SECOND_PER_HOUR);
}

ALWAYS_INLINE int TimeSlice::GetMinutes() const
{
    return static_cast<int>(_slice % TimeDefs::NANO_SECOND_PER_HOUR  /
                            TimeDefs::NANO_SECOND_PER_MINUTE);
}

ALWAYS_INLINE int TimeSlice::GetSeconds() const
{
    return static_cast<int>(_slice %  TimeDefs::NANO_SECOND_PER_MINUTE /
                            TimeDefs::NANO_SECOND_PER_SECOND);
}

ALWAYS_INLINE int TimeSlice::GetMilliSeconds() const
{
    return static_cast<int>(_slice % TimeDefs::NANO_SECOND_PER_SECOND /
                            TimeDefs::NANO_SECOND_PER_MILLI_SECOND);
}

ALWAYS_INLINE int TimeSlice::GetMicroSeconds() const
{
    return static_cast<int>(_slice % TimeDefs::NANO_SECOND_PER_MILLI_SECOND / TimeDefs::NANO_SECOND_PER_MICRO_SECOND);
}

ALWAYS_INLINE int TimeSlice::GetNanoSeconds() const
{
    return static_cast<int>(_slice % TimeDefs::NANO_SECOND_PER_MICRO_SECOND);
}

ALWAYS_INLINE int TimeSlice::GetTotalDays() const
{
    return static_cast<int>(_slice / TimeDefs::NANO_SECOND_PER_DAY);
}

ALWAYS_INLINE int TimeSlice::GetTotalHours() const
{
    return static_cast<int>(_slice / TimeDefs::NANO_SECOND_PER_HOUR);
}

ALWAYS_INLINE int TimeSlice::GetTotalMinutes() const
{
    return static_cast<int>(_slice / TimeDefs::NANO_SECOND_PER_MINUTE);
}

ALWAYS_INLINE int TimeSlice::GetTotalSeconds() const
{
    return static_cast<int>(_slice / TimeDefs::NANO_SECOND_PER_SECOND);
}

ALWAYS_INLINE Int64 TimeSlice::GetTotalMilliSeconds() const
{
    return static_cast<Int64>(_slice / TimeDefs::NANO_SECOND_PER_MILLI_SECOND);
}

ALWAYS_INLINE const Int64 TimeSlice::GetTotalMicroSeconds() const
{
    return static_cast<Int64>(_slice / TimeDefs::NANO_SECOND_PER_MICRO_SECOND);
}

ALWAYS_INLINE const Int64 &TimeSlice::GetTotalNanoSeconds() const
{
    return _slice;
}

ALWAYS_INLINE TimeSlice TimeSlice::operator +(const TimeSlice &slice) const
{
    return TimeSlice(_slice + slice._slice);
}

ALWAYS_INLINE TimeSlice TimeSlice::operator -(const TimeSlice &slice) const
{
    return TimeSlice(_slice - slice._slice);
}

ALWAYS_INLINE TimeSlice &TimeSlice::operator +=(const TimeSlice &slice)
{
    _slice += slice._slice;
    return *this;
}

ALWAYS_INLINE TimeSlice &TimeSlice::operator -=(const TimeSlice &slice)
{
    _slice -= slice._slice;
    return *this;
}

ALWAYS_INLINE bool TimeSlice::operator ==(const TimeSlice &slice) const
{
    return _slice == slice._slice;
}

ALWAYS_INLINE bool TimeSlice::operator !=(const TimeSlice &slice) const
{
    return !(*this == slice);
}

ALWAYS_INLINE bool TimeSlice::operator <(const TimeSlice &slice) const
{
    return _slice < slice._slice;
}

ALWAYS_INLINE bool TimeSlice::operator >(const TimeSlice &slice) const
{
    return _slice > slice._slice;
}

ALWAYS_INLINE bool TimeSlice::operator <=(const TimeSlice &slice) const
{
    return _slice <= slice._slice;
}

ALWAYS_INLINE bool TimeSlice::operator >=(const TimeSlice &slice) const
{
    return _slice >= slice._slice;
}

ALWAYS_INLINE TimeSlice &TimeSlice::operator =(const TimeSlice &slice)
{
    _slice = slice._slice;
    return *this;
}

ALWAYS_INLINE TimeSlice &TimeSlice::operator =(Int64 nanoSecSlice)
{
    _slice = nanoSecSlice;
    return *this;
}

ALWAYS_INLINE LibString TimeSlice::ToString() const
{
    int days = GetDays();
    if(days != 0)
        return LibString().AppendFormat("%d %02d:%02d:%02d.%09d",
                                    days, GetHours(), GetMinutes(), GetSeconds(), static_cast<Int32>(_slice % TimeDefs::NANO_SECOND_PER_SECOND));
    else
        return LibString().AppendFormat("%02d:%02d:%02d.%09d",
                                    GetHours(), GetMinutes(), GetSeconds(), static_cast<Int32>(_slice % TimeDefs::NANO_SECOND_PER_SECOND));
}

ALWAYS_INLINE TimeSlice::TimeSlice(const Int64 &slice)
:_slice(slice)
{

}

ALWAYS_INLINE TimeSlice TimeSlice::FromSeconds(Int64 seconds)
{
    return TimeSlice(TimeDefs::NANO_SECOND_PER_SECOND * seconds);
}

ALWAYS_INLINE TimeSlice TimeSlice::FromMilliSeconds(Int64 milliseconds)
{
    return TimeSlice(TimeDefs::NANO_SECOND_PER_MILLI_SECOND * milliseconds);
}

ALWAYS_INLINE TimeSlice TimeSlice::FromMicroSeconds(Int64 microseconds)
{
    return TimeSlice(TimeDefs::NANO_SECOND_PER_MICRO_SECOND * microseconds);
}

ALWAYS_INLINE TimeSlice TimeSlice::FromNanoSeconds(Int64 nanoseconds)
{
    return TimeSlice(nanoseconds);
}

KERNEL_END

#endif
