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
 * Date: 2020-10-08 21:37:32
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/LibTime.h>
#include <kernel/comp/Lock/Defs/MetaLocker.h>
#include <kernel/comp/Lock/Impl/ConditionLocker.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/Utils/TimeUtil.h>

KERNEL_BEGIN

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
// nsec是时刻中的纳秒部分，注意溢出
static inline void FixAbsTime(UInt64 milliSecond, struct timespec &abstime)
{
    const auto ns = TimeUtil::GetFastNanoTimestamp() + static_cast<Int64>(milliSecond * TimeDefs::NANO_SECOND_PER_MILLI_SECOND);
    abstime.tv_sec = ns / TimeDefs::NANO_SECOND_PER_SECOND;
    abstime.tv_nsec = ns % TimeDefs::NANO_SECOND_PER_SECOND;
}

static inline void FixAbsTime(UInt64 second, UInt64 microSec, struct timespec &abstime)
{
    const auto ns = TimeUtil::GetFastNanoTimestamp() + static_cast<Int64>(second) * TimeDefs::NANO_SECOND_PER_SECOND + static_cast<Int64>(microSec) * TimeDefs::NANO_SECOND_PER_MICRO_SECOND;
    abstime.tv_sec = ns / TimeDefs::NANO_SECOND_PER_SECOND;
    abstime.tv_nsec = ns % TimeDefs::NANO_SECOND_PER_SECOND;
}
#endif


ConditionLocker::ConditionLocker()
    :_waitNum{0}
    ,_isSinal{false}

    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
    ,_ev(NULL)
    #endif
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    _ev = CreateEvent(NULL, true, false, NULL);
#else
    int ret = pthread_cond_init(&_ev, NULL);
    if(ret != 0)
    {
        printf("\nret=%d\n", ret);
        perror("cond init error!");
    }
#endif
}

ConditionLocker::~ConditionLocker()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    if(UNLIKELY(!_ev))
        return;

    if(UNLIKELY(!CloseHandle(_ev)))
    {
        CRYSTAL_TRACE("close handler fail! _ev[%p]\n", _ev);
        _ev = NULL;
        return;
    }

    _ev = NULL;
#else
    int ret = pthread_cond_destroy(&_ev);
    if(ret != 0)
    {
        printf("\nret=%d\n", ret);
        perror("cond destroy error");
    }
#endif
}

Int32 ConditionLocker::Wait()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    Int64 waitRet = WAIT_OBJECT_0;
    bool oldSinal = true;
    while(!_isSinal)
    {
        oldSinal = false;
        ++_waitNum;
        Unlock();       
        waitRet = WaitForMultipleObjects(1, &_ev, true, INFINITE);
        Lock();
        --_waitNum;

        // 不论是否被唤醒都重置事件避免消耗
        ResetEvent(_ev);

        if(waitRet == WAIT_TIMEOUT)
        {// 无论是否被唤醒（因为唤醒的时机恰好是超时）超时被唤醒
            _isSinal = false;
            return Status::WaitTimeOut;
        }

        // 出现错误则直接return
        if(!CRYSTAL_IS_EVENT_SINAL_WAKE_UP(waitRet))
        {
            _isSinal = false;
            return Status::WaitFailure;
        }
    }

    if(oldSinal)
        ResetEvent(_ev);

    _isSinal = false;
    return Status::Success;
#else
    ++_waitNum;
    if(!_isSinal.exchange(false))
    {
        auto ret = pthread_cond_wait(&_ev, &_handle);
        _isSinal = false;
        if(ret != 0)
        {
            --_waitNum;
            printf("\nret=%d\n", ret);
            perror("cConSync::WaitCon -error waitcon");
            _isSinal = false;
            return Status::WaitFailure;
        }
    }
    
    --_waitNum;
    return Status::Success;
#endif

}

Int32 ConditionLocker::TimeWait(UInt64 second, UInt64 microSec)
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    Int64 waitRet = WAIT_OBJECT_0;
    bool oldSinal = true;
    DWORD waitTimeMs = static_cast<DWORD>(second*TimeDefs::MILLI_SECOND_PER_SECOND + microSec / TimeDefs::MICRO_SECOND_PER_MILLI_SECOND);
    while(!_isSinal)
    {
        oldSinal = false;
        ++_waitNum;
        Unlock();
        waitRet = WaitForMultipleObjects(1, &_ev, true, waitTimeMs);
        Lock();
        --_waitNum;

        // 不论是否被唤醒都重置事件避免消耗
        ResetEvent(_ev);

        if(waitRet == WAIT_TIMEOUT)
        {// 无论是否被唤醒（因为唤醒的时机恰好是超时）超时被唤醒
            _isSinal = false;
            return Status::WaitTimeOut;
        }

        // 出现错误则直接return
        if(!CRYSTAL_IS_EVENT_SINAL_WAKE_UP(waitRet))
        {
            _isSinal = false;
            return Status::WaitFailure;
        }
    }

    if(oldSinal)
        ResetEvent(_ev);

    _isSinal = false;
    return Status::Success;
#else

    // 微妙转换纳秒
    struct timespec abstime; // nsec是时刻中的纳秒部分，注意溢出
    FixAbsTime(second, microSec, abstime);

    ++_waitNum;
    if(!_isSinal.exchange(false))
    {
        // CRYSTAL_TRACE("time wait second[%llu] microSec[%llu] %lld seconds %lld nano sec"
        // , second, microSec, abstime.tv_sec, abstime.tv_nsec);
        int ret = pthread_cond_timedwait(&_ev, &_handle, &abstime);
        _isSinal = false;
        if(ret == ETIMEDOUT)
        {
            --_waitNum;
            return Status::WaitTimeOut;
        }

        if(ret != 0)
        {
            --_waitNum;
            printf("\nret=%d\n", ret);
            perror("pthread cond timewait error");
            return Status::WaitFailure;
        }
    }

    --_waitNum;
    return Status::Success;
#endif
}

Int32 ConditionLocker::TimeWait(UInt64 milliSecond)
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    Int64 waitRet = WAIT_OBJECT_0;
    bool oldSinal = true;
    while(!_isSinal)
    {
        oldSinal = false;
        ++_waitNum;
        Unlock();
        waitRet = WaitForMultipleObjects(1, &_ev, true, static_cast<DWORD>(milliSecond));
        Lock();
        --_waitNum;

        // 不论是否被唤醒都重置事件避免消耗
        ResetEvent(_ev);

        if(waitRet == WAIT_TIMEOUT)
        {// 无论是否被唤醒（因为唤醒的时机恰好是超时）超时被唤醒
            _isSinal = false;
            return Status::WaitTimeOut;
        }

        // 出现错误则直接return
        if(!CRYSTAL_IS_EVENT_SINAL_WAKE_UP(waitRet))
        {
            _isSinal = false;
            return Status::WaitFailure;
        }
    }

    if(oldSinal)
        ResetEvent(_ev);

    _isSinal = false;
    return Status::Success;
#else
    // nsec是时刻中的纳秒部分，注意溢出
    struct timespec abstime; 
    FixAbsTime(milliSecond, abstime);

    ++_waitNum;
    if(!_isSinal.exchange(false))
    {
        // CRYSTAL_TRACE("time wait milliSecond[%llu] %lld seconds %lld nano sec", milliSecond, abstime.tv_sec, abstime.tv_nsec);
        int ret = pthread_cond_timedwait(&_ev, &_handle, &abstime);
        _isSinal = false;
        if(ret == ETIMEDOUT)
        {
            --_waitNum;
            return Status::WaitTimeOut;
        }

        if(ret != 0)
        {
            --_waitNum;
            printf("\nret=%d\n", ret);
            perror("pthread cond timewait error");
            return Status::WaitFailure;
        }
    }

    --_waitNum;
    return Status::Success;
#endif
}


bool ConditionLocker::Sinal()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    if(!_isSinal.exchange(true))
    {// 至少有一个waiter或者没有waiter

        // 保证不丢失信号
        Lock();
        _isSinal = true;
        if(_waitNum > 0)
            SetEvent(_ev);
        Unlock();
    }

    return _isSinal.load();
#else
    if(!_isSinal.exchange(true))
    {// 保证只有一个Sinal调用者进入 , sinal 为false说明,wait那边把sinal置为false,或者没有waiter

        // 有waiter情况下 _waitNum必定大于0
        Lock();
        if(LIKELY(_waitNum > 0))
        {
            int ret = pthread_cond_signal(&_ev);
            if(ret != 0)
            {
                printf("\nret=%d\n", ret);
                perror("signal fail\n");
                Unlock();
                return false;
            }
        }
        Unlock();
    }

    return _isSinal.load();
#endif
}

void ConditionLocker::Broadcast()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    bool isSinal = false;
    while(_waitNum > 0)
        Sinal();

//     if(LIKELY(isSinal))
//     {
//         _isSinal = true;
//         ResetEvent(_event.load());
//     }
#else
    Lock();
    _isSinal = true;
    Unlock();

    int ret = pthread_cond_broadcast(&_ev);
    if(ret != 0)
    {
        printf("\nret=%d\n", ret);
        perror("cond broadcast error");
        return;
    }

#endif
}

void ConditionLocker::ResetSinal()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    ResetEvent(_ev);
#endif
}

KERNEL_END
