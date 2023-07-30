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
 * Date: 2022-04-23 23:16:12
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>

#include <kernel/comp/Poller/PollerInc.h>
#include <kernel/comp/NetEngine/Poller/Defs/TcpPollerConfig.h>
#include <kernel/comp/TimeSlice.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerDirty.h>
#include <kernel/comp/NetEngine/LibIocp.h>
#include <kernel/comp/NetEngine/Poller/interface/IPollerMgr.h>
#include <kernel/comp/thread/LibThread.h>
#include <kernel/comp/NetEngine/Defs/LibConnectInfo.h>
#include <kernel/comp/NetEngine/LibPacket.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/IocpTcpSession.h>
#include <kernel/comp/NetEngine/Poller/Defs/CloseSessionInfo.h>
#include <kernel/comp/NetEngine/Poller/impl/IpRule/IpRuleMgr.h>
#include <kernel/comp/NetEngine/Defs/BuildSessionInfo.h>
#include <kernel/comp/NetEngine/LibSocket.h>
#include <kernel/comp/NetEngine/LibAddr.h>
#include <kernel/comp/NetEngine/Defs/IoData.h>
#include <kernel/comp/Service/Service.h>
#include <kernel/comp/NetEngine/Defs/LibListenInfo.h>
#include <kernel/comp/NetEngine/Defs/LibConnectInfo.h>
#include <kernel/comp/Timer/Timer.h>
#include <kernel/comp/Utils/Utils.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/TcpPollerMgr.h>
#include <kernel/comp/NetEngine/Poller/impl/IpRule/IpRuleMgrFactory.h>
#include <kernel/comp/NetEngine/Poller/impl/IpRule/IpRuleMgr.h>
#include <kernel/comp/Utils/BitUtil.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerConfig.h>

#include <kernel/comp/NetEngine/Poller/impl/Tcp/IocpTcpPoller.h>
#include <kernel/comp/TlsMemoryCleanerComp.h>


#if CRYSTAL_TARGET_PLATFORM_WINDOWS

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IocpTcpPoller);

IocpTcpPoller::IocpTcpPoller(TcpPollerMgr *pollerMgr, UInt64 pollerId, const TcpPollerInstConfig* cfg)
:_pollerId(pollerId)
,_tcpPollerMgr(pollerMgr)
,_pollerMgr(NULL)
,_serviceProxy(NULL)
,_poller(NULL)
,_memoryCleaner(NULL)
,_cfg(cfg)
,_sessionCount{0}
,_sessionPendingCount{0}
,_iocp(NULL)
,_wakeupSessionId(0)
,_quitIocpWorker(NULL)
,_monitor(NULL)
,_eventLoopThread(NULL)
,_pollerInstMonitorPriorityLevel(-1)
{

}

IocpTcpPoller::~IocpTcpPoller()
{
    _Clear();
}

void IocpTcpPoller::Release()
{
    IocpTcpPoller::Delete_IocpTcpPoller(this);
}

void IocpTcpPoller::OnRegisterComps()
{
    // 注册ip规则组件
    RegisterComp<IpRuleMgrFactory>();
    // 注册poller组件
    RegisterComp<PollerFactory>();
    // 注册定时清理内存
    RegisterComp<TlsMemoryCleanerCompFactory>();
}

void IocpTcpPoller::Clear()
{
    _Clear();
    CompHostObject::Clear();
}

LibString IocpTcpPoller::ToString() const
{
    LibString info;
    info.AppendFormat("iocp tcp poller comp object info:%s\n", CompHostObject::ToString().c_str());

    info.AppendFormat("iocp tcp poller poller id:%llu, ", _pollerId)
        .AppendFormat("session quantity:%llu", _sessionIdRefSession.size());

    info.AppendFormat("comps info:\n");
    auto &allComps = GetAllComps();
    for(auto comp : allComps)
        info.AppendFormat("%s\n", comp->ToString().c_str());

    return info;
}

UInt64 IocpTcpPoller::CalcLoadScore() const
{
    return _poller->CalcLoadScore() + _sessionCount + _sessionPendingCount;
}

void IocpTcpPoller::PostSend(Int32 level, UInt64 sessionId, LibPacket *packet)
{
    auto ev = AsynSendEvent::New_AsynSendEvent();
    ev->_sessionId = sessionId;
    ev->_packets = LibList<LibPacket *>::New_LibList();
    ev->_packets->PushBack(packet);

    if(LIKELY(_poller->IsEnable()))
        _poller->Push(level, ev);
    else
    {
        ContainerUtil::DelContainer(*ev->_packets, [](LibPacket *ptr){
            LibPacket::Delete_LibPacket(ptr);
        });

        LibList<LibPacket *>::Delete_LibList(ev->_packets);
        ev->_packets = NULL;
        ev->Release();
    }
}

void IocpTcpPoller::PostSend(Int32 level, UInt64 sessionId, LibList<LibPacket *> *packets)
{
    auto ev = AsynSendEvent::New_AsynSendEvent();
    ev->_sessionId = sessionId;
    ev->_packets = packets;

    if(LIKELY(_poller->IsEnable()))
        _poller->Push(level, ev);
    else
    {
        ContainerUtil::DelContainer(*ev->_packets, [](LibPacket *ptr){
            LibPacket::Delete_LibPacket(ptr);
        });

        LibList<LibPacket *>::Delete_LibList(ev->_packets);
        ev->_packets = NULL;
        ev->Release();
    }
}

void IocpTcpPoller::PostNewSession(Int32 level, BuildSessionInfo *buildSessionInfo)
{
    auto newSessionEv = NewSessionEvent::New_NewSessionEvent();
    newSessionEv->_buildInfo = buildSessionInfo;
    _poller->Push(level, newSessionEv);
}

void IocpTcpPoller::PostAddlisten(Int32 level, LibListenInfo *listenInfo)
{
    auto ev = AddListenEvent::New_AddListenEvent();
    ev->_addListenInfoList.push_back(listenInfo);
    _poller->Push(level, ev);
}

void IocpTcpPoller::PostAddlistenList(Int32 level, std::vector<LibListenInfo *> &listenInfoList)
{
    auto ev = AddListenEvent::New_AddListenEvent();
    ev->_addListenInfoList.swap(listenInfoList);

    _poller->Push(level, ev);
}

void IocpTcpPoller::PostConnect(Int32 level, LibConnectInfo *connectInfo)
{
    // 必须指定service id
    if(UNLIKELY(connectInfo->_fromServiceId == 0))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("connect info must specify a service id connect info:%s"), connectInfo->ToString().c_str());
        LibConnectInfo::Delete_LibConnectInfo(connectInfo);
        return;
    }

    auto ev = AsynConnectEvent::New_AsynConnectEvent();
    ev->_connectInfo = connectInfo;
    _poller->Push(level, ev);
}

void IocpTcpPoller::PostCloseSession(UInt64 fromServiceId, Int32 level,  UInt64 sessionId, Int64 closeMillisecondTime, bool forbidRead, bool forbidWrite)
{
    auto closeEv = CloseSessionEvent::New_CloseSessionEvent();
    closeEv->_sessionId = sessionId;
    closeEv->_closeMillisecondTime = closeMillisecondTime;
    closeEv->_forbidRead = forbidRead;
    closeEv->_forbidWrite = forbidWrite;
    closeEv->_fromServiceId = fromServiceId;
    closeEv->_priorityLevel = level;
    _poller->Push(level, closeEv);
}

void IocpTcpPoller::PostIpControl(Int32 level, const std::list<IpControlInfo *> &controlList)
{
    auto ev = IpRuleControlEvent::New_IpRuleControlEvent();
    ev->_ipControlList = controlList;
    _poller->Push(level, ev);
}

void IocpTcpPoller::PostQuitServiceSessionsEvent(UInt64 serviceId, Int32 level)
{
    auto ev = QuitServiceSessionsEvent::New_QuitServiceSessionsEvent();
    ev->_fromServiceId = serviceId;
    ev->_priorityLevel = level;
    _poller->Push(level, ev);
}

IocpTcpSession *IocpTcpPoller::_GetSession(UInt64 sessionId)
{
    auto iter = _sessionIdRefSession.find(sessionId);
    return iter == _sessionIdRefSession.end() ? NULL : iter->second;
}

std::set<IocpTcpSession *> &IocpTcpPoller::_GetSessionsByIp(const LibString &ip)
{
    DEF_STATIC_THREAD_LOCAL_DECLEAR std::set<IocpTcpSession *> s_empty;
    auto iter = _ipRefSessions.find(ip);
    return iter == _ipRefSessions.end() ? s_empty : iter->second;
}

IocpTcpSession *IocpTcpPoller::_CreateSession(BuildSessionInfo *sessionInfo)
{
    g_Log->NetInfo(LOGFMT_OBJ_TAG("will create session by build session info:%s"), sessionInfo->ToString().c_str());
    if(UNLIKELY(sessionInfo->_serviceId == 0))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("must specify a belong service id: connect remote service id from LibConnectInfo, others from accept session service id sessionInfo:%s"), sessionInfo->ToString().c_str());
        return NULL;
    }

    // socket
    auto sock = LibSocket::NewThreadLocal_LibSocket();
    Int32 err = sock->TakeOver<_Build::TL>(sessionInfo->_sock
        , sessionInfo->_family
        , ProtocolType::ToIpProtocol(sessionInfo->_protocolType)
        , IPPROTO_TCP
        , sessionInfo->_isWinSockNonBlock
        , sessionInfo->_isLinker);
    if(err != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("take over sock fail err:%d, sessionInfo:%s"), err, sessionInfo->ToString().c_str());
        LibSocket::DeleteThreadLocal_LibSocket(sock);
        return NULL;
    }

    // 设置特性
    if(UNLIKELY(sock->IsNonBlocking() == false))
        sock->SetNonBlocking();
    if(UNLIKELY(sock->IsNoDelay() == false))
        sock->SetNoDelay(true);
    if(UNLIKELY(sock->IsReuseAddr() == false))
        sock->EnableReuseAddr();
    if(UNLIKELY(sock->IsReusePort() == false))
        sock->EnableReusePort();

    // 地址信息
    sock->GetAddr()->BindAddrInfo(sessionInfo->_localAddr, sessionInfo->_remoteAddr);

    auto &sessionOption = sessionInfo->_sessionOption;
    auto newSession = IocpTcpSession::NewThreadLocal_IocpTcpSession(sessionInfo->_sessionId, sessionInfo->_isLinker, sessionInfo->_isFromConnect);
    newSession->SetSocket(sock);
    newSession->SetAcceptHandleCountLimit(_cfg->_handleAcceptPerFrameLimit);
    newSession->SetRecvHandleBytesLimit(_cfg->_handleRecvBytesPerFrameLimit);
    newSession->SetSendHandleBytesLimit(_cfg->_handleSendBytesPerFrameLimit);
    newSession->SetBufferCapacity(_cfg->_bufferCapacity);
    newSession->SetDirtyHelper(_poller->GetDirtyHelper());
    newSession->SetServiceProxy(_serviceProxy);
    newSession->SetPollerMgr(_pollerMgr);
    newSession->SetPriorityLevel(sessionInfo->_priorityLevel);
    newSession->SetProtocolStack(sessionInfo->_protocolStack);
    newSession->SetServiceId(sessionInfo->_serviceId);
    newSession->SetPollerId(GetPollerId());
    newSession->SetMaxPacketSize(sessionOption._maxPacketSize);
    newSession->SetOption(sessionOption);
    newSession->SetProtocolType(sessionInfo->_protocolType);
    if(sessionOption._forbidRecv)
        newSession->ForbidRecv();

    // 使用option配置的缓存大小
    if(sessionOption._sockRecvBufferSize)
        newSession->SetBufferCapacity(sessionOption._sockRecvBufferSize);

    // 不是监听者若没有指定协议栈的需要从服务那边取
    if(!newSession->IsLinker() && (newSession->GetProtocolStack() == NULL))
    {
        auto stackFromService = _serviceProxy->GetProtocolStack(newSession);
        if(UNLIKELY(!stackFromService))
            g_Log->NetError(LOGFMT_OBJ_TAG("stack from service zero, session:%s"), newSession->ToString().c_str());
        newSession->SetProtocolStack(stackFromService);
    }

    err = newSession->Init();
    if(err != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("init session fail session info:%s, err:%d"), newSession->ToString().c_str(), err);
        IocpTcpSession::DeleteThreadLocal_IocpTcpSession(newSession);
        return NULL;
    }

    if(!sessionInfo->_isFromConnect)
    {// connect 已经关联过iocp 不可重复关联
        err = _iocp->Reg(sock->GetSock(), newSession->GetId());
        if(err != Status::Success)
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("reg session to iocp fail session fail session info:%s, err:%d"), newSession->ToString().c_str(), err);
            IocpTcpSession::DeleteThreadLocal_IocpTcpSession(newSession);
            return NULL;
        }
    }

    if(sock->IsLinker())
        err = sock->PostAsyncAccept();
    else
        err = sock->PostZeroWSARecv();

    if(err != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("socket post zero recv or async accept fail err:%d, newSession:%s"), err, newSession->ToString().c_str());
        IocpTcpSession::DeleteThreadLocal_IocpTcpSession(newSession);
        return NULL;
    }

    // pending 数量转真实数量
    _pollerMgr->ReduceSessionPending(1);
    _pollerMgr->AddSessionQuantity(1);

    _sessionIdRefSession.insert(std::make_pair(newSession->GetId(), newSession));
    auto iterIp = _ipRefSessions.find(sessionInfo->_remoteAddr._ip);
    if(iterIp == _ipRefSessions.end())
        iterIp = _ipRefSessions.insert(std::make_pair(sessionInfo->_remoteAddr._ip, std::set<IocpTcpSession *>())).first;
    iterIp->second.insert(newSession);

    g_Log->NetInfo(LOGFMT_OBJ_TAG("create session by build session info suc session info:%s"), newSession->ToString().c_str());
    ++_sessionCount;
    if(newSession->IsLinker())
        _pollerMgr->AddListenerSessionCount(1);
    else if(newSession->IsConnectToRemote())
        _pollerMgr->AddConnectedSessionCount(1);
    else
        _pollerMgr->AddAcceptedSessionCount(1);

    return newSession;
}

IocpTcpSession *IocpTcpPoller::_CreateSession(LibListenInfo *listenInfo)
{
    g_Log->NetInfo(LOGFMT_OBJ_TAG("will create session by listen info:%s"), listenInfo->ToString().c_str());

   if(UNLIKELY(listenInfo->_serviceId == 0))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("must specify a belong service id： listenInfo:%s"), listenInfo->ToString().c_str());
        return NULL;
    }

    // socket
    auto sock = LibSocket::NewThreadLocal_LibSocket();

    Int32 errCode = sock->Create<_Build::TL>(listenInfo->_family
                    , ProtocolType::ToIpProtocol(listenInfo->_protocolType));
    if(errCode != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("create socket fail listen info:%s, errCode:%d"), listenInfo->ToString().c_str(), errCode);
        LibSocket::DeleteThreadLocal_LibSocket(sock);
        return NULL;
    }

    errCode = sock->SetNonBlocking();
    if(errCode != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("set non blocking fail listen info:%s, errCode:%d"), listenInfo->ToString().c_str(), errCode);
        LibSocket::DeleteThreadLocal_LibSocket(sock);
        return NULL;
    }

    errCode = sock->EnableReuseAddr();
    if(errCode != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("enable reuse addr fail listen info:%s, errCode:%d"), listenInfo->ToString().c_str(), errCode);
        LibSocket::DeleteThreadLocal_LibSocket(sock);
        return NULL;
    }

    errCode = sock->EnableReusePort();
    if(errCode != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("enable reuse port fail listen info:%s, errCode:%d"), listenInfo->ToString().c_str(), errCode);
        LibSocket::DeleteThreadLocal_LibSocket(sock);
        return NULL;
    }

    errCode = sock->BindTo(listenInfo->_ip.c_str(), listenInfo->_port);
    if(errCode != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("bind to address fail listen info:%s, errCode:%d"), listenInfo->ToString().c_str(), errCode);
        LibSocket::DeleteThreadLocal_LibSocket(sock);
        return NULL;
    }

    errCode = sock->SetNoDelay(true);
    if(errCode != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("enable reuse port fail listen info:%s, errCode:%d"), listenInfo->ToString().c_str(), errCode);
        LibSocket::DeleteThreadLocal_LibSocket(sock);
        return NULL;
    }

    auto &sessionOption = listenInfo->_sessionOption;
    if(sessionOption._sockRecvBufferSize)
    {
        errCode = sock->SetRecvBufSize(sessionOption._sockRecvBufferSize);
        if(errCode != Status::Success)
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("set recv buffer size fail fail listen info:%s, errCode:%d"), listenInfo->ToString().c_str(), errCode);
            LibSocket::DeleteThreadLocal_LibSocket(sock);
            return NULL;
        }
    }

    if(sessionOption._sockSendBufferSize)
    {
        errCode = sock->SetSendBufSize(sessionOption._sockSendBufferSize);
        if(errCode != Status::Success)
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("set send buffer size fail fail listen info:%s, errCode:%d"), listenInfo->ToString().c_str(), errCode);
            LibSocket::DeleteThreadLocal_LibSocket(sock);
            return NULL;
        }
    }

    errCode = sock->Listen();
    if(errCode != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("listen fail listen info:%s, errCode:%d"), listenInfo->ToString().c_str(), errCode);
        LibSocket::DeleteThreadLocal_LibSocket(sock);
        return NULL;
    }

    // 本地地址信息
    errCode = sock->UpdateLocalAddr();
    if(errCode != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("update local addr listen info:%s, errCode:%d"), listenInfo->ToString().c_str(), errCode);
        LibSocket::DeleteThreadLocal_LibSocket(sock);
        return NULL;
    }

    const auto newSessionId = _pollerMgr->NewSessionId();
    auto newSession = IocpTcpSession::NewThreadLocal_IocpTcpSession(newSessionId, true, false);
    newSession->SetSocket(sock);
    newSession->SetAcceptHandleCountLimit(_cfg->_handleAcceptPerFrameLimit);
    newSession->SetRecvHandleBytesLimit(_cfg->_handleRecvBytesPerFrameLimit);
    newSession->SetSendHandleBytesLimit(_cfg->_handleSendBytesPerFrameLimit);
    newSession->SetBufferCapacity(_cfg->_bufferCapacity);
    newSession->SetDirtyHelper(_poller->GetDirtyHelper());
    newSession->SetServiceProxy(_serviceProxy);
    newSession->SetPollerMgr(_pollerMgr);
    newSession->SetPriorityLevel(listenInfo->_priorityLevel);
    newSession->SetServiceId(listenInfo->_serviceId);
    newSession->SetPollerId(GetPollerId());
    newSession->SetMaxPacketSize(sessionOption._maxPacketSize);
    newSession->SetOption(sessionOption);
    newSession->SetProtocolType(listenInfo->_protocolType);
    if(sessionOption._forbidRecv)
        newSession->ForbidRecv();

    // 使用option配置的缓存大小
    if(sessionOption._sockRecvBufferSize)
        newSession->SetBufferCapacity(sessionOption._sockRecvBufferSize);
        
    errCode = newSession->Init();
    if(errCode != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("init session fail listen session:%s, errCode:%d"), newSession->ToString().c_str(), errCode);
        IocpTcpSession::DeleteThreadLocal_IocpTcpSession(newSession);
        return NULL;
    }

    errCode = _iocp->Reg(sock->GetSock(), newSessionId);
    if(errCode != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("reg to iocp fail listen session:%s, errCode:%d"), newSession->ToString().c_str(), errCode);
        IocpTcpSession::DeleteThreadLocal_IocpTcpSession(newSession);
        return NULL;
    }

    errCode = sock->PostAsyncAccept();
    if(errCode != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("listen socket post async accept fail errCode:%d, listen session:%s")
                    , errCode, newSession->ToString().c_str());
        IocpTcpSession::DeleteThreadLocal_IocpTcpSession(newSession);
        return NULL;
    }

    _pollerMgr->AddSessionQuantity(1);
    _sessionIdRefSession.insert(std::make_pair(newSession->GetId(), newSession));

    ++_sessionCount;
    _pollerMgr->AddListenerSessionCount(1);

    g_Log->NetInfo(LOGFMT_OBJ_TAG("create session by listen info suc session info:%s"), newSession->ToString().c_str());
    return newSession;
}

void IocpTcpPoller::_CloseSession(IocpTcpSession *session, Int32 closeReasonEnum, UInt64 stub)
{
    g_Log->NetInfo(LOGFMT_OBJ_TAG("poller id:%llu, closeReasonEnum:%d,%s, stub:%llu, close session :%s")
                , GetPollerId(), closeReasonEnum, CloseSessionInfo::GetCloseReason(closeReasonEnum), stub, session->ToString().c_str());
    
    _poller->GetDirtyHelper()->Clear(session);

    // session关闭的消息
    auto sessionDestroyEv = SessionDestroyEvent::New_SessionDestroyEvent();
    sessionDestroyEv->_closeReason = closeReasonEnum;
    sessionDestroyEv->_sessionId = session->GetId();
    sessionDestroyEv->_serviceId = session->GetServiceId();
    sessionDestroyEv->_priorityLevel = session->GetPriorityLevel();
    sessionDestroyEv->_stub = stub;
    _serviceProxy->PostMsg(sessionDestroyEv->_serviceId, sessionDestroyEv->_priorityLevel, sessionDestroyEv);

    // 从ip字典中移除
    if(LIKELY(!session->IsLinker()))
    {
        auto sock = session->GetSock();
        if(LIKELY(sock))
        {
            auto addr = sock->GetAddr();
            if(LIKELY(addr))
            {
                auto &sessions = _GetSessionsByIp(addr->GetRemoteIpStr());
                sessions.erase(session);
            }
        }
    }

    session->Close();
    _sessionIdRefSession.erase(session->GetId());

    --_sessionCount;
    if(session->IsLinker())
        _pollerMgr->ReduceListenerSessionCount(1);
    else if(session->IsConnectToRemote())
        _pollerMgr->ReduceConnectedSessionCount(1);
    else
        _pollerMgr->ReduceAcceptedSessionCount(1);

    _pollerMgr->ReduceSessionQuantity(1);

    IocpTcpSession::DeleteThreadLocal_IocpTcpSession(session);
}

bool IocpTcpPoller::_CanClose(IocpTcpSession *session) const
{
    if(UNLIKELY(session->IsLinker()))
        return true;

    return !session->CanRecv() && !session->CanSend();
}

void IocpTcpPoller::_TryCloseSession(IocpTcpSession *session, Int32 closeReasonEnum, UInt64 stub)
{
    if(UNLIKELY(session->IsLinker()))
    {
        session->MaskClose(closeReasonEnum);
        g_Log->NetDebug(LOGFMT_OBJ_TAG("try close a linker session close reason:%d,%s, stub:%llu")
                    , closeReasonEnum, CloseSessionInfo::GetCloseReason(closeReasonEnum), stub);
        _CloseSession(session, closeReasonEnum, stub);
        return;
    }

    g_Log->NetDebug(LOGFMT_OBJ_TAG("try close session close reason:%d,%s, stub:%llu")
                , closeReasonEnum, CloseSessionInfo::GetCloseReason(closeReasonEnum), stub);
    if(session->CanRecv() || session->CanSend())
    {
        session->MaskClose(closeReasonEnum);
        auto var = _poller->GetDirtyHelper()->MaskDirty(session, PollerDirty::CLOSE, true);
        auto &varDict = var->BecomeDict();
        varDict[1] = closeReasonEnum;
        varDict[2] = stub;
        g_Log->NetInfo(LOGFMT_OBJ_TAG("session mask close dirty flag, and will close session later poller id:%llu, session info:%s."), GetPollerId(), session->ToString().c_str());
        return;
    }

    session->MaskClose(closeReasonEnum);
    _CloseSession(session, closeReasonEnum, stub);
}

void IocpTcpPoller::_ControlCloseSession(IocpTcpSession *session, Int32 closeReason, Int64 closeMillisecondTime, UInt64 stub, bool forbidRead, bool forbidWrite)
{
    // 判断是否已经关闭过
    if(session->WillSessionClose())
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("session had close before status:will close later session:%s."), session->ToString().c_str());
        return;
    }

    const auto opCloseTime = LibTime::NowMilliTimestamp();
    g_Log->NetInfo(LOGFMT_OBJ_TAG("will local force close session:%s, opCloseTime:%lld")
                , session->ToString().c_str(), opCloseTime);

    if(forbidRead)
    {
        session->ForbidAccept();
        session->ForbidRecv();
    }

    if(forbidWrite)
        session->ForbidSend();
    session->MaskClose(closeReason);

    const auto sessionId = session->GetId();
    if(closeMillisecondTime)
    {
        auto newTimer = LibTimer::NewThreadLocal_LibTimer(_poller->GetTimerMgr());
        auto __delayCloseSession = [this, sessionId, stub, opCloseTime, closeReason](LibTimer *delayCloseTimer)->void 
        {
            do
            {
                auto session = _GetSession(sessionId);
                if(!session)
                {
                    g_Log->NetWarn(LOGFMT_OBJ_TAG("session had close before when delay close session sessionId:%llu, stub:%llu"), sessionId, stub);
                    break;
                }

                g_Log->NetInfo(LOGFMT_OBJ_TAG("session%s delay close timeout, opCloseTime:%lld, realCloseTime:%lld")
                                , session->ToString().c_str(), opCloseTime, LibTime::NowMilliTimestamp());

                _TryCloseSession(session, closeReason, stub);

            } while (false);
            
            LibTimer::DeleteThreadLocal_LibTimer(delayCloseTimer);
        };

        auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(__delayCloseSession, void, LibTimer *);
        newTimer->SetTimeOutHandler(delg);
        const Int64 delayMilliseconds = (opCloseTime > closeMillisecondTime) ? 0 : (closeMillisecondTime - opCloseTime);
        newTimer->Schedule(delayMilliseconds);

        g_Log->NetInfo(LOGFMT_OBJ_TAG("will close a session delay milliseconds:%lld, sessionId:%llu"), delayMilliseconds, sessionId);
        return;
    }

    _TryCloseSession(session, closeReason, stub);
}

bool IocpTcpPoller::_TryHandleConnecting(IoEvent &io, Int32 ioErrCode)
{
    auto ioData = io._ioData;
    if(UNLIKELY(!ioData))
        return false;

    if(ioData->_ioType != IoEventType::IO_CONNECT)
        return false;

    auto iter = _sessionIdRefAsynConnectPendingInfo.find(io._sessionId);
    if(iter == _sessionIdRefAsynConnectPendingInfo.end())
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("connect pending info removed before sessionId:%llu, io event :%s")
                        , io._sessionId, io.ToString().c_str());
        return true;
    }

    auto connectPendingInfo = iter->second;
    if(ioErrCode != Status::Success)
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("try handle connecting fail io err code:%d, io event:%s"), ioErrCode, io.ToString().c_str());

        // 没有超时重试就直接失败
        if(connectPendingInfo->_reconnectTimer == NULL)
            _OnConnectFailure(connectPendingInfo->_connectInfo, connectPendingInfo, ioErrCode);

        return true;
    }

    _OnConnectSuc(connectPendingInfo);

    return true;
}

Int32 IocpTcpPoller::_OnHostInit()
{
    _iocp = CRYSTAL_NEW(LibIocp);
    _pollerMgr = _tcpPollerMgr->GetOwner()->CastTo<IPollerMgr>();
    _serviceProxy = _tcpPollerMgr->GetServiceProxy();

    _monitor = CRYSTAL_NEW(LibThread);
    _monitor->AddTask(this, &IocpTcpPoller::_OnMonitorThread);
    _eventLoopThread = CRYSTAL_NEW(LibThread);
    _eventLoopThread->AddTask(this, &IocpTcpPoller::_OnPollEventLoop);

    _pollerInstMonitorPriorityLevel = _cfg->_pollerInstMonitorPriorityLevel < 0 ? _cfg->_maxPriorityLevel : _cfg->_pollerInstMonitorPriorityLevel;

    g_Log->NetInfo(LOGFMT_OBJ_TAG("iocp tcp poller inited."));

    return Status::Success;
}

Int32 IocpTcpPoller::_OnCompsCreated()
{
    _poller = GetComp<Poller>();
    _memoryCleaner = GetComp<TlsMemoryCleanerComp>();

    TimeSlice span(0, 0, _cfg->_maxPieceTimeInMicroseconds);
    _poller->SetMaxPriorityLevel(_cfg->_maxPriorityLevel);
    _poller->SetMaxPieceTime(span);
    _poller->SetMaxSleepMilliseconds(_cfg->_maxSleepMilliseconds);
    _poller->SetPepareEventWorkerHandler(this, &IocpTcpPoller::_OnPollerPrepare);
    _poller->SetEventWorkerCloseHandler(this, &IocpTcpPoller::_OnPollerWillDestroy);

    _poller->Subscribe(PollerEventType::Write, this, &IocpTcpPoller::_OnWrite);
    _poller->Subscribe(PollerEventType::AsynConnect, this, &IocpTcpPoller::_OnAsynConnect);
    _poller->Subscribe(PollerEventType::NewSession, this, &IocpTcpPoller::_OnNewSession);
    _poller->Subscribe(PollerEventType::Monitor, this, &IocpTcpPoller::_OnMonitor);
    _poller->Subscribe(PollerEventType::CloseSession, this, &IocpTcpPoller::_OnCloseSession);
    _poller->Subscribe(PollerEventType::AddListen, this, &IocpTcpPoller::_OnAddListen);
    _poller->Subscribe(PollerEventType::IpRuleControl, this, &IocpTcpPoller::_OnIpRuleControl);
    _poller->Subscribe(PollerEventType::QuitServiceSessionsEvent, this, &IocpTcpPoller::_OnQuitServiceSessionsEvent);
    _poller->Subscribe(PollerEventType::RealDoQuitServiceSessionEvent, this, &IocpTcpPoller::_OnRealDoQuitServiceSessionEvent);

    // 脏回调
    auto dirtyHelper = _poller->GetDirtyHelper();
    dirtyHelper->Init(PollerDirty::END);
    auto deleg = DelegateFactory::Create(this, &IocpTcpPoller::_OnDirtySessionRead);
    dirtyHelper->SetHandler(PollerDirty::READ, deleg);
    deleg = DelegateFactory::Create(this, &IocpTcpPoller::_OnDirtySessionWrite);
    dirtyHelper->SetHandler(PollerDirty::WRITE, deleg);
    deleg = DelegateFactory::Create(this, &IocpTcpPoller::_OnDirtySessionClose);
    dirtyHelper->SetHandler(PollerDirty::CLOSE, deleg);
    deleg = DelegateFactory::Create(this, &IocpTcpPoller::_OnDirtySessionAccept);
    dirtyHelper->SetHandler(PollerDirty::ACCEPT, deleg);

    // ip rule mgr 设置
    auto ipRuleMgr = GetComp<IpRuleMgr>();
    auto config = _pollerMgr->GetConfig();
    if (!ipRuleMgr->SetBlackWhiteListFlag(config->_blackWhiteListFlag))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("SetBlackWhiteListFlag fail black white list flag:%u"), config->_blackWhiteListFlag);
        if (GetOwner())
            GetOwner()->SetErrCode(this, Status::Failed);
        return Status::Failed;
    }

    #if _DEBUG
        _memoryCleaner->SetIntervalMs(1000);
    #endif

    // 手动启动
    _memoryCleaner->SetTimerMgr(_poller->GetTimerMgr());
    _memoryCleaner->SetManualStart();

    g_Log->NetInfo(LOGFMT_OBJ_TAG("iocp comps created suc %s."), ToString().c_str());
    return Status::Success;
}

Int32 IocpTcpPoller::_OnHostWillStart()
{
    g_Log->NetInfo(LOGFMT_OBJ_TAG("iocp tcp poller will start."));
    return Status::Success;
}

Int32 IocpTcpPoller::_OnHostStart()
{
    _monitor->Start();
    _eventLoopThread->Start();
    g_Log->NetInfo(LOGFMT_OBJ_TAG("iocp tcp poller start."));
    return Status::Success;
}

void IocpTcpPoller::_OnHostBeforeCompsWillClose()
{
    g_Log->NetInfo(LOGFMT_OBJ_TAG("iocp tcp poller before comps will close."));
    // 先关闭iocp worker线程
    if(_monitor && _monitor->HalfClose())
    {
        // 事件唤醒iocp worker线程
        if(_quitIocpWorker)
            _quitIocpWorker->Invoke();

        _monitor->FinishClose();
    }

    if(_poller)
        _poller->QuitLoop();
    if(_eventLoopThread)
        _eventLoopThread->Close();
}

void IocpTcpPoller::_OnHostWillClose()
{
    g_Log->NetInfo(LOGFMT_OBJ_TAG("iocp tcp poller will close."));
}

void IocpTcpPoller::_OnHostClose()
{
    g_Log->NetInfo(LOGFMT_OBJ_TAG("iocp tcp poller closed."));
    _Clear();
}

void IocpTcpPoller::_Clear()
{
    CRYSTAL_RELEASE_SAFE(_quitIocpWorker);
    CRYSTAL_DELETE_SAFE(_iocp);
    CRYSTAL_DELETE_SAFE(_monitor);
    CRYSTAL_DELETE_SAFE(_eventLoopThread);
}

bool IocpTcpPoller::_OnPollerPrepare(Poller *poller)
{
    const auto workerThreadId = poller->GetWorkerThreadId();

    Int32 err = _memoryCleaner->ManualStart();
    if(err != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("fail memory cleaner ManualStart, err = [%d], poller worker threadId = [%llu]")
                        , err, workerThreadId);

        SetErrCode(poller, err);
        return false;
    }

    err = _iocp->Create();
    if(err != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("fail create iocp. err = [%d], poller worker threadId = [%llu]")
                        , err, workerThreadId);

        SetErrCode(poller, err);
        return false;
    }

    // wakeup sessionId
    _wakeupSessionId = _pollerMgr->NewSessionId();

    // 退出回调
    auto quitIocplambda = [this]()->void {
        auto ret = _iocp->PostQuit(_wakeupSessionId);
        if(ret != Status::Success)
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("iocp post quit fail ret:%d, wakeupSessionId:%llu"), ret, _wakeupSessionId);
        }
    };
    _quitIocpWorker = KERNEL_CREATE_CLOSURE_DELEGATE(quitIocplambda, void);

    return true;
}

void IocpTcpPoller::_OnPollerWillDestroy(Poller *poller)
{
    ContainerUtil::DelContainer(_sessionIdRefSession, [](IocpTcpSession *&session)
    {
        session->Close();
        IocpTcpSession::DeleteThreadLocal_IocpTcpSession(session);
        session = NULL;
    });

    ContainerUtil::DelContainer(_sessionIdRefAsynConnectPendingInfo, [this](LibConnectPendingInfo *&pendingInfo)
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("destroy session pending connect info when poller destroy:%s"), pendingInfo->ToString().c_str());
        if(pendingInfo->_connectInfo)
        {
            LibConnectInfo::Delete_LibConnectInfo(pendingInfo->_connectInfo);
            pendingInfo->_connectInfo = NULL;
        }
        if(pendingInfo->_reconnectTimer)
        {
            LibTimer::DeleteThreadLocal_LibTimer(pendingInfo->_reconnectTimer);
            pendingInfo->_reconnectTimer = NULL;
        }

        LibConnectPendingInfo::DeleteThreadLocal_LibConnectPendingInfo(pendingInfo);
        pendingInfo = NULL;
        --_sessionPendingCount;
    });

    _memoryCleaner->ManualClose();
}

void IocpTcpPoller::_OnWrite(PollerEvent *ev)
{
    auto sendEv = ev->CastTo<AsynSendEvent>();
    SmartPtr<LibList<LibPacket *>, AutoDelMethods::CustomDelete> packets = sendEv->_packets;
    auto closure = [this](void *ptr)
    {
        if(UNLIKELY(!ptr))
            return;

        auto packetList = KernelCastTo<LibList<LibPacket *>>(ptr);
        ContainerUtil::DelContainer(*packetList, [this](LibPacket *&packet)
        {
            g_Log->NetWarn(LOGFMT_OBJ_TAG("packet not send packet:%s"), packet->ToString().c_str());
            packet->ReleaseUsingPool();
            packet = NULL;
        });

        LibList<LibPacket *>::Delete_LibList(packetList);
        packetList = NULL;
        ptr = NULL;
    };
    sendEv->_packets = NULL;

    packets.SetClosureDelegate(closure);

    auto session = _GetSession(sendEv->_sessionId);
    if(!session)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("session not exists event:%s"), sendEv->ToString().c_str());
        return;
    }

    if(UNLIKELY(session->IsLinker()))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("cant post send to link session event:%s"), sendEv->ToString().c_str());
        return;
    }

    session->SendPackets(packets.pop());
    session->ResetFrameSendHandleBytes();
}

void IocpTcpPoller::_OnAsynConnect(PollerEvent *ev)
{
    AsynConnectEvent *connectEv = ev->CastTo<AsynConnectEvent>();
    SmartPtr<LibConnectInfo, AutoDelMethods::CustomDelete> connectInfo = connectEv->_connectInfo;
    connectInfo.SetClosureDelegate([](void *ptr){
        auto connectInfo = KernelCastTo<LibConnectInfo>(ptr);
        LibConnectInfo::Delete_LibConnectInfo(connectInfo);
    });
    g_Log->NetInfo(LOGFMT_OBJ_TAG("recv a connect event: :%s"), connectEv->ToString().c_str());

    Int32 errCode = Status::Success;
    auto newPending = _CreateNewConectPendingInfo(connectInfo, connectInfo->_retryTimes);
    if(!newPending)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("_CreateNewConectPendingInfo fail connect info:%s"), connectInfo->ToString().c_str());
        _OnConnectFailure(connectInfo.pop(), NULL, Status::Error);
        return;
    }

    bool giveup = false;
    errCode = _CheckConnect(newPending, giveup);
    if((errCode != Status::Success) && 
        (errCode != Status::SockError_Pending) && 
        ((newPending->_leftRetryTimes <= 0) || giveup))
    {
        _OnConnectFailure(connectInfo.pop(), newPending, errCode);
        return;
    }

    {// 定时重连
        auto connectInfoCache = connectInfo.pop();
        
        if(newPending->_leftRetryTimes > 0)
        {
            newPending->_reconnectTimer = LibTimer::NewThreadLocal_LibTimer(_poller->GetTimerMgr());
            auto __tryAgainTimeOut = [connectInfoCache, this](LibTimer *timer) mutable -> void 
            {
                // pending
                auto newPending = timer->GetParams()[1].AsPtr<LibConnectPendingInfo>();

                // 次数不够
                if(newPending->_leftRetryTimes <= 0)
                {// 没次数
                    g_Log->NetError(LOGFMT_OBJ_TAG("%s try connect fail by trying many times!"), newPending->ToString().c_str());
                    _OnConnectFailure(connectInfoCache, newPending, Status::Socket_ConnectTimeOut);
                    return;
                }

                // 移除上一次pending
                const auto leftRetryTimes = --newPending->_leftRetryTimes;
                auto connectInfo = newPending->_connectInfo;
                newPending->_reconnectTimer = NULL;
                _DestroyConnect(newPending, false);

                newPending = _CreateNewConectPendingInfo(connectInfo, leftRetryTimes);
                if(!newPending)
                {
                    g_Log->NetError(LOGFMT_OBJ_TAG("try again time out _CreateNewConectPendingInfo fail connect info:%s"), connectInfo->ToString().c_str());
                    _OnConnectFailure(connectInfoCache, NULL, Status::Error);
                    LibTimer::DeleteThreadLocal_LibTimer(timer);
                    return;
                }
                timer->GetParams().BecomeDict()[1] = newPending;

                // 连接
                newPending->_reconnectTimer = timer;
                bool giveup = false;
                auto errCode = _CheckConnect(newPending, giveup);
                if((errCode != Status::Success) && 
                    (errCode != Status::SockError_Pending) && 
                    ((newPending->_leftRetryTimes <= 0) || giveup))
                {// 次数不足失败
                    _OnConnectFailure(connectInfoCache, newPending, errCode);
                    return;
                }

                // 重新定时
                g_Log->NetWarn(LOGFMT_OBJ_TAG("try again time out try connect again connect pending info:%s"), newPending->ToString().c_str());
            };

            auto delg = DelegateFactory::Create<decltype(__tryAgainTimeOut), void, LibTimer *>(__tryAgainTimeOut);
            newPending->_reconnectTimer->GetParams().BecomeDict()[1] = newPending;
            newPending->_reconnectTimer->SetTimeOutHandler(delg);
            newPending->_reconnectTimer->Schedule(connectInfoCache->_periodMs);
        }
        
        g_Log->NetInfo(LOGFMT_OBJ_TAG("%s waiting for connecting success..."), newPending->ToString().c_str());
    }
}

void IocpTcpPoller::_OnNewSession(PollerEvent *ev)
{
    auto newSessionEv = ev->CastTo<NewSessionEvent>();
    SmartPtr<BuildSessionInfo, AutoDelMethods::CustomDelete> buildSessionInfo = newSessionEv->_buildInfo;
    buildSessionInfo.SetClosureDelegate([](void *buildInfo){
        BuildSessionInfo::Delete_BuildSessionInfo(KernelCastTo<BuildSessionInfo>(buildInfo));
        buildInfo = NULL;
    });

    auto newSession = _CreateSession(buildSessionInfo);
    if(UNLIKELY(!newSession))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("create session failure by build session info:%s"), buildSessionInfo->ToString().c_str());

        // 有存根才需要回包
        if(buildSessionInfo->_stub)
        {
            if(buildSessionInfo->_isFromConnect)
            {// connector必须要connectres
                auto connectRes = AsynConnectResEvent::New_AsynConnectResEvent();
                connectRes->_errCode = Status::SockError_CreateSessionFail;
                connectRes->_localAddr = buildSessionInfo->_localAddr;
                connectRes->_targetAddr = buildSessionInfo->_remoteAddr;
                connectRes->_family = buildSessionInfo->_family;
                connectRes->_protocolType = buildSessionInfo->_protocolType;
                connectRes->_priorityLevel = buildSessionInfo->_priorityLevel;
                connectRes->_sessionPollerId = GetPollerId();
                connectRes->_fromServiceId = buildSessionInfo->_serviceId;
                connectRes->_stub = buildSessionInfo->_stub;
                _serviceProxy->PostMsg(connectRes->_fromServiceId, connectRes->_priorityLevel, connectRes);
            }
            else if(buildSessionInfo->_isLinker)
            {// TODO:
                auto listenRes = AddListenResEvent::New_AddListenResEvent();
                listenRes->_errCode = Status::SockError_CreateSessionFail;
                listenRes->_localAddr = buildSessionInfo->_localAddr;
                listenRes->_family = buildSessionInfo->_family;
                listenRes->_serviceId = buildSessionInfo->_serviceId;
                listenRes->_stub = buildSessionInfo->_stub;
                listenRes->_priorityLevel = buildSessionInfo->_priorityLevel;
                listenRes->_protocolType = buildSessionInfo->_protocolType;
                _serviceProxy->PostMsg(listenRes->_serviceId, listenRes->_priorityLevel, listenRes);
            }
        }
        
        SocketUtil::DestroySocket(buildSessionInfo->_sock);
        return;
    }

    // session创建消息
    auto sessionCreatedEv = SessionCreatedEvent::New_SessionCreatedEvent();
    sessionCreatedEv->_sessionId = buildSessionInfo->_sessionId;
    sessionCreatedEv->_localAddr = buildSessionInfo->_localAddr;
    sessionCreatedEv->_targetAddr = buildSessionInfo->_remoteAddr;
    sessionCreatedEv->_family = buildSessionInfo->_family;
    sessionCreatedEv->_protocolType = buildSessionInfo->_protocolType;
    sessionCreatedEv->_priorityLevel = buildSessionInfo->_priorityLevel;
    sessionCreatedEv->_sessionType = buildSessionInfo->_sessionOption._sessionType;
    sessionCreatedEv->_sessionPollerId = GetPollerId();
    sessionCreatedEv->_belongServiceId = buildSessionInfo->_serviceId;
    sessionCreatedEv->_stub = buildSessionInfo->_stub;
    sessionCreatedEv->_isFromConnect = buildSessionInfo->_isFromConnect;
    sessionCreatedEv->_isLinker = buildSessionInfo->_isLinker;
    sessionCreatedEv->_protocolStackType = buildSessionInfo->_sessionOption._protocolStackType;

    _serviceProxy->PostMsg(sessionCreatedEv->_belongServiceId, sessionCreatedEv->_priorityLevel, sessionCreatedEv);

    // 有存根才需要回包
    if(buildSessionInfo->_stub)
    {
        if(buildSessionInfo->_isFromConnect)
        {// connector必须要connectres
            auto connectRes = AsynConnectResEvent::New_AsynConnectResEvent();
            connectRes->_errCode = Status::Success;
            connectRes->_localAddr = buildSessionInfo->_localAddr;
            connectRes->_targetAddr = buildSessionInfo->_remoteAddr;
            connectRes->_family = buildSessionInfo->_family;
            connectRes->_protocolType = buildSessionInfo->_protocolType;
            connectRes->_priorityLevel = buildSessionInfo->_priorityLevel;
            connectRes->_sessionPollerId = GetPollerId();
            connectRes->_fromServiceId = buildSessionInfo->_serviceId;
            connectRes->_stub = buildSessionInfo->_stub;
            connectRes->_sessionId = buildSessionInfo->_sessionId;
            _serviceProxy->PostMsg(connectRes->_fromServiceId, connectRes->_priorityLevel, connectRes);
        }
        else if(buildSessionInfo->_isLinker)
        {// TODO:
            auto listenRes = AddListenResEvent::New_AddListenResEvent();
            listenRes->_errCode = Status::Success;
            listenRes->_sessionId = buildSessionInfo->_sessionId;
            listenRes->_localAddr = buildSessionInfo->_localAddr;
            listenRes->_family = buildSessionInfo->_family;
            listenRes->_serviceId = buildSessionInfo->_serviceId;
            listenRes->_stub = buildSessionInfo->_stub;
            listenRes->_priorityLevel = buildSessionInfo->_priorityLevel;
            listenRes->_protocolType = buildSessionInfo->_protocolType;
            _serviceProxy->PostMsg(listenRes->_serviceId, listenRes->_priorityLevel, listenRes);
        }
    }

    g_Log->NetInfo(LOGFMT_OBJ_TAG("new session created suc poller id:%llu, session amount:%llu, session info:%s")
                    , _pollerId, _sessionIdRefSession.size(), newSession->ToString().c_str());
    // g_Log->NetDebug(LOGFMT_OBJ_TAG("new session created suc poller id:%llu, session amount:%llu, session info:%s")
    //                 , _pollerId, _sessionIdRefSession.size(), newSession->ToString().c_str());
}

void IocpTcpPoller::_OnMonitor(PollerEvent *ev)
{
    auto monitorEv = ev->CastTo<MonitorPollerEvent>();
    auto &ioEvent = monitorEv->_io;
    const Int32 ioErrCode = monitorEv->_errCode;

    do
    {
        if(_TryHandleConnecting(ioEvent, ioErrCode))
            break;

        auto session = _GetSession(ioEvent._sessionId);
        if(!session)
        {
            g_Log->NetWarn(LOGFMT_OBJ_TAG("unknown session id:%llu,"), ioEvent._sessionId);
            break;
        }

        if(ioErrCode != Status::Success)
        {
            if(!session->HasDataToRecv())
                session->ForbidRecv();
            session->ForbidSend();

            // TODO:先打标记，远程断开，还不明确数据还能不能继续读取
            g_Log->NetWarn(LOGFMT_OBJ_TAG("SOCK_ERR session id=[%llu], iocp poller error sessionId:%llu ioErrCode:%d, io event:%s")
                                    , ioEvent._sessionId, ioErrCode, ioEvent.ToString().c_str());
            
            if(!session->WillSessionClose())
                _TryCloseSession(session, CloseSessionInfo::SOCK_ERR, 0);
            break;
        }

        // 监听
        if(session->IsLinker())
        {
            _OnAccept(session, ioEvent);
            break;
        }

        // 数据传输
        const Int32 ioType = ioEvent._ioData->_ioType;
        // g_Log->NetDebug(LOGFMT_OBJ_TAG("net data transfer io event:%s"), ioEvent.ToString().c_str());

        switch (ioType)
        {
        case IoEventType::IO_RECV:
        {
            session->OnRecv(ioEvent);
            session->ResetFrameRecvHandleBytes();
        }
            break;
        case IoEventType::IO_SEND:
        {
            session->OnSend(ioEvent);
            session->ResetFrameSendHandleBytes();
        }
            break;
        default:
            g_Log->NetError(LOGFMT_OBJ_TAG("bad iotype:%d, sessionid:%llu"), ioType, ioEvent._sessionId);
            break;
        }
    } while (false);
    
    // 有iodata需要释放iodata
    if(ioEvent._ioData)
    {
        if(ioEvent._ioData->_tlStream)
        {
            LibStreamTL::DeleteThreadLocal_LibStream(ioEvent._ioData->_tlStream);
            ioEvent._ioData->_tlStream = NULL;
        }

        IoData::DeleteThreadLocal_IoData(ioEvent._ioData);
        ioEvent._ioData = NULL;
    }
}

void IocpTcpPoller::_OnCloseSession(PollerEvent *ev)
{
    auto closeSessionEv = ev->CastTo<CloseSessionEvent>();
    auto session = _GetSession(closeSessionEv->_sessionId);
    if(UNLIKELY(!session))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("session not exists or is closed before ev:%s"), closeSessionEv->ToString().c_str());
        
        // session关闭的消息
        if(closeSessionEv->_stub)
        {
            auto sessionDestroyEv = SessionDestroyEvent::New_SessionDestroyEvent();
            sessionDestroyEv->_closeReason = CloseSessionInfo::CLOSED_BEFORE;
            sessionDestroyEv->_sessionId = closeSessionEv->_sessionId;
            sessionDestroyEv->_serviceId = closeSessionEv->_fromServiceId;
            sessionDestroyEv->_priorityLevel = closeSessionEv->_priorityLevel;
            sessionDestroyEv->_stub = closeSessionEv->_stub;
            _serviceProxy->PostMsg(closeSessionEv->_fromServiceId, closeSessionEv->_priorityLevel, sessionDestroyEv);
        }

        return;
    }

    _ControlCloseSession(session, CloseSessionInfo::LOCAL_FORCE_CLOSE, closeSessionEv->_closeMillisecondTime, closeSessionEv->_stub, closeSessionEv->_forbidRead, closeSessionEv->_forbidWrite);
}

void IocpTcpPoller::_OnAddListen(PollerEvent *ev)
{
    auto listenEvent = ev->CastTo<AddListenEvent>();
    auto &linstenInfoList = listenEvent->_addListenInfoList;
    for(auto listenInfo : linstenInfoList)
    {
        // ip地址
        if(!SocketUtil::IsIp(listenInfo->_ip))
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("bad ip :%s, listenInfo:%s"), listenInfo->_ip.c_str(), listenInfo->ToString().c_str());
            if(listenInfo->_stub)
            {
                auto listenRes = AddListenResEvent::New_AddListenResEvent();
                listenRes->_errCode = Status::SockError_IllegalIp;
                listenRes->_family = listenInfo->_family;
                listenRes->_serviceId = listenInfo->_serviceId;
                listenRes->_stub = listenInfo->_stub;
                listenRes->_priorityLevel = listenInfo->_priorityLevel;
                listenRes->_protocolType = listenInfo->_protocolType;
                _serviceProxy->PostMsg(listenRes->_serviceId, listenRes->_priorityLevel, listenRes);
            }

            continue;
        }

        auto newSession = _CreateSession(listenInfo);
        if(!newSession)
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("create session fail listen info:%s"), listenInfo->ToString().c_str());
            
            if(listenInfo->_stub)
            {
                auto listenRes = AddListenResEvent::New_AddListenResEvent();
                listenRes->_errCode = Status::SockError_CreateSessionFail;
                listenRes->_family = listenInfo->_family;
                listenRes->_serviceId = listenInfo->_serviceId;
                listenRes->_stub = listenInfo->_stub;
                listenRes->_priorityLevel = listenInfo->_priorityLevel;
                listenRes->_protocolType = listenInfo->_protocolType;
                _serviceProxy->PostMsg(listenRes->_serviceId, listenRes->_priorityLevel, listenRes);
            }
            continue;
        }
        
        // session创建消息
        auto sock = newSession->GetSock();
        auto addr = sock->GetAddr();
        auto sessionCreatedEv = SessionCreatedEvent::New_SessionCreatedEvent();
        sessionCreatedEv->_sessionId = newSession->GetId();
        sessionCreatedEv->_localAddr = addr->GetLocalBriefAddr();
        sessionCreatedEv->_targetAddr = addr->GetRemoteBriefAddr();
        sessionCreatedEv->_family = sock->GetFamily();
        sessionCreatedEv->_protocolType = listenInfo->_protocolType;
        sessionCreatedEv->_priorityLevel = listenInfo->_priorityLevel;
        sessionCreatedEv->_sessionType = listenInfo->_sessionOption._sessionType;
        sessionCreatedEv->_sessionPollerId = GetPollerId();
        sessionCreatedEv->_belongServiceId = listenInfo->_serviceId;
        sessionCreatedEv->_stub = listenInfo->_stub;
        sessionCreatedEv->_isFromConnect = false;
        sessionCreatedEv->_isLinker = true;
        sessionCreatedEv->_protocolStackType = listenInfo->_sessionOption._protocolStackType;

        _serviceProxy->PostMsg(sessionCreatedEv->_belongServiceId, sessionCreatedEv->_priorityLevel, sessionCreatedEv);

        // listenres
        if(listenInfo->_stub)
        {
            auto listenRes = AddListenResEvent::New_AddListenResEvent();
            listenRes->_errCode = Status::Success;
            listenRes->_localAddr = addr->GetLocalBriefAddr();
            listenRes->_family = sock->GetFamily();
            listenRes->_serviceId = listenInfo->_serviceId;
            listenRes->_stub = listenInfo->_stub;
            listenRes->_priorityLevel = listenInfo->_priorityLevel;
            listenRes->_protocolType = listenInfo->_protocolType;
            listenRes->_sessionId = newSession->GetId();
            _serviceProxy->PostMsg(listenRes->_serviceId, listenRes->_priorityLevel, listenRes);
        }

        g_Log->NetInfo(LOGFMT_OBJ_TAG("add listen :%s, session info:%s")
                    , listenInfo->ToString().c_str(), newSession->ToString().c_str());
    }

    ContainerUtil::DelContainer(linstenInfoList, [](LibListenInfo *&listenInfo){
        LibListenInfo::Delete_LibListenInfo(listenInfo);
        listenInfo = NULL;
    });
}

void IocpTcpPoller::_OnIpRuleControl(PollerEvent *ev)
{
    auto ipCtrlEv = ev->CastTo<IpRuleControlEvent>();
    if(UNLIKELY(ipCtrlEv->_ipControlList.empty()))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("ip rule control list empty poller id:%llu"), GetPollerId());
        return;
    }

    g_Log->NetDebug(LOGFMT_OBJ_TAG("ip rule control:%s"), ipCtrlEv->ToString().c_str());

    auto ipRuleMgr = GetComp<IpRuleMgr>();
    auto &ctrlList = ipCtrlEv->_ipControlList;
    for(auto ctrlInfo : ctrlList)
    {
        if(!SocketUtil::IsIp(ctrlInfo->_ip))
        {
            g_Log->NetWarn(LOGFMT_OBJ_TAG("invalid ip:%s"), ctrlInfo->_ip.c_str());
            continue;
        }

        for(auto flag : ctrlInfo->_controlFlow)
        {
            if (flag == IpControlInfo::ADD_WHITE)
                ipRuleMgr->PushWhite(ctrlInfo->_ip);
            else if (flag == IpControlInfo::ADD_BLACK)
                ipRuleMgr->PushBlack(ctrlInfo->_ip);
            else if (flag == IpControlInfo::ERASE_WHITE)
                ipRuleMgr->EraseWhite(ctrlInfo->_ip);
            else if(flag == IpControlInfo::ERASE_BLACK)
                ipRuleMgr->EraseBlack(ctrlInfo->_ip);
        }

        // 检验ip
        if(!ipRuleMgr->Check(ctrlInfo->_ip))
        {
            // 延迟1ms关闭
            auto &sessions = _GetSessionsByIp(ctrlInfo->_ip);
            for(auto session : sessions)
                _ControlCloseSession(session, CloseSessionInfo::BY_BLACK_WHITE_LIST_CHECK, 1, 0);
        }
    }
}

void IocpTcpPoller::_OnQuitServiceSessionsEvent(PollerEvent *ev)
{
    auto quiteSessionEv = ev->CastTo<QuitServiceSessionsEvent>();
    std::map<UInt32, std::set<UInt64>> sessions;
    for(auto iter : _sessionIdRefSession)
    {
        if(iter.second->GetServiceId() == quiteSessionEv->_fromServiceId)
        {
            auto session = iter.second;
            auto iterSessions = sessions.find(session->GetPriorityLevel());
            if(iterSessions == sessions.end())
                iterSessions = sessions.insert(std::make_pair(session->GetPriorityLevel(), std::set<UInt64>())).first;

            auto &sessionSet = iterSessions->second;
            sessionSet.insert(session->GetId());
        }
    }

    g_Log->NetInfo(LOGFMT_OBJ_TAG("will quit service session service id:%llu priorityLevel count:%llu, current poller id:%llu")
            , quiteSessionEv->_fromServiceId, static_cast<UInt64>(sessions.size()), GetPollerId());

    for(auto iter : sessions)
    {
        auto level = iter.first;
        auto &sessionIds = iter.second;

        auto newEv = RealDoQuitServiceSessionEvent::New_RealDoQuitServiceSessionEvent();
        newEv->_fromServiceId = quiteSessionEv->_fromServiceId;

        for(auto sessionId : sessionIds)
        {
            auto quitSessionInfo = QuitSessionInfo::New_QuitSessionInfo();
            quitSessionInfo->_sessionId = sessionId;
            quitSessionInfo->_priorityLevel = level;
            newEv->_quitSessionInfo->PushBack(quitSessionInfo);
        }

        _poller->Push(level, newEv);
    }
}

void IocpTcpPoller::_OnRealDoQuitServiceSessionEvent(PollerEvent *ev)
{
    auto quitSessionEv = ev->CastTo<RealDoQuitServiceSessionEvent>();
    std::set<IocpTcpSession *> sessions;
    for(auto node = quitSessionEv->_quitSessionInfo->Begin(); node;)
    {
        auto quitSessionInfo = node->_data;
        auto session = _GetSession(quitSessionInfo->_sessionId);
        if(session)
            sessions.insert(session);
            
        QuitSessionInfo::Delete_QuitSessionInfo(quitSessionInfo);
        node = quitSessionEv->_quitSessionInfo->Erase(node);
    }

    g_Log->NetInfo(LOGFMT_OBJ_TAG("real do quit service session service id:%llu, session count:%llu, current poller id:%llu")
            , quitSessionEv->_fromServiceId, static_cast<UInt64>(sessions.size()), GetPollerId());

    for(auto session : sessions)
        _ControlCloseSession(session, CloseSessionInfo::LOCAL_FORCE_CLOSE, 0, 0, true, false);
}

void IocpTcpPoller::_OnConnectSuc(LibConnectPendingInfo *&connectPendingInfo)
{
    g_Log->NetDebug(LOGFMT_OBJ_TAG("connect suc pending info:%s"), connectPendingInfo->ToString().c_str());

    SocketUtil::UpdateConnectContext(connectPendingInfo->_newSock);

    // 更新本地地址
    auto connectInfo = connectPendingInfo->_connectInfo;
    const auto family = connectInfo->_family;
    const bool isIpv4 = AF_INET == family;
    auto newBuildSessionInfo = BuildSessionInfo::New_BuildSessionInfo(isIpv4);
    newBuildSessionInfo->_protocolType = connectInfo->_protocolType;
    newBuildSessionInfo->_family = family;
    newBuildSessionInfo->_sessionId = connectPendingInfo->_sessionId;
    SocketUtil::UpdateLocalAddr(connectPendingInfo->_newSock, newBuildSessionInfo->_localAddr);
    SocketUtil::UpdateRemoteAddr(family, connectPendingInfo->_newSock, newBuildSessionInfo->_remoteAddr);
    newBuildSessionInfo->_sock = connectPendingInfo->_newSock;
    newBuildSessionInfo->_serviceId = connectInfo->_fromServiceId;
    newBuildSessionInfo->_stub = connectInfo->_stub;
    newBuildSessionInfo->_priorityLevel = connectInfo->_priorityLevel;
    newBuildSessionInfo->_isFromConnect = true;
    newBuildSessionInfo->_protocolStack = connectInfo->_stack;
    newBuildSessionInfo->_sessionOption = connectInfo->_sessionOption;

    _sessionIdRefAsynConnectPendingInfo.erase(connectPendingInfo->_sessionId);
    --_sessionPendingCount;
    
    // windows下connect是在datatrasfer poller上
    PostNewSession(newBuildSessionInfo->_priorityLevel, newBuildSessionInfo);

    connectPendingInfo->_newSock = INVALID_SOCKET;
    connectPendingInfo->_sessionId = 0;
    _DestroyConnect(connectPendingInfo, true);
}

void IocpTcpPoller::_OnConnectFailure(LibConnectInfo *connectInfo, LibConnectPendingInfo *connectPending, Int32 errCode)
{
    g_Log->NetWarn(LOGFMT_OBJ_TAG("connect fail errCode:%d pending info:%s, connectInfo:%s")
                , errCode, connectPending ? connectPending->ToString().c_str() : "pending info is null" , connectInfo->ToString().c_str());

    // 失败的回执
    if(connectInfo->_stub)
    {
        auto res = AsynConnectResEvent::New_AsynConnectResEvent();
        res->_errCode = errCode;
        res->_family = connectInfo->_family;
        res->_protocolType = connectInfo->_protocolType;
        res->_priorityLevel = connectInfo->_priorityLevel;
        res->_fromServiceId = connectInfo->_fromServiceId;
        res->_stub = connectInfo->_stub;

        auto &localAddr = res->_localAddr;
        localAddr._ip = connectInfo->_localIp;
        localAddr._port = connectInfo->_localPort;
        localAddr._ipAndPort.AppendFormat("%s:%hu", localAddr._ip.c_str(), localAddr._port);
        auto &targetAddr = res->_targetAddr;
        targetAddr._ip = connectInfo->_targetIp;
        targetAddr._port = connectInfo->_targetPort;
        targetAddr._ipAndPort.AppendFormat("%s:%hu", targetAddr._ip.c_str(), targetAddr._port);
        _serviceProxy->PostMsg(res->_fromServiceId, res->_priorityLevel, res);
    }

    if(LIKELY(connectPending))
        _DestroyConnect(connectPending, true);
    else
        LibConnectInfo::Delete_LibConnectInfo(connectInfo);
}

void IocpTcpPoller::_OnAccept(IocpTcpSession *session, IoEvent &io)
{
    auto listenSock = session->GetSock();
    auto listenSockHandle = listenSock->GetSock();

    // iodata 不管成功与失败都需要移除
    SmartPtr<IoData, AutoDelMethods::CustomDelete> ioDataPtr = io._ioData;
    ioDataPtr.SetClosureDelegate([&io](void *ptr){
        auto ioData = KernelCastTo<IoData>(ptr);
        IoData::DeleteThreadLocal_IoData(ioData);
        ptr = NULL;
        io._ioData = NULL;
    });

    Int32 err = Status::Success;
    auto newSock = io._ioData->_sock;
    do
    {
        UInt64 totalSessionNum = 0;
        const UInt64 sessionQuantityLimit = _pollerMgr->GetSessionQuantityLimit();
        if(!_pollerMgr->CheckAddSessionPending(1, totalSessionNum))
        {
            g_Log->NetWarn(LOGFMT_OBJ_TAG("check add session pending fail, total session num:%llu, session quantity limit:%llu, accept session:%s, io event:%s")
                    , totalSessionNum, sessionQuantityLimit, session->ToString().c_str(), io.ToString().c_str());
            err = Status::Socket_SessionOverLimit;
            break;
        }

        err = SocketUtil::SetNoBlock(newSock);
        if(UNLIKELY(err != Status::Success))
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("set no block fail when accept a new sock:%d, err:%d, io event:%s")
                    , newSock, err, io.ToString().c_str());
            break;
        }

        err = SocketUtil::UpdateAcceptContext(listenSock->GetSock(), newSock);
        if(UNLIKELY(err != Status::Success))
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("UpdateAcceptContext fail when accept a new sock:%d, err:%d, io event:%s")
                    , newSock, err, io.ToString().c_str());
            break;
        }

        BriefSockAddr remoteAddr(true);
        SocketUtil::UpdateRemoteAddr(listenSock->GetFamily(), newSock, remoteAddr);

        // 检查ip
        auto ipRuleMgr = GetComp<IpRuleMgr>();
        if(!ipRuleMgr->Check(remoteAddr._ip))
        {
            g_Log->NetWarn(LOGFMT_OBJ_TAG("ip not check suc addr info:%s"), remoteAddr.ToString().c_str());
            err = Status::BlackWhiteCheckFail;
            break;
        }

        err = SocketUtil::MakeReUseAddr(newSock);
        if(err != Status::Success)
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("MakeReUseAddr fail io event:%s, err:%d, accept session:%s")
                , io.ToString().c_str(), err, session->ToString().c_str());
            break;
        }

        err = SocketUtil::MakeReUsePort(newSock);
        if(err != Status::Success)
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("MakeReUsePort fail io event:%s, err:%d, accept session:%s")
                    , io.ToString().c_str(), err, session->ToString().c_str());
            break;
        }

        err = SocketUtil::MakeNoDelay(newSock, true);
        if(err != Status::Success)
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("MakeNoDelay fail io event:%s, err:%d, accept session:%s")
                        , io.ToString().c_str(), err, session->ToString().c_str());
            break;
        }

        auto &sessionOption = session->GetOption();
        if(sessionOption._sockRecvBufferSize)
        {
            err = SocketUtil::SetRecvBufSize(newSock, sessionOption._sockRecvBufferSize);
            if(err != Status::Success)
            {
                g_Log->NetError(LOGFMT_OBJ_TAG("SetRecvBufSize fail io event:%s, err:%d, accept session:%s")
                            , io.ToString().c_str(), err, session->ToString().c_str());
                break;
            }
        }

        if(sessionOption._sockSendBufferSize)
        {
            err = SocketUtil::SetSendBufSize(newSock, sessionOption._sockSendBufferSize);
            if(err != Status::Success)
            {
                g_Log->NetError(LOGFMT_OBJ_TAG("SetSendBufSize fail io event:%s, err:%d, accept session:%s")
                                , io.ToString().c_str(), err, session->ToString().c_str());
                break;
            }
        }

    } while (false);

    if(err != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("listen session %s, accept new socket fail io event:%s, err:%d"), session->ToString().c_str(), io.ToString().c_str(), err);
        if(io._ioData)
            SocketUtil::DestroySocket(io._ioData->_sock);

        return;
    }

    const UInt64 newSessionId = _pollerMgr->NewSessionId();
    const auto family = listenSock->GetFamily();
    const bool isIpv4 = AF_INET == family;
    auto newBuildSessionInfo = BuildSessionInfo::New_BuildSessionInfo(isIpv4);
    newBuildSessionInfo->_protocolType = session->GetProtocolType();
    newBuildSessionInfo->_family = family;
    newBuildSessionInfo->_sessionId = newSessionId;
    SocketUtil::UpdateLocalAddr(newSock, newBuildSessionInfo->_localAddr);
    SocketUtil::UpdateRemoteAddr(family, newSock, newBuildSessionInfo->_remoteAddr);
    newBuildSessionInfo->_sock = newSock;
    newBuildSessionInfo->_serviceId = session->GetServiceId();
    newBuildSessionInfo->_stub = 0;
    newBuildSessionInfo->_priorityLevel = session->GetPriorityLevel();
    newBuildSessionInfo->_isFromConnect = false;
    newBuildSessionInfo->_protocolStack = NULL;
    newBuildSessionInfo->_sessionOption = session->GetOption();
    newBuildSessionInfo->_sessionOption._forbidRecv = false;
    _tcpPollerMgr->OnAcceptedSuc(newBuildSessionInfo);

    g_Log->NetInfo(LOGFMT_OBJ_TAG("accepted new session suc newBuildSessionInfo:%s, from accept session:%s.")
                    , newBuildSessionInfo->ToString().c_str(), session->ToString().c_str());

    // Post new async-accept request.
    err = listenSock->PostAsyncAccept();
    if(err != Status::Success)
        g_Log->NetError(LOGFMT_OBJ_TAG("listen sock post async accept fail err:%d, listen sock:%s"), err, listenSock->ToString().c_str());
}

// 脏事件
void IocpTcpPoller::_OnDirtySessionAccept(LibDirtyHelper<void *, UInt32> *dirtyHelper, void *&session, Variant *params)
{
    auto iocpSession = KernelCastTo<IocpTcpSession>(session);
    g_Log->NetWarn(LOGFMT_OBJ_TAG("iocp dont need accept dirty, session:%s"), iocpSession->ToString().c_str());
}

void IocpTcpPoller::_OnDirtySessionWrite(LibDirtyHelper<void *, UInt32> *dirtyHelper, void *&session, Variant *params)
{
    auto iocpSession = KernelCastTo<IocpTcpSession>(session);
    g_Log->NetWarn(LOGFMT_OBJ_TAG("dirty send session:%s"), iocpSession->ToString().c_str());
}

void IocpTcpPoller::_OnDirtySessionRead(LibDirtyHelper<void *, UInt32> *dirtyHelper, void *&session, Variant *params)
{
    auto iocpSession = KernelCastTo<IocpTcpSession>(session);
    // g_Log->NetDebug(LOGFMT_OBJ_TAG("dirty read session:%s"), iocpSession->ToString().c_str());
    iocpSession->ContinueRecv();
}

void IocpTcpPoller::_OnDirtySessionClose(LibDirtyHelper<void *, UInt32> *dirtyHelper, void *&session, Variant *params)
{
    auto iocpSession = KernelCastTo<IocpTcpSession>(session);

     auto closeReason = (*params)[1].AsInt32();
     UInt64 stub = 0;
     if (params->FindDict(2) != params->EndDict())
         stub = (*params)[2].AsUInt64();

     if (!iocpSession->HasDataToSend())
         iocpSession->ForbidSend();
     if (!iocpSession->HasDataToRecv())
         iocpSession->ForbidRecv();

     if(_CanClose(iocpSession))
     {
         g_Log->NetDebug(LOGFMT_OBJ_TAG("session close dirty do close session session info:%s, closeReason:%d, %s, stub:%llu")
                        , iocpSession->ToString().c_str(), closeReason, CloseSessionInfo::GetCloseReason(closeReason), stub);
         dirtyHelper->Clear(session, PollerDirty::CLOSE);
         _CloseSession(iocpSession, closeReason, stub);
         return;
     }

     g_Log->NetInfo(LOGFMT_OBJ_TAG("session will still mask close flag and close session in future closeReason:%d,%s, session info:%s.")
                    , closeReason, CloseSessionInfo::GetCloseReason(closeReason), iocpSession->ToString().c_str());
}

void IocpTcpPoller::_OnMonitorThread(LibThread *t)
{
 // 等待poller线程ready
    while(!IsReady())
    {
        SystemUtil::ThreadSleep(100);
        g_Log->NetWarn(LOGFMT_OBJ_TAG("waiting for iocp tcp poller ready..."));

        if(GetErrCode() != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("error happen:%d"), GetErrCode());
            return;
        }
    }

    g_Log->NetInfo(LOGFMT_OBJ_TAG("iocp tcp poller epoll monitor start threadid = [%llu]"), SystemUtil::GetCurrentThreadId());

    IoEvent io;
    Int32 errCode = Status::Success;
    while(!t->IsDestroy())
    {
        // epoll wait
        Int32 ret = _iocp->WaitForCompletion(io, errCode);
        if(ret == Status::Ignore)
        {
            g_Log->NetDebug(LOGFMT_OBJ_TAG("ignore iocpcompletion"));
            continue;
        }

        // 监听事件
        auto ev = MonitorPollerEvent::New_MonitorPollerEvent();
        ev->_io = io;
        ev->_errCode = errCode;
        _poller->Push(_pollerInstMonitorPriorityLevel, ev);
    }

    g_Log->NetInfo(LOGFMT_OBJ_TAG("iocp tcp poller epoll monitor monitor thread finish thread id = %llu"), SystemUtil::GetCurrentThreadId());
}

void IocpTcpPoller::_OnPollEventLoop(LibThread *t)
{
    auto defObj = TlsUtil::GetDefTls();
    if(UNLIKELY(defObj->_poller))
        g_Log->Warn(LOGFMT_OBJ_TAG("poller already existes int current thread please check:%p, will assign new poller:%p, thread id:%llu")
        , defObj->_poller, _poller, defObj->_threadId);
        
    defObj->_poller = _poller;
    defObj->_pollerTimerMgr = _poller->GetTimerMgr();

    g_Log->NetInfo(LOGFMT_OBJ_TAG("iocp tcp poller event loop start."));
    g_Log->NetInfo(LOGFMT_OBJ_TAG("epoll tcp poller event loop prepare loop..."));
    if(!_poller->PrepareLoop())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("prepare poller loop fail please check."));
        return;
    }

    MaskReady(true);
    g_Log->NetInfo(LOGFMT_OBJ_TAG("epoll tcp poller event loop start loop."));
    _poller->EventLoop();
    g_Log->NetInfo(LOGFMT_OBJ_TAG("epoll tcp poller event loop on loop end..."));
    _poller->OnLoopEnd();
    g_Log->NetInfo(LOGFMT_OBJ_TAG("epoll tcp poller event loop finish."));
    MaskReady(false);
}

void IocpTcpPoller::_DestroyConnect(LibConnectPendingInfo *&connectPendingInfo, bool destroyConnectInfo)
{
    g_Log->NetTrace(LOGFMT_OBJ_TAG("destroy connect pending %s"), connectPendingInfo->ToString().c_str());
    if(connectPendingInfo->_sessionId)
    {
        _pollerMgr->ReduceSessionPending(1);
        auto iter = _sessionIdRefAsynConnectPendingInfo.find(connectPendingInfo->_sessionId);
        if(iter != _sessionIdRefAsynConnectPendingInfo.end())
        {            
            _sessionIdRefAsynConnectPendingInfo.erase(iter);
            --_sessionPendingCount;
        }

        if(LIKELY(SocketUtil::IsValidSock(connectPendingInfo->_newSock)))
            SocketUtil::DestroySocket(connectPendingInfo->_newSock);

        connectPendingInfo->_newSock = INVALID_SOCKET;
        connectPendingInfo->_sessionId = 0;
    }

    if(connectPendingInfo->_reconnectTimer)
    {
        LibTimer::DeleteThreadLocal_LibTimer(connectPendingInfo->_reconnectTimer);
        connectPendingInfo->_reconnectTimer = NULL;
    }

    if(connectPendingInfo->_connectInfo && destroyConnectInfo)
        LibConnectInfo::Delete_LibConnectInfo(connectPendingInfo->_connectInfo);
    connectPendingInfo->_connectInfo = NULL;

    LibConnectPendingInfo::DeleteThreadLocal_LibConnectPendingInfo(connectPendingInfo);
    connectPendingInfo = NULL;  
}

Int32 IocpTcpPoller::_CheckConnect(LibConnectPendingInfo *&connectPendingInfo, bool &giveup)
{
     giveup = false;
    // 如果connectPending没移除则先移除
    if(UNLIKELY(connectPendingInfo->_sessionId))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("connect pending info had already effect. pending info:%s"), connectPendingInfo->ToString().c_str());
        return Status::Repeat;
    }

    auto connectInfo = connectPendingInfo->_connectInfo;
    g_Log->NetDebug(LOGFMT_OBJ_TAG("check connect :%s"), connectInfo->ToString().c_str());

    // ip合法性
    if(!SocketUtil::IsIp(connectInfo->_targetIp))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("illegal target ip:%s, stub:%llu, LibConnectInfo:%s")
            , connectInfo->_targetIp.c_str(), connectInfo->_stub, connectInfo->ToString().c_str());
        giveup = true;
        return Status::SockError_IllegalIp;
    }

    // 本地ip
    if(!connectInfo->_localIp.empty() && !SocketUtil::IsIp(connectInfo->_localIp))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("illegal local ip:%s, stub:%llu, LibConnectInfo:%s")
                    , connectInfo->_targetIp.c_str(), connectInfo->_stub, connectInfo->ToString().c_str());
        
        giveup = true;
        return Status::SockError_IllegalIp;
    }

    // 黑白名单
    auto ipRuleMgr = GetComp<IpRuleMgr>();
    if(!ipRuleMgr->Check(connectInfo->_targetIp))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("refuse connecting by black white list remote =%s!"), connectInfo->_targetIp.c_str());
        giveup = true;
        return Status::BlackWhiteCheckFail;
    }

    bool doBind = !connectInfo->_localIp.empty() || (connectInfo->_localPort != 0);
    
    // 连接数限制
    Int32 errCode = Status::Success;
    _pollerMgr->AddSessionPending(1);
    auto newSesssionId = _pollerMgr->NewSessionId();
    connectPendingInfo->_sessionId = newSesssionId;
    connectPendingInfo->_newSock = INVALID_SOCKET;
    errCode = SocketUtil::CreateSock2(connectPendingInfo->_newSock, connectInfo->_family);
    if(errCode != Status::Success)
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("create sock fail errcode = %d!"), errCode);
        return errCode;
    }

    // 设置发送缓冲
    auto &sessionOption = connectInfo->_sessionOption;
    if(sessionOption._sockSendBufferSize)
    {
        errCode = SocketUtil::SetSendBufSize(connectPendingInfo->_newSock, sessionOption._sockSendBufferSize);
        if(errCode != Status::Success)
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("set sock send buff fail connect pending info:%s, errCode:%d")
                        , connectPendingInfo->ToString().c_str(), errCode);
            return errCode;
        }
    }

    // 设置接收缓冲
    if(sessionOption._sockRecvBufferSize)
    {
        errCode = SocketUtil::SetRecvBufSize(connectPendingInfo->_newSock, sessionOption._sockRecvBufferSize);
        if(errCode != Status::Success)
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("set sock recv buff fail connect pending info:%s, errCode:%d")
                        , connectPendingInfo->ToString().c_str(), errCode);
            return errCode;
        }
    }

    // 绑定前需要保证端口重用
    errCode = SocketUtil::MakeReUseAddr(connectPendingInfo->_newSock, true);
    if(errCode != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("make reuse addr fail connect pending info:%s"), connectPendingInfo->ToString().c_str());
        return errCode;
    }

    errCode = SocketUtil::MakeReUsePort(connectPendingInfo->_newSock, true);
    if(errCode != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("make reuse port fail connect pending info:%s"), connectPendingInfo->ToString().c_str());
        return errCode;
    }

    errCode = SocketUtil::SetNoBlock(connectPendingInfo->_newSock);
    if(errCode != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("set no block fail connect pending info:%s"), connectPendingInfo->ToString().c_str());
        return errCode;
    }

    errCode = SocketUtil::MakeNoDelay(connectPendingInfo->_newSock, true);
    if(errCode != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("make no delay fail connect pending info:%s"), connectPendingInfo->ToString().c_str());
        return errCode;
    }

    errCode = _iocp->Reg(connectPendingInfo->_newSock, connectPendingInfo->_sessionId);
    if(errCode != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("iocp reg new connect sock fail errCode:%d, new sock:%d, session id:%llu"), errCode, connectPendingInfo->_newSock, connectPendingInfo->_sessionId);
        return errCode;
    }

    // 绑定本地ip的话可以通过多网卡来实现消息的负载均衡(内网可以流转的更快)
    if(doBind)
    {
        errCode = SocketUtil::Bind(connectPendingInfo->_newSock, connectInfo->_localIp, connectInfo->_localPort, connectInfo->_family);
        if(errCode != Status::Success)
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("bind to ip[%s:%hu] sock[%d] fail when check connect!")
                                        , connectInfo->_localIp.c_str(), connectInfo->_localPort, connectPendingInfo->_newSock);
            return errCode;
        }
    }

    auto newIoData = IoData::NewThreadLocal_IoData();
    newIoData->_sessionId = connectPendingInfo->_sessionId;
    newIoData->_sock = connectPendingInfo->_newSock;
    newIoData->_ioType = IoEventType::IO_CONNECT;
    errCode = SocketUtil::PostConnect(newIoData, connectInfo->_family, connectInfo->_targetIp, connectInfo->_targetPort);
    if(errCode != Status::Success && errCode != Status::SockError_Pending)
    {
        IoData::DeleteThreadLocal_IoData(newIoData);
        g_Log->NetError(LOGFMT_OBJ_TAG("post connect fail errCode:%d, pending info:%s"), connectPendingInfo->ToString().c_str());
        return errCode;
    }

    ++_sessionPendingCount;
    _sessionIdRefAsynConnectPendingInfo.insert(std::make_pair(connectPendingInfo->_sessionId, connectPendingInfo));
    g_Log->NetInfo(LOGFMT_OBJ_TAG("suc add connect pending info and wait for iocp connect suc back pending info:%s"), connectPendingInfo->ToString().c_str());
    return errCode;
}

LibConnectPendingInfo *IocpTcpPoller::_CreateNewConectPendingInfo(LibConnectInfo *connectInfo, Int32 leftTimes)
{
    auto connectPendingInfo = LibConnectPendingInfo::NewThreadLocal_LibConnectPendingInfo();
    if(SocketUtil::IsIpv4(connectInfo->_targetIp))
    {
        connectPendingInfo->_remoteAddr._addr._isIpv4 = true;
        if (!SocketUtil::FillTcpAddrInfo(connectInfo->_targetIp.c_str(), connectInfo->_targetPort, connectInfo->_family, connectPendingInfo->_remoteAddr._addr._data._sinV4))
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("FillTcpAddrInfo ipv4 fail ip[%s] port[%hu]"), connectInfo->_targetIp.c_str(), connectInfo->_targetPort);
            
            return NULL;
        }  
    }
    else
    {
        connectPendingInfo->_remoteAddr._addr._isIpv4 = false;
        if (!SocketUtil::FillTcpAddrInfo(connectInfo->_targetIp.c_str(), connectInfo->_targetPort, connectInfo->_family, connectPendingInfo->_remoteAddr._addr._data._sinV6))
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("FillTcpAddrInfo ipv6 fail ip[%s] port[%hu]"), connectInfo->_targetIp.c_str(), connectInfo->_targetPort);
            
            return NULL;
        }  
    }

    connectPendingInfo->_connectInfo = connectInfo;
    connectPendingInfo->_leftRetryTimes = leftTimes;

    return connectPendingInfo;
}

KERNEL_END

#endif
