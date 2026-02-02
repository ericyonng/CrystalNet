// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2026-01-17 23:01:31
// Author: Eric Yonng
// Description:
#include <pch.h>
#include <Comps/MiddleNode/Impl/MiddleNodeMgr.h>
#include <Comps/MiddleNode/Impl/MiddleNodeMgrFactory.h>
#include <protocols/protocols.h>
#include <Comps/User/User.h>

#include "Comps/config/impl/ConfigLoader.h"
#include "cpp/RoleAuthConfig.h"

SERVICE_BEGIN


void ReqInfo::Release()
{
    ReqInfo::DeleteThreadLocal_ReqInfo(this);
}


bool ReqInfoCompare::operator()(const ReqInfo *lhs, const ReqInfo *rhs) const
{
    if(lhs == rhs)
        return false;

    if(lhs->_msTime == rhs->_msTime)
    {
        return lhs->_req->packetid() < rhs->_req->packetid();
    }

    return lhs->_msTime < rhs->_msTime;
}

MiddleNodeMgr::MiddleNodeMgr()
    :IMiddleNodeMgr(KERNEL_NS::RttiUtil::GetTypeId<MiddleNodeMgr>())
,_waitConfirmId(0)
,_reSendTimer(NULL)
,_reSendInterval(KERNEL_NS::TimeSlice::FromSeconds(10))
{
    
}

MiddleNodeMgr::~MiddleNodeMgr()
{
    _Clear();
}

void MiddleNodeMgr::Release()
{
    MiddleNodeMgr::DeleteByAdapter_MiddleNodeMgr(MiddleNodeMgrFactory::_buildType.V, this);
}

Int32 MiddleNodeMgr::_OnGlobalSysInit()
{
    // 注册消息请求
    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_SendDataRequest, this, &MiddleNodeMgr::_OnSendDataRequest);
    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_BroadcastSendDataConfirmResponse, this, &MiddleNodeMgr::_OnBroadcastSendDataConfirmResponse);

    _reSendTimer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    _reSendTimer->SetTimeOutHandler(this, &MiddleNodeMgr::_OnResendTimer);
    
    return Status::Success;
}

void MiddleNodeMgr::_OnGlobalSysClose()
{
    
}

void MiddleNodeMgr::_OnSendDataRequest(KERNEL_NS::LibPacket *&packet)
{
    auto req = packet->GetCoder<SendDataRequest>();

    // 1.判断packetId是不是已收到, 收到的话丢弃, 并返回response
    // 2.记录已收到(布隆过滤器), 并转发给所有有广播权限的User
    // 3. 待User
    // 收到的包转发给有需要广播的用户
    // 1.直接广播给每个User,
    auto userMgr = GetGlobalSys<IUserMgr>();
    auto packetId = req->packetid();

    do
    {
        if(KERNEL_NS::SimpleBitmapUtil::IsSet(_packetIdBitmap, req->packetid()))
        {
            if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
                g_Log->Debug(LOGFMT_OBJ_TAG("packetid:%lld, has recieved"), req->packetid());
            break;
        }

        // 先放队列中
        auto reqData = ReqInfo::NewThreadLocal_ReqInfo();
        reqData->_req = req;
        reqData->_msTime = KERNEL_NS::LibTime::NowMilliTimestamp();
        _dataSortedByTime.insert(reqData);
        packet->PopCoder();
        KERNEL_NS::SimpleBitmapUtil::Set(_packetIdBitmap, req->packetid());

        if(_waitConfirmId == 0)
        {
            _waitConfirmId = req->packetid();
            _requestWaitConfirm.clear_reqlist();
            _requestWaitConfirm.set_packetid(_waitConfirmId);
            for(auto d : _dataSortedByTime)
            {
                _requestWaitConfirm.mutable_reqlist()->Add(std::move(*(d->_req)));
                d->Release();
            }
            _dataSortedByTime.clear();

            auto &allUsers = userMgr->GetAllUsers();
            for(auto iter : allUsers)
            {
                auto user = iter.second;
                auto config = GetService()->GetComp<ConfigLoader>()->GetComp<RoleAuthConfigMgr>()->GetConfigById(user->GetUserBaseInfo()->accountname());
                if(!config)
                    continue;

                if(config->_accountType != AccountType::BroadcastRole)
                {
                    continue;
                }
                
                UNUSED(user->Send(Opcodes::OpcodeConst::OPCODE_BroadcastSendDataNty, _requestWaitConfirm));
            }

            // 10秒没确认, 则重传
            _reSendTimer->Schedule(_reSendInterval);

            if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
                g_Log->Debug(LOGFMT_OBJ_TAG("BroadcastSendDataNty req id:%lld, list size:%d"), _waitConfirmId, _requestWaitConfirm.reqlist().size());
        }

        // 如果没有待收的则转发全部
    }
    while (false);

    // 返回包
    if(auto user = userMgr->GetUserBySessionId(packet->GetSessionId()))
    {
        SendDataResponse res;
        res.set_packetid(packetId);
        user->Send(Opcodes::OpcodeConst::OPCODE_SendDataResponse, res, packet->GetPacketId());
    }
}

void MiddleNodeMgr::_OnBroadcastSendDataConfirmResponse(KERNEL_NS::LibPacket *&packet)
{
    auto res = packet->GetCoder<BroadcastSendDataConfirmResponse>();
    if(res->packetid() != _waitConfirmId)
    {
        if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
            g_Log->Debug(LOGFMT_OBJ_TAG("confirm response not waiting packet id:%lld, res packet id:%lld"), _waitConfirmId, res->packetid());
        return;
    }
    
    _requestWaitConfirm.clear_reqlist();
    _requestWaitConfirm.set_packetid(0);

    if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
        g_Log->Debug(LOGFMT_OBJ_TAG("confirm response, packetId:%lld"), _waitConfirmId);
    _waitConfirmId = 0;
                             
    // 继续发包
    for(auto data :_dataSortedByTime)
    {
        if(_waitConfirmId == 0)
        {
            _waitConfirmId = data->_req->packetid();
            _requestWaitConfirm.set_packetid(_waitConfirmId);
        }

        _requestWaitConfirm.mutable_reqlist()->Add(std::move(*(data->_req)));
        data->Release();
    }
    _dataSortedByTime.clear();

    // 发送
    if (_waitConfirmId)
    {
        auto userMgr = GetGlobalSys<IUserMgr>();
        auto &allUsers = userMgr->GetAllUsers();
        for(auto iter : allUsers)
        {
            auto user = iter.second;
            auto config = GetService()->GetComp<ConfigLoader>()->GetComp<RoleAuthConfigMgr>()->GetConfigById(user->GetUserBaseInfo()->accountname());
            if(!config)
                continue;

            if(config->_accountType != AccountType::BroadcastRole)
            {
                continue;
            }
                
            UNUSED(user->Send(Opcodes::OpcodeConst::OPCODE_BroadcastSendDataNty, _requestWaitConfirm));
        }

        _reSendTimer->Schedule(_reSendInterval);

        if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
            g_Log->Debug(LOGFMT_OBJ_TAG("_OnBroadcastSendDataConfirmResponse and BroadcastSendDataNty packet id:%lld"), _waitConfirmId);
    }

}

void MiddleNodeMgr::_OnResendTimer(KERNEL_NS::LibTimer *timer)
{
    // 已确认
    if(_waitConfirmId == 0)
    {
        timer->Cancel();
        return;
    }

    // 超时重传
    auto userMgr = GetGlobalSys<IUserMgr>();
    auto &allUsers = userMgr->GetAllUsers();
    for(auto iter : allUsers)
    {
        auto user = iter.second;
        auto config = GetService()->GetComp<ConfigLoader>()->GetComp<RoleAuthConfigMgr>()->GetConfigById(user->GetUserBaseInfo()->accountname());
        if(!config)
            continue;

        if(config->_accountType != AccountType::BroadcastRole)
        {
            continue;
        }
                
        UNUSED(user->Send(Opcodes::OpcodeConst::OPCODE_BroadcastSendDataNty, _requestWaitConfirm));
    }

    if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
        g_Log->Debug(LOGFMT_OBJ_TAG("_OnResendTimer and BroadcastSendDataNty req id:%lld"), _waitConfirmId);
}

void MiddleNodeMgr::_Clear()
{
    KERNEL_NS::ContainerUtil::DelSetContainer<decltype(_dataSortedByTime), KERNEL_NS::AutoDelMethods::Release>(_dataSortedByTime);

    if(_reSendTimer)
    {
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(_reSendTimer);
    }
    _reSendTimer = NULL;
}


SERVICE_END