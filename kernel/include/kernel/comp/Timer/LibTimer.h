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
 * Author: Eric Yonng
 * Date: 2021-03-17 10:31:56
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TIMER_LIB_TIMER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TIMER_LIB_TIMER_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/Delegate/LibDelegate.h>
#include <kernel/comp/Timer/TimeData.h>
#include <kernel/comp/Utils/TimeUtil.h>
#include <kernel/comp/TimeSlice.h>

KERNEL_BEGIN

class TimerMgr;

// TODO:改变了timermgr
class KERNEL_EXPORT LibTimer
{
    POOL_CREATE_OBJ_DEFAULT(LibTimer);

    friend class TimerMgr;

public:
    // 若mgr不设置则，默认从tlsstack上拿pollerTimerMgr，前提是本线程已经初始化了TimerMgr并设置到TlsStack上，且注意时序
    LibTimer(TimerMgr *mgr = NULL);
    virtual ~LibTimer();

public:
    void Cancel();
    void Schedule(Int64 milliSec);
    void Schedule(const TimeSlice &slice);
    void Schedule(Int64 startTime, Int64 milliSecPeriod);
    void ScheduleMicro(Int64 startTime, Int64 microSecPeriod);
    void ScheduleNano(Int64 startTime, Int64 nanoSecPeriod);
    bool IsScheduling() const;
    void GiveupTimerData();

    // 设置超时回调
    template<typename ObjType>
    void SetTimeOutHandler(ObjType *obj, void (ObjType::*handler)(LibTimer *));
    void SetTimeOutHandler(void (*handler)(LibTimer *));
    void SetTimeOutHandler(IDelegate<void, LibTimer *> *delg);
    template<typename LambdaType>
    void SetTimeOutHandler(LambdaType &&lambdaType);
    // 设置cancel回调
    template<typename ObjType>
    void SetCancelHandler(ObjType *obj, void (ObjType::*handler)(LibTimer *));
    void SetCancelHandler(void (*handler)(LibTimer *));
    void SetCancelHandler(IDelegate<void, LibTimer *> *delg);
    // mgr调用
    void OnTimeOut();

    LibString ToString() const;
    TimerMgr *GetMgr();
    Variant &GetParams();
    void ClearParams();
    void MoveParams(Variant *params);

    const TimeData *GetData() const;

private:
    /// TODO:通过获取线程的poller获取timer mgr
    TimerMgr *_mgr;
    TimeData *_data;
    IDelegate<void, LibTimer *> *_timeroutHandler;
    IDelegate<void, LibTimer *> *_cancelHandler;
};

ALWAYS_INLINE void LibTimer::Schedule(Int64 milliSec)
{
    ScheduleNano(TimeUtil::GetFastNanoTimestamp(), milliSec * TimeDefs::NANO_SECOND_PER_MILLI_SECOND);
}

ALWAYS_INLINE void LibTimer::Schedule(const TimeSlice &slice)
{
    ScheduleNano(TimeUtil::GetFastNanoTimestamp(), slice.GetTotalNanoSeconds());
}

ALWAYS_INLINE void LibTimer::Schedule(Int64 startTime, Int64 milliSecPeriod)
{
    ScheduleNano(startTime, milliSecPeriod * TimeDefs::NANO_SECOND_PER_MILLI_SECOND);
}

ALWAYS_INLINE void LibTimer::ScheduleMicro(Int64 startTime, Int64 microSecPeriod)
{
    ScheduleNano(startTime, microSecPeriod * TimeDefs::NANO_SECOND_PER_MICRO_SECOND);
}

ALWAYS_INLINE bool LibTimer::IsScheduling() const
{
    return _data && _data->_isScheduing;
}

ALWAYS_INLINE void LibTimer::GiveupTimerData()
{
    _data = NULL;
}

template<typename ObjType>
ALWAYS_INLINE void LibTimer::SetTimeOutHandler(ObjType *obj, void (ObjType::*handler)(LibTimer *))
{
    CRYSTAL_DELETE_SAFE(_timeroutHandler);
    _timeroutHandler = DelegateFactory::Create(obj, handler);
    if(LIKELY(_data))
        _data->UpdateTimerInfo();
}

ALWAYS_INLINE void LibTimer::SetTimeOutHandler(void (*handler)(LibTimer *))
{
    CRYSTAL_DELETE_SAFE(_timeroutHandler);
    _timeroutHandler = DelegateFactory::Create(handler);
    if(LIKELY(_data))
        _data->UpdateTimerInfo();
}

ALWAYS_INLINE void LibTimer::SetTimeOutHandler(IDelegate<void, LibTimer *> *delg)
{
    CRYSTAL_DELETE_SAFE(_timeroutHandler);
    _timeroutHandler = delg;
    if(LIKELY(_data))
        _data->UpdateTimerInfo();
}

template<typename LambdaType>
ALWAYS_INLINE void LibTimer::SetTimeOutHandler(LambdaType &&lambdaType)
{
    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(lambdaType, void, LibTimer *);
    SetTimeOutHandler(delg);
}

// 设置cancel回调
template<typename ObjType>
ALWAYS_INLINE void LibTimer::SetCancelHandler(ObjType *obj, void (ObjType::*handler)(LibTimer *))
{
    CRYSTAL_DELETE_SAFE(_cancelHandler);
    _cancelHandler = DelegateFactory::Create(obj, handler);
}

ALWAYS_INLINE void LibTimer::SetCancelHandler(void (*handler)(LibTimer *))
{
    CRYSTAL_DELETE_SAFE(_cancelHandler);
    _cancelHandler = DelegateFactory::Create(handler);
}

ALWAYS_INLINE void LibTimer::SetCancelHandler(IDelegate<void, LibTimer *> *delg)
{
    CRYSTAL_DELETE_SAFE(_cancelHandler);
    _cancelHandler = delg;
}

ALWAYS_INLINE void LibTimer::OnTimeOut()
{
    if(LIKELY(_timeroutHandler))
       _timeroutHandler->Invoke(this);
}

ALWAYS_INLINE TimerMgr *LibTimer::GetMgr()
{
    return _mgr;
}

ALWAYS_INLINE Variant &LibTimer::GetParams()
{
    return *_data->_params;
}

ALWAYS_INLINE void LibTimer::MoveParams(Variant *params)
{
    _data->MoveParams(params);
}

ALWAYS_INLINE const TimeData *LibTimer::GetData() const
{
    return _data;
}

KERNEL_END

#endif
