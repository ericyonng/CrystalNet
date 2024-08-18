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
 * Date: 2023-12-31 18:45:08
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Timer/TimeWheelTimer.h>
#include <kernel/comp/Timer/TimeWheel.h>
#include <kernel/comp/Timer/TimerWheelTask.h>
#include <kernel/comp/Variant/Variant.h>
#include <kernel/comp/Log/log.h>
#include <kernel/common/statics.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(TimeWheelTimer);

TimeWheelTimer::TimeWheelTimer(TimerWheel *timerWheel)
:_timeWheel(timerWheel)
,_attachTimerTask(TimerWheelTask::NewThreadLocal_TimerWheelTask())
,_params(Variant::New_Variant())
,_timeoutCallback(NULL)
{
    _attachTimerTask->_attachTimer = this;
    
    if(UNLIKELY(!_timeWheel))
    {// 若为空则使用线程本地存储的定时管理器
        auto defObj = TlsUtil::GetDefTls();
        if(LIKELY(defObj))
            _timeWheel = defObj->_timerWheel;

        if(UNLIKELY(!_timeWheel))
            g_Log->Error(LOGFMT_OBJ_TAG("timer wheel is null please check"));
    }
}

TimeWheelTimer::~TimeWheelTimer()
{
    if(LIKELY(_attachTimerTask))
        _timeWheel->OnTimerDestroy(_attachTimerTask);
    _attachTimerTask = NULL;

    CRYSTAL_DELETE_SAFE(_timeoutCallback);

    if(_params)
    {
        Variant::Delete_Variant(_params);
        _params = NULL;
    }
}

void TimeWheelTimer::Cancel()
{
    _timeWheel->CancelTimer(this);
}

void TimeWheelTimer::Schedule(Int64 startTime, Int64 milliSecPeriod)
{
    // 保证过期时间不小于当前时间
    const auto curTime = LibTime::NowMilliTimestamp();
    Int64 expiredTime = startTime + milliSecPeriod;
    expiredTime = expiredTime < curTime ? curTime : expiredTime;

    // 在定时中先取消定时
    if(_attachTimerTask->_isScheduling)
        Cancel();

    // 重新加入定时
    _timeWheel->ScheduleTimer(this, expiredTime, milliSecPeriod);
}

bool TimeWheelTimer::IsScheduling() const
{
    return _attachTimerTask && _attachTimerTask->_isScheduling;
}

void TimeWheelTimer::SetTimeOutHandler(IDelegate<void, TimeWheelTimer *> *delg)
{
    CRYSTAL_DELETE_SAFE(_timeoutCallback);
    _timeoutCallback = delg;
    if(LIKELY(_attachTimerTask))
        _attachTimerTask->UpdateTimerInfo();
}

LibString TimeWheelTimer::ToString() const
{
    LibString info;
    info.AppendFormat("time wheel[%p], _attachTimerTask[%s], _timeroutHandler:[%p, owner:%s, callback:%s]"
    , _timeWheel, _attachTimerTask ? _attachTimerTask->ToString().c_str() : "", _timeoutCallback, _timeoutCallback ? _timeoutCallback->GetOwnerRtti().c_str() : "", _timeoutCallback ? _timeoutCallback->GetCallbackRtti().c_str() : "");

    return info;
}

void TimeWheelTimer::ClearParams()
{
    _params->BecomeNil();
}

void TimeWheelTimer::MoveParams(Variant *params)
{
    if(UNLIKELY(!params))
        return;
        
    _params->MoveFrom(*params);
    Variant::Delete_Variant(params);
    params = NULL;
}

KERNEL_END
