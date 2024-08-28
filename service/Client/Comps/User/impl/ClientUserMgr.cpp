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
 * Date: 2023-08-12 18:17:38
 * Author: Eric Yonng
 * Description: 
*/
#include <pch.h>
#include <Comps/User/impl/ClientUserMgr.h>
#include <Comps/User/impl/ClientUserMgrFactory.h>
#include <Comps/User/impl/ClientUser.h>
#include <MyTestService.h>

SERVICE_BEGIN

bool ClientUserHeartbeatComp::operator()(const IClientUser *l, const IClientUser *r) const
{
    if(!l || !r)
        return l < r;

    if(l == r)
        return false;

    if(l->GetHeartbeatExpireTime() == r->GetHeartbeatExpireTime())
        return l < r;

    return l->GetHeartbeatExpireTime() < r->GetHeartbeatExpireTime();
}

POOL_CREATE_OBJ_DEFAULT_IMPL(IClientUserMgr);
POOL_CREATE_OBJ_DEFAULT_IMPL(ClientUserMgr);

ClientUserMgr::ClientUserMgr()
:IClientUserMgr(KERNEL_NS::RttiUtil::GetTypeId<ClientUserMgr>())
,_heartbeatRemoveUserTimer(NULL)
,_targetAddrConfig(AddrConfig::NewThreadLocal_AddrConfig())
{

}

ClientUserMgr::~ClientUserMgr()
{
    _Clear();
}

void ClientUserMgr::Release()
{
    ClientUserMgr::DeleteByAdapter_ClientUserMgr(ClientUserMgrFactory::_buildType.V, this);
}

void ClientUserMgr::OnRegisterComps()
{

}

void ClientUserMgr::OnWillStartup()
{

}

void ClientUserMgr::OnStartup()
{
    KERNEL_NS::LibString accountName;
    auto &random = KERNEL_NS::LibInt64Random<KERNEL_NS::_Build::TL>::GetInstance();

    if(_testLoginAccountName.empty())
        accountName.AppendFormat("bot_user_%lld", random.Gen(0, 127));
    else
        accountName = _testLoginAccountName;

    KERNEL_NS::LibString ip = _targetAddrConfig->_remoteIp._ip;
    if(_targetAddrConfig->_remoteIp._isHostName)
    {
        auto err = KERNEL_NS::IPUtil::GetIpByHostName(_targetAddrConfig->_remoteIp._ip, ip, {}, 0, false, true, _targetAddrConfig->_remoteIp._toIpv4);
        if(err != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("GetIpByHostName fail remote ip%s"), _targetAddrConfig->_remoteIp.ToString());
            return;
        }
    }

    LoginInfo loginInfo;
    loginInfo.set_loginmode(LoginMode::REGISTER);
    loginInfo.set_accountname(accountName.GetRaw());
    loginInfo.set_targetip(ip.GetRaw());
    loginInfo.set_port(_targetAddrConfig->_remotePort);

    KERNEL_NS::LibString pwd;
    _rsa.PubKeyEncrypt("12345678", pwd);
    pwd = KERNEL_NS::LibBase64::Encode(pwd);
    loginInfo.set_pwd(pwd.GetRaw());

    KERNEL_NS::LibString randText = "123456";
    KERNEL_NS::LibString cypherText;
    _rsa.PubKeyEncrypt(randText.data(), cypherText);
    cypherText = KERNEL_NS::LibBase64::Encode(cypherText);
    loginInfo.set_cyphertext(cypherText.GetRaw());
    loginInfo.set_origintext(randText.GetRaw());
    loginInfo.set_versionid(10101);
    auto registerInfo = loginInfo.mutable_userregisterinfo();
    registerInfo->set_accountname(accountName.GetRaw());
    registerInfo->set_pwd(loginInfo.pwd());
    registerInfo->set_createphoneimei("123456");
    auto err = Login(loginInfo, _targetAddrConfig->_protocolStackType);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("login fail err:%d, account name:%s")
        , err, accountName.c_str());
    }
    else
    {
        g_Log->Info(LOGFMT_OBJ_TAG("login pending account name:%s"), accountName.c_str());
    }
}

Int32 ClientUserMgr::Login(const LoginInfo &loginInfo, Int32 stackType)
{
    Int32 err = Status::Success;
    do
    {
        auto user = GetUser(loginInfo.accountname());
        if(user)
        {
            if(user->IsLogined())
                return Status::Success;

            user->SetLoginInfo(loginInfo);
            err = user->Login(stackType);
            break;
        }

        KERNEL_NS::SmartPtr<ClientUser, KERNEL_NS::AutoDelMethods::CustomDelete> \
        newUser = ClientUser::NewThreadLocal_ClientUser(this);

        newUser.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<ClientUser *>(p);
            ptr->WillClose();
            ptr->Close();
            ClientUser::DeleteThreadLocal_ClientUser(ptr);
        });

        auto clientInfo = newUser->GetClientInfo();
        clientInfo->set_accountname(loginInfo.accountname());
        clientInfo->set_phoneimei(loginInfo.loginphoneimei());
        clientInfo->set_clientstatus(ClientUserStatus::UNLOGIN);
        clientInfo->set_lasttoken(loginInfo.logintoken());

        auto err = newUser->Init();
        if(err != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("init fail err:%d, new user:%s"), err, newUser->ToString().c_str());
            return err;
        }

        err = newUser->Start();
        if(err != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("start fail err:%d, new user:%s"), err, newUser->ToString().c_str());
            return err;
        }

        _accountNameRefUser.insert(std::make_pair(clientInfo->accountname(), newUser.AsSelf()));

        newUser->SetLoginInfo(loginInfo);
        err = newUser->Login(stackType);
        if(err != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("login fail err:%d, user:%s"), err, newUser->ToString().c_str());
            _accountNameRefUser.erase(clientInfo->accountname());
            return err;
        }

        newUser.pop();
    } while (false);
    
    // 30秒内没登录成功则移除
    auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    timer->GetMgr()->TakeOverLifeTime(timer, [](KERNEL_NS::LibTimer *t){
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
    });
    timer->SetTimeOutHandler([loginInfo, this](KERNEL_NS::LibTimer *t)
    {
        auto user = GetUser(loginInfo.accountname());
        if(!user)
        {
            KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
            return;
        }

        if(user->IsLogined())
        {
            KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
            return;
        }

        _heartbeatQueue.erase(user);

        // 未登录则移除user
        g_Log->Warn(LOGFMT_OBJ_TAG("user is not logined will remove user:%s"), user->ToString().c_str());
        if(user->GetSessionId())
        {
            CloseSession(user->GetSessionId(), 0, true, true);
            RemoveUserBySessionId(user->GetSessionId());
        }

        user->WillClose();
        user->Close();

        _accountNameRefUser.erase(loginInfo.accountname());
        user->Release();
    });

    // 30秒内登录
    timer->Schedule(30 * 1000);
    
    return err;
}

IClientUser *ClientUserMgr::GetUser(const KERNEL_NS::LibString &accountName)
{
    auto iter = _accountNameRefUser.find(accountName);
    return iter == _accountNameRefUser.end() ? NULL : iter->second;
}

const IClientUser *ClientUserMgr::GetUser(const KERNEL_NS::LibString &accountName) const
{
    auto iter = _accountNameRefUser.find(accountName);
    return iter == _accountNameRefUser.end() ? NULL : iter->second;
}

IClientUser *ClientUserMgr::GetUserBySessinId(UInt64 sessionId)
{
    auto iter = _sessionIdRefUser.find(sessionId);
    return iter == _sessionIdRefUser.end() ? NULL : iter->second;
}

const IClientUser *ClientUserMgr::GetUserBySessinId(UInt64 sessionId) const
{
    auto iter = _sessionIdRefUser.find(sessionId);
    return iter == _sessionIdRefUser.end() ? NULL : iter->second;
}

void ClientUserMgr::AddUserBySessionId(UInt64 sessionId, IClientUser *user)
{
    _sessionIdRefUser.insert(std::make_pair(sessionId, user));
}

void ClientUserMgr::RemoveUserBySessionId(UInt64 sessionId)
{
    _sessionIdRefUser.erase(sessionId);
}

Int32 ClientUserMgr::_OnGlobalSysInit()
{
    _heartbeatRemoveUserTimer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    _heartbeatRemoveUserTimer->SetTimeOutHandler(this, &ClientUserMgr::_OnHeartbeatTimeOut);

    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_LoginRes, this, &ClientUserMgr::_OnLoginRes);
    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_ClientHeartbeatRes, this, &ClientUserMgr::_OnClientHeartbeatRes);
    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_LogoutNty, this, &ClientUserMgr::_OnClientLogoutNty);
    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_LoginFinishRes, this, &ClientUserMgr::_OnLoginFinishRes);
    
    auto ini = GetApp()->GetIni();
    if(!ini->ReadStr(GetService()->GetServiceName().c_str(), "UserRsaPublicKey", _rsaPublicKey) || _rsaPublicKey.empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("UserRsaPublicKey lack"));

        return Status::Failed;
    }
    _rsaPublicKey.strip();

    _rsaPublicKeyRaw = KERNEL_NS::LibBase64::Decode(_rsaPublicKey);
    if(!_rsa.ImportKey(&_rsaPublicKeyRaw, NULL, KERNEL_NS::LibRsa::PUB_PKC8_FLAG))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("ImportKey fail"));
        return Status::Failed;
    }

    {// 目标地址
        KERNEL_NS::LibString cache;
        if(!ini->ReadStr(GetService()->GetServiceName().c_str(), "TestTargetAddr", cache))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("check read TestTargetAddr config fail service name:%s"), GetService()->GetServiceName().c_str());
            return Status::ConfigError;
        }
        cache.strip();
        
        if(!_targetAddrConfig->Parse(cache))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("check parse TestTargetAddr config fail service name:%s, value:%s")
                    , GetService()->GetServiceName().c_str(), cache.c_str());
            return Status::ConfigError;
        }
    }

    ini->ReadStr(GetService()->GetServiceName().c_str(), "TestLoginAccountName", _testLoginAccountName);

    return Status::Success;
}

Int32 ClientUserMgr::_OnGlobalSysCompsCreated()
{
    return Status::Success;
}

Int32 ClientUserMgr::_OnHostStart()
{
    return Status::Success;
}

void ClientUserMgr::_OnGlobalSysClose()
{

}

void ClientUserMgr::_OnLoginRes(KERNEL_NS::LibPacket *&packet)
{
    auto sessionId = packet->GetSessionId();
    auto user = GetUserBySessinId(sessionId);
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user is remove before login res, packet:%s"), packet->ToString().c_str());
        return;
    }

    auto loginRes = packet->GetCoder<LoginRes>();
    if(loginRes->errcode() != Status::Success)
    {
        // 账号已存在直接登录
        if(loginRes->errcode() == Status::UserAllReadyExistsCantRegisterAgain)
        {
            user->SetUserStatus(ClientUserStatus::UNLOGIN);
            auto &loginInfo = user->GetLoginInfo();
            g_Log->Warn(LOGFMT_OBJ_TAG("account exists turn login directerly account:%s"), loginInfo.accountname().c_str());
            loginInfo.set_loginmode(LoginMode::PASSWORD);
            auto errCode = user->Login(_targetAddrConfig->_protocolStackType);
            if(errCode != Status::Success)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("login fail errCode:%d, account:%s"), errCode, loginInfo.accountname().c_str());
            }
        }
        else
        {
            g_Log->Error(LOGFMT_OBJ_TAG("login fail err:%d, user:%s"), loginRes->errcode(), user->ToString().c_str());
        }

        return;
    }

    if(loginRes->userid() == 0)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("login fail user id is zero, user:%s"), user->ToString().c_str());
        return;
    }

    auto clientInfo = user->GetClientInfo();
    if(clientInfo->clientstatus() != ClientUserStatus::LOGINING)
    {
        if((clientInfo->clientstatus() == ClientUserStatus::CLIENT_LOGIN_ENDING) || 
         clientInfo->clientstatus() == ClientUserStatus::CLIENT_LOGIN_ENDING_FINISH)
         {
            g_Log->Warn(LOGFMT_OBJ_TAG("repeate login res user:%s"), user->ToString().c_str());
            return;
         }

         if(!user->IsLogout())
         {
            user->Logout();
         }

         g_Log->Warn(LOGFMT_OBJ_TAG("client status err user:%s"), user->ToString().c_str());
        return;
    }


    user->SetServerTime(loginRes->servertime());
    clientInfo->set_userid(loginRes->userid());
    clientInfo->set_clientstatus(ClientUserStatus::LOGINED);

    auto &loginInfo = user->GetLoginInfo();
    if(loginInfo.loginmode() == LoginMode::REGISTER)
    {
        user->OnUserCreated();
        g_Log->Info(LOGFMT_OBJ_TAG("user regisster success user:%s"), user->ToString().c_str());
    }

    user->OnLogin();
    user->OnLoginFinish();

    _heartbeatQueue.erase(user);
    user->UpdateHeartbeatExpireTime();
    _heartbeatQueue.insert(user);

    // 事件
    auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::USER_LOGIN);
    ev->SetParam(Params::USER_OBJ, user);
    GetEventMgr()->FireEvent(ev);

    // 保持心跳
    auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    timer->GetMgr()->TakeOverLifeTime(timer, [](KERNEL_NS::LibTimer *t){
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
    });
    timer->SetTimeOutHandler([sessionId, this](KERNEL_NS::LibTimer *t)
    {
        auto user = GetUserBySessinId(sessionId);
        if(UNLIKELY(!user))
        {
            KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
            return;
        }

        if(!user->IsLogined())
        {
            KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
            return;
        }

        ClientHeartbeatReq req;
        auto packetId = NewPacketId(sessionId);
        Send(sessionId, Opcodes::OpcodeConst::OPCODE_ClientHeartbeatReq, req, packetId);
        if(UNLIKELY(packetId < 0))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("send heartbeat req fail sessionId:%llu"), sessionId);
        }
    });

    // 15秒一次
    timer->Schedule(15 * 1000);

    _RestartHeartbeatTimer();

    // 登陆结束消息
    LoginFinishReq req;
    user->Send(Opcodes::OpcodeConst::OPCODE_LoginFinishReq, req);
    user->SetUserStatus(ClientUserStatus::CLIENT_LOGIN_ENDING);

    // TODO:测试
    // auto logoutTimer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    // logoutTimer->SetTimeOutHandler([sessionId, this](KERNEL_NS::LibTimer *t){
    //     auto user = GetUserBySessinId(sessionId);
    //     if(UNLIKELY(!user))
    //     {
    //         KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
    //         return;
    //     }

    //     user->Logout();
    //     KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
    // });
    // logoutTimer->Schedule(5000);
}

void ClientUserMgr::_OnClientHeartbeatRes(KERNEL_NS::LibPacket *&packet)
{
    auto user = GetUserBySessinId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        return;
    }

    auto res = packet->GetCoder<ClientHeartbeatRes>();
    user->SetServerTime(res->servertimems());

    _heartbeatQueue.erase(user);
    user->UpdateHeartbeatExpireTime();
    _heartbeatQueue.insert(user);

    _RestartHeartbeatTimer();
}

void ClientUserMgr::_OnClientLogoutNty(KERNEL_NS::LibPacket *&packet)
{
    auto user = GetUserBySessinId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        return;
    }
 
    auto nty = packet->GetCoder<LogoutNty>();
    const auto &reasonName = LogoutReason::ENUMS_Name(nty->logoutreason());
    g_Log->Warn(LOGFMT_OBJ_TAG("client logout reason:%d,%s, user:%s"), nty->logoutreason(), reasonName.c_str(), user->ToString().c_str());

    user->OnLogout();
}

void ClientUserMgr::_OnLoginFinishRes(KERNEL_NS::LibPacket *&packet)
{
    auto user = GetUserBySessinId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    if(user->GetUserStatus() != ClientUserStatus::CLIENT_LOGIN_ENDING)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user status error, user:%s"), user->ToString().c_str());

        if(!user->IsLogout())
            user->Logout();
        return;
    }

    user->SetUserStatus(ClientUserStatus::CLIENT_LOGIN_ENDING_FINISH);
    auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::CLIENT_USER_LOGIN_FINISH);
    ev->SetParam(Params::USER_OBJ, user);
    GetEventMgr()->FireEvent(ev);
}

void ClientUserMgr::_RestartHeartbeatTimer()
{
    _heartbeatRemoveUserTimer->Cancel();
    if(_heartbeatQueue.empty())
        return;

    auto firstNode = *_heartbeatQueue.begin();
    const auto nowTime = KERNEL_NS::LibTime::NowMilliTimestamp();
    const auto diff = firstNode->GetHeartbeatExpireTime() > nowTime ? (firstNode->GetHeartbeatExpireTime() - nowTime) : 0;
    _heartbeatRemoveUserTimer->Schedule(diff);
}

void ClientUserMgr::_OnHeartbeatTimeOut(KERNEL_NS::LibTimer *t)
{
    const auto nowTime = KERNEL_NS::LibTime::NowMilliTimestamp();
    std::vector<IClientUser *> toRemove;
    for(auto iter = _heartbeatQueue.begin(); iter != _heartbeatQueue.end();)
    {
        auto user = *iter;
        if(user->GetHeartbeatExpireTime() > nowTime)
            break;

        toRemove.push_back(user);
        iter = _heartbeatQueue.erase(iter);
    }

    for(auto user : toRemove)
    {
        if(user->IsLogined())
            user->Logout();

        const auto accountName = user->GetClientInfo()->accountname();
        auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
        timer->GetMgr()->TakeOverLifeTime(timer, [](KERNEL_NS::LibTimer *t){
            KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
        });
        timer->SetTimeOutHandler([accountName, this](KERNEL_NS::LibTimer *t)
        {
            auto user = GetUser(accountName);
            if(!user)
            {
                KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
                return;
            }

            // 重新登录了
            if(user->IsLogined() || user->IsLogining())
            {
                KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
                return;
            }


            // 即将移除user
            auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::USER_WILL_REMOVE);
            ev->SetParam(Params::USER_ID, user->GetUserId());
            GetEventMgr()->FireEvent(ev);

            if(user->GetSessionId())
                _sessionIdRefUser.erase(user->GetSessionId());

            _heartbeatQueue.erase(user);
            user->WillClose();
            user->Close();

            _accountNameRefUser.erase(accountName);

            user->Release();
            KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
        });

        // 15秒后移除
        timer->Schedule(15 * 1000);
        g_Log->Info(LOGFMT_OBJ_TAG("15 seconds remove user:%s"), user->ToString().c_str());
    }
}

void ClientUserMgr::_Clear()
{
    KERNEL_NS::ContainerUtil::DelContainer2(_accountNameRefUser);
    _sessionIdRefUser.clear();
    _heartbeatQueue.clear();

    if(_heartbeatRemoveUserTimer)
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(_heartbeatRemoveUserTimer);
    _heartbeatRemoveUserTimer = NULL;

    if(_targetAddrConfig)
        AddrConfig::DeleteThreadLocal_AddrConfig(_targetAddrConfig);

    _targetAddrConfig = NULL;
}


SERVICE_END