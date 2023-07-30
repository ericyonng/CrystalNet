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
 * Date: 2022-09-18 19:53:40
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <service/common/BaseComps/GlobalSys/IGlobalSys.h>
#include <service/common/BaseComps/SessionMgrComp/SessionMgr.h>
#include <service/common/BaseComps/Storage/storage.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IGlobalSys);

IGlobalSys::IGlobalSys()
:_onServiceWillStartupStub(INVALID_LISTENER_STUB)
,_onServiceStartupStub(INVALID_LISTENER_STUB)
{
}

IGlobalSys::~IGlobalSys()
{
    _Clear();
}

Int64 IGlobalSys::Send(UInt64 sessionId, KERNEL_NS::LibPacket *packet) const
{
    const ISessionMgr *sessionMgr = GetGlobalSys<ISessionMgr>();
    auto session = sessionMgr->GetSession(sessionId);
    if(UNLIKELY(!session))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("session not found sessionId:%llu"), sessionId);
        return -1;
    }

    auto sessionInfo = session->GetSessionInfo();
    auto service = IGlobalSys::GetCurrentService();
    auto serviceProxy = service->GetServiceProxy();
    const auto packetId = packet->GetPacketId();
    serviceProxy->TcpSendMsg(sessionInfo->_pollerId, sessionInfo->_priorityLevel, sessionId, packet);

    return packetId;
}

void IGlobalSys::Send(UInt64 sessionId, const std::list<KERNEL_NS::LibPacket *> &packets) const
{
    auto sessionMgr = GetGlobalSys<ISessionMgr>();
    auto session = sessionMgr->GetSession(sessionId);
    if(UNLIKELY(!session))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("session not found sessionId:%llu"), sessionId);
        return;
    }
    
    auto sessionInfo = session->GetSessionInfo();
    auto service = IGlobalSys::GetCurrentService();
    auto serviceProxy = service->GetServiceProxy();
    serviceProxy->TcpSendMsg(sessionInfo->_pollerId, sessionInfo->_priorityLevel, sessionId, packets);
}

Int64 IGlobalSys::Send(UInt64 sessionId, Int32 opcode, const KERNEL_NS::ICoder &coder, Int64 packetId) const
{
    auto service = IGlobalSys::GetCurrentService();

    auto sessionMgr = service->GetComp<ISessionMgr>();
    auto session = sessionMgr->GetSession(sessionId);
    if(UNLIKELY(!session))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("session not found sessionId:%llu"), sessionId);
        return -1;
    }

    auto sessionInfo = session->GetSessionInfo();
    auto newCoderFactory = sessionInfo->_protocolStack->GetCoderFactory(opcode);
    if(UNLIKELY(!newCoderFactory))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("bad opcode:%d, sessionInfo:%s"), opcode, sessionInfo->ToString().c_str());
        return -1;
    }

    auto newCoder = newCoderFactory->Create(&coder);
    auto newPacket = KERNEL_NS::LibPacket::New_LibPacket();
    newPacket->SetSessionId(sessionId);
    newPacket->SetLocalAddr(sessionInfo->_localAddr);
    newPacket->SetRemoteAddr(sessionInfo->_remoteAddr);
    newPacket->SetPacketId((packetId > 0) ? packetId : sessionMgr->NewPacketId(sessionId));
    newPacket->SetOpcode(opcode);
    newPacket->SetCoder(newCoder);
    const auto newPacketId = newPacket->GetPacketId();
    Send(sessionId, newPacket);

    return newPacketId;
}


void IGlobalSys::CloseSession(UInt64 sessionId, Int64 closeMillisecondTime, bool forbidRead, bool forbidWrite) const
{
    const ISessionMgr *sessionMgr = GetGlobalSys<ISessionMgr>();
    auto session = sessionMgr->GetSession(sessionId);
    if(UNLIKELY(!session))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("session not found sessionId:%llu"), sessionId);
        return;
    }

    auto sessionInfo = session->GetSessionInfo();
    auto service = IGlobalSys::GetCurrentService();
    auto serviceProxy = service->GetServiceProxy();
    serviceProxy->TcpCloseSession(sessionInfo->_pollerId, GetService()->GetServiceId(), sessionInfo->_priorityLevel, sessionId, closeMillisecondTime, forbidRead, forbidWrite);
}

void IGlobalSys::AddWhite(const KERNEL_NS::LibString &ip, Int32 level)
{
    auto service = IGlobalSys::GetCurrentService();
    if(ip.empty() || !KERNEL_NS::SocketUtil::IsIp(ip))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("invalid ip:%s"), ip.c_str());
        return;
    }

    auto serviceProxy = service->GetServiceProxy();
    serviceProxy->AddWhite(ip, level);
}

void IGlobalSys::AddBlack(const KERNEL_NS::LibString &ip, Int32 level)
{
    auto service = IGlobalSys::GetCurrentService();
    if(ip.empty() || !KERNEL_NS::SocketUtil::IsIp(ip))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("invalid ip:%s"), ip.c_str());
        return;
    }

    auto serviceProxy = service->GetServiceProxy();
    serviceProxy->AddBlack(ip, level);
}

void IGlobalSys::EraseWhite(const KERNEL_NS::LibString &ip, Int32 level)
{
    auto service = IGlobalSys::GetCurrentService();
    if(ip.empty() || !KERNEL_NS::SocketUtil::IsIp(ip))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("invalid ip:%s"), ip.c_str());
        return;
    }

    auto serviceProxy = service->GetServiceProxy();
    serviceProxy->EraseWhite(ip, level);
}

void IGlobalSys::EraseBlack(const KERNEL_NS::LibString &ip, Int32 level)
{
    auto service = IGlobalSys::GetCurrentService();
    if(ip.empty() || !KERNEL_NS::SocketUtil::IsIp(ip))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("invalid ip:%s"), ip.c_str());
        return;
    }

    auto serviceProxy = service->GetServiceProxy();
    serviceProxy->EraseBlack(ip, level);
}

void IGlobalSys::AddWhite(const std::list<KERNEL_NS::LibString> &ips, Int32 level)
{
    auto service = IGlobalSys::GetCurrentService();
    if(ips.empty())
        return;

    auto serviceProxy = service->GetServiceProxy();
    serviceProxy->AddWhite(ips, level);
}

void IGlobalSys::AddBlack(const std::list<KERNEL_NS::LibString> &ips, Int32 level)
{
    auto service = IGlobalSys::GetCurrentService();
    if(ips.empty())
        return;

    auto serviceProxy = service->GetServiceProxy();
    serviceProxy->AddBlack(ips, level);
}

void IGlobalSys::EraseWhite(const std::list<KERNEL_NS::LibString> &ips, Int32 level)
{
    auto service = IGlobalSys::GetCurrentService();
    if(ips.empty())
        return;

    auto serviceProxy = service->GetServiceProxy();
    serviceProxy->EraseWhite(ips, level);
}

void IGlobalSys::EraseBlack(const std::list<KERNEL_NS::LibString> &ips, Int32 level)
{
    auto service = IGlobalSys::GetCurrentService();
    if (ips.empty())
        return;

    auto serviceProxy = service->GetServiceProxy();
    serviceProxy->EraseBlack(ips, level);
}

void IGlobalSys::ControlIpPipline(const std::list<KERNEL_NS::IpControlInfo *> &controlInfoList, Int32 level)
{
    auto service = IGlobalSys::GetCurrentService();
    if(controlInfoList.empty())
        return;

    auto ipRuleMgr = service->GetComp<KERNEL_NS::IpRuleMgr>();
    for(auto ctrlInfo : controlInfoList)
    {
        if(!KERNEL_NS::SocketUtil::IsIp(ctrlInfo->_ip))
            continue;

        for(auto ctrlFlag : ctrlInfo->_controlFlow)
        {
            if (ctrlFlag == KERNEL_NS::IpControlInfo::ADD_WHITE)
                ipRuleMgr->PushWhite(ctrlInfo->_ip);
            else if (ctrlFlag == KERNEL_NS::IpControlInfo::ADD_BLACK)
                ipRuleMgr->PushBlack(ctrlInfo->_ip);
            else if (ctrlFlag == KERNEL_NS::IpControlInfo::ERASE_WHITE)
                ipRuleMgr->EraseWhite(ctrlInfo->_ip);
            else if(ctrlFlag == KERNEL_NS::IpControlInfo::ERASE_BLACK)
                ipRuleMgr->EraseBlack(ctrlInfo->_ip);
        }
    }

    auto serviceProxy = service->GetServiceProxy();
    serviceProxy->ControlIpPipline(controlInfoList, level);
}


Int32 IGlobalSys::_OnSysInit()
{
    SetEventMgr(GetService()->GetEventMgr());

    _onServiceWillStartupStub = GetEventMgr()->AddListener(EventEnums::SERVICE_WILL_STARTUP, this, &IGlobalSys::_OnWillStartupEv);
    _onServiceStartupStub = GetEventMgr()->AddListener(EventEnums::SERVICE_STARTUP, this, &IGlobalSys::_OnStartupEv);

    auto st = _OnGlobalSysInit();
    if(st != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("global sys init fail %s"), ToString().c_str());
        return st;
    }

    return Status::Success;
}

Int32 IGlobalSys::_OnSysCompsCreated()
{
    // 全局系统需要启动时加载数据
    auto storageInfo = GetStorageInfo();
    if(storageInfo)
        storageInfo->AddFlags(StorageFlagType::LOAD_DATA_ON_STARTUP_FLAG);
        
    auto st = _OnGlobalSysCompsCreated();
    if(st != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("_OnGlobalSysCompsCreated fail st:%d, global sys:%s"), GetObjName().c_str());
        return st;
    }

    return Status::Success;
}

void IGlobalSys::_OnSysClose() 
{
    _Clear();

    _OnGlobalSysClose();
}

void IGlobalSys::_OnGlobalSysClose()
{

}

void IGlobalSys::_Clear()
{
    if(_onServiceWillStartupStub != INVALID_LISTENER_STUB)
        GetEventMgr()->RemoveListenerX(_onServiceWillStartupStub);

    if(_onServiceStartupStub != INVALID_LISTENER_STUB)
        GetEventMgr()->RemoveListenerX(_onServiceStartupStub);
}

void IGlobalSys::_OnWillStartupEv(KERNEL_NS::LibEvent *ev)
{
    OnWillStartup();
}

void IGlobalSys::_OnStartupEv(KERNEL_NS::LibEvent *ev)
{
    OnStartup();
}

SERVICE_END
