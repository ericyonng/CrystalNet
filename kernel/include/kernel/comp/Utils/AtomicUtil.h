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
 * Date: 2023-04-29 13:27:00
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_ATOMIC_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_ATOMIC_UTIL_H__

#pragma once

#include <kernel/kernel_inc.h>

KERNEL_BEGIN

class KERNEL_EXPORT AtomicUtil
{
public:
    static Int32 Get(volatile Int32 *ptr);
    static Int64 Get(volatile Int64 *ptr);

    /*
    * 设置ptr的值为新的value，并返回操作之前的旧值
    */
    static Int32 Set(volatile Int32 *ptr, Int32 value);
    static Int64 Set(volatile Int64 *ptr, Int64 value);

    /*
    * 自增且返回操作之前的值
    */
   static Int32 FetchAndAdd(volatile Int32 *ptr, Int32 value);
   static Int64 FetchAndAdd(volatile Int64 *ptr, Int64 value);

   /*
   * 自减且返回操作之前的值
   */
   static Int32 FetchAndSub(volatile Int32 *ptr, Int32 value);
   static Int64 FetchAndSub(volatile Int64 *ptr, Int64 value);

   /*
   * 按位与并更新ptr的内存, 返回操作之前ptr的值
   */
   static Int32 FetchAndAnd(volatile Int32 *ptr, Int32 value);
   static Int64 FetchAndAnd(volatile Int64 *ptr, Int64 value);

   /*
   * 按位或并更新ptr的内存, 返回操作之前ptr的值
   */
   static Int32 FetchAndOr(volatile Int32 *ptr, Int32 value);
   static Int64 FetchAndOr(volatile Int64 *ptr, Int64 value);

   /*
   * 按位异或并更新ptr的内存, 返回操作之前ptr的值
   */
   static Int32 FetchAndXOr(volatile Int32 *ptr, Int32 value);
   static Int64 FetchAndXOr(volatile Int64 *ptr, Int64 value);

   /*
   * 比较并交换, 返回操作之前ptr的值
   */
   static Int32 CompareAndSwap(volatile Int32 *ptr, Int32 exchange, Int32 comparand);
   static Int64 CompareAndSwap(volatile Int64 *ptr, Int64 exchange, Int64 comparand);

    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        /**
        * 注意:linux下不会更新expectValue
        * @param(destMemoryAddress):要操作的内存位置
        * @param(expectValue):预先期望值, 操作失败会返回destMemoryAddress的新值
        * @param(newValue):要填入内存位置的新值
        * @return(bool):若destMemoryAddress内存位置的值与expectValue不同则返回失败并且更新expectValue值为新值, 若相同则返回true
        **/
        static bool CompareAndSwap(volatile Int128 *destMemoryAddress, Int128 *expectValue, const Int128 &newValue);
    #else
        static bool CompareAndSwap(Int128 *destMemoryAddress, Int128 *expectValue, const Int128 &newValue);
    #endif
};

ALWAYS_INLINE Int32 AtomicUtil::Get(volatile Int32 *ptr)
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    return __sync_fetch_and_add(ptr, 0);
#elif CRYSTAL_TARGET_PLATFORM_WINDOWS
    return ::InterlockedExchangeAdd((volatile LONG *)ptr, 0);
#elif CRYSTAL_TARGET_PLATFORM_IPHONE
    return __sync_fetch_and_add(ptr, 0);
#elif CRYSTAL_TARGET_PLATFORM_MAC
    return __sync_fetch_and_add(ptr, 0);
#elif CRYSTAL_TARGET_PLATFORM_ANDROID
    return __sync_fetch_and_add(ptr, 0);
#endif
}

ALWAYS_INLINE Int64 AtomicUtil::Get(volatile Int64 *ptr)
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    return __sync_fetch_and_add(ptr, 0);
#elif CRYSTAL_TARGET_PLATFORM_WINDOWS
    return ::InterlockedExchangeAdd64(ptr, 0);
#elif CRYSTAL_TARGET_PLATFORM_IPHONE
    return __sync_fetch_and_add(ptr, 0);
#elif CRYSTAL_TARGET_PLATFORM_MAC
    return __sync_fetch_and_add(ptr, 0);
#elif CRYSTAL_TARGET_PLATFORM_ANDROID
    return __sync_fetch_and_add(ptr, 0);
#endif
}

ALWAYS_INLINE Int32 AtomicUtil::Set(volatile Int32 *ptr, Int32 value)
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    return __sync_lock_test_and_set(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_WINDOWS
    return ::InterlockedExchange((volatile LONG *)ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_IPHONE
    return __sync_lock_test_and_set(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_MAC
    return __sync_lock_test_and_set(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_ANDROID
    return __sync_lock_test_and_set(ptr, value);
#endif
}

ALWAYS_INLINE Int64 AtomicUtil::Set(volatile Int64 *ptr, Int64 value)
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    return __sync_lock_test_and_set(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_WINDOWS
    return ::InterlockedExchange64(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_IPHONE
    return __sync_lock_test_and_set(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_MAC
    return __sync_lock_test_and_set(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_ANDROID
    return __sync_lock_test_and_set(ptr, value);
#endif
}

ALWAYS_INLINE Int32 AtomicUtil::FetchAndAdd(volatile Int32 *ptr, Int32 value)
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    return __sync_fetch_and_add(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_WINDOWS
    return ::InterlockedExchangeAdd((volatile LONG *)ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_IPHONE
    return __sync_fetch_and_add(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_MAC
    return __sync_fetch_and_add(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_ANDROID
    return __sync_fetch_and_add(ptr, value);
#endif
}

ALWAYS_INLINE Int64 AtomicUtil::FetchAndAdd(volatile Int64 *ptr, Int64 value)
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    return __sync_fetch_and_add(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_WINDOWS
    return ::InterlockedExchangeAdd64(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_IPHONE
    return __sync_fetch_and_add(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_MAC
    return __sync_fetch_and_add(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_ANDROID
    return __sync_fetch_and_add(ptr, value);
#endif
}

ALWAYS_INLINE Int32 AtomicUtil::FetchAndSub(volatile Int32 *ptr, Int32 value)
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    return __sync_fetch_and_sub(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_WINDOWS
    return ::InterlockedExchangeAdd((volatile LONG *)ptr, - value);
#elif CRYSTAL_TARGET_PLATFORM_IPHONE
    return __sync_fetch_and_sub(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_MAC
    return __sync_fetch_and_sub(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_ANDROID
    return __sync_fetch_and_sub(ptr, value);
#endif
}

ALWAYS_INLINE Int64 AtomicUtil::FetchAndSub(volatile Int64 *ptr, Int64 value)
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    return __sync_fetch_and_sub(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_WINDOWS
    return ::InterlockedExchangeAdd64(ptr, - value);
#elif CRYSTAL_TARGET_PLATFORM_IPHONE
    return __sync_fetch_and_sub(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_MAC
    return __sync_fetch_and_sub(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_ANDROID
    return __sync_fetch_and_sub(ptr, value);
#endif
}

ALWAYS_INLINE Int32 AtomicUtil::FetchAndAnd(volatile Int32 *ptr, Int32 value)
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    return __sync_fetch_and_and(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_WINDOWS
    return _InterlockedAnd((volatile LONG *)ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_IPHONE
    return __sync_fetch_and_and(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_MAC
    return __sync_fetch_and_and(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_ANDROID
    return __sync_fetch_and_and(ptr, value);
#endif
}

ALWAYS_INLINE Int64 AtomicUtil::FetchAndAnd(volatile Int64 *ptr, Int64 value)
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    return __sync_fetch_and_and(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_WINDOWS
    return ::InterlockedAnd64(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_IPHONE
    return __sync_fetch_and_and(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_MAC
    return __sync_fetch_and_and(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_ANDROID
    return __sync_fetch_and_and(ptr, value);
#endif
}

ALWAYS_INLINE Int32 AtomicUtil::FetchAndOr(volatile Int32 *ptr, Int32 value)
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    return __sync_fetch_and_or(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_WINDOWS
    return _InterlockedOr((volatile LONG *)ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_IPHONE
    return __sync_fetch_and_or(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_MAC
    return __sync_fetch_and_or(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_ANDROID
    return __sync_fetch_and_or(ptr, value);
#endif
}

ALWAYS_INLINE Int64 AtomicUtil::FetchAndOr(volatile Int64 *ptr, Int64 value)
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    return __sync_fetch_and_or(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_WINDOWS
    return ::InterlockedOr64(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_IPHONE
    return __sync_fetch_and_or(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_MAC
    return __sync_fetch_and_or(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_ANDROID
    return __sync_fetch_and_or(ptr, value);
#endif
}

ALWAYS_INLINE Int32 AtomicUtil::FetchAndXOr(volatile Int32 *ptr, Int32 value)
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    return __sync_fetch_and_xor(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_WINDOWS
    return _InterlockedXor((volatile LONG *)ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_IPHONE
    return __sync_fetch_and_xor(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_MAC
    return __sync_fetch_and_xor(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_ANDROID
    return __sync_fetch_and_xor(ptr, value);
#endif
}

ALWAYS_INLINE Int64 AtomicUtil::FetchAndXOr(volatile Int64 *ptr, Int64 value)
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    return __sync_fetch_and_xor(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_WINDOWS
    return ::InterlockedXor64(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_IPHONE
    return __sync_fetch_and_xor(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_MAC
    return __sync_fetch_and_xor(ptr, value);
#elif CRYSTAL_TARGET_PLATFORM_ANDROID
    return __sync_fetch_and_xor(ptr, value);
#endif
}

ALWAYS_INLINE Int32 AtomicUtil::CompareAndSwap(volatile Int32 *ptr, Int32 exchange, Int32 comparand)
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    return __sync_val_compare_and_swap((sint32 *)ptr, comparand, exchange);
#elif CRYSTAL_TARGET_PLATFORM_WINDOWS
    return ::InterlockedCompareExchange((volatile LONG *)ptr, exchange, comparand);
#elif CRYSTAL_TARGET_PLATFORM_IPHONE
    return __sync_val_compare_and_swap((sint32 *)ptr, comparand, exchange);
#elif CRYSTAL_TARGET_PLATFORM_MAC
    return __sync_val_compare_and_swap((sint32 *)ptr, comparand, exchange);
#elif CRYSTAL_TARGET_PLATFORM_ANDROID
    return __sync_val_compare_and_swap((sint32 *)ptr, comparand, exchange);
#endif
}

ALWAYS_INLINE Int64 AtomicUtil::CompareAndSwap(volatile Int64 *ptr, Int64 exchange, Int64 comparand)
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    return __sync_val_compare_and_swap(ptr, comparand, exchange);
#elif CRYSTAL_TARGET_PLATFORM_WINDOWS
    return ::InterlockedCompareExchange64(ptr, exchange, comparand);
#elif CRYSTAL_TARGET_PLATFORM_IPHONE
    return __sync_val_compare_and_swap(ptr, comparand, exchange);
#elif CRYSTAL_TARGET_PLATFORM_MAC
    return __sync_val_compare_and_swap(ptr, comparand, exchange);
#elif CRYSTAL_TARGET_PLATFORM_ANDROID
    return __sync_val_compare_and_swap(ptr, comparand, exchange);
#endif
}

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
ALWAYS_INLINE bool AtomicUtil::CompareAndSwap(volatile Int128 *memoryAddress, Int128 *expectValue, const Int128 &newValue)
{
    return ::_InterlockedCompareExchange128(reinterpret_cast<volatile Int64 *>(memoryAddress), newValue.high, newValue.low, reinterpret_cast<Int64 *>(expectValue));
}
#else
ALWAYS_INLINE bool CompareAndSwap(Int128 *destMemoryAddress, Int128 *expectValue, const Int128 &newValue)
{
    Int128 oldValue = *expectValue;
    *expectValue = ::__sync_val_compare_and_swap(destMemoryAddress, *expectValue, newValue);
    return oldValue == *expectValue;
}

#endif

KERNEL_END

#endif
