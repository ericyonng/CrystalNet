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
 * Date: 2023-08-08 13:14:36
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <Comps/User/impl/UserSessionMgr.h>
#include <Comps/User/impl/UserSessionMgrFactory.h>
#include <Comps/User/impl/UserMgr.h>
#include <Comps/User/interface/IUser.h>
#include <Comps/config/config.h>
#include <Comps/User/impl/UserSessionDefs.h>
#include <MyTestService.h>
#include <protocols/protocols.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(UserSessionMgr);

UserSessionMgr::UserSessionMgr()
:_eventMgr(NULL)
,_userMgr(NULL)
,_heartbeatTimer(NULL)
,_heartbeatExpireTime(30)
,_sessionCreatedStub(INVALID_LISTENER_STUB)
,_sessionMsgRecvStub(INVALID_LISTENER_STUB)
,_userWillRemoveStub(INVALID_LISTENER_STUB)
,_sessionWillRemoveStub(INVALID_LISTENER_STUB)
{

}

UserSessionMgr::~UserSessionMgr()
{
    _Clear();
}

void UserSessionMgr::Release()
{
    UserSessionMgr::DeleteByAdapter_UserSessionMgr(UserSessionMgrFactory::_buildType.V, this);
}

Int32 UserSessionMgr::_OnInit()
{
    _userMgr = GetOwner()->CastTo<UserMgr>();
    _eventMgr = _userMgr->GetEventMgr();
    
    _heartbeatTimer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    _heartbeatTimer->SetTimeOutHandler(this, &UserSessionMgr::_OnHeartbeatTimeOut);

    _heartbeatExpireTime = _userMgr->GetService()->GetComp<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::USER_HEARTBEAT_EXPIRE_TIME)->_int64Value;

    _userMgr->GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_ClientHeartbeatReq, this, &UserSessionMgr::_OnHeartbeatReq);
    _RegisterEvents();
    return Status::Success;
}

Int32 UserSessionMgr::_OnStart()
{
    return Status::Success;
}

void UserSessionMgr::_OnWillClose()
{

}

void UserSessionMgr::_OnClose()
{
    _Clear();
}

void UserSessionMgr::_Clear()
{
    _UnRegisterEvents();

    if(_heartbeatTimer)
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(_heartbeatTimer);
    _heartbeatTimer = NULL;

    KERNEL_NS::ContainerUtil::DelContainer(_sessionIdRefLoginPendingInfo, [](LoginPendingInfo *pendingInfo){
        LoginPendingInfo::DeleteThreadLocal_LoginPendingInfo(pendingInfo);
    });
}

void UserSessionMgr::_AddToHeartbeatQueue(IUser *user)
{
    _userHeartbeatQueue.erase(user);
    _userHeartbeatQueue.insert(user);
}

void UserSessionMgr::_RemoveFromHeartbeatQueue(IUser *user)
{
    _userHeartbeatQueue.erase(user);
}

void UserSessionMgr::_RestartHeartbeatTimer()
{
    _heartbeatTimer->Cancel();
    if(_userHeartbeatQueue.empty())
        return;

    auto firstUser = *_userHeartbeatQueue.begin();
    const auto expireTime = firstUser->GetHeartbeatExpireTime();
    const auto nowTime = KERNEL_NS::LibTime::NowMilliTimestamp();
    const auto diff = expireTime > nowTime ? (expireTime - nowTime) : 0;
    _heartbeatTimer->Schedule(diff);
}

void UserSessionMgr::_RegisterEvents()
{
    _sessionCreatedStub = _eventMgr->AddListener(EventEnums::SESSION_CREATED, this, &UserSessionMgr::_OnSessionCreated);
    _sessionMsgRecvStub = _eventMgr->AddListener(EventEnums::SERVICE_MSG_RECV, this, &UserSessionMgr::_OnMsgRecv);
    _userWillRemoveStub = _eventMgr->AddListener(EventEnums::USER_WILL_REMOVE, this, &UserSessionMgr::_OnUserWillRemove);
    _sessionWillRemoveStub = _eventMgr->AddListener(EventEnums::SESSION_WILL_DESTROY, this, &UserSessionMgr::_OnSessionWillDestroy);
}

void UserSessionMgr::_UnRegisterEvents()
{
    if(_sessionCreatedStub != INVALID_LISTENER_STUB)
        _eventMgr->RemoveListenerX(_sessionCreatedStub);

    if(_sessionMsgRecvStub != INVALID_LISTENER_STUB)
        _eventMgr->RemoveListenerX(_sessionMsgRecvStub);

    if(_userWillRemoveStub != INVALID_LISTENER_STUB)
        _eventMgr->RemoveListenerX(_userWillRemoveStub);

    if(_sessionWillRemoveStub != INVALID_LISTENER_STUB)
        _eventMgr->RemoveListenerX(_sessionWillRemoveStub);
}

void UserSessionMgr::_OnSessionCreated(KERNEL_NS::LibEvent *ev)
{
    auto sessionId = ev->GetParam(Params::SESSION_ID).AsUInt64();
    auto sessionType = ev->GetParam(Params::SESSION_TYPE).AsInt32();
    auto isLinker = ev->GetParam(Params::IS_FROM_LINKER).AsBool();
    auto isFromConnect = ev->GetParam(Params::IS_FROM_CONNECT).AsBool();

    if(isLinker || isFromConnect)
        return;

    // 关注用户的会话
    if((sessionType != SessionType::OUTER) && (sessionType != SessionType::OUTER_NO_LIMIT))
        return;

    auto expireConfig = _userMgr->GetService()->GetComp<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::USER_LOGIN_EXPIRE_TIME);
    KERNEL_NS::SmartPtr<LoginPendingInfo, KERNEL_NS::AutoDelMethods::CustomDelete> pending = LoginPendingInfo::NewThreadLocal_LoginPendingInfo(sessionId);
    pending.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<LoginPendingInfo *>(p);
        LoginPendingInfo::DeleteThreadLocal_LoginPendingInfo(ptr);
    });
    pending->_expiredTime = KERNEL_NS::LibTime::NowTimestamp() + expireConfig->_int64Value;

    _sessionIdRefLoginPendingInfo.insert(std::make_pair(sessionId, pending.AsSelf()));
    pending->_timer->SetTimeOutHandler([this, sessionId](KERNEL_NS::LibTimer *t)
    {
        auto iter = _sessionIdRefLoginPendingInfo.find(sessionId);
        auto pending = iter->second;
        _sessionIdRefLoginPendingInfo.erase(iter);

        // 已经登录了那么就移除pending
        auto user = _userMgr->GetUserBySessionId(sessionId);
        _RemoveFromHeartbeatQueue(user);

        // 未在规定时间内登录的移除连接
        if(!user || !user->IsLogined())
        {
            _userMgr->CloseSession(sessionId, 0, true, true);
        }
        else
        {// 开启心跳
            user->UpdateHeartbeatExpireTime(_heartbeatExpireTime * 1000);
            _AddToHeartbeatQueue(user);
            _RestartHeartbeatTimer();
        }

        LoginPendingInfo::DeleteThreadLocal_LoginPendingInfo(pending);
    });

    pending->_timer->Schedule(expireConfig->_int64Value * 1000);
    pending.pop();
}

void UserSessionMgr::_OnMsgRecv(KERNEL_NS::LibEvent *ev)
{
    auto sessionId = ev->GetParam(Params::SESSION_ID).AsUInt64();
    auto user = _userMgr->GetUserBySessionId(sessionId);
    if(UNLIKELY(!user))
        return;

    _RemoveFromHeartbeatQueue(user);

    if(user->IsLogined())
    {
        user->UpdateHeartbeatExpireTime(_heartbeatExpireTime * 1000);
        _AddToHeartbeatQueue(user);
    }

    _RestartHeartbeatTimer();
}

void UserSessionMgr::_OnUserWillRemove(KERNEL_NS::LibEvent *ev)
{
    auto userId = ev->GetParam(Params::USER_ID).AsUInt64();
    auto user = _userMgr->GetUser(userId);
    if(UNLIKELY(!user))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("get user fail userId:%llu"), userId);
        return;
    }

    // 移除
    auto iter = _sessionIdRefLoginPendingInfo.find(user->GetSessionId());
    if(iter != _sessionIdRefLoginPendingInfo.end())
    {
        LoginPendingInfo::DeleteThreadLocal_LoginPendingInfo(iter->second);
        _sessionIdRefLoginPendingInfo.erase(iter);
    }

    _RemoveFromHeartbeatQueue(user);

    _RestartHeartbeatTimer();
}

void UserSessionMgr::_OnSessionWillDestroy(KERNEL_NS::LibEvent *ev)
{
    auto sessionId = ev->GetParam(Params::SESSION_ID).AsUInt64();
    // 移除
    auto iter = _sessionIdRefLoginPendingInfo.find(sessionId);
    if(iter != _sessionIdRefLoginPendingInfo.end())
    {
        LoginPendingInfo::DeleteThreadLocal_LoginPendingInfo(iter->second);
        _sessionIdRefLoginPendingInfo.erase(iter);
    }

    auto user = _userMgr->GetUserBySessionId(sessionId);
    if(UNLIKELY(user))
    {
        if(user->IsLogined())
            user->Logout(LogoutReason::OTHER_REASON);

        _RemoveFromHeartbeatQueue(user);
        _RestartHeartbeatTimer();
    }
}

void UserSessionMgr::_OnHeartbeatReq(KERNEL_NS::LibPacket *&packet)
{
    auto sessionId = packet->GetSessionId();
    auto user = _userMgr->GetUserBySessionId(sessionId);
    if(UNLIKELY(!user))
        return;

    _RemoveFromHeartbeatQueue(user);

    if(user->IsLogined())
    {
        user->UpdateHeartbeatExpireTime(_heartbeatExpireTime * 1000);
        _AddToHeartbeatQueue(user);
    }

    _RestartHeartbeatTimer();

    ClientHeartbeatRes res;
    res.set_servertimems(KERNEL_NS::LibTime::NowMilliTimestamp());
    user->Send(Opcodes::OpcodeConst::OPCODE_ClientHeartbeatRes, res, packet->GetPacketId());
}

void UserSessionMgr::_OnHeartbeatTimeOut(KERNEL_NS::LibTimer *t)
{
    // TODO:心跳超时处理
    const auto nowTime = KERNEL_NS::LibTime::NowMilliTimestamp();
    auto sessionMgr = _userMgr->GetGlobalSys<ISessionMgr>();
    for(auto iter = _userHeartbeatQueue.begin(); iter != _userHeartbeatQueue.end();)
    {
        auto user = *iter;
        if(user->GetHeartbeatExpireTime() > nowTime)
            break;

        iter = _userHeartbeatQueue.erase(iter);

        auto session = sessionMgr->GetSession(user->GetSessionId());
        if(LIKELY(session))
            user->Logout(LogoutReason::TIMEOUT);
    }

    _RestartHeartbeatTimer();
}


SERVICE_END
