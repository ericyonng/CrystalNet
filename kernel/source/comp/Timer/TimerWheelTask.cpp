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
 * Date: 2023-12-31 20:05:04
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Timer/TimerWheelTask.h>
#include <kernel/comp/Timer/TimeWheelTimer.h>

KERNEL_BEGIN

TimerWheelTask::TimerWheelTask()
:_head(NULL)
,_pre(NULL)
,_next(NULL)
,_expiredTime(0)
,_period(0)
,_isScheduling(false)
,_attachTimer(NULL)
,_isInTicking(false)
{

}

void TimerWheelTask::UpdateTimerInfo()
{
    if(LIKELY(_attachTimer))
        _timerInfo = _attachTimer->ToString();
}

LibString TimerWheelTask::ToString() const
{
    LibString info;
    const auto &now = LibTime::Now();
    info.AppendFormat("_expiredTime=[%lld], _period=[%lld], _owner=[%p], _isScheduing=[%d], left time=[%lld](ms), timer info:%s"
    ,  _expiredTime, _period, _attachTimer, _isScheduling, _expiredTime - now.GetMilliTimestamp(), _timerInfo.c_str());

    return info;
}

KERNEL_END