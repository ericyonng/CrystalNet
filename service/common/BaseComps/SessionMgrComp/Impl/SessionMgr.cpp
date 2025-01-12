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
 * Date: 2022-09-18 22:04:44
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/kernel.h>
#include <service/common/BaseComps/SessionMgrComp/Defs/ServiceSession/ServiceSession.h>
#include <service/common/BaseComps/SessionMgrComp/Impl/SessionMgr.h>
#include <service/common/BaseComps/Event/Event.h>


#include <service/common/BaseComps/SessionMgrComp/Impl/SessionMgrFactory.h>

SERVICE_BEGIN


POOL_CREATE_OBJ_DEFAULT_IMPL(ISessionMgr);

POOL_CREATE_OBJ_DEFAULT_IMPL(SessionMgr);

SessionMgr::SessionMgr()
:ISessionMgr(KERNEL_NS::RttiUtil::GetTypeId<SessionMgr>())
,_sessionAmount{0}
,_sessionWillCreatedStub(0)
,_sessionDestroyStub(0)

{

}

SessionMgr::~SessionMgr()
{
    _Clear();
}

void SessionMgr::Release()
{
    SessionMgr::DeleteByAdapter_SessionMgr(SessionMgrFactory::_buildType.V, this);
}

KERNEL_NS::LibString SessionMgr::ToString() const
{
    return ISessionMgr::ToString();
}

ServiceSession *SessionMgr::GetSession(UInt64 sessionId)
{
    auto iter = _sessionIdRefSession.find(sessionId);
    return iter == _sessionIdRefSession.end() ? NULL : iter->second;
}

const ServiceSession *SessionMgr::GetSession(UInt64 sessionId) const
{
    auto iter = _sessionIdRefSession.find(sessionId);
    return iter == _sessionIdRefSession.end() ? NULL : iter->second;
}

Int64 SessionMgr::NewPacketId(UInt64 sessionId)
{
    auto iter = _sessionIdRefMaxPacketId.find(sessionId);
    if(iter == _sessionIdRefMaxPacketId.end())
        iter = _sessionIdRefMaxPacketId.insert(std::make_pair(sessionId, 0)).first;

    // 服务端推给客户端的最大包id
    return ++iter->second;
}

UInt64 SessionMgr::GetSessionAmount() const
{
    return _sessionAmount;
}

const std::map<UInt64, ServiceSession *> &SessionMgr::GetSessions()
{
    return _sessionIdRefSession;
}

Int32 SessionMgr::_OnGlobalSysInit()
{
    // 事件
    _RegisterEvents();

    return Status::Success;
}

void SessionMgr::_OnGlobalSysClose()
{
    _Clear();
}

void SessionMgr::_Clear()
{
    _UnRegisterEvents();

    KERNEL_NS::ContainerUtil::DelContainer2(_sessionIdRefSession);
}

void SessionMgr::_RegisterEvents()
{
    if(_sessionWillCreatedStub)
        return;

    auto eventMgr = GetEventMgr();
    _sessionWillCreatedStub = eventMgr->AddListener(EventEnums::SESSION_WILL_CREATED, this, &SessionMgr::_OnSessionWillCreated);
    _sessionDestroyStub = eventMgr->AddListener(EventEnums::SESSION_DESTROY, this, &SessionMgr::_OnSessionDestroy);
}

void SessionMgr::_UnRegisterEvents()
{
    if(_sessionWillCreatedStub == 0)
        return;
        
    auto eventMgr = GetEventMgr();
    eventMgr->RemoveListenerX(_sessionWillCreatedStub);
    eventMgr->RemoveListenerX(_sessionDestroyStub);
}

void SessionMgr::_OnSessionWillCreated(KERNEL_NS::LibEvent *ev)
{
    auto sessionId = ev->GetParam(Params::SESSION_ID).AsUInt64();
    auto localAddr = ev->GetParam(Params::LOCAL_ADDR).AsPtr<KERNEL_NS::BriefSockAddr>();
    auto remoteAddr = ev->GetParam(Params::REMOTE_ADDR).AsPtr<KERNEL_NS::BriefSockAddr>();
    auto priorityLevel = ev->GetParam(Params::PRIORITY_LEVEL).AsUInt32();
    auto sessionType = ev->GetParam(Params::SESSION_TYPE).AsInt32();
    auto protocolStackType = ev->GetParam(Params::PROTOCOL_STACK).AsInt32();
    auto sesionPollerId = ev->GetParam(Params::SESSION_POLLER_ID).AsUInt64();
    auto serviceId = ev->GetParam(Params::SERVICE_ID).AsUInt64();
    auto remoteOriginAddr = ev->GetParam(Params::TARGET_ADDR_IP_CONFIG).AsPtr<KERNEL_NS::AddrIpConfig>();
    auto protocolStack = GetService()->GetProtocolStack(protocolStackType);
    auto isFromConnect = ev->GetParam(Params::IS_FROM_CONNECT).AsBool();
    // if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
    // {
    //     g_Log->Info(LOGFMT_OBJ_TAG("session will created sessionid:%llu, localAddr:%s, remoteAddr:%s, priorityLevel:%u, pollerId:%llu, "
    //                             "serviceId:%llu, isFromConnect:%s")
    //                             , sessionId
    //                             , localAddr->ToString().c_str()
    //                             , remoteAddr->ToString().c_str()
    //                             , priorityLevel
    //                             , sesionPollerId
    //                             , serviceId
    //                             , isFromConnect ? "true" :"false"
    //                             );
    // }


    ServiceSessionInfo sessionInfo;
    sessionInfo._serviceId = serviceId;
    sessionInfo._sessionId = sessionId;
    sessionInfo._pollerId = sesionPollerId;
    sessionInfo._priorityLevel = priorityLevel;
    sessionInfo._sessionType = sessionType;
    sessionInfo._localAddr = *localAddr;
    sessionInfo._remoteAddr = *remoteAddr;
    sessionInfo._protocolStack = protocolStack;
    sessionInfo._remoteOriginAddr = *remoteOriginAddr;
    sessionInfo._isFromLinker = !isFromConnect;


    // 消息发送限速
    switch (sessionInfo._sessionType)
    {
    case SessionType::INNER:
    case SessionType::OUTER_NO_LIMIT:
        sessionInfo._sessionSendBytesLimit = 0;
        break;
    case SessionType::OUTER:
        sessionInfo._sessionSendBytesLimit = GetApp()->GetKernelConfig()._sessionSendPacketContentLimit;
        break;
    default:
        break;
    }

    // 消息发送限速
    switch (sessionInfo._sessionType)
    {
    case SessionType::INNER:
    case SessionType::OUTER_NO_LIMIT:
        sessionInfo._sessionRecvBytesLimit = 0;
        break;
    case SessionType::OUTER:
        sessionInfo._sessionRecvBytesLimit = GetApp()->GetKernelConfig()._sessionRecvPacketContentLimit;
        break;
    default:
        break;
    }

    _CreateSession(sessionInfo);
}

void SessionMgr::_OnSessionDestroy(KERNEL_NS::LibEvent *ev)
{
    auto sessionId = ev->GetParam(Params::SESSION_ID).AsUInt64();

    // g_Log->Info(LOGFMT_OBJ_TAG("session destroy:sessionId:%llu, closeReason:%d, %s, serviceId:%llu, priorityLevel:%u, stub:%llu")
    //             , sessionId, closeReason, KERNEL_NS::CloseSessionInfo::GetCloseReason(closeReason), serviceId
    //             , priorityLevel, stub);

    auto session = GetSession(sessionId);
    _DestroySession(session);
}

ServiceSession *SessionMgr::_CreateSession(const ServiceSessionInfo &sessionInfo)
{
    auto newSession = ServiceSessionFactory::Create(this);
    newSession->SetSessionInfo(sessionInfo);

    _MakeSessionDict(newSession);
    UInt64 currentAmount = ++_sessionAmount;

    if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
        g_Log->Debug(LOGFMT_OBJ_TAG("create new session sessionAmount:%llu session id:%llu [%s:%hu %s %s(%s):%hu] service id:%llu, poller id:%llu, msg priority level:%u, session type:%d, send bytes limit:%llu, recv bytes limit:%llu")
    , currentAmount
    , sessionInfo._sessionId
    , sessionInfo._localAddr._ip.c_str(), sessionInfo._localAddr._port
    , sessionInfo._isFromLinker ? "<=" : "=>"
    , sessionInfo._remoteOriginAddr._ip.c_str(), sessionInfo._remoteAddr._ip.c_str(), sessionInfo._remoteAddr._port

    , sessionInfo._serviceId,  sessionInfo._pollerId, sessionInfo._priorityLevel, sessionInfo._sessionType
    , sessionInfo._sessionSendBytesLimit, sessionInfo._sessionRecvBytesLimit);

    // g_Log->Info(LOGFMT_OBJ_TAG("created new session:%s"), newSession->ToString().c_str());
    return newSession;
}

void SessionMgr::_DestroySession(ServiceSession *session)
{
    // g_Log->Info(LOGFMT_OBJ_TAG("destroy session :%s"), session->ToString().c_str());
    auto sessionInfo = session->GetSessionInfo();
    UInt64 currentAmount = --_sessionAmount;
    _sessionIdRefSession.erase(session->GetSessionId());

    if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
        g_Log->Debug(LOGFMT_OBJ_TAG("destroy session sessionAmount:%llu session id:%llu [%s:%hu %s %s(%s):%hu] service id:%llu, poller id:%llu, msg priority level:%u, session type:%d, send bytes limit:%llu, recv bytes limit:%llu")
    , currentAmount
    , sessionInfo->_sessionId
    , sessionInfo->_localAddr._ip.c_str(), sessionInfo->_localAddr._port
    , sessionInfo->_isFromLinker ? "<=" : "=>"
    , sessionInfo->_remoteOriginAddr._ip.c_str(), sessionInfo->_remoteAddr._ip.c_str(), sessionInfo->_remoteAddr._port
    , sessionInfo->_serviceId,  sessionInfo->_pollerId, sessionInfo->_priorityLevel, sessionInfo->_sessionType
    , sessionInfo->_sessionSendBytesLimit, sessionInfo->_sessionRecvBytesLimit);

    session->Release();
}

void SessionMgr::_MakeSessionDict(ServiceSession *session)
{
    _sessionIdRefSession.insert(std::make_pair(session->GetSessionId(), session));
}

SERVICE_END
