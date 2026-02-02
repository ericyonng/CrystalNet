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
 * Date: 2025-01-12 00:27:28
 * Author: Eric Yonng
 * Description:
*/

#include <pch.h>
#include <kernel/comp/Timing/Impl/Timing.h>
#include <kernel/comp/Timing/Impl/TimingFactory.h>

#include "kernel/comp/Timer/LibTimer.h"

KERNEL_BEGIN

Timing::Timing()
:CompObject(RttiUtil::GetTypeId<Timing>())
,_flushInterval(TimeSlice::FromMilliSeconds(1))
,_currentTime(LibTime::Now())
,_timer(NULL)
{
    
}

Timing::~Timing()
{
    if(_timer)
        LibTimer::DeleteThreadLocal_LibTimer(_timer);

    _timer = NULL;
}

void Timing::Release()
{
    Timing::DeleteByAdapter_Timing(TimingFactory::_buildType.V, this);
}

void Timing::ReStart(const TimeSlice& ts)
{
    if(UNLIKELY(!_timer))
    {
        _timer = LibTimer::NewThreadLocal_LibTimer();
        _timer->SetTimeOutHandler(this, &Timing::_OnTick);
    }

    _currentTime = LibTime::Now();

    if(UNLIKELY(!ts.IsZero()))
        _flushInterval = ts;
    
    _timer->Schedule(_flushInterval);
}

Int32 Timing::_OnStart()
{
    ReStart(_flushInterval);

    return Status::Success;
}

void Timing::_OnTick(LibTimer *t)
{
    _currentTime += _flushInterval;
}

KERNEL_END
