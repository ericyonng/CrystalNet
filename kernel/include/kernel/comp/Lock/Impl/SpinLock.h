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
 * Date: 2020-11-08 18:58:46
 * Author: Eric Yonng
 * Description: 只适用于保护占用时间少的共享数据 比如共享数据的赋值操作 TODO:需要多线程环境测试语义正确性
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOCK_IMPL_SPIN_LOCKER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOCK_IMPL_SPIN_LOCKER_H__

#pragma once

#include <kernel/kernel_inc.h>

KERNEL_BEGIN

#if CRYSTAL_TARGET_PLATFORM_LINUX

class KERNEL_EXPORT SpinLock
{
    SpinLock(const SpinLock&) = delete;
    SpinLock(SpinLock&&) = delete;
    SpinLock &operator =(const SpinLock&) = delete;

public:
    SpinLock();
    virtual ~SpinLock();

public:
    void Lock();
    void Unlock();
    bool TryLock();

private:
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    pthread_spinlock_t _handle;
#else
    CRITICAL_SECTION _handle;
#endif

};

ALWAYS_INLINE SpinLock::~SpinLock()
{
    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        pthread_spin_destroy(&_handle);
    #else
        ::DeleteCriticalSection(&_handle);
    #endif
}

ALWAYS_INLINE void SpinLock::Lock()
{
    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        pthread_spin_lock(&_handle);
    #else
        ::EnterCriticalSection(&_handle);
    #endif
}

ALWAYS_INLINE void SpinLock::Unlock()
{
    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        pthread_spin_unlock(&_handle);
    #else
        ::LeaveCriticalSection(&_handle);
    #endif
}

ALWAYS_INLINE bool SpinLock::TryLock()
{
    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        return pthread_spin_trylock(&_handle) == 0;
    #else
        return ::TryEnterCriticalSection(&_handle);
    #endif
}

#else

class KERNEL_EXPORT SpinLock
{
    SpinLock(const SpinLock&) = delete;
    SpinLock(SpinLock&&) = delete;
    SpinLock &operator =(const SpinLock&) = delete;

public:
    SpinLock();
    virtual ~SpinLock(){}

public:
    void Lock(Int64 loop_cnt_to_yield = SPINNING_COUNT);
    void Unlock();
    bool TryLock();

private:
    std::atomic_bool _flag;
};

ALWAYS_INLINE SpinLock::SpinLock()
{
    _flag = ATOMIC_VAR_INIT(false);
}

ALWAYS_INLINE void SpinLock::Lock(Int64 loop_cnt_to_yield)
{
    // 自旋休眠次数
    const Int64 loop_count_to_yield = loop_cnt_to_yield;

    // test and wait 直到flag被clear
    bool exp = false;
    while(!_flag.compare_exchange_weak(exp, true))
    {
        exp = false;

        // _mm_pause();
        if (LIKELY(--loop_cnt_to_yield))
            continue;

        // 自旋次数达到则切换线程
        #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        #if CRYSTAL_TARGET_PLATFORM_LINUX || CRYSTAL_TARGET_PLATFORM_ANDROID || CRYSTAL_TARGET_PLATFORM_MAC
            asm volatile ("rep;nop" : : : "memory");
        #else
            asm volatile ("nop");
        #endif
        #else // WINDOWS platform
            YieldProcessor();
        #endif // Non-WINDOWS platform

        #ifdef _DEBUG
            printf("\nperhaps in dead loop!\n");
        #endif
        loop_cnt_to_yield = loop_count_to_yield;
    }
}

ALWAYS_INLINE void SpinLock::Unlock()
{
    _flag.store(false);
}

ALWAYS_INLINE bool SpinLock::TryLock()
{
    bool exp = false;
    if(!_flag.compare_exchange_weak(exp, true))
        return false;

    return true;
}

#endif


KERNEL_END

#endif
