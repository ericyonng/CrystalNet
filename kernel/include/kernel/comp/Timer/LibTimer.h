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

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/Delegate/Delegate.h>
#include <kernel/comp/Timer/TimeData.h>
#include <kernel/comp/Utils/TimeUtil.h>

KERNEL_BEGIN

class TimerMgr;

class KERNEL_EXPORT LibTimer
{
    POOL_CREATE_OBJ_DEFAULT(LibTimer);

public:
    // 若mgr不设置则，默认从tlsstack上拿pollerTimerMgr，前提是本线程已经初始化了TimerMgr并设置到TlsStack上，且注意时序
    LibTimer(TimerMgr *mgr = NULL);
    virtual ~LibTimer();

public:
    void Cancel();
    void Schedule(Int64 milliSec);
    void Schedule(Int64 startTime, Int64 milliSecPeriod);
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
    TimerMgr *_mgr;
    TimeData *_data;
    IDelegate<void, LibTimer *> *_timeroutHandler;
    IDelegate<void, LibTimer *> *_cancelHandler;
};

ALWAYS_INLINE void LibTimer::Schedule(Int64 milliSec)
{
    Schedule(TimeUtil::GetFastMicroTimestamp(), milliSec);
}

ALWAYS_INLINE bool LibTimer::IsScheduling() const
{
    return _data->_isScheduing;
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
}

ALWAYS_INLINE void LibTimer::SetTimeOutHandler(void (*handler)(LibTimer *))
{
    CRYSTAL_DELETE_SAFE(_timeroutHandler);
    _timeroutHandler = DelegateFactory::Create(handler);
}

ALWAYS_INLINE void LibTimer::SetTimeOutHandler(IDelegate<void, LibTimer *> *delg)
{
    CRYSTAL_DELETE_SAFE(_timeroutHandler);
    _timeroutHandler = delg;
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
