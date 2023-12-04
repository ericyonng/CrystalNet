/*!
 * MIT License
 *  
 * Copyright (c) 2020 Eric Yonng<120453674@qq.com>
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *  
 * 
 * Date: 2020-10-08 21:25:34
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOCK_IMPL_CONDITION_LOCKER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOCK_IMPL_CONDITION_LOCKER_H__

#pragma once

#include <kernel/comp/Lock/Impl/Locker.h>
#include <atomic>

KERNEL_BEGIN

class KERNEL_EXPORT ConditionLocker : public Locker
{
public:
    ConditionLocker();
    virtual ~ConditionLocker();

public:
    Int32 Wait();
    Int32 TimeWait(UInt64 second, UInt64 microSec);
    Int32 TimeWait(Int64 second, Int64 microSec);
    Int32 TimeWait(UInt64 milliSecond);

    // Int32 Wait(UInt64 second, UInt64 microSec);
    // Int32 Wait(UInt64 milliSecond = INFINITE);
    // Int32 DeadWait();

    // 可以不必加锁 sinal是给一个正在等待的线程发送信号，
    // 如果刚好有线程等待加锁只会让被唤醒的线程多产生一次线程切换
    // 如果没有线程等待，加锁再发信号不会唤醒任何线程,会使信号丢失
    // 返回当前sinal状态
    bool Sinal();   // 无需加锁 保证信号不丢失
    bool HasWaiter();
    void Broadcast();   // 无需加锁 保证信号不丢失
    void ResetSinal();
    void ResetSinalFlag();
    bool IsSinal() const;

private:
    std::atomic<Int64> _waitNum;
    std::atomic_bool _isSinal;

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    HANDLE _ev;
#else
    pthread_cond_t _ev;
#endif

};

ALWAYS_INLINE Int32 ConditionLocker::TimeWait(Int64 second, Int64 microSec)
{
    return TimeWait(static_cast<UInt64>(second), static_cast<UInt64>(microSec));
}

ALWAYS_INLINE bool ConditionLocker::HasWaiter()
{
    return _waitNum > 0;
}

ALWAYS_INLINE void ConditionLocker::ResetSinalFlag()
{
    _isSinal = false;
}

ALWAYS_INLINE bool ConditionLocker::IsSinal() const
{
    return _isSinal;
}
KERNEL_END

#endif