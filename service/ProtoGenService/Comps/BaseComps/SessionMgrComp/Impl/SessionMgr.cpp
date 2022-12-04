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
#include <service/ProtoGenService/Comps/BaseComps/SessionMgrComp/Defs/ServiceSession/ServiceSession.h>
#include <service/ProtoGenService/Comps/BaseComps/SessionMgrComp/Impl/SessionMgr.h>
#include <service/ProtoGenService/Comps/BaseComps/Event/Event.h>


#include <service/ProtoGenService/Comps/BaseComps/SessionMgrComp/Impl/SessionMgrFactory.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(ISessionMgr);

POOL_CREATE_OBJ_DEFAULT_IMPL(SessionMgr);

SessionMgr::SessionMgr()
:_sessionWillCreatedStub(0)
, _sessionDestroyStub(0)
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
    auto protocolType = ev->GetParam(Params::PROTOCOL_TYPE).AsInt32();
    auto priorityLevel = ev->GetParam(Params::PRIORITY_LEVEL).AsUInt32();
    auto sessionType = ev->GetParam(Params::SESSION_TYPE).AsInt32();
    auto sesionPollerId = ev->GetParam(Params::SESSION_POLLER_ID).AsUInt64();
    auto serviceId = ev->GetParam(Params::SERVICE_ID).AsUInt64();
    auto stub = ev->GetParam(Params::STUB).AsUInt64();
    auto isFromConnect = ev->GetParam(Params::IS_FROM_CONNECT).AsBool();
    auto isFromLinker = ev->GetParam(Params::IS_FROM_LINKER).AsBool();

    g_Log->Info(LOGFMT_OBJ_TAG("session will created sessionid:%llu, localAddr:%s, remoteAddr:%s, protocolType:%d, priorityLevel:%u, pollerId:%llu, "
                            "serviceId:%llu, stub:%llu, isFromConnect:%d, isFromLinker:%d")
                            , sessionId
                            , localAddr->ToString().c_str()
                            , remoteAddr->ToString().c_str()
                            , protocolType
                            , priorityLevel
                            , sesionPollerId
                            , serviceId
                            , stub
                            , isFromConnect
                            , isFromLinker
                            );

    ServiceSessionInfo sessionInfo;
    sessionInfo._serviceId = serviceId;
    sessionInfo._sessionId = sessionId;
    sessionInfo._pollerId = sesionPollerId;
    sessionInfo._priorityLevel = priorityLevel;
    sessionInfo._sessionType = sessionType;
    sessionInfo._localAddr = *localAddr;
    sessionInfo._remoteAddr = *remoteAddr;
    _CreateSession(sessionInfo);
}

void SessionMgr::_OnSessionDestroy(KERNEL_NS::LibEvent *ev)
{
    auto sessionId = ev->GetParam(Params::SESSION_ID).AsUInt64();
    auto closeReason = ev->GetParam(Params::SESSION_CLOSE_REASON).AsInt32();
    auto serviceId = ev->GetParam(Params::SERVICE_ID).AsUInt64();
    auto priorityLevel = ev->GetParam(Params::PRIORITY_LEVEL).AsUInt32();
    auto stub = ev->GetParam(Params::STUB).AsUInt64();

    g_Log->Info(LOGFMT_OBJ_TAG("session destroy:sessionId:%llu, closeReason:%d, %s, serviceId:%llu, priorityLevel:%u, stub:%llu")
                , sessionId, closeReason, KERNEL_NS::CloseSessionInfo::GetCloseReason(closeReason), serviceId
                , priorityLevel, stub);

    auto session = GetSession(sessionId);
    _DestroySession(session);
}


ServiceSession *SessionMgr::_CreateSession(const ServiceSessionInfo &sessionInfo)
{
    auto newSession = ServiceSessionFactory::Create(this);
    newSession->SetSessionInfo(sessionInfo);

    _MakeSessionDict(newSession);

    g_Log->Info(LOGFMT_OBJ_TAG("created new session:%s"), newSession->ToString().c_str());
    return newSession;
}

void SessionMgr::_DestroySession(ServiceSession *session)
{
    g_Log->Info(LOGFMT_OBJ_TAG("destroy session :%s"), session->ToString().c_str());

    _sessionIdRefSession.erase(session->GetSessionId());

    session->Release();
}

void SessionMgr::_MakeSessionDict(ServiceSession *session)
{
    _sessionIdRefSession.insert(std::make_pair(session->GetSessionId(), session));
}

SERVICE_END
