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
 * Date: 2023-10-22 17:38:58
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/kernel.h>
#include <service_common/ServiceCommon.h>
#include <service/common/common.h>
#include <service/TestService/Common/ServiceCommon.h>

#include <Comps/Notify/impl/NotifyGlobal.h>
#include <Comps/Notify/impl/NotifyGlobalFactory.h>
#include <Comps/User/User.h>
#include <Comps/Offline/Offline.h>
#include <Comps/UserSys/UserSys.h>


SERVICE_BEGIN


NotifyGlobal::NotifyGlobal()
:INotifyGlobal(KERNEL_NS::RttiUtil::GetTypeId<NotifyGlobal>())
{

}

NotifyGlobal::~NotifyGlobal()
{

}

void NotifyGlobal::Release()
{
    NotifyGlobal::DeleteByAdapter_NotifyGlobal(NotifyGlobalFactory::_buildType.V, this);
}

void NotifyGlobal::SendNotify(UInt64 userId, const KERNEL_NS::LibString &titleId
, const std::vector<VariantParam> &titleParams
, const KERNEL_NS::LibString &contentId
, const std::vector<VariantParam> &contentParams)
{
    auto guidMgr = GetGlobalSys<IGlobalUidMgr>();
    const auto &nowTime = KERNEL_NS::LibTime::Now();
    UserNotifyDataItem item;
    item.set_createtime(nowTime.GetMilliTimestamp());
    item.set_notifyid(guidMgr->NewGuid());
    item.set_notifytitlewordid(titleId.GetRaw());
    
    for(auto &titleParam : titleParams)
        *item.add_titleparams() = titleParam;

    item.set_notifycontentwordid(contentId.GetRaw());

    for(auto &contentParam : contentParams)
        *item.add_contentparams() = contentParam;

    auto user = GetGlobalSys<IUserMgr>()->GetUser(userId);
    if(!user)
    {
        auto offlineGlobal = GetGlobalSys<IOfflineGlobal>();
        offlineGlobal->AddOfflineData(OfflineType_ENUMS_NOTIFY, userId, item);
        return;
    }

    // 超过一定数量就移除
    auto notifyMgr = user->GetSys<INotifyMgr>();
    notifyMgr->AddNotify(item);
}

Int32 NotifyGlobal::_OnGlobalSysInit()
{
    Subscribe(Opcodes::OpcodeConst::OPCODE_ReadNotifyReq, this, &NotifyGlobal::_OnReadNotifyReq);
    Subscribe(Opcodes::OpcodeConst::OPCODE_OnekeyClearNotifyReq, this, &NotifyGlobal::_OnOnekeyClearNotifyReq);
    return Status::Success;
}

void NotifyGlobal::_OnReadNotifyReq(KERNEL_NS::LibPacket *&packet)
{
    auto userMgr = GetGlobalSys<IUserMgr>();
    auto user = userMgr->GetLoginedUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    auto req = packet->GetCoder<ReadNotifyReq>();
    auto notifyMgr = user->GetSys<INotifyMgr>();
    auto err = notifyMgr->ReadNotify(req->notifyid());

    ReadNotifyRes res;
    res.set_errcode(err);
    user->Send(Opcodes::OpcodeConst::OPCODE_ReadNotifyRes, res, packet->GetPacketId());
}

void NotifyGlobal::_OnOnekeyClearNotifyReq(KERNEL_NS::LibPacket *&packet)
{
    auto userMgr = GetGlobalSys<IUserMgr>();
    auto user = userMgr->GetLoginedUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    auto req = packet->GetCoder<OnekeyClearNotifyReq>();
    auto notifyMgr = user->GetSys<INotifyMgr>();
    notifyMgr->OnekeyClearNotify(req->cleartype());

    OnekeyClearNotifyRes res;
    res.set_errcode(Status::Success);
    user->Send(Opcodes::OpcodeConst::OPCODE_OnekeyClearNotifyRes, res, packet->GetPacketId());
}

SERVICE_END
