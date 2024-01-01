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
 * Date: 2023-12-31 18:44:58
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TIME_WHEEL_TIMER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TIME_WHEEL_TIMER_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/Delegate/LibDelegate.h>
#include <kernel/comp/TimeSlice.h>
#include <kernel/comp/LibTime.h>

KERNEL_BEGIN

class TimerWheelTask;
class Variant;
class TimerWheel;
class TimeSlice;

class KERNEL_EXPORT TimeWheelTimer
{
    POOL_CREATE_OBJ_DEFAULT(TimeWheelTimer);

public:
    TimeWheelTimer(TimerWheel *timerWheel = NULL);
    ~TimeWheelTimer();

    void Cancel();
    void Schedule(Int64 milliSec);
    void Schedule(const TimeSlice &slice);
    void Schedule(Int64 startTime, Int64 milliSecPeriod);
    bool IsScheduling() const;

    // 设置超时回调
    template<typename ObjType>
    void SetTimeOutHandler(ObjType *obj, void (ObjType::*handler)(TimeWheelTimer *));
    void SetTimeOutHandler(void (*handler)(TimeWheelTimer *));
    void SetTimeOutHandler(IDelegate<void, TimeWheelTimer *> *delg);
    template<typename LambdaType>
    void SetTimeOutHandler(LambdaType &&lambdaType);

    // TimerWheel调用
    void OnTimeOut();

    LibString ToString() const;
    TimerWheel *GetTimerWheel();
    Variant &GetParams();
    void ClearParams();
    void MoveParams(Variant *params);

    const TimerWheelTask *GetTimerTask() const;

private:
    friend class TimerWheel;

    TimerWheel *_timeWheel;
    TimerWheelTask *_attachTimerTask;
    Variant *_params;
    IDelegate<void, TimeWheelTimer *> *_timeoutCallback;
};

ALWAYS_INLINE void TimeWheelTimer::Schedule(Int64 milliSec)
{
    Schedule(LibTime::NowMilliTimestamp(), milliSec);
}

ALWAYS_INLINE void TimeWheelTimer::Schedule(const TimeSlice &slice)
{
    Schedule(slice.GetTotalMilliSeconds());
}

template<typename ObjType>
ALWAYS_INLINE void TimeWheelTimer::SetTimeOutHandler(ObjType *obj, void (ObjType::*handler)(TimeWheelTimer *))
{
    auto delg = KERNEL_NS::DelegateFactory::Create(obj, handler);
    SetTimeOutHandler(delg);
}

ALWAYS_INLINE void TimeWheelTimer::SetTimeOutHandler(void (*handler)(TimeWheelTimer *))
{
    auto delg = KERNEL_NS::DelegateFactory::Create(handler);
    SetTimeOutHandler(delg);
}

template<typename LambdaType>
ALWAYS_INLINE void TimeWheelTimer::SetTimeOutHandler(LambdaType &&lambdaType)
{
    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(lambdaType, void, TimeWheelTimer *);
    SetTimeOutHandler(delg);
}

ALWAYS_INLINE void TimeWheelTimer::OnTimeOut()
{
    if(LIKELY(_timeoutCallback))
        _timeoutCallback->Invoke(this);
}

ALWAYS_INLINE TimerWheel *TimeWheelTimer::GetTimerWheel()
{
    return _timeWheel;
}

ALWAYS_INLINE Variant &TimeWheelTimer::GetParams()
{
    return *_params;
}

ALWAYS_INLINE const TimerWheelTask *TimeWheelTimer::GetTimerTask() const
{
    return _attachTimerTask;
}

KERNEL_END

#endif