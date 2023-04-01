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
 * Date: 2022-01-08 03:43:04
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CPU_LIB_CPU_COUNTER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CPU_LIB_CPU_COUNTER_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>

KERNEL_BEGIN

class KERNEL_EXPORT LibCpuFrequency
{
public:
    static void InitFrequancy();

    static UInt64 _countPerSecond;
    static UInt64 _countPerMillisecond;
    static UInt64 _countPerMicroSecond;
    static UInt64 _countPerNanoSecond;
};

ALWAYS_INLINE void LibCpuFrequency::InitFrequancy()
{
    _countPerSecond = KERNEL_NS::CrystalGetCpuCounterFrequancy();
    _countPerMillisecond = std::max<UInt64>(_countPerSecond / TimeDefs::MILLI_SECOND_PER_SECOND, 1);
    _countPerMicroSecond = std::max<UInt64>(_countPerSecond / TimeDefs::MICRO_SECOND_PER_SECOND, 1);

#if CRYSTAL_TARGET_PLATFORM_LINUX
    _countPerNanoSecond = std::max<UInt64>(_countPerSecond / TimeDefs::NANO_SECOND_PER_SECOND, 1);
#else
    _countPerNanoSecond = _countPerMicroSecond * 1000;
#endif // CRYSTAL_TARGET_PLATFORM_LINUX
}


class KERNEL_EXPORT LibCpuSlice
{
    POOL_CREATE_OBJ_DEFAULT(LibCpuSlice);

public:
    LibCpuSlice(UInt64 count);
    ~LibCpuSlice() {}

    UInt64 GetTotalCount() const;
    UInt64 GetTotalNanoseconds() const;
    UInt64 GetTotalMicroseconds() const;
    UInt64 GetTotalMilliseconds() const;
    UInt64 GetTotalSeconds() const;

    static LibCpuSlice FromSeconds(UInt64 seconds);
    static LibCpuSlice FromMilliseconds(UInt64 milliseconds);
    static LibCpuSlice FromMicroseconds(UInt64 microseconds);
    static LibCpuSlice FromNanoseconds(UInt64 nanoseconds);
    LibCpuSlice &SetSeconds(UInt64 seconds);
    LibCpuSlice &SetMilliseconds(UInt64 milliseconds);
    LibCpuSlice &SetMicroseconds(UInt64 microseconds);
    LibCpuSlice &SetNanoseconds(UInt64 nanoseconds);

private:
    UInt64 _count;
};

ALWAYS_INLINE LibCpuSlice::LibCpuSlice(UInt64 count)
:_count(count)
{

}

ALWAYS_INLINE UInt64 LibCpuSlice::GetTotalCount() const
{
    return _count;
}

ALWAYS_INLINE UInt64 LibCpuSlice::GetTotalNanoseconds() const
{
    return _count / LibCpuFrequency::_countPerNanoSecond;
}

ALWAYS_INLINE UInt64 LibCpuSlice::GetTotalMicroseconds() const
{
    return _count / LibCpuFrequency::_countPerMicroSecond;
}

ALWAYS_INLINE UInt64 LibCpuSlice::GetTotalMilliseconds() const
{
    return _count / LibCpuFrequency::_countPerMillisecond;
}

ALWAYS_INLINE UInt64 LibCpuSlice::GetTotalSeconds() const
{
    return _count / LibCpuFrequency::_countPerSecond;
}

ALWAYS_INLINE LibCpuSlice LibCpuSlice::FromSeconds(UInt64 seconds)
{
    return LibCpuSlice(seconds * LibCpuFrequency::_countPerSecond);
}

ALWAYS_INLINE LibCpuSlice LibCpuSlice::FromMilliseconds(UInt64 milliseconds)
{
    return LibCpuSlice(milliseconds * LibCpuFrequency::_countPerMillisecond);
}

ALWAYS_INLINE LibCpuSlice LibCpuSlice::FromMicroseconds(UInt64 microseconds)
{
    return LibCpuSlice(microseconds * LibCpuFrequency::_countPerMicroSecond);
}

ALWAYS_INLINE LibCpuSlice LibCpuSlice::FromNanoseconds(UInt64 nanoseconds)
{
    return LibCpuSlice(nanoseconds * LibCpuFrequency::_countPerNanoSecond);
}

ALWAYS_INLINE LibCpuSlice &LibCpuSlice::SetSeconds(UInt64 seconds)
{
    _count = seconds * LibCpuFrequency::_countPerSecond;
    return *this;
}

ALWAYS_INLINE LibCpuSlice &LibCpuSlice::SetMilliseconds(UInt64 milliseconds)
{
    _count = milliseconds * LibCpuFrequency::_countPerMillisecond;

    return *this;
}

ALWAYS_INLINE LibCpuSlice &LibCpuSlice::SetMicroseconds(UInt64 microseconds)
{
    _count = microseconds * LibCpuFrequency::_countPerMicroSecond;

    return *this;
}

ALWAYS_INLINE LibCpuSlice &LibCpuSlice::SetNanoseconds(UInt64 nanoseconds)
{
    _count = nanoseconds * LibCpuFrequency::_countPerNanoSecond;

    return *this;
}

class KERNEL_EXPORT LibCpuCounter
{
    POOL_CREATE_OBJ_DEFAULT(LibCpuCounter);

public:
    LibCpuCounter();
    LibCpuCounter(UInt64 count);

    static LibCpuCounter Current();

    // counter更新
    LibCpuCounter &Update();

    // counter流逝的时间
    UInt64 ElapseCount(const LibCpuCounter &start) const;
    UInt64 ElapseNanoseconds(const LibCpuCounter &start) const;
    UInt64 ElapseMicroseconds(const LibCpuCounter &start) const;
    UInt64 ElapseMilliseconds(const LibCpuCounter &start) const;
    UInt64 ElapseSeconds(const LibCpuCounter &start) const;

    LibCpuCounter &operator = (const LibCpuCounter &other);
    LibCpuCounter &operator = (UInt64 count);
    bool operator <(const LibCpuCounter &other) const;
    bool operator >(const LibCpuCounter &other) const;
    bool operator >=(const LibCpuCounter &other) const;
    bool operator <=(const LibCpuCounter &other) const;
    bool operator ==(const LibCpuCounter &other) const;
    bool operator !=(const LibCpuCounter &other) const;

    // 得到绝对值的slice
    LibCpuSlice operator -(const LibCpuCounter &other) const;
    // 得到减后的值,取非负数，最小值为0
    LibCpuCounter operator -(const LibCpuSlice &other) const;
    LibCpuCounter operator +(const LibCpuSlice &slice) const;

    LibCpuCounter &operator -=(const LibCpuSlice &other);
    LibCpuCounter &operator += (const LibCpuSlice &slice);
    operator UInt64() const;

    // 当前counter值
    UInt64 GetCurCount() const;

private:
    UInt64 _count;          // linux下是当前tsc计数 windows下是QueryPerformanceCounter计数器
};

ALWAYS_INLINE LibCpuCounter::LibCpuCounter()
    :_count(0)
{
    
}

ALWAYS_INLINE LibCpuCounter::LibCpuCounter(UInt64 count)
:_count(count)
{

}

ALWAYS_INLINE LibCpuCounter LibCpuCounter::Current()
{
    return LibCpuCounter().Update();
}

ALWAYS_INLINE LibCpuCounter &LibCpuCounter::Update()
{
    #if CRYSTAL_TARGET_PLATFORM_LINUX
        _count = KERNEL_NS::CrystalNativeRdTsc();
    #endif

    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
    LARGE_INTEGER li;
    ::QueryPerformanceCounter(&li);
    _count = li.QuadPart;
    #endif

    return *this;
}

ALWAYS_INLINE UInt64 LibCpuCounter::ElapseCount(const LibCpuCounter &start) const
{
    return _count - start._count;
}

#if CRYSTAL_TARGET_PLATFORM_LINUX
ALWAYS_INLINE UInt64 LibCpuCounter::ElapseNanoseconds(const LibCpuCounter &start) const
{
    return (_count - start._count) / LibCpuFrequency::_countPerNanoSecond;
}
#else
ALWAYS_INLINE UInt64 LibCpuCounter::ElapseNanoseconds(const LibCpuCounter &start) const
{
    return (_count - start._count) * 1000 / LibCpuFrequency::_countPerMicroSecond;
}
#endif // CRYSTAL_TARGET_PLATFORM_LINUX

ALWAYS_INLINE UInt64 LibCpuCounter::ElapseMicroseconds(const LibCpuCounter &start) const
{
    return (_count - start._count) / LibCpuFrequency::_countPerMicroSecond;
}

ALWAYS_INLINE UInt64 LibCpuCounter::ElapseMilliseconds(const LibCpuCounter &start) const
{
    return (_count - start._count) / LibCpuFrequency::_countPerMillisecond;
}

ALWAYS_INLINE UInt64 LibCpuCounter::ElapseSeconds(const LibCpuCounter &start) const
{
    return (_count - start._count) / LibCpuFrequency::_countPerSecond;
}

ALWAYS_INLINE LibCpuCounter &LibCpuCounter::operator = (const LibCpuCounter &other)
{
    _count = other._count;

    return *this;
}

ALWAYS_INLINE LibCpuCounter &LibCpuCounter::operator = (UInt64 count)
{
    _count = count;
    return *this;
}

ALWAYS_INLINE bool LibCpuCounter::operator <(const LibCpuCounter &other) const
{
    return _count < other._count;
}

ALWAYS_INLINE bool LibCpuCounter::operator >(const LibCpuCounter &other) const
{
    return _count > other._count;
}

ALWAYS_INLINE bool LibCpuCounter::operator >=(const LibCpuCounter &other) const
{
    return _count >= other._count;
}

ALWAYS_INLINE bool LibCpuCounter::operator <=(const LibCpuCounter &other) const
{
    return _count <= other._count;
}

ALWAYS_INLINE bool LibCpuCounter::operator ==(const LibCpuCounter &other) const
{
    return _count == other._count;
}

ALWAYS_INLINE bool LibCpuCounter::operator !=(const LibCpuCounter &other) const
{
    return _count != other._count;
}

ALWAYS_INLINE LibCpuSlice LibCpuCounter::operator -(const LibCpuCounter &other) const
{
    if(UNLIKELY(_count < other._count))
        return LibCpuSlice(other._count - _count);

    return LibCpuSlice(_count - other._count);
}

ALWAYS_INLINE LibCpuCounter LibCpuCounter::operator -(const LibCpuSlice &other) const
{
    const auto count = other.GetTotalCount();
    if(UNLIKELY(_count < count))
        return LibCpuCounter(0);

    return LibCpuCounter(_count - count);
}

ALWAYS_INLINE LibCpuCounter LibCpuCounter::LibCpuCounter::operator +(const LibCpuSlice &slice) const
{
    return LibCpuCounter(_count + slice.GetTotalCount());
}

ALWAYS_INLINE LibCpuCounter &LibCpuCounter::operator -=(const LibCpuSlice &other)
{
    const auto count = other.GetTotalCount();
    if(UNLIKELY(_count < count))
    {
        _count = 0;
        return *this;
    }

    _count -= count;
    return *this;
}

ALWAYS_INLINE LibCpuCounter &LibCpuCounter::operator += (const LibCpuSlice &slice)
{
    _count += slice.GetTotalCount();
    return *this;
}

ALWAYS_INLINE LibCpuCounter::operator UInt64() const
{
    return _count;
}

ALWAYS_INLINE UInt64 LibCpuCounter::GetCurCount() const
{
    return _count;
}

KERNEL_END

#endif
