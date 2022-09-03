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
 * Date: 2021-03-17 17:48:56
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Timer/TimerMgr.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/Timer/LibTimer.h>
#include <kernel/comp/Timer/TimerDefs.h>
#include <kernel/comp/Variant/variant_inc.h>
#include <kernel/comp/Cpu/LibCpuCounter.h>
#include <kernel/comp/Log/log.h>

KERNEL_BEGIN

TimerMgr::TimerMgr() 
:_launchThreadId(0)
,_driving(0)
,_curTime(0)
,_curMaxId(0)
,_wakeupCb(NULL)
{
    
}

TimerMgr::~TimerMgr()
{
    ContainerUtil::DelContainer<TimeData *, AutoDelMethods::Release>(_allTimeData);
    CRYSTAL_DELETE_SAFE(_wakeupCb);
}

void TimerMgr::Launch(IDelegate<void> *wakeupThreadCb)
{
    if(UNLIKELY(_launchThreadId))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("already Launch before _launchThreadId = [%llu], current thread id = [%llu]"), _launchThreadId, SystemUtil::GetCurrentThreadId());
        CRYSTAL_DELETE_SAFE(wakeupThreadCb);
        return;
    }

    _launchThreadId = SystemUtil::GetCurrentThreadId();
    if(_wakeupCb)
        CRYSTAL_DELETE_SAFE(_wakeupCb);
    _wakeupCb = wakeupThreadCb;
}

void TimerMgr::Drive(const LibCpuCounter &performance, UInt64 pieceTimeInMicroseconds)
{
    _BeforeDrive();

    LibCpuCounter performanceTemp;
    _curTime = TimeUtil::GetMicroTimestamp();

    for(auto iter = _expireQueue.begin(); iter != _expireQueue.end();)
    {
        // 未过期的为止
        auto timeData = *iter;
        if(timeData->_expiredTime > _curTime)
            break;

        if(timeData->_isScheduing)
        {
            timeData->_owner->OnTimeOut();

            // 如果没有添加则重新计时
            if(timeData->_isScheduing && !timeData->_asynData->IsMaskRegister())
               _AsynRegister(timeData, timeData->_period, timeData->_expiredTime + timeData->_period);
        }

        iter = _expireQueue.erase(iter);

        // 已经消耗超时
        #if CRYSTAL_TARGET_PLATFORM_LINUX
        if(UNLIKELY(performanceTemp.Update().ElapseMicroseconds(performance) >= pieceTimeInMicroseconds))
            break;
        #else
        if(UNLIKELY(performanceTemp.Update().ElapseMicroseconds(performance) >= pieceTimeInMicroseconds))
            break;
        #endif
    }

    _AfterDrive();
}

KERNEL_END