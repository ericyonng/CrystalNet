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
 * Date: 2023-08-12 21:57:38
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <Comps/User/impl/ClientUser.h>
#include <Comps/User/interface/IClientUserMgr.h>
#include <Comps/SysLogic/SysLogic.h>
#include <Comps/User/interface/IClientSys.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IClientUser);
POOL_CREATE_OBJ_DEFAULT_IMPL(ClientUser);

ClientUser::ClientUser(IClientUserMgr *userMgr)
:IClientUser(KERNEL_NS::RttiUtil::GetTypeId<ClientUser>())
,_userMgr(userMgr)
,_clientInfo(CRYSTAL_NEW(ClientUserInfo))
,_activedSessionId(0)
,_serverTime(KERNEL_NS::LibTime::NowMilliTimestamp())
,_lastCpuBegin(static_cast<Int64>(KERNEL_NS::CrystalRdTsc()))
,_heartbeatExpireTime(KERNEL_NS::LibTime::NowMilliTimestamp())
,_maxPacketId(0)
{
    AddFlag(LogicSysFlagsType::DISABLE_FOCUS_BY_SERVICE_FLAG);
    SetInterfaceTypeId(KERNEL_NS::RttiUtil::GetTypeId<IClientUser>());
}

ClientUser::~ClientUser()
{
    _Clear();
}

void ClientUser::Release()
{
    ClientUser::DeleteThreadLocal_ClientUser(this);
}

void ClientUser::OnRegisterComps()
{
}

Int64 ClientUser::GetNowServerTime() const
{
    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        // linux精度到nanosecond
        const auto nowCpu = KERNEL_NS::CrystalRdTsc();
        const auto cpuSlice = static_cast<Int64>((nowCpu - static_cast<UInt64>(_lastCpuBegin)) * KERNEL_NS::TimeDefs::MILLI_SECOND_PER_SECOND / KERNEL_NS::LibCpuFrequency::_countPerSecond);
        return _serverTime + cpuSlice;
    #else
        const auto nowCpu = KERNEL_NS::CrystalRdTsc();
        const auto cpuSlice = static_cast<Int64>((nowCpu - static_cast<UInt64>(_lastCpuBegin)) * KERNEL_NS::TimeDefs::MILLI_SECOND_PER_SECOND / KERNEL_NS::LibCpuFrequency::_countPerSecond);
        return _serverTime + cpuSlice;
    #endif
}

void ClientUser::SetServerTime(Int64 serverTime)
{
    _serverTime = serverTime;
    _lastCpuBegin = static_cast<Int64>(KERNEL_NS::CrystalRdTsc());
}

IClientUserMgr *ClientUser::GetUserMgr()
{
    return _userMgr;
}

const IClientUserMgr *ClientUser::GetUserMgr() const
{
    return _userMgr;
}

Int64 ClientUser::Send(KERNEL_NS::LibPacket *packet) const
{
    // TODO:需要rpc
    if(UNLIKELY(_activedSessionId == 0))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("no session cant send"));
        packet->ReleaseUsingPool();
        return -1;
    }

    if(packet->GetPacketId() == 0)
        packet->SetPacketId(NewPacketId());

    _userMgr->Send(_activedSessionId, packet);

    return packet->GetPacketId();
}

void ClientUser::Send(const std::list<KERNEL_NS::LibPacket *> &packets) const
{
    // TODO:需要rpc
    if(UNLIKELY(_activedSessionId == 0))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("no session cant send"));
        for(auto iter : packets)
            iter->ReleaseUsingPool();
        return;
    }

    _userMgr->Send(_activedSessionId, packets);
}

Int64 ClientUser::Send(Int32 opcode, const KERNEL_NS::ICoder &coder, Int64 packetId) const 
{
    // TODO:需要rpc
    if(UNLIKELY(_activedSessionId == 0))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("no session cant send"));
        return -1;
    }

    packetId = packetId > 0 ? packetId : NewPacketId();
    _userMgr->Send(_activedSessionId, opcode, coder, packetId);

    return packetId;
}

void ClientUser::OnLogin()
{
    auto &userSyss = GetCompsByType(ServiceCompType::USER_SYS_COMP);
    for(auto userSys : userSyss)
        userSys->CastTo<IClientSys>()->OnLogin();

    // TODO:
    g_Log->Info(LOGFMT_OBJ_TAG("login user:%s"), ToString().c_str());
}

void ClientUser::OnLoginFinish()
{
    auto &userSyss = GetCompsByType(ServiceCompType::USER_SYS_COMP);
    for(auto userSys : userSyss)
        userSys->CastTo<IClientSys>()->OnLoginFinish();

    SetUserStatus(ClientUserStatus::LOGINED);

    g_Log->Info(LOGFMT_OBJ_TAG("OnLoginFinish user:%s"), ToString().c_str());
}

void ClientUser::OnLogout()
{
    // 事件
    auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::USER_WILL_LOGOUT);
    ev->SetParam(Params::USER_OBJ, this);
    _userMgr->GetEventMgr()->FireEvent(ev);

    auto &userSyss = GetCompsByType(ServiceCompType::USER_SYS_COMP);
    for(auto userSys : userSyss)
        userSys->CastTo<IClientSys>()->OnLogout();

    // 会话踢下线
    if(_activedSessionId)
    {
        g_Log->Debug(LOGFMT_OBJ_TAG("user offline %s"), ToString().c_str());
        _userMgr->CloseSession(_activedSessionId, 0, true, true);
        _userMgr->RemoveUserBySessionId(_activedSessionId);
        _activedSessionId = 0;
    }

    SetUserStatus(ClientUserStatus::LOGOUTED);

    g_Log->Info(LOGFMT_OBJ_TAG("OnLogout user:%s"), ToString().c_str());
}

void ClientUser::OnUserCreated()
{
    auto &userSyss = GetCompsByType(ServiceCompType::USER_SYS_COMP);
    for(auto userSys : userSyss)
        userSys->CastTo<IClientSys>()->OnUserCreated();

    g_Log->Info(LOGFMT_OBJ_TAG("OnUserCreated user:%s"), ToString().c_str());
}

Int32 ClientUser::GetUserStatus() const
{
    return _clientInfo->clientstatus();
}

void ClientUser::SetUserStatus(Int32 status) 
{
    _clientInfo->set_clientstatus(status);
}

ClientUserInfo *ClientUser::GetClientInfo()
{
    return _clientInfo;
}

const ClientUserInfo *ClientUser::GetClientInfo() const
{
    return _clientInfo;
}

UInt64 ClientUser::GetUserId() const 
{ 
    return _clientInfo->userid();
}

void ClientUser::BindSession(UInt64 sessionId)
{
    _activedSessionId = sessionId;

    _userMgr->AddUserBySessionId(sessionId, this);
}

UInt64 ClientUser::GetSessionId() const
{
    return _activedSessionId;
}

void ClientUser::Logout()
{
    // todo 发送logout消息, 定时15秒退出
    SetUserStatus(ClientUserStatus::LOGOUTING);

    LogoutReq req;
    Send(Opcodes::OpcodeConst::OPCODE_LogoutReq, req);
}

Int32 ClientUser::Login(Int32 stackType)
{
    if(IsLogined())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("already login %s"), ToString().c_str());
        return Status::Repeat;
    }

    if(_clientInfo->clientstatus() == ClientUserStatus::LOGINING)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user logining %s"), ToString().c_str());
        return Status::Repeat;
    }

    if(_clientInfo->clientstatus() == ClientUserStatus::LOGOUTING)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user logouting %s"), ToString().c_str());
        return Status::Failed;
    }

    _clientInfo->set_clientstatus(ClientUserStatus::LOGINING);

    if(UNLIKELY(_activedSessionId))
    {
        _userMgr->CloseSession(_activedSessionId, 0, true, true);
        _userMgr->RemoveUserBySessionId(_activedSessionId);
        _activedSessionId = 0;
    }
    
    // 创建session并
    auto sysMgr = _userMgr->GetGlobalSys<ISysLogicMgr>();
    UInt64 stub = 0;
    
    KERNEL_NS::AddrIpConfig remoteIp;
    remoteIp._isHostName = false;
    remoteIp._ip = _loginInfo.targetip();
    KERNEL_NS::AddrIpConfig localIp;
    localIp._isHostName = false;
    localIp._ip = "0.0.0.0";

    auto err = sysMgr->AsynTcpConnect(remoteIp, _loginInfo.port(), stub, this, &ClientUser::_OnLoginConnectRes, localIp, 0, NULL, 3, 30,  SessionType::OUTER
    , KERNEL_NS::SocketUtil::IsIpv4(_loginInfo.targetip()) ? AF_INET : AF_INET6, stackType);

    if(err != Status::Success)
    {
        _clientInfo->set_clientstatus(ClientUserStatus::UNLOGIN);
        g_Log->Error(LOGFMT_OBJ_TAG("connect fail err:%d, user:%s"), err, ToString().c_str());
        return err;
    }

    return err;
}

void ClientUser::SetLoginInfo(const LoginInfo &loginInfo)
{
    _loginInfo = loginInfo;
}

const LoginInfo &ClientUser::GetLoginInfo() const 
{
    return _loginInfo;
}

LoginInfo &ClientUser::GetLoginInfo()
{
    return _loginInfo;
}

bool ClientUser::IsLogined() const
{
    auto status = GetUserStatus();
    return (status == ClientUserStatus::LOGINED) || 
    (status == ClientUserStatus::CLIENT_LOGIN_ENDING) ||
    (status == ClientUserStatus::CLIENT_LOGIN_ENDING_FINISH);
}

bool ClientUser::IsLogining() const
{
    return GetUserStatus() == ClientUserStatus::LOGINING;
}

bool ClientUser::IsLogout() const
{
    return (GetUserStatus() == ClientUserStatus::LOGOUTING) || (GetUserStatus() == ClientUserStatus::LOGOUTED);
}

const KERNEL_NS::BriefSockAddr *ClientUser::GetUserAddr() const
{
    auto session = _userMgr->GetGlobalSys<ISessionMgr>()->GetSession(_activedSessionId);
    if(UNLIKELY(!session))
    {
        return NULL;
    }

    return &(session->GetSessionInfo()->_remoteAddr);
}

Int64 ClientUser::GetHeartbeatExpireTime() const
{           
    return _heartbeatExpireTime;
}

void ClientUser::UpdateHeartbeatExpireTime()
{
    _heartbeatExpireTime = KERNEL_NS::LibTime::NowMilliTimestamp() + 30 * 1000;
}

Int64 ClientUser::NewPacketId() const
{
    return --_maxPacketId;
}

KERNEL_NS::LibString ClientUser::ToString() const
{
    const auto &statusName = ClientUserStatus::ENUMS_Name(_clientInfo->clientstatus());
    KERNEL_NS::LibString info;
    info.AppendFormat("account name:%s, user id:%llu, last token:%s, token expire time:%lld, client status:%d,%s, _activedSessionId:%llu, _heartbeatTime:%lld, current server time:%lld, max packet id:%lld"
    , _clientInfo->accountname().c_str(), static_cast<UInt64>(_clientInfo->userid()), _clientInfo->lasttoken().c_str()
    , static_cast<Int64>(_clientInfo->tokenexpiretime()), _clientInfo->clientstatus(), statusName.c_str(), _activedSessionId
    , _heartbeatExpireTime, GetNowServerTime(), _maxPacketId);

    return info;
}

Int32 ClientUser::_OnSysInit()
{   
    // 创建事件管理器
    auto eventMgr = KERNEL_NS::EventManager::New_EventManager();
    SetEventMgr(eventMgr);

    _RegisterEvents();
    return Status::Success;
}

Int32 ClientUser::_OnSysCompsCreated()
{
    // user需要存储的子系统
    auto &allComps = _userMgr->GetCompsByType(ServiceCompType::USER_SYS_COMP);
    for(auto comp : allComps)
    {
        auto userSys = comp->CastTo<IClientSys>();
        _sysNameRefUserSys.insert(std::make_pair(userSys->GetObjName(), userSys));
    }

    return Status::Success;
}

void ClientUser::_OnSysClose()
{
    {
        auto comps = GetCompsByType(ServiceCompType::USER_SYS_COMP);
        for(auto comp : comps)
            comp->CastTo<ILogicSys>()->SetEventMgr(NULL);
    }

    {
        auto comps = GetCompsByType(ServiceCompType::LOGIC_SYS);
        for(auto comp : comps)
            comp->CastTo<ILogicSys>()->SetEventMgr(NULL);
    }
}

IClientSys *ClientUser::_GetSysBy(const KERNEL_NS::LibString &sysName)
{
    auto iter = _sysNameRefUserSys.find(sysName);
    return iter == _sysNameRefUserSys.end() ? NULL : iter->second;
}

const IClientSys *ClientUser::_GetSysBy(const KERNEL_NS::LibString &sysName) const
{
    auto iter = _sysNameRefUserSys.find(sysName);
    return iter == _sysNameRefUserSys.end() ? NULL : iter->second;
}

void ClientUser::_Clear()
{
   CRYSTAL_RELEASE_SAFE(_clientInfo);

   auto eventMgr = GetEventMgr();
   KERNEL_NS::EventManager::Delete_EventManager(eventMgr);
   SetEventMgr(NULL);
}

void ClientUser::_RegisterEvents()
{

}

void ClientUser::_OnLoginConnectRes(UInt64 stub, Int32 errCode, const KERNEL_NS::Variant *params, bool &doRemove)
{
    if(errCode != Status::Success)
    {
        _clientInfo->set_clientstatus(ClientUserStatus::UNLOGIN);
        g_Log->Error(LOGFMT_OBJ_TAG("connect server fail errCode:%d, user:%s"), errCode, ToString().c_str());
        return;
    }

    _activedSessionId = (*params)[Params::SESSION_ID].AsUInt64();
    auto remoteAddr = (*params)[Params::REMOTE_ADDR].AsPtr<KERNEL_NS::BriefSockAddr>();

    g_Log->Info(LOGFMT_OBJ_TAG("connect to server success session id:%llu, remoteAddr:%s, user:%s")
    , _activedSessionId, remoteAddr->ToString().c_str(), ToString().c_str());

    // 发起登录
    LoginReq req;
    *req.mutable_loginuserinfo() = _loginInfo;
    Send(Opcodes::OpcodeConst::OPCODE_LoginReq, req);
    // if(newPacektId < 0)
    // {
    //     _clientInfo->set_clientstatus(ClientUserStatus::UNLOGIN);
    //     _activedSessionId = 0;
    //     g_Log->Warn(LOGFMT_OBJ_TAG("send login req fail"));
    //     return;
    // }

    _userMgr->AddUserBySessionId(_activedSessionId, this);
}


SERVICE_END
