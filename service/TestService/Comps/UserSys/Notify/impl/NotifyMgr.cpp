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
 * Date: 2023-10-22 18:18:50
 * Author: Eric Yonng
 * Description: 
*/
#include <pch.h>
#include <service_common/ServiceCommon.h>
#include <service/common/common.h>
#include <service/TestService/Common/ServiceCommon.h>

#include <Comps/UserSys/Notify/impl/NotifyMgr.h>
#include <Comps/UserSys/Notify/impl/NotifyMgrFactory.h>
#include <Comps/UserSys/Notify/impl/NotifyMgrStorageFactory.h>
#include <protocols/protocols.h>
#include <Comps/config/config.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(INotifyMgr);
POOL_CREATE_OBJ_DEFAULT_IMPL(NotifyMgr);

OBJ_GET_OBJ_TYPEID_IMPL(NotifyMgr)

NotifyMgr::NotifyMgr()
:_notifyData(new UserNotifyData)
{

}

NotifyMgr::~NotifyMgr()
{
    _Clear();
}

void NotifyMgr::Release()
{
    NotifyMgr::DeleteByAdapter_NotifyMgr(NotifyMgrFactory::_buildType.V, this);
}

Int32 NotifyMgr::OnLoaded(const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db)
{
    if(!_notifyData->Decode(db))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("notify parse fail db size:%lld"), db.GetReadableSize()); 
        return Status::ParseFail;
    }

    const Int32 sz = _notifyData->itemlist_size();
    for(Int32 idx = 0; idx < sz; ++idx)
    {
        auto notify = _notifyData->mutable_itemlist(idx);
        _notifyIdRefNotify.insert(std::make_pair(notify->notifyid(), notify));
    }

    return Status::Success;
}

Int32 NotifyMgr::OnSave(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const
{
    if(!_notifyData->Encode(db))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("notify serialize fail %s"), _notifyData->ToJsonString().c_str()); 
        return Status::SerializeFail;
    }

    return Status::Success;
}

void NotifyMgr::OnLogin()
{
    
}

void NotifyMgr::OnLoginFinish()
{
    UserNotifyDataNty nty;
    *nty.mutable_usernotifydata() = *_notifyData;
    Send(Opcodes::OpcodeConst::OPCODE_UserNotifyDataNty, nty);
}

void NotifyMgr::OnLogout()
{

}

void NotifyMgr::AddNotify(const UserNotifyDataItem &item)
{
    auto limitConfig = GetService()->GetComp<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::NOTIFY_MAX_LIMIT);
    
    // 移除多的
    RemoveUserNotifyDataItemNty removeNty;
    while(_notifyData->itemlist_size() >= limitConfig->_value)
    {
        auto &firstItem = _notifyData->itemlist(0);
        removeNty.add_notifyids(firstItem.notifyid());

        _notifyIdRefNotify.erase(firstItem.notifyid());

        _notifyData->mutable_itemlist()->DeleteSubrange(0, 1);
    }

    if(removeNty.notifyids_size() != 0)
        Send(Opcodes::OpcodeConst::OPCODE_RemoveUserNotifyDataItemNty, removeNty);

    *_notifyData->add_itemlist() = item;
    MaskDirty();

    AddUserNotifyDataItemNty nty;
    *nty.add_itemlist() = item;
    _notifyIdRefNotify.insert(std::make_pair(item.notifyid(), _notifyData->mutable_itemlist(_notifyData->itemlist_size() - 1)));

    Send(Opcodes::OpcodeConst::OPCODE_AddUserNotifyDataItemNty, nty);
}

Int32 NotifyMgr::ReadNotify(UInt64 notifyId)
{
    auto iter = _notifyIdRefNotify.find(notifyId);
    if(iter == _notifyIdRefNotify.end())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("notify is not found notify id:%llu user:%s"), notifyId, GetUser()->ToString().c_str());
        return Status::NotFound;
    }

    if(iter->second->isread() > 0)
    {
        return Status::Success;
    }

    iter->second->set_isread(1);
    MaskDirty();

    // 更新
    UserNotifyChangeNty nty;
    *nty.add_itemlist() = *iter->second;
    Send(Opcodes::OpcodeConst::OPCODE_UserNotifyChangeNty, nty);
    
    return Status::Success;
}

void NotifyMgr::OnekeyClearNotify(Int32 clearType)
{
    if(!ClearNotifyType_ENUMS_IsValid(clearType))
        return;

    if(clearType == ClearNotifyType_ENUMS_OnlyRead)
    {
        const Int32 count = _notifyData->itemlist_size();
        for(Int32 idx = count - 1; idx >= 0; --idx)
        {
            auto &item = _notifyData->itemlist(idx);
            if(item.isread() == 0)
                continue;

            _notifyIdRefNotify.erase(item.notifyid());
            _notifyData->mutable_itemlist()->DeleteSubrange(idx, 1);
        }

        MaskDirty();
    }
    else if(clearType == ClearNotifyType_ENUMS_ClearAll)
    {
        _notifyIdRefNotify.clear();
        _notifyData->clear_itemlist();
        MaskDirty();
    }

    UserNotifyDataNty nty;
    *nty.mutable_usernotifydata() = *_notifyData;
    Send(Opcodes::OpcodeConst::OPCODE_UserNotifyDataNty, nty);
}

Int32 NotifyMgr::_OnUserSysInit()
{
    _RegisterEvents();

    // 监听离线消息
    GetUser()->RegisterOfflineHandler(OfflineType_ENUMS_NOTIFY, this, &NotifyMgr::_OnOfflineHandle);

    return Status::Success;
}

Int32 NotifyMgr::_OnHostStart()
{
    return Status::Success;
}

void NotifyMgr::_OnSysClose()
{
    _UnRegisterEvents();

    _Clear();
}

void NotifyMgr::_Clear()
{
    if(_notifyData)
    {
        _notifyData->Release();
        _notifyData = NULL;
    }
}

void NotifyMgr::_RegisterEvents()
{

}

void NotifyMgr::_UnRegisterEvents()
{

}

// 在AfterLoaded被调用
void NotifyMgr::_OnOfflineHandle(const OfflineData &offlineData)
{
    if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
    {
        g_Log->Debug(LOGFMT_OBJ_TAG("handle notify offline data:%s"), offlineData.ToJsonString().c_str());
    }

    KERNEL_NS::SmartPtr<KERNEL_NS::LibStreamTL, KERNEL_NS::AutoDelMethods::CustomDelete> stream = KERNEL_NS::LibStreamTL::NewThreadLocal_LibStream();
    stream.SetClosureDelegate([](void *p){
        KERNEL_NS::LibStreamTL::DeleteThreadLocal_LibStream(KERNEL_NS::KernelCastTo<KERNEL_NS::LibStreamTL>(p));
    });

    if(!stream->DeserializeFrom(offlineData.offlinedata()))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("DeserializeFrom string fail offline data:%s"), offlineData.ToJsonString().c_str());
        return;
    }

    UserNotifyDataItem item;
    if(!item.Decode(*stream))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("Decode from stream fail offline data:%s"), offlineData.ToJsonString().c_str());
        return;
    }

    *_notifyData->add_itemlist() = item;
}

SERVICE_END
