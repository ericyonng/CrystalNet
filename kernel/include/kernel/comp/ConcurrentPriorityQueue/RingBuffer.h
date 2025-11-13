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
 * Date: 2025-11-13 23:47:58
 * Author: Eric Yonng
 * Description: 
 * 1. 使用环形缓冲区实现FIFO
 * 2. 可以用环形缓冲区实现高效的消息队列, 当缓冲区满那么生产者等待, 如果缓冲区没有可读的则消费者等待
 * 测试：多个线程操作测试Pop出来的元素地址不冲突
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CONCURRENT_PRIORITY_QUEUE_RING_BUFFER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CONCURRENT_PRIORITY_QUEUE_RING_BUFFER_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>

KERNEL_BEGIN

template<Int64 N>
concept IsPowerOfTwo = (N > 1) && ((N & (N - 1)) == 0);

// 先入后出, Elem必须是可移动的
template<typename Elem, Int64 CapacitySize = 16 * 1024, typename BuildType = _Build::MT>
requires std::movable<Elem> && requires(BuildType build)
{
    build.V;

    // TODO:2的整数倍
    requires IsPowerOfTwo<CapacitySize>;
}
class RingBuffer
{
    POOL_CREATE_TEMPLATE_OBJ_DEFAULT(RingBuffer, Elem, CapacitySize, BuildType)

public:
    RingBuffer()
    :_read{0}
    ,_writeable{0}
    {
    }


    bool Push(const Elem &e);
    bool Push(Elem &&e);
    bool Pop(Elem &e);
    bool IsFull() const noexcept;
    static constexpr Int64 Capacity() noexcept;
    void Clear();
    Int64 ReadableSize() const noexcept;
    Int64 WritableSize() const noexcept;
    bool CanRead() const noexcept;
    bool CanWrite() const noexcept;
    
private:
    static Int64 _NextIndex(Int64 index);

    static Int64 _Modulo(Int64 index);

private:
    Elem _elems[CapacitySize];
    // 已写
    std::atomic<Int64> _read;
    // 可写pos
    std::atomic<Int64> _writeable;
};

template<typename Elem, Int64 CapacitySize, typename BuildType>
requires std::movable<Elem> && requires(BuildType build)
{
    build.V;

    // TODO:2的整数倍
    requires IsPowerOfTwo<CapacitySize>;
}
ALWAYS_INLINE auto RingBuffer<Elem, CapacitySize, BuildType>::_NextIndex(Int64 index) -> Int64
{
    return _Modulo(index + 1);
}

template<typename Elem, Int64 CapacitySize, typename BuildType>
requires std::movable<Elem> && requires(BuildType build)
{
    build.V;

    // TODO:2的整数倍
    requires IsPowerOfTwo<CapacitySize>;
}
ALWAYS_INLINE Int64 RingBuffer<Elem, CapacitySize, BuildType>::_Modulo(Int64 index)
{
    return index & (CapacitySize - 1);
}

template<typename Elem, Int64 CapacitySize, typename BuildType>
requires std::movable<Elem> && requires(BuildType build)
{
    build.V;

    // TODO:2的整数倍
    requires IsPowerOfTwo<CapacitySize>;
}
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_IMPL(RingBuffer, Elem, CapacitySize, BuildType);


template<typename Elem, Int64 CapacitySize, typename BuildType>
requires std::movable<Elem> && requires(BuildType build)
{
    build.V;

    // TODO:2的整数倍
    requires IsPowerOfTwo<CapacitySize>;
}
ALWAYS_INLINE bool RingBuffer<Elem, CapacitySize, BuildType>::Push(const Elem &e)
{
    do
    {
        auto writeable = _writeable.load(std::memory_order_relaxed);
        auto nextWriteIndex = _NextIndex(writeable);
        auto currentRead = _read.load(std::memory_order_acquire);

        // 写满返回
        if (UNLIKELY(nextWriteIndex == currentRead))
            return false;
        
        if (_writeable.compare_exchange_weak(writeable, nextWriteIndex, std::memory_order_release, std::memory_order_relaxed))
        {
            // 左值拷贝, 右值移动
            _elems[writeable] = e;
            return true;
        }
    }
    while (true);

    return true;
}

template<typename Elem, Int64 CapacitySize, typename BuildType>
requires std::movable<Elem> && requires(BuildType build)
{
    build.V;

    // TODO:2的整数倍
    requires IsPowerOfTwo<CapacitySize>;
}
ALWAYS_INLINE bool RingBuffer<Elem, CapacitySize, BuildType>::Push(Elem &&e)
{
    do
    {
        auto writeable = _writeable.load(std::memory_order_relaxed);
        auto nextWriteIndex = _NextIndex(writeable);
        auto currentRead = _read.load(std::memory_order_acquire);

        // 写满返回
        if (UNLIKELY(nextWriteIndex == currentRead))
            return false;
        
        if (_writeable.compare_exchange_weak(writeable, nextWriteIndex, std::memory_order_release, std::memory_order_relaxed))
        {
            // 左值拷贝, 右值移动
            _elems[writeable] = std::forward<Elem>(e);
            return true;
        }
    }
    while (true);

    return true;
}

template<typename Elem, Int64 CapacitySize, typename BuildType>
requires std::movable<Elem> && requires(BuildType build)
{
    build.V;

    // TODO:2的整数倍
    requires IsPowerOfTwo<CapacitySize>;
}
ALWAYS_INLINE bool RingBuffer<Elem, CapacitySize, BuildType>::Pop(Elem &e)
{
    auto currentRead = _read.load(std::memory_order_relaxed);
    do
    {
        Int64 currentTail = _writeable.load(std::memory_order_acquire);
        if (UNLIKELY(currentTail == currentRead))
            return false;

        // read指针只保护当前元素, 所以需要即时读取数据
        e = std::move(_elems[currentRead & (CapacitySize - 1)]);
        auto nextRead = _NextIndex(currentRead);
        if (_read.compare_exchange_weak(currentRead, nextRead, std::memory_order_release, std::memory_order_relaxed))
            return true;
    }
    while (true);

    return true;
}

template<typename Elem, Int64 CapacitySize, typename BuildType>
requires std::movable<Elem> && requires(BuildType build)
{
    build.V;

    // TODO:2的整数倍
    requires IsPowerOfTwo<CapacitySize>;
}
ALWAYS_INLINE bool RingBuffer<Elem, CapacitySize, BuildType>::IsFull() const noexcept
{
    auto writeable = _writeable.load(std::memory_order_acquire);
    return _NextIndex(writeable) == _read.load(std::memory_order_acquire);
}

template<typename Elem, Int64 CapacitySize, typename BuildType>
requires std::movable<Elem> && requires(BuildType build)
{
    build.V;

    // TODO:2的整数倍
    requires IsPowerOfTwo<CapacitySize>;
}
ALWAYS_INLINE constexpr Int64 RingBuffer<Elem, CapacitySize, BuildType>::Capacity() noexcept
{
    return CapacitySize;
}

template<typename Elem, Int64 CapacitySize, typename BuildType>
requires std::movable<Elem> && requires(BuildType build)
{
    build.V;

    // TODO:2的整数倍
    requires IsPowerOfTwo<CapacitySize>;
}
ALWAYS_INLINE void RingBuffer<Elem, CapacitySize, BuildType>::Clear()
{
    _writeable.store(0, std::memory_order_release);
    _read.store(0, std::memory_order_release);
}

template<typename Elem, Int64 CapacitySize, typename BuildType>
requires std::movable<Elem> && requires(BuildType build)
{
    build.V;

    // TODO:2的整数倍
    requires IsPowerOfTwo<CapacitySize>;
}
ALWAYS_INLINE Int64 RingBuffer<Elem, CapacitySize, BuildType>::ReadableSize() const noexcept
{
    const auto writeable = _writeable.load(std::memory_order_acquire);
    const auto readable = _read.load(std::memory_order_acquire);
    if (writeable >= readable)
        return writeable - readable;

    // 数据是两部分:从0 到writeable, 从readable到最后一个元素
    return (writeable + CapacitySize) - readable;
}

template<typename Elem, Int64 CapacitySize, typename BuildType>
requires std::movable<Elem> && requires(BuildType build)
{
    build.V;

    // TODO:2的整数倍
    requires IsPowerOfTwo<CapacitySize>;
}
ALWAYS_INLINE Int64 RingBuffer<Elem, CapacitySize, BuildType>::WritableSize() const noexcept
{
    return CapacitySize - ReadableSize();
}

template<typename Elem, Int64 CapacitySize, typename BuildType>
requires std::movable<Elem> && requires(BuildType build)
{
    build.V;

    // TODO:2的整数倍
    requires IsPowerOfTwo<CapacitySize>;
}
ALWAYS_INLINE bool RingBuffer<Elem, CapacitySize, BuildType>::CanRead() const noexcept
{
    return _read.load(std::memory_order_acquire) != _writeable.load(std::memory_order_acquire);
}

template<typename Elem, Int64 CapacitySize, typename BuildType>
requires std::movable<Elem> && requires(BuildType build)
{
    build.V;

    // TODO:2的整数倍
    requires IsPowerOfTwo<CapacitySize>;
}
ALWAYS_INLINE bool RingBuffer<Elem, CapacitySize, BuildType>::CanWrite() const noexcept
{
    auto writeable = _writeable.load(std::memory_order_acquire);
    return _NextIndex(writeable) != _read.load(std::memory_order_acquire);
}

KERNEL_END

#endif
