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
 * Date: 2021-03-17 14:39:09
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Timer/LibTimer.h>
#include <kernel/comp/Timer/TimerMgr.h>
#include <kernel/comp/Variant/variant_inc.h>
#include <kernel/comp/LibTime.h>
#include <kernel/comp/Utils/TlsUtil.h>
#include <kernel/comp/Log/log.h>


KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(LibTimer);


LibTimer::LibTimer(TimerMgr *mgr)
    :_mgr(mgr)
    ,_data(NULL)
    ,_timeroutHandler(NULL)
    ,_cancelHandler(NULL)
{
    if(UNLIKELY(!_mgr))
    {// 若为空则使用线程本地存储的定时管理器
        auto defObj = TlsUtil::GetDefTls();
        if(LIKELY(defObj))
            _mgr = defObj->_pollerTimerMgr;

        if(UNLIKELY(!_mgr))
            g_Log->Error(LOGFMT_OBJ_TAG("timer mgr is null please check"));
    }

    _data = _mgr->NewTimeData(this);
}

LibTimer::~LibTimer()
{
    if(LIKELY(_data))
        _mgr->OnTimerDestroy(_data);
    _data = NULL;

    CRYSTAL_DELETE_SAFE(_cancelHandler);
    CRYSTAL_DELETE_SAFE(_timeroutHandler);
}

void LibTimer::Cancel()
{
    if(LIKELY(_data->_isScheduing))
    {
        _mgr->UnRegister(_data);

        _data->_isScheduing = false;

        if(_cancelHandler)
            _cancelHandler->Invoke(this);
    }
}

void LibTimer::Schedule(Int64 startTime, Int64 milliSecPeriod)
{
    Int64 newPeriod = milliSecPeriod * TimeDefs::NANO_SECOND_PER_MILLI_SECOND;

    // 保证过期时间不小于当前时间
    const auto curTime = TimeUtil::GetFastNanoTimestamp();
    Int64 expiredTime = (startTime + newPeriod);
    expiredTime = expiredTime < curTime ? curTime : expiredTime;

    // 在定时中先取消定时
    if(_data->_isScheduing)
        Cancel();

    // 重新加入定时
    _mgr->Register(_data, expiredTime, newPeriod);
}

LibString LibTimer::ToString() const
{
    LibString info;
    info.AppendFormat("_mgr[%p], _data[%s], _timeroutHandler[%p], _cancelHandler[%p]"
    , _mgr, _data->ToString().c_str(), _timeroutHandler, _cancelHandler);

    return info;
}

void LibTimer::ClearParams()
{
    _data->_params->BecomeNil();
}

KERNEL_END
