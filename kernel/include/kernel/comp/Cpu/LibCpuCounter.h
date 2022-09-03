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
#include <kernel/comp/Utils/TimeUtil.h>
#include <kernel/comp/memory/memory.h>

KERNEL_BEGIN

class KERNEL_EXPORT LibCpuCounter
{
    POOL_CREATE_OBJ_DEFAULT(LibCpuCounter);

public:
    LibCpuCounter();

    static void InitFrequancy();
    static LibCpuCounter Current();

    // counter更新
    LibCpuCounter &Update();

    // counter流逝的时间
    UInt64 ElapseCount(const LibCpuCounter &start) const;
#if CRYSTAL_TARGET_PLATFORM_LINUX
    UInt64 ElapseNanoseconds(const LibCpuCounter &start) const;
#endif // CRYSTAL_TARGET_PLATFORM_LINUX
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
    LibCpuCounter operator -(const LibCpuCounter &other) const;
    LibCpuCounter operator +(const LibCpuCounter &other) const;
    operator UInt64() const;

    // 当前counter值
    UInt64 GetCurCount() const;

private:
    UInt64 _count;          // linux下是当前tsc计数 windows下是QueryPerformanceCounter计数器
    static UInt64 _countPerSecond;
    static UInt64 _countPerMillisecond;
    static UInt64 _countPerMicroSecond;
#if CRYSTAL_TARGET_PLATFORM_LINUX
    static UInt64 _countPerNanoSecond;
#endif // CRYSTAL_TARGET_PLATFORM_LINUX
};

ALWAYS_INLINE void LibCpuCounter::InitFrequancy()
{
    _countPerSecond = KERNEL_NS::CrystalGetCpuCounterFrequancy();
    _countPerMillisecond = std::max<UInt64>(_countPerSecond / TimeDefs::MILLI_SECOND_PER_SECOND, 1);
    _countPerMicroSecond = std::max<UInt64>(_countPerSecond / TimeDefs::MICRO_SECOND_PER_SECOND, 1);

#if CRYSTAL_TARGET_PLATFORM_LINUX
    _countPerNanoSecond = std::max<UInt64>(_countPerSecond / TimeDefs::NANO_SECOND_PER_SECOND, 1);
#endif // CRYSTAL_TARGET_PLATFORM_LINUX
}

ALWAYS_INLINE LibCpuCounter LibCpuCounter::Current()
{
    return LibCpuCounter().Update();
}

ALWAYS_INLINE LibCpuCounter &LibCpuCounter::Update()
{
    #if CRYSTAL_TARGET_PLATFORM_LINUX
        _count = KERNEL_NS::CrystalRdTsc();
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
    return (_count - start._count) / _countPerNanoSecond;
}
#endif // CRYSTAL_TARGET_PLATFORM_LINUX

ALWAYS_INLINE UInt64 LibCpuCounter::ElapseMicroseconds(const LibCpuCounter &start) const
{
    return (_count - start._count) / _countPerMicroSecond;
}

ALWAYS_INLINE UInt64 LibCpuCounter::ElapseMilliseconds(const LibCpuCounter &start) const
{
    return (_count - start._count) / _countPerMillisecond;
}

ALWAYS_INLINE UInt64 LibCpuCounter::ElapseSeconds(const LibCpuCounter &start) const
{
    return (_count - start._count) / _countPerSecond;
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

ALWAYS_INLINE LibCpuCounter LibCpuCounter::operator -(const LibCpuCounter &other) const
{
    LibCpuCounter newCounter = *this;
    if(UNLIKELY(newCounter._count < other._count))
    {
        newCounter._count = 0;
        return newCounter;
    }

    newCounter._count -= other._count;
    return newCounter;
}

ALWAYS_INLINE LibCpuCounter LibCpuCounter::operator +(const LibCpuCounter &other) const
{
    LibCpuCounter newCounter = *this;
    newCounter._count += other._count;
    return newCounter;
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
