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
 * Date: 2021-03-17 10:49:52
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Timer/TimeData.h>
#include <kernel/comp/Timer/AsynTimeData.h>
#include <kernel/comp/Timer/LibTimer.h>
#include <kernel/comp/Timer/TimerMgr.h>
#include <kernel/comp/Variant/variant_inc.h>
#include <kernel/comp/LibTime.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(TimeData);

TimeData::TimeData(Int64 id, LibTimer *timer)
    :_id(id)
    ,_expiredTime(0)
    ,_period(0)
    , _owner(timer)
    ,_isScheduing(false)
    ,_asynData(AsynTimeData::NewThreadLocal_AsynTimeData(this))
    ,_params(Variant::New_Variant())
    , _next(NULL)

{

}

TimeData::~TimeData()
{
    if(_owner)
        _owner->GiveupTimerData();

    _owner = NULL;
    _isScheduing = false;
    AsynTimeData::DeleteThreadLocal_AsynTimeData(_asynData);
    _asynData = NULL;
    Variant::Delete_Variant(_params);
    _params = NULL;
    _next = NULL;
}

void TimeData::MoveParams(Variant *params)
{
    if(UNLIKELY(!params))
        return;
        
    _params->MoveFrom(*params);
    Variant::Delete_Variant(params);
    params = NULL;
}

LibString TimeData::ToString() const
{
    LibString info;
    const auto &now = LibTime::Now();
    info.AppendFormat("_id=[%lld], _expiredTime=[%lld], _period=[%lld], _owner=[%p], _isScheduing=[%d], left time=[%lld](ms)"
    , _id,  _expiredTime, _period, _owner, _isScheduing, _expiredTime - now.GetMicroTimestamp());

    return info;
}

KERNEL_END

