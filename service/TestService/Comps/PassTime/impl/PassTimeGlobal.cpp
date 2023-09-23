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
 * Date: 2023-09-17 19:55:11
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <Comps/PassTime/impl/PassTimeGlobal.h>
#include <Comps/PassTime/impl/PassTimeGlobalFactory.h>
#include <Comps/PassTime/impl/PassTimeGlobalStorageFactory.h>
#include <Comps/config/config.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IPassTimeGlobal);
POOL_CREATE_OBJ_DEFAULT_IMPL(PassTimeGlobal);

PassTimeGlobal::PassTimeGlobal()
:_key(0)
,_passTimeData(new PassTimeData)
,_timer(KERNEL_NS::LibTimer::NewThreadLocal_LibTimer())
{
    _timer->SetTimeOutHandler(this, &PassTimeGlobal::_OnZeroTimeOut);
}

PassTimeGlobal::~PassTimeGlobal()
{
    _Clear();
}

void PassTimeGlobal::Release()
{
    PassTimeGlobal::DeleteByAdapter_PassTimeGlobal(PassTimeGlobalFactory::_buildType.V, this);
}

void PassTimeGlobal::OnRegisterComps()
{
    RegisterComp<PassTimeGlobalStorageFactory>();
}

Int32 PassTimeGlobal::OnLoaded(UInt64 key, const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db)
{
    _key = key;
    KERNEL_NS::LibString data;
    const auto len = static_cast<size_t>(db.GetReadableSize());
    if(!_passTimeData->FromJsonString(db.GetReadBegin(), len))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("parse pass time data fail key:%llu"), key);
        return Status::ParseFail;
    }

    return Status::Success;
}

Int32 PassTimeGlobal::OnSave(UInt64 key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const
{
    if(_key != key)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("bad key:%llu, _key:%llu"), key, _key);
        return Status::SerializeFail;
    }

    KERNEL_NS::LibString data;
    if(!_passTimeData->ToJsonString(&(data.GetRaw())))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("parse pass time data serialize fail key:%llu"), key);
        return Status::SerializeFail;
    }

    db.Write(data.data(), static_cast<Int64>(data.length()));
    return Status::Success;
}

void PassTimeGlobal::CheckPassTime()
{
    const auto &nowTime = KERNEL_NS::LibTime::Now();
    _DoCheckPassTime(nowTime);

    // 调整时间
    const auto &sliceToNextZeroTime = nowTime.GetIntervalTo(KERNEL_NS::TimeSlice(0));
    _timer->Schedule(sliceToNextZeroTime);
}

void PassTimeGlobal::_OnZeroTimeOut(KERNEL_NS::LibTimer *t)
{
    const auto &nowTime = KERNEL_NS::LibTime::Now();
    _DoCheckPassTime(nowTime);

    // 调整时间
    const auto &sliceToNextZeroTime = nowTime.GetIntervalTo(KERNEL_NS::TimeSlice(0));
    _timer->Schedule(sliceToNextZeroTime);
}

void PassTimeGlobal::_DoCheckPassTime(const KERNEL_NS::LibTime &nowTime)
{
    if(_key == 0)
    {
        _key = GetGlobalSys<IGlobalUidMgr>()->NewGuid();
        _passTimeData->set_lastpassdaytime(nowTime.GetMilliTimestamp());
        MaskNumberKeyAddDirty(_key);
        return;
    }

    const auto &lastPassDayTime = KERNEL_NS::LibTime::FromMilliSeconds(_passTimeData->lastpassdaytime());
    auto &allLogicComps = GetService()->GetCompsByType(ServiceCompType::LOGIC_SYS);
    
    // 跨天
    if(nowTime.GetLocalDay() != lastPassDayTime.GetLocalDay())
    {
        for(auto &comp : allLogicComps)
            comp->CastTo<ILogicSys>()->OnPassDay(nowTime);

        // 跨周
        const auto firstDayOfWeekConfig = GetGlobalSys<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::FIRST_DAY_OF_WEEK);
        if(nowTime.GetLocalDayOfWeek() == firstDayOfWeekConfig->_value)
        {
            for(auto &comp : allLogicComps)
                comp->CastTo<ILogicSys>()->OnPassWeek(nowTime);
        }

        // 跨月
        if(nowTime.GetLocalMonth() != lastPassDayTime.GetLocalMonth())
        {
            for(auto &comp : allLogicComps)
                comp->CastTo<ILogicSys>()->OnPassMonth(nowTime);
        }

        // 跨年
        if(nowTime.GetLocalYear() != lastPassDayTime.GetLocalYear())
        {
            for(auto &comp : allLogicComps)
                comp->CastTo<ILogicSys>()->OnPassYear(nowTime);
        }

        // 结束
        for(auto &comp : allLogicComps)
            comp->CastTo<ILogicSys>()->OnPassTimeEnd(nowTime);

        _passTimeData->set_lastpassdaytime(nowTime.GetMilliTimestamp());
        MaskNumberKeyModifyDirty(_key);
    }
}

void PassTimeGlobal::_Clear()
{
    CRYSTAL_RELEASE_SAFE(_passTimeData);
    if(_timer)
    {
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(_timer);
        _timer = NULL;
    }
}

SERVICE_END
