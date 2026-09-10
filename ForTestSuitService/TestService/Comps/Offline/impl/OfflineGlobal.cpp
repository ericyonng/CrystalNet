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
 * Date: 2023-10-21 19:22:11
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/kernel.h>
#include <service_common/ServiceCommon.h>
#include <service/common/common.h>
#include <service/TestService/Common/ServiceCommon.h>

#include <Comps/Offline/impl/OfflineGlobal.h>
#include <Comps/Offline/impl/OfflineGlobalFactory.h>
#include <Comps/Offline/impl/OfflineGlobalStorageFactory.h>
#include <protocols/protocols.h>
#include <Comps/User/User.h>

SERVICE_BEGIN


OfflineGlobal::OfflineGlobal()
:IOfflineGlobal(KERNEL_NS::RttiUtil::GetTypeId<OfflineGlobal>())
,_afteUserLoadedStub(INVALID_LISTENER_STUB)
{

}

OfflineGlobal::~OfflineGlobal()
{
    _Clear();
}

void OfflineGlobal::Release()
{
    OfflineGlobal::DeleteByAdapter_OfflineGlobal(OfflineGlobalFactory::_buildType.V, this);
}

void OfflineGlobal::OnRegisterComps()
{
    RegisterComp<OfflineGlobalStorageFactory>();
}

Int32 OfflineGlobal::OnLoaded(UInt64 key, const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db)
{
    auto iter = _idRefOfflineData.find(key);
    if(iter != _idRefOfflineData.end())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("repeated key:%llu"), key);
        return Status::Failed;
    }

    auto offlineData = CRYSTAL_NEW(OfflineData);
    if(UNLIKELY(!offlineData->Decode(db)))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("parse offline data fail key:%llu"), key);
        return Status::ParseFail;
    }

    _MakeDict(key, offlineData);
    return Status::Success;
}

Int32 OfflineGlobal::OnSave(UInt64 key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const 
{
    auto iter = _idRefOfflineData.find(key);
    if(iter == _idRefOfflineData.end())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("serialize offline data fail key:%llu not found"), key);
        return Status::SerializeFail;
    }

    if(UNLIKELY(!iter->second->Encode(db)))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("serialize fail key:%llu"), key);
        return Status::SerializeFail;
    } 

    return Status::Success;
}

bool OfflineGlobal::AddOfflineData(Int32 offlineType, UInt64 userId, const KERNEL_NS::LibString &offlineData)
{
    if(UNLIKELY(!OfflineType::ENUMS_IsValid(offlineType)))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("invaid offline type:%d, user id:%llu"), offlineType, userId);
        return false;
    }

    if(UNLIKELY(userId == 0))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("invaid user id:%llu"), userId);
        return false;
    }
    auto newOfflineData = new OfflineData;
    newOfflineData->set_offlinetype(offlineType);
    newOfflineData->set_userid(userId);
    newOfflineData->set_offlinedata(offlineData.data(), offlineData.size());
    newOfflineData->set_createtime(KERNEL_NS::LibTime::NowMilliTimestamp());

    const auto id = GetGlobalSys<IGlobalUidMgr>()->NewGuid();

    _MakeDict(id, newOfflineData);

    MaskNumberKeyAddDirty(id);

    if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
    {
        g_Log->Debug(LOGFMT_OBJ_TAG("add a new offline id:%llu, offline type:%d, user id:%llu, data size:%llu")
        , id, offlineType, userId, static_cast<UInt64>(offlineData.size()));
    }

    return true;
}

Int32 OfflineGlobal::_OnGlobalSysInit()
{
    _RegisterEvents();
    return Status::Success;
}

void OfflineGlobal::_OnGlobalSysClose()
{
    _UnRegisterEvents();
    _Clear();
}

void OfflineGlobal::_Clear()
{
    KERNEL_NS::ContainerUtil::DelContainer2(_idRefOfflineData);
    _userIdRefIdRefOfflineData.clear();
}

void OfflineGlobal::_RegisterEvents()
{
    if(_afteUserLoadedStub == INVALID_LISTENER_STUB)
    {
        _afteUserLoadedStub = GetService()->GetEventMgr()->AddListener(EventEnums::AFTER_USER_LOADED, this, &OfflineGlobal::_OnAfterUserLoaded);
    }
}

void OfflineGlobal::_UnRegisterEvents()
{
    if(_afteUserLoadedStub != INVALID_LISTENER_STUB)
    {
        GetService()->GetEventMgr()->RemoveListenerX(_afteUserLoadedStub);
    }
}
    
void OfflineGlobal::_OnAfterUserLoaded(KERNEL_NS::LibEvent *ev)
{
    auto user = ev->GetParam(Params::USER_OBJ).AsPtr<IUser>();
    if(UNLIKELY(!user))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("have no user obj please check"));
        return;
    }

    auto iterUser = _userIdRefIdRefOfflineData.find(user->GetUserId());
    if(iterUser == _userIdRefIdRefOfflineData.end())
        return;

    auto &userOfflineDict = iterUser->second;
    for(auto iter = userOfflineDict.begin(); iter != userOfflineDict.end();)
    {
        auto offlineData = iter->second;
        const auto id = iter->first;
        user->OnOfflineHandle(*offlineData);

        _idRefOfflineData.erase(id);
        iter = userOfflineDict.erase(iter);
        offlineData->Release();

        MaskNumberKeyDeleteDirty(id);
    }

    _userIdRefIdRefOfflineData.erase(iterUser);
}

void OfflineGlobal::_MakeDict(UInt64 id, OfflineData *offlineData)
{
    _idRefOfflineData.insert(std::make_pair(id, offlineData));

    auto iter = _userIdRefIdRefOfflineData.find(offlineData->userid());
    if(iter == _userIdRefIdRefOfflineData.end())
        iter = _userIdRefIdRefOfflineData.insert(std::make_pair(offlineData->userid(), std::map<UInt64, OfflineData *>())).first;

    iter->second.insert(std::make_pair(id, offlineData));
}


SERVICE_END

