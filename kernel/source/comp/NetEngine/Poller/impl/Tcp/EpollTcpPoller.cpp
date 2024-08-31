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
 * Date: 2022-04-21 22:48:40
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/EpollTcpPoller.h>

#if CRYSTAL_TARGET_PLATFORM_LINUX

#include <kernel/comp/Variant/Variant.h>
#include <kernel/comp/SmartPtr.h>

#include <kernel/comp/Delegate/LibDelegate.h>
#include <kernel/comp/LibDirtyHelper.h>
#include <kernel/comp/LibList.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerEvent.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerInnerEvent.h>

#include <kernel/comp/Poller/PollerInc.h>
#include <kernel/comp/TimeSlice.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerDirty.h>
#include <kernel/comp/NetEngine/LibEpoll.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/TcpPollerMgr.h>
#include <kernel/comp/NetEngine/Poller/interface/IPollerMgr.h>
#include <kernel/comp/thread/LibThread.h>
#include <kernel/comp/NetEngine/Defs/LibConnectInfo.h>
#include <kernel/comp/NetEngine/LibPacket.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/EpollTcpSession.h>
#include <kernel/comp/NetEngine/Poller/Defs/CloseSessionInfo.h>
#include <kernel/comp/NetEngine/Defs/BuildSessionInfo.h>
#include <kernel/comp/NetEngine/LibSocket.h>
#include <kernel/comp/NetEngine/LibAddr.h>
#include <kernel/comp/Utils/SocketUtil.h>
#include <kernel/comp/Timer/Timer.h>
#include <kernel/comp/Service/Service.h>
#include <kernel/comp/NetEngine/Defs/LibListenInfo.h>
#include <kernel/comp/NetEngine/Poller/impl/IpRule/IpRuleMgrFactory.h>
#include <kernel/comp/NetEngine/Poller/impl/IpRule/IpRuleMgr.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerConfig.h>
#include <kernel/comp/TlsMemoryCleanerComp.h>
#include <kernel/comp/NetEngine/Defs/ProtocolType.h>
#include <kernel/comp/Tls/TlsDefaultObj.h>
#include <kernel/comp/Tls/TlsCompsOwner.h>
#include <kernel/comp/Utils/TlsUtil.h>
#include <kernel/comp/NetEngine/Defs/AddrIpConfig.h>
#include <kernel/comp/Utils/IPUtil.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(EpollTcpPoller);

EpollTcpPoller::EpollTcpPoller(TcpPollerMgr *pollerMgr, UInt64 pollerId, const TcpPollerInstConfig *cfg)
:CompHostObject(KERNEL_NS::RttiUtil::GetTypeId<EpollTcpPoller>())
,_pollerId(pollerId)
,_tcpPollerMgr(pollerMgr)
,_pollerMgr(NULL)
,_serviceProxy(NULL)
,_poller(NULL)
,_cfg(cfg)
,_sessionCount{0}
,_sessionPendingCount{0}
,_epoll(NULL)
,_wakeupSessionId(0)
,_wakeupEventFd(0)
,_monitor(NULL)
,_eventLoopThread(NULL)
,_pollerInstMonitorPriorityLevel(-1)
{

}

EpollTcpPoller::~EpollTcpPoller()
{
    _Clear();
}

void EpollTcpPoller::Release()
{
    EpollTcpPoller::Delete_EpollTcpPoller(this);
}

void EpollTcpPoller::OnRegisterComps()
{
    // 注册poller组件
    RegisterComp<PollerFactory>();
    RegisterComp<IpRuleMgrFactory>();
}

void EpollTcpPoller::Clear()
{
    _Clear();
    CompHostObject::Clear();
}

LibString EpollTcpPoller::ToString() const
{
    LibString info;
    info.AppendFormat("epoll tcp poller comp object info:%s\n", CompHostObject::ToString().c_str());

    info.AppendFormat("epoll tcp poller poller id:%llu, ", _pollerId)
        .AppendFormat("session quantity:%llu", static_cast<UInt64>(_sessionIdRefSession.size()));

    info.AppendFormat("comps info:\n");
    auto &allComps = GetAllComps();
    for(auto comp : allComps)
        info.AppendFormat("%s\n", comp->ToString().c_str());

    return info;
}

UInt64 EpollTcpPoller::CalcLoadScore() const
{
    return _poller->CalcLoadScore() + _sessionCount.load(std::memory_order_acquire) + _sessionPendingCount.load(std::memory_order_acquire);
}

void EpollTcpPoller::PostSend(Int32 level, UInt64 sessionId, LibPacket *packet)
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

void EpollTcpPoller::PostSend(Int32 level, UInt64 sessionId, LibList<LibPacket *> *packets)
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

void EpollTcpPoller::PostNewSession(Int32 level, BuildSessionInfo *buildSessionInfo)
{
    if(UNLIKELY(!_poller->IsEnable()))
    {
        if(g_Log->IsEnable(LogLevel::Warn))
            g_Log->Warn(LOGFMT_OBJ_TAG("poller disable build session:%s"),  buildSessionInfo->ToString().c_str());
        BuildSessionInfo::Delete_BuildSessionInfo(buildSessionInfo);
        return;
    }

    // TODO: accept/connect suc
    auto newSessionEv = NewSessionEvent::New_NewSessionEvent();
    newSessionEv->_buildInfo = buildSessionInfo;
    _poller->Push(level, newSessionEv);
}

void EpollTcpPoller::PostAddlisten(Int32 level, LibListenInfo *listenInfo)
{
    if(UNLIKELY(!_poller->IsEnable()))
    {
        if(g_Log->IsEnable(LogLevel::Warn))
            g_Log->Warn(LOGFMT_OBJ_TAG("poller disable listenInfo:%s"),  listenInfo->ToString().c_str());
        LibListenInfo::Delete_LibListenInfo(listenInfo);
        return;
    }

    auto ev = AddListenEvent::New_AddListenEvent();
    ev->_addListenInfoList.push_back(listenInfo);
    _poller->Push(level, ev);
}

void EpollTcpPoller::PostAddlistenList(Int32 level, std::vector<LibListenInfo *> &listenInfoList)
{
    if(UNLIKELY(!_poller->IsEnable()))
    {
        if(g_Log->IsEnable(LogLevel::Warn))
        {
            const Int32 listSize = static_cast<Int32>(listenInfoList.size());
            LibString info;
            info.AppendFormat("AddListenEvent: listen info array amount:%d, array:[", listSize);

            for(Int32 idx = 0; idx < listSize; ++idx)
                info.AppendFormat("[%d]=%s\n", idx, listenInfoList[idx]->ToString().c_str());

            info.AppendFormat("]");
            g_Log->Warn(LOGFMT_OBJ_TAG("poller disable listen info list:%s"), info.c_str());
        }

        ContainerUtil::DelContainer(listenInfoList, [](LibListenInfo *&listenInfo){
            LibListenInfo::Delete_LibListenInfo(listenInfo);
            listenInfo = NULL;
        });
        return;
    }

    auto ev = AddListenEvent::New_AddListenEvent();
    ev->_addListenInfoList.swap(listenInfoList);

    _poller->Push(level, ev);
}

void EpollTcpPoller::PostConnect(Int32 level, LibConnectInfo *connectInfo)
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

void EpollTcpPoller::PostCloseSession(UInt64 fromServiceId, Int32 level, UInt64 sessionId, Int64 closeMillisecondTimeDelay, bool forbidRead, bool forbidWrite)
{
    auto closeEv = CloseSessionEvent::New_CloseSessionEvent();
    closeEv->_sessionId = sessionId;
    closeEv->_closeMillisecondTime = closeMillisecondTimeDelay ? (LibTime::NowMilliTimestamp() + closeMillisecondTimeDelay) : 0;
    closeEv->_forbidRead = forbidRead;
    closeEv->_forbidWrite = forbidWrite;
    closeEv->_fromServiceId = fromServiceId;
    closeEv->_priorityLevel = level;
    _poller->Push(level, closeEv);
}

void EpollTcpPoller::PostIpControl(Int32 level, const std::list<IpControlInfo *> &controlList)
{
    auto ev = IpRuleControlEvent::New_IpRuleControlEvent();
    ev->_ipControlList = controlList;
    _poller->Push(level, ev);
}

void EpollTcpPoller::PostQuitServiceSessionsEvent(UInt64 serviceId, Int32 level)
{
    auto ev = QuitServiceSessionsEvent::New_QuitServiceSessionsEvent();
    ev->_fromServiceId = serviceId;
    ev->_priorityLevel = level;
    _poller->Push(level, ev);
}

void EpollTcpPoller::_SendData(EpollTcpSession *session, LibList<LibPacket *> *packets)
{
    // session->BeginTransaction();
    session->SendPackets(packets);
    // session->CommitTransaction();
    session->ResetFrameSendHandleBytes();
    // g_Log->NetTrace(LOGFMT_OBJ_TAG("send data session id:%llu, packets count:%llu"), session->GetId(), packets->GetAmount());
}

bool EpollTcpPoller::_TryHandleConnecting(UInt64 sessionId, Int32 events)
{
    auto iter = _sessionIdRefAsynConnectPendingInfo.find(sessionId);
    if(iter == _sessionIdRefAsynConnectPendingInfo.end())
        return false;

    auto connectPendingInfo = iter->second;
    bool connected = false;
    if (events & EPOLLOUT)
    {
        int optval;
        LibSockLen optlen = sizeof(int);
        if (SocketUtil::GetSockOp(connectPendingInfo->_newSock, SOL_SOCKET, SO_ERROR, &optval, &optlen) == Status::Success && optval == 0)
            connected = true;
    }
    
    if(UNLIKELY(!connected))
    {
        // 连接成功 
        g_Log->NetTrace(LOGFMT_OBJ_TAG("epoll poller info:%s _TryHandleConnecting fail connect and will _ReleaseConnect info:%s")
                                    , ToString().c_str(), connectPendingInfo->ToString().c_str());   
        g_Log->NetDebug(LOGFMT_OBJ_TAG("epoll poller info:%s _TryHandleConnecting fail connect and will _ReleaseConnect info:%s")
                                    , ToString().c_str(), connectPendingInfo->ToString().c_str());   

        // 没定时器重试
        if(!connectPendingInfo->_reconnectTimer)
            _OnConnectFailure(connectPendingInfo->_connectInfo, connectPendingInfo, Status::Socket_ConnectFail);
        return true;
    }
        
    // 连接成功 
    g_Log->NetTrace(LOGFMT_OBJ_TAG("epoll poller info:%s _TryHandleConnecting suc connect info:%s")
                                , ToString().c_str(), connectPendingInfo->ToString().c_str());   

    _OnConnectSuc(connectPendingInfo);
    // TODO:连接成功,创建session,定时器移除重连
    // 判断出错
    // 若成功则
    return true;
}

Int32 EpollTcpPoller::_OnPriorityLevelCompsCreated()
{
    _poller = GetComp<Poller>();
    TimeSlice span(0, 0, _cfg->_maxPieceTimeInMicroseconds);
    _poller->SetMaxPriorityLevel(_cfg->_maxPriorityLevel);
    _poller->SetMaxPieceTime(span);
    _poller->SetMaxSleepMilliseconds(_cfg->_maxSleepMilliseconds);
    _poller->SetPepareEventWorkerHandler(this, &EpollTcpPoller::_OnPollerPrepare);
    _poller->SetEventWorkerCloseHandler(this, &EpollTcpPoller::_OnPollerWillDestroy);
    
    _poller->Subscribe(PollerEventType::Write, this, &EpollTcpPoller::_OnWrite);
    _poller->Subscribe(PollerEventType::AsynConnect, this, &EpollTcpPoller::_OnAsynConnect);
    _poller->Subscribe(PollerEventType::NewSession, this, &EpollTcpPoller::_OnNewSession);
    _poller->Subscribe(PollerEventType::Monitor, this, &EpollTcpPoller::_OnMonitor);
    _poller->Subscribe(PollerEventType::CloseSession, this, &EpollTcpPoller::_OnCloseSession);
    _poller->Subscribe(PollerEventType::AddListen, this, &EpollTcpPoller::_OnAddListen);
    _poller->Subscribe(PollerEventType::IpRuleControl, this, &EpollTcpPoller::_OnIpRuleControl);
    _poller->Subscribe(PollerEventType::QuitServiceSessionsEvent, this, &EpollTcpPoller::_OnQuitServiceSessionsEvent);
    _poller->Subscribe(PollerEventType::RealDoQuitServiceSessionEvent, this, &EpollTcpPoller::_OnRealDoQuitServiceSessionEvent);

    // 脏回调
    auto dirtyHelper = _poller->GetDirtyHelper();
    dirtyHelper->Init(PollerDirty::END);
    auto deleg = DelegateFactory::Create(this, &EpollTcpPoller::_OnDirtySessionRead);
    dirtyHelper->SetHandler(PollerDirty::READ, deleg);
    deleg = DelegateFactory::Create(this, &EpollTcpPoller::_OnDirtySessionWrite);
    dirtyHelper->SetHandler(PollerDirty::WRITE, deleg);
    deleg = DelegateFactory::Create(this, &EpollTcpPoller::_OnDirtySessionClose);
    dirtyHelper->SetHandler(PollerDirty::CLOSE, deleg);
    deleg = DelegateFactory::Create(this, &EpollTcpPoller::_OnDirtySessionAccept);
    dirtyHelper->SetHandler(PollerDirty::ACCEPT, deleg);

    return Status::Success;
}

Int32 EpollTcpPoller::_OnCompsCreated()
{
    // ip rule mgr 设置
    auto ipRuleMgr = GetComp<IpRuleMgr>();
    auto config = _pollerMgr->GetConfig();
    if(!ipRuleMgr->SetBlackWhiteListFlag(config->_blackWhiteListFlag))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("SetBlackWhiteListFlag fail black white list flag:%u"), config->_blackWhiteListFlag);
        if(GetOwner())
            GetOwner()->SetErrCode(this, Status::Failed);
        return Status::Failed;
    }
    
    g_Log->NetInfo(LOGFMT_OBJ_TAG("comps created suc %s."), ToString().c_str());
    return Status::Success;
}

Int32 EpollTcpPoller::_OnHostInit()
{
    _epoll = CRYSTAL_NEW(LibEpoll);
    _pollerMgr = _tcpPollerMgr->GetOwner()->CastTo<IPollerMgr>();
    _serviceProxy = _tcpPollerMgr->GetServiceProxy();

    _monitor = CRYSTAL_NEW(LibThread);
    _monitor->AddTask(this, &EpollTcpPoller::_OnMonitorThread);
    _monitor->SetThreadName(LibString().AppendFormat("Poller%llumonitor", _pollerId));

    _eventLoopThread = CRYSTAL_NEW(LibThread);
    _eventLoopThread->AddTask(this, &EpollTcpPoller::_OnPollEventLoop);
    _eventLoopThread->SetThreadName(LibString().AppendFormat("Poller%lluLoop", _pollerId));

    _pollerInstMonitorPriorityLevel = _cfg->_pollerInstMonitorPriorityLevel < 0 ? _cfg->_maxPriorityLevel : _cfg->_pollerInstMonitorPriorityLevel;

    g_Log->NetInfo(LOGFMT_OBJ_TAG("epoll tcp poller inited."));
    return Status::Success;
}

Int32 EpollTcpPoller::_OnHostWillStart()
{
    g_Log->NetInfo(LOGFMT_OBJ_TAG("epoll tcp poller will start."));
    return Status::Success;
}

Int32 EpollTcpPoller::_OnHostStart()
{
    _monitor->Start();
    _eventLoopThread->Start();
    g_Log->NetInfo(LOGFMT_OBJ_TAG("epoll tcp poller started."));
    return Status::Success;
}

void EpollTcpPoller::_OnHostBeforeCompsWillClose()
{
    g_Log->NetInfo(LOGFMT_OBJ_TAG("epoll tcp poller before comps will close."));
    
    // 先关闭epoll worker线程
    if(_monitor)
    {
        if(_monitor->HalfClose())
        {
            // 事件唤醒epoll worker thread
            if(_wakeupEventFd)
            {
                UInt64 one = 1;
                ::write(_wakeupEventFd, &one, sizeof(one));
            }
            _monitor->FinishClose();
        }
    }

    if(_poller)
        _poller->QuitLoop();
    if(_eventLoopThread)
        _eventLoopThread->Close();
}

void EpollTcpPoller::_OnHostWillClose()
{
    g_Log->NetInfo(LOGFMT_OBJ_TAG("epoll tcp poller will close."));
}

void EpollTcpPoller::_OnHostClose()
{
    g_Log->NetInfo(LOGFMT_OBJ_TAG("epoll tcp poller closed."));
    _Clear();
}

void EpollTcpPoller::_Clear()
{
    CRYSTAL_DELETE_SAFE(_epoll);
    CRYSTAL_DELETE_SAFE(_monitor);
    CRYSTAL_DELETE_SAFE(_eventLoopThread);
}

bool EpollTcpPoller::_OnPollerPrepare(Poller *poller)
{
    const auto workerThreadId = poller->GetWorkerThreadId();
    #if _DEBUG
        auto memoryCleaner = KERNEL_NS::TlsUtil::GetTlsCompsOwner()->GetComp<TlsMemoryCleanerComp>();
        memoryCleaner->SetIntervalMs(1000);
    #endif

    Int32 err = _epoll->Create();
    if(err != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("fail create epoll. err = [%d], poller worker threadId = [%llu]")
                        , err, workerThreadId);
        return false;
    }

    // 创建唤醒监控线程session
    // 唤醒monitor
    _wakeupEventFd = SocketUtil::CreateEventFd();
    if( _wakeupEventFd < 0 )
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("fail create monitor wakeup event fd poller worker threadId = [%llu]"), workerThreadId);
        return false;
    }

    // wakeup sessionId
    _wakeupSessionId = _pollerMgr->NewSessionId();

    // 添加事件
    err = _epoll->AddEvent(_wakeupEventFd, _wakeupSessionId, __ADD_LIB_EPOLL_WAKE_UP_EVENT_FLAGS_DEF__);
    if(err != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("fail AddEvent monitor wakeup  err = [%d], poller worker threadId = [%llu], session id:%llu"), err, 
                    workerThreadId, _wakeupSessionId);
        return false;
    }

    return true;
}

void EpollTcpPoller::_OnPollerWillDestroy(Poller *poller)
{
    // TODO:销毁session, 销毁connectinfo
    if(_wakeupEventFd)
    {
        ::close(_wakeupEventFd);
        _wakeupEventFd = 0;
    }

    ContainerUtil::DelContainer(_sessionIdRefSession, [](EpollTcpSession *&session)
    {
        session->Close();
        EpollTcpSession::DeleteThreadLocal_EpollTcpSession(session);
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
}

void EpollTcpPoller::_OnWrite(PollerEvent *ev)
{
    auto sendEv = ev->CastTo<AsynSendEvent>();
    SmartPtr<LibList<LibPacket *>, AutoDelMethods::CustomDelete> sendPackets = sendEv->_packets;
    sendPackets.SetClosureDelegate([](void *ptr){
        LibList<LibPacket *> *packets = KernelCastTo<LibList<LibPacket *>>(ptr);
        ContainerUtil::DelContainer(*packets, [](LibPacket *&packet){
            LibPacket::Delete_LibPacket(packet);
            packet = NULL;
        });
        LibList<LibPacket *>::Delete_LibList(packets);
    });
    sendEv->_packets = NULL;

    auto session = _GetSession(sendEv->_sessionId);
    if(!session)
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("session is missing session id:%llu"), sendEv->_sessionId);
        return;
    }

    if(UNLIKELY(session->IsLinker()))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("session is linker cant send data session:%s, event:%s"), ToString().c_str(), sendEv->ToString().c_str());
        return;
    }

    _SendData(session, sendPackets.pop());
}

void EpollTcpPoller::_OnAsynConnect(PollerEvent *ev)
{
    AsynConnectEvent *connectEv = ev->CastTo<AsynConnectEvent>();
    SmartPtr<LibConnectInfo, AutoDelMethods::CustomDelete> connectInfo = connectEv->_connectInfo;
    connectInfo.SetClosureDelegate([](void *ptr){
        auto connectInfo = KernelCastTo<LibConnectInfo>(ptr);
        LibConnectInfo::Delete_LibConnectInfo(connectInfo);
    });
    g_Log->NetInfo(LOGFMT_OBJ_TAG("recv a connect event: :%s"), connectEv->ToString().c_str());

    Int32 errCode = Status::Success;
    KERNEL_NS::LibString currentIp;
    connectInfo->_currentSwitchTargetIpLeft = connectInfo->_targetIp._mostSwitchIpCount;
    if(!_TryGetNewTargetIp(connectInfo->_targetIp, connectInfo->_failureIps, currentIp))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("_TryGetNewTargetIp fail connect info:%s"), connectInfo->ToString().c_str());
        _OnConnectFailure(connectInfo.pop(), NULL, Status::Error);
        return;
    }

    auto newPending = _CreateNewConectPendingInfo(connectInfo, currentIp, connectInfo->_retryTimes);
    if(!newPending)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("_CreateNewConectPendingInfo fail connect info:%s"), connectInfo->ToString().c_str());
        _OnConnectFailure(connectInfo.pop(), NULL, Status::Error);
        return;
    }

    bool giveup = false;
    errCode = _CheckConnect(newPending, giveup);
    if(errCode == Status::Success)
    {
        _OnConnectSuc(newPending);
        return;
    }

    if(errCode == Status::SockError_Pending)
    {// pending
        _OnConnectPending(newPending);
    }
    else if(newPending->_leftRetryTimes <= 0 || giveup)
    {
        _OnConnectFailure(connectInfo.pop(), newPending, errCode);
        return;
    }

    {// 定时重连
        auto connectInfoCache = connectInfo.pop();
        {
            newPending->_reconnectTimer = LibTimer::NewThreadLocal_LibTimer(_poller->GetTimerMgr());
            auto __tryAgainTimeOut = [connectInfoCache, this](LibTimer *timer) mutable -> void 
            {
                // pending
                auto newPending = timer->GetParams()[1].AsPtr<LibConnectPendingInfo>();
                auto connectInfo = newPending->_connectInfo;
                // 记录失败的ip
                connectInfo->_failureIps.insert(newPending->_currentTargetIp);
                KERNEL_NS::LibString newTargetIp;
                newTargetIp = newPending->_currentTargetIp;

                do
                {
                    // 次数不够
                    if(newPending->_leftRetryTimes <= 0)
                    {// 没次数

                        // 有剩余尝试切换次数
                        if((newPending->_connectInfo->_currentSwitchTargetIpLeft--) > 0)
                        {
                            // TODO: 切换ip
                            if(_TryGetNewTargetIp(newPending->_connectInfo->_targetIp, newPending->_connectInfo->_failureIps, newTargetIp))
                            {
                                if(g_Log->IsEnable(LogLevel::NetInfo))
                                    g_Log->NetInfo(LOGFMT_OBJ_TAG("connect to %s[%s]:%hu fail, will try connect new ip:%s[%s]:%hu...\n connect info:%s")
                                , connectInfo->_targetIp._ip.c_str(), newPending->_currentTargetIp.c_str(), connectInfo->_targetPort
                                , connectInfo->_targetIp._ip.c_str(), newTargetIp.c_str()
                                , connectInfo->_targetPort, connectInfo->ToString().c_str());
                                break;
                            }
                        }
                        
                        if(g_Log->IsEnable(LogLevel::NetError))
                            g_Log->NetError(LOGFMT_OBJ_TAG("%s try connect fail by trying many times!"), newPending->ToString().c_str());

                        _OnConnectFailure(connectInfoCache, newPending, Status::Socket_ConnectTimeOut);
                        return;
                    }
                } while (false);

                // 移除上一次pending
                const auto leftRetryTimes = --newPending->_leftRetryTimes;
                newPending->_reconnectTimer = NULL;
                _DestroyConnect(newPending, false);

                newPending = _CreateNewConectPendingInfo(connectInfo, newTargetIp, leftRetryTimes);
                if(!newPending)
                {
                    g_Log->NetError(LOGFMT_OBJ_TAG("try again time out _CreateNewConectPendingInfo fail current ip:%s[%s], \nconnect info:%s")
                    , connectInfo->_targetIp._ip.c_str(), newTargetIp.c_str(), connectInfo->ToString().c_str());
                    _OnConnectFailure(connectInfoCache, NULL, Status::Error);
                    LibTimer::DeleteThreadLocal_LibTimer(timer);
                    return;
                }
                timer->GetParams().BecomeDict()[1] = newPending;

                // 连接
                newPending->_reconnectTimer = timer;
                bool giveup = false;
                auto errCode = _CheckConnect(newPending, giveup);
                if(errCode == Status::Success)
                {// 成功
                    _OnConnectSuc(newPending);
                    return;
                }

                if(errCode == Status::SockError_Pending)
                {// pending epoll 事件监听
                    _OnConnectPending(newPending);
                }
                else if(newPending->_leftRetryTimes <= 0 || giveup)
                {// 次数不足失败
                    _OnConnectFailure(connectInfoCache, newPending, errCode);
                    return;
                }

                // 重新定时
                g_Log->NetWarn(LOGFMT_OBJ_TAG("connect to:%s[%s]:%hu timeout, will try again\n pending info:%s")
                , connectInfo->_targetIp._ip.c_str(), newPending->_currentTargetIp.c_str(), connectInfo->_targetPort
                , newPending->ToString().c_str());
            };

            auto delg = DelegateFactory::Create<decltype(__tryAgainTimeOut), void, LibTimer *>(__tryAgainTimeOut);
            newPending->_reconnectTimer->GetParams().BecomeDict()[1] = newPending;
            newPending->_reconnectTimer->SetTimeOutHandler(delg);
            // 没配置定时时长默认30s
            if(connectInfoCache->_periodMs != 0)
                newPending->_reconnectTimer->Schedule(connectInfoCache->_periodMs);
            else
                newPending->_reconnectTimer->Schedule(KERNEL_NS::TimeSlice::FromSeconds(30));
        }
        
        g_Log->NetInfo(LOGFMT_OBJ_TAG("%s[%s]:%hu waiting for connecting success...\npending info:%s")
            , connectInfoCache->_targetIp._ip.c_str(), newPending->_currentTargetIp.c_str(), connectInfoCache->_targetPort
            , newPending->ToString().c_str());
    }
}

void EpollTcpPoller::_OnNewSession(PollerEvent *ev)
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
                connectRes->_sessionId = buildSessionInfo->_sessionId;
                connectRes->_targetConfig = buildSessionInfo->_remoteOriginIpConfig;
                connectRes->_failureIps = buildSessionInfo->_failureIps;
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
                listenRes->_sessionId = buildSessionInfo->_sessionId;
                _serviceProxy->PostMsg(listenRes->_serviceId, listenRes->_priorityLevel, listenRes);
            }
        }
        
        SocketUtil::DestroySocket(buildSessionInfo->_sock);
        return;
    }

    // 添加epoll监听
    if(UNLIKELY(newSession->IsLinker()))
        _epoll->AddEvent(buildSessionInfo->_sock, buildSessionInfo->_sessionId, __ADD_LIB_EPOLL_ACCEPT_EVENT_FLAGS_DEF__);
    else
        _epoll->AddEvent(buildSessionInfo->_sock, buildSessionInfo->_sessionId, __ADD_LIB_EPOLL_EVENT_FLAGS_DEF__);

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
    sessionCreatedEv->_targetConfig = buildSessionInfo->_remoteOriginIpConfig;
    sessionCreatedEv->_failureIps = buildSessionInfo->_failureIps;
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
            connectRes->_targetConfig = buildSessionInfo->_remoteOriginIpConfig;
            connectRes->_failureIps = buildSessionInfo->_failureIps;
            _serviceProxy->PostMsg(connectRes->_fromServiceId, connectRes->_priorityLevel, connectRes);
        }
        else if(buildSessionInfo->_isLinker)
        {// TODO:
            auto listenRes = AddListenResEvent::New_AddListenResEvent();
            listenRes->_errCode = Status::Success;
            listenRes->_localAddr = buildSessionInfo->_localAddr;
            listenRes->_family = buildSessionInfo->_family;
            listenRes->_serviceId = buildSessionInfo->_serviceId;
            listenRes->_stub = buildSessionInfo->_stub;
            listenRes->_priorityLevel = buildSessionInfo->_priorityLevel;
            listenRes->_protocolType = buildSessionInfo->_protocolType;
            listenRes->_sessionId = buildSessionInfo->_sessionId;
            _serviceProxy->PostMsg(listenRes->_serviceId, listenRes->_priorityLevel, listenRes);
        }
    }

    g_Log->NetInfo(LOGFMT_OBJ_TAG("new session created suc poller id:%llu, session amount:%llu, session info:%s")
                    , _pollerId, static_cast<UInt64>(_sessionIdRefSession.size()), newSession->ToString().c_str());
    g_Log->NetDebug(LOGFMT_OBJ_TAG("new session created suc poller id:%llu, session amount:%llu, session info:%s")
                    , _pollerId, static_cast<UInt64>(_sessionIdRefSession.size()), newSession->ToString().c_str());
}

void EpollTcpPoller::_OnMonitor(PollerEvent *ev)
{
    auto monitorEv = ev->CastTo<MonitorPollerEvent>();
    struct epoll_event *eventArray = KernelCastTo<epoll_event>(monitorEv->_epEvents._bytes);
    const Int32 eventNum = monitorEv->_count;
    for(Int32 idx = 0; idx < eventNum; ++idx)
    {
        epoll_event &ev = eventArray[idx];

        // 判断是否是主动连接
        if(_TryHandleConnecting(ev.data.u64, ev.events))
            continue;
        
        const UInt64 sessionId = ev.data.u64;
        auto session = _GetSession(sessionId);
        if(!session)
        {
            g_Log->NetWarn(LOGFMT_OBJ_TAG("lose session when epoll event coming sessionId = [%llu], events = [%d]")
                        , static_cast<UInt64>(ev.data.u64), ev.events);
            continue;
        }

        if(ev.events & (EPOLLHUP | EPOLLERR))
        {
            // TODO:先打标记，远程断开，还不明确数据还能不能继续读取
            // 缓存中没有数据则可以关闭
            if(!session->HasDataToRecv())
                session->ForbidRecv();
            session->ForbidSend();

            g_Log->NetWarn(LOGFMT_OBJ_TAG("session:%s epollhup or epollerr happen."), session->ToString().c_str());
            
            if(LIKELY(!session->WillSessionClose()))
                _TryCloseSession(session, CloseSessionInfo::SOCK_ERR, 0);
        }
        else
        {
            // 接收数据
            if (ev.events & EPOLLIN)
            {
                if (session->IsLinker())
                {
                    _OnAccept(session);
                    continue;
                }

                if(LIKELY(session->CanRecv()))
                    session->OnRecv();
                else
                    g_Log->NetWarn(LOGFMT_OBJ_TAG("session cant recv now %s"), session->ToString().c_str());
            }

            // 发送数据事件
            if (ev.events & EPOLLOUT)
            {
                // Maybe in session removed while calling OnRecv() method.
                if ((ev.events & EPOLLIN) && 
                        UNLIKELY(!_GetSession(sessionId)))
                {
                    g_Log->NetWarn(LOGFMT_OBJ_TAG("session not found session id %llu"), sessionId);
                    continue;
                }

                session->ClearSendEagain();
                if(LIKELY(session->CanSend()))
                    session->ContinueSend();
                else
                    g_Log->NetWarn(LOGFMT_OBJ_TAG("session cant send now %s"), session->ToString().c_str());
            }
        }
    }
}

void EpollTcpPoller::_OnCloseSession(PollerEvent *ev)
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

void EpollTcpPoller::_OnAddListen(PollerEvent *ev)
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
            g_Log->NetWarn(LOGFMT_OBJ_TAG("create session fail listen info:%s"), listenInfo->ToString().c_str());
            
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
        sessionCreatedEv->_sessionType = listenInfo->_sessionOption._sessionType;
        sessionCreatedEv->_priorityLevel = listenInfo->_priorityLevel;
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

void EpollTcpPoller::_OnIpRuleControl(PollerEvent *ev)
{
    auto ipCtrlEv = ev->CastTo<IpRuleControlEvent>();
    if(UNLIKELY(ipCtrlEv->_ipControlList.empty()))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("ip rule control list empty poller id:%llu"), GetPollerId());
        return;
    }

    g_Log->NetInfo(LOGFMT_OBJ_TAG("ip rule control:%s"), ipCtrlEv->ToString().c_str());
    
    auto ipRuleMgr = GetComp<IpRuleMgr>();
    auto &ctrlList = ipCtrlEv->_ipControlList;
    for(auto ctrlInfo :ctrlList)
    {
        if(!SocketUtil::IsIp(ctrlInfo->_ip))
        {
            g_Log->NetWarn(LOGFMT_OBJ_TAG("invalid ip:%s"), ctrlInfo->_ip.c_str());
            continue;
        }

        for(auto flag : ctrlInfo->_controlFlow)
        {
            if(flag == IpControlInfo::ADD_WHITE)
                ipRuleMgr->PushWhite(ctrlInfo->_ip);
            else if(flag == IpControlInfo::ADD_BLACK)
                ipRuleMgr->PushBlack(ctrlInfo->_ip);
            else if(flag == IpControlInfo::ERASE_WHITE)
                ipRuleMgr->EraseWhite(ctrlInfo->_ip);
            else if(flag == IpControlInfo::ERASE_BLACK)
                ipRuleMgr->EraseBlack(ctrlInfo->_ip);
        }

        if(!ipRuleMgr->Check(ctrlInfo->_ip))
        {
            // 延迟1ms关闭
            auto sessions = _GetSessionsByIp(ctrlInfo->_ip);
            if(sessions)
            {
                for(auto session : *sessions)
                    _ControlCloseSession(session, CloseSessionInfo::BY_BLACK_WHITE_LIST_CHECK, 1, 0);
            }

        }
    }
}

void EpollTcpPoller::_OnQuitServiceSessionsEvent(PollerEvent *ev)
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

void EpollTcpPoller::_OnRealDoQuitServiceSessionEvent(PollerEvent *ev)
{
    auto quitSessionEv = ev->CastTo<RealDoQuitServiceSessionEvent>();
    std::set<EpollTcpSession *> sessions;
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

void EpollTcpPoller::_OnConnectSuc(LibConnectPendingInfo *&connectPendingInfo)
{
    g_Log->NetDebug(LOGFMT_OBJ_TAG("connect to:%s[%s]:%hu success.\npending info:%s")
    , connectPendingInfo->_connectInfo->_targetIp._ip.c_str(), connectPendingInfo->_currentTargetIp.c_str()
    , connectPendingInfo->_connectInfo->_targetPort, connectPendingInfo->ToString().c_str());

    // 从本poller中监听中移除
    _epoll->DelEvent(connectPendingInfo->_newSock, connectPendingInfo->_sessionId, EPOLLOUT | EPOLLET);

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
    newBuildSessionInfo->_remoteOriginIpConfig = connectInfo->_targetIp;
    newBuildSessionInfo->_failureIps = connectInfo->_failureIps;

    // newBuildSessionInfo->_sockRecvBufferBytes = connectInfo->_sockRecvBufferBytes;
    // newBuildSessionInfo->_sockSendBufferBytes = connectInfo->_sockSendBufferBytes;
    _tcpPollerMgr->OnConnectRemoteSuc(newBuildSessionInfo);

    _sessionIdRefAsynConnectPendingInfo.erase(connectPendingInfo->_sessionId);
    --_sessionPendingCount;

    connectPendingInfo->_newSock = INVALID_SOCKET;
    connectPendingInfo->_sessionId = 0;
    _DestroyConnect(connectPendingInfo, true);
}

void EpollTcpPoller::_OnConnectPending(LibConnectPendingInfo *&connectPendingInfo)
{
    _epoll->AddEvent(connectPendingInfo->_newSock, connectPendingInfo->_sessionId, EPOLLOUT | EPOLLET);
    ++_sessionPendingCount;
    _sessionIdRefAsynConnectPendingInfo.insert(std::make_pair(connectPendingInfo->_sessionId, connectPendingInfo));
    g_Log->NetDebug(LOGFMT_OBJ_TAG("connect pending add connect pending info event, pending info:%s"), connectPendingInfo->ToString().c_str());
    g_Log->NetTrace(LOGFMT_OBJ_TAG("connect pending add connect pending info event, pending info:%s"), connectPendingInfo->ToString().c_str());
}

void EpollTcpPoller::_OnConnectFailure(LibConnectInfo *connectInfo, LibConnectPendingInfo *connectPending, Int32 errCode)
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
        localAddr._ip = connectInfo->_localIp._ip;
        localAddr._port = connectInfo->_localPort;
        localAddr._ipAndPort.AppendFormat("%s:%hu", localAddr._ip.c_str(), localAddr._port);
        auto &targetAddr = res->_targetAddr;
        targetAddr._ip = connectPending->_currentTargetIp;
        targetAddr._port = connectInfo->_targetPort;
        targetAddr._ipAndPort.AppendFormat("%s:%hu", targetAddr._ip.c_str(), targetAddr._port);
        res->_targetConfig = connectInfo->_targetIp;
        res->_failureIps = connectInfo->_failureIps;

        _serviceProxy->PostMsg(res->_fromServiceId, res->_priorityLevel, res);
    }

    if(LIKELY(connectPending))
        _DestroyConnect(connectPending, true);
    else
        LibConnectInfo::Delete_LibConnectInfo(connectInfo);
}

void EpollTcpPoller::_OnAccept(EpollTcpSession *session)
{
    auto dirtyHelper = _poller->GetDirtyHelper();
    dirtyHelper->Clear(session, PollerDirty::ACCEPT);

    if(UNLIKELY(!session->CanAccept()))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("cant accept new session because session cant accept! session info:%s"), session->ToString().c_str());
        return;
    }

    auto &curFrameAcceptCount = session->GetCurFrameAcceptHandleCount();
    const auto frameCountLimit = session->GetFrameAcceptHandleLimit();
    
    auto sock = session->GetSock();
    auto sockHandle = sock->GetSock();

    for(;;)
    {
        KernelSockAddrIn addrIn(sock->IsIpv4());
        LibSockLen len = addrIn._isIpv4 ? sizeof(addrIn._data._sinV4) : sizeof(addrIn._data._sinV6);
        SOCKET newSock = INVALID_SOCKET;
        newSock = ::accept(sockHandle, reinterpret_cast<struct sockaddr *>(&addrIn._data), &len);

        // 单帧accpet循环次数限制,避免其他session饥饿
        if(++curFrameAcceptCount >= frameCountLimit)
        {
            g_Log->NetDebug(LOGFMT_OBJ_TAG("accept count over limit will continue in future accept session:%s."), session->ToString().c_str());
            session->ResetFrameAcceptHandleCount();
            dirtyHelper->MaskDirty(session, PollerDirty::ACCEPT);
            break;
        }

        if(LIKELY(newSock > 0))
        {
            // todo 新的连接
            Int32 err = _OnAcceptedNew(newSock, session);
            if(err != Status::Success)
            {
                g_Log->NetWarn(LOGFMT_OBJ_TAG("_OnAcceptedNew new sock fail err:%d, newSock:%d, accept session:%s"), err, newSock, session->ToString().c_str());
                SocketUtil::DestroySocket(newSock);
                continue;
            }
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {// 可以退出循环
                g_Log->NetDebug(LOGFMT_OBJ_TAG("EAGAIN or EWOULDBLOCK coming when accept in a dead loop accept session:%s"), session->ToString().c_str());
                break;
            }
            else if (errno == EINTR)
            {
                g_Log->NetDebug(LOGFMT_OBJ_TAG("sinal inerrupt when accept accept session:%s"), session->ToString().c_str());
            }
            else
            {
                g_Log->NetWarn(LOGFMT_OBJ_TAG("unknown sock error errno = [%d, %s] when accept in a dead loop accept session:%s")
                            , errno, SystemUtil::GetErrString(errno).c_str(), session->ToString().c_str());
            }
        }
    }    
}

Int32 EpollTcpPoller::_OnAcceptedNew(SOCKET sock, EpollTcpSession *session)
{
    g_Log->NetInfo(LOGFMT_OBJ_TAG("accept new socket sock:%d, accept session info:%s"), sock, session->ToString().c_str());
    
    // 校验ip
    auto acceptSock = session->GetSock();

    BriefSockAddr remoteAddr(true);
    SocketUtil::UpdateRemoteAddr(acceptSock->GetFamily(), sock, remoteAddr);

    // 检查ip
    auto ipRuleMgr = GetComp<IpRuleMgr>();
    if(!ipRuleMgr->Check(remoteAddr._ip))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("ip not check suc addr info:%s"), remoteAddr.ToString().c_str());
        return Status::BlackWhiteCheckFail;
    }
 
    UInt64 totalSessionNum = 0;
    const UInt64 sessionQuantityLimit = _pollerMgr->GetSessionQuantityLimit();
    if(!_pollerMgr->CheckAddSessionPending(1, totalSessionNum))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("check add session pending fail, sock:%d, total session num:%llu, session quantity limit:%llu, accept session:%s")
                , sock, totalSessionNum, sessionQuantityLimit, session->ToString().c_str());
        return Status::Socket_SessionOverLimit;
    }

    Int32 err = SocketUtil::SetNoBlock(sock);
    if(err != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("set socket no block fail sock:%d, err:%d, accept session:%s"), sock, err, session->ToString().c_str());
        return err;
    }

    err = SocketUtil::MakeReUseAddr(sock);
    if(err != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("MakeReUseAddr fail sock:%d, err:%d, accept session:%s"), sock, err, session->ToString().c_str());
        return err;
    }

    err = SocketUtil::MakeReUsePort(sock);
    if(err != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("MakeReUsePort fail sock:%d, err:%d, accept session:%s"), sock, err, session->ToString().c_str());
        return err;
    }

    err = SocketUtil::MakeNoDelay(sock, true);
    if(err != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("MakeNoDelay fail sock:%d, err:%d, accept session:%s"), sock, err, session->ToString().c_str());
        return err;
    }

    auto &sessionOption = session->GetOption();
    if(sessionOption._sockRecvBufferSize)
    {
        err = SocketUtil::SetRecvBufSize(sock, sessionOption._sockRecvBufferSize);
        if(err != Status::Success)
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("SetRecvBufSize fail sock:%d, err:%d, accept session:%s"), sock, err, session->ToString().c_str());
            return err;
        }
    }
    if(sessionOption._sockSendBufferSize)
    {
        err = SocketUtil::SetSendBufSize(sock, sessionOption._sockSendBufferSize);
        if(err != Status::Success)
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("SetSendBufSize fail sock:%d, err:%d, accept session:%s"), sock, err, session->ToString().c_str());
            return err;
        }
    }

    const UInt64 newSessionId = _pollerMgr->NewSessionId();
    const auto family = acceptSock->GetFamily();
    const bool isIpv4 = AF_INET == family;
    auto newBuildSessionInfo = BuildSessionInfo::New_BuildSessionInfo(isIpv4);
    newBuildSessionInfo->_protocolType = session->GetProtocolType();
    newBuildSessionInfo->_family = family;
    newBuildSessionInfo->_sessionId = newSessionId;
    SocketUtil::UpdateLocalAddr(sock, newBuildSessionInfo->_localAddr);
    SocketUtil::UpdateRemoteAddr(family, sock, newBuildSessionInfo->_remoteAddr);
    newBuildSessionInfo->_sock = sock;
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

    return Status::Success;
}

void EpollTcpPoller::_OnDirtySessionAccept(LibDirtyHelper<void *, UInt32> *dirtyHelper, void *&session, Variant *params)
{
    auto acceptSession = KernelCastTo<EpollTcpSession>(session);
    g_Log->NetDebug(LOGFMT_OBJ_TAG("session accept dirty coming session%s"), acceptSession->ToString().c_str());

    _OnAccept(acceptSession);
}

void EpollTcpPoller::_OnDirtySessionWrite(LibDirtyHelper<void *, UInt32> *dirtyHelper, void *&session, Variant *params)
{
    auto epollTcpSession = KernelCastTo<EpollTcpSession>(session);
    // g_Log->NetDebug(LOGFMT_OBJ_TAG("dirty write session:%s"), epollTcpSession->ToString().c_str());
    epollTcpSession->ContinueSend();
}

void EpollTcpPoller::_OnDirtySessionRead(LibDirtyHelper<void *, UInt32> *dirtyHelper, void *&session, Variant *params)
{
    auto epollTcpSession = KernelCastTo<EpollTcpSession>(session);
    // g_Log->NetDebug(LOGFMT_OBJ_TAG("dirty read session:%s"), epollTcpSession->ToString().c_str());
    epollTcpSession->OnRecv();
}

void EpollTcpPoller::_OnDirtySessionClose(LibDirtyHelper<void *, UInt32> *dirtyHelper, void *&session, Variant *params)
{
    auto epollSession = KernelCastTo<EpollTcpSession>(session);

     auto closeReason = (*params)[1].AsInt32();
     UInt64 stub = 0;
     if(params->FindDict(2) != params->EndDict())
         stub = (*params)[2].AsUInt64();

     if (!epollSession->HasDataToSend())
         epollSession->ForbidSend();
     else if((!epollSession->IsLinker()) && epollSession->CanSend())
        dirtyHelper->MaskDirty(session, PollerDirty::WRITE);
         
     if (!epollSession->HasDataToRecv())
         epollSession->ForbidRecv();
     else if((!epollSession->IsLinker()) && epollSession->CanRecv())
        dirtyHelper->MaskDirty(session, PollerDirty::READ);

     if(_CanClose(epollSession))
     {
         g_Log->NetDebug(LOGFMT_OBJ_TAG("session close dirty do close session session info:%s, closeReason:%d, %s, stub:%llu")
                        , epollSession->ToString().c_str(), closeReason, CloseSessionInfo::GetCloseReason(closeReason), stub);
         dirtyHelper->Clear(session, PollerDirty::CLOSE);
         _CloseSession(epollSession, closeReason, stub);
         return;
     }

    if(g_Log->IsEnable(LogLevel::NetDebug))
    {
        g_Log->NetDebug(LOGFMT_OBJ_TAG("session will still mask close flag and close session in future closeReason:%d,%s, session info:%s.")
                        , closeReason, CloseSessionInfo::GetCloseReason(closeReason), epollSession->ToString().c_str());
    }
}

void EpollTcpPoller::_DestroyConnect(LibConnectPendingInfo *&connectPendingInfo, bool destroyConnectInfo)
{
    g_Log->NetTrace(LOGFMT_OBJ_TAG("destroy connect pending %s"), connectPendingInfo->ToString().c_str());
    if(connectPendingInfo->_sessionId)
    {
        _pollerMgr->ReduceSessionPending(1);
        auto iter = _sessionIdRefAsynConnectPendingInfo.find(connectPendingInfo->_sessionId);
        if(iter != _sessionIdRefAsynConnectPendingInfo.end())
        {
            if(LIKELY(SocketUtil::IsValidSock(connectPendingInfo->_newSock)))
                _epoll->DelEvent(connectPendingInfo->_newSock, connectPendingInfo->_sessionId, EPOLLOUT | EPOLLET);
            
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

void EpollTcpPoller::_OnMonitorThread(LibThread *t)
{
    // 等待poller线程ready
    while(!IsReady())
    {
        SystemUtil::ThreadSleep(1000);
        g_Log->Warn(LOGFMT_OBJ_TAG("waiting for epoll tcp poller ready..."));

        if(GetErrCode() != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("error happen errCode:%d"), GetErrCode());
            return;
        }
    }

    g_Log->NetInfo(LOGFMT_OBJ_TAG("epoll tcp poller epoll monitor start threadid = [%llu]"), SystemUtil::GetCurrentThreadId());

    auto poller = KERNEL_NS::TlsUtil::GetPoller();
    poller->PrepareLoop();

    while(!t->IsDestroy())
    {
        // epoll wait
        Int32 ret = _epoll->Wait();
        if(ret <= 0)
        {
            g_Log->NetTrace(LOGFMT_OBJ_TAG("epoll wait ret = [%d], poller id:%llu"), ret, _pollerId);
            continue;
        }

        // 监听事件
        auto ev = MonitorPollerEvent::New_MonitorPollerEvent();
        ev->_count = ret;
        ev->_epEvents._bytes = g_MemoryPool->Alloc<Byte8>(sizeof(epoll_event) * ret);
        ::memcpy(ev->_epEvents._bytes, _epoll->GetEvs(), sizeof(epoll_event) * ret);

        _poller->Push(_pollerInstMonitorPriorityLevel, ev);
    }

    poller->OnLoopEnd();

    g_Log->NetInfo(LOGFMT_OBJ_TAG("epoll tcp poller epoll monitor monitor thread finish thread id = %llu"), SystemUtil::GetCurrentThreadId());
}

void EpollTcpPoller::_OnPollEventLoop(LibThread *t)
{
    if(! _OnThreadStart())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("_OnThreadStart fail."));
        return;
    }

    g_Log->NetInfo(LOGFMT_OBJ_TAG("epoll tcp poller event loop start."));
    g_Log->NetInfo(LOGFMT_OBJ_TAG("epoll tcp poller event loop prepare loop..."));
    if(!_poller->PrepareLoop())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("poller prepare loop fail please check"));
        return;
    }

    MaskReady(true);

    g_Log->NetInfo(LOGFMT_OBJ_TAG("epoll tcp poller event loop start loop."));
    _poller->EventLoop();
    g_Log->NetInfo(LOGFMT_OBJ_TAG("epoll tcp poller event loop on loop end."));
    _poller->OnLoopEnd();
    g_Log->NetInfo(LOGFMT_OBJ_TAG("epoll tcp poller event loop finish."));
    MaskReady(false);
}

bool EpollTcpPoller::_OnThreadStart()
{
    // 用 EpollTcpPoller 的poller 替换当前线程的poller组件
    auto defObj = TlsUtil::GetDefTls();
    if(!defObj->_tlsComps->AttachComp(_poller))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("AttachComp fail comp:%s, current epoll tcp poller:%s."), _poller->ToString().c_str(), ToString().c_str());
        return false;
    }

    g_Log->Info(LOGFMT_OBJ_TAG("thread started thread id:%llu."), SystemUtil::GetCurrentThreadId());

    return true;
}

Int32 EpollTcpPoller::_CheckConnect(LibConnectPendingInfo *&connectPendingInfo, bool &giveup)
{
    giveup = false;
    // 如果connectPending没移除则先移除
    if(UNLIKELY(connectPendingInfo->_sessionId))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("connect pending info had already effect. pending info:%s"), connectPendingInfo->ToString().c_str());
        return Status::Repeat;
    }

    auto connectInfo = connectPendingInfo->_connectInfo;
    g_Log->NetTrace(LOGFMT_OBJ_TAG("check connect :%s"), connectInfo->ToString().c_str());

    // ip合法性
    if(!SocketUtil::IsIp(connectPendingInfo->_currentTargetIp))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("illegal target ip:%s, stub:%llu, LibConnectInfo:%s")
            , connectPendingInfo->_currentTargetIp.c_str(), connectInfo->_stub, connectInfo->ToString().c_str());
        giveup = true;
        return Status::SockError_IllegalIp;
    }

    // 本地ip
    if(!connectInfo->_localIp._ip.empty() && !SocketUtil::IsIp(connectInfo->_localIp._ip))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("illegal local ip:%s, stub:%llu, LibConnectInfo:%s")
                    , connectInfo->_localIp._ip.c_str(), connectInfo->_stub, connectInfo->ToString().c_str());
        
        giveup = true;
        return Status::SockError_IllegalIp;
    }

    // 黑白名单
    auto ipRuleMgr = GetComp<IpRuleMgr>();
    if(!ipRuleMgr->Check(connectPendingInfo->_currentTargetIp))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("refuse connecting by black white list remote =%s!"), connectInfo->_targetIp.ToString().c_str());
        giveup = true;
        return Status::BlackWhiteCheckFail;
    }

    bool doBind = !connectInfo->_localIp._ip.empty() || (connectInfo->_localPort != 0);
    
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

    // 绑定本地ip的话可以通过多网卡来实现消息的负载均衡(内网可以流转的更快)
    if(doBind)
    {
        errCode = SocketUtil::Bind(connectPendingInfo->_newSock, connectInfo->_localIp._ip, connectInfo->_localPort, connectInfo->_family);
        if(errCode != Status::Success)
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("bind to ip[%s:%hu] sock[%d] fail when check connect!")
                                        , connectInfo->_localIp._ip.c_str(), connectInfo->_localPort, connectPendingInfo->_newSock);
            return errCode;
        }
    }

    errCode = SocketUtil::Connect(connectPendingInfo->_newSock, &connectPendingInfo->_remoteAddr._addr);
    if(errCode == Status::Success)
    {
        g_Log->NetInfo(LOGFMT_OBJ_TAG("check connect suc pending info:%s"), connectPendingInfo->ToString().c_str());
        return Status::Success;
    }
    else if(errCode == Status::SockError_EWOULDBLOCK)
    {
        g_Log->NetInfo(LOGFMT_OBJ_TAG("sock ewould block pending info:%s"), connectPendingInfo->ToString().c_str());
        return Status::SockError_Pending;
    }
    else
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("%s connect fail errno = [%d, %s] errCode=[%d]")
                    , connectPendingInfo->ToString().c_str(), errno, SystemUtil::GetErrString(errno).c_str(), errCode);
        return Status::Socket_Error;
    }

    g_Log->NetError(LOGFMT_OBJ_TAG("%s connect fail unknown"), connectInfo->ToString().c_str());

    return Status::Socket_Error;
}

LibConnectPendingInfo *EpollTcpPoller::_CreateNewConectPendingInfo(LibConnectInfo *connectInfo, const KERNEL_NS::LibString &currentTargetIp, Int32 leftTimes)
{
    auto connectPendingInfo = LibConnectPendingInfo::NewThreadLocal_LibConnectPendingInfo();
    if(SocketUtil::IsIpv4(currentTargetIp))
    {
        connectPendingInfo->_remoteAddr._addr._isIpv4 = true;
        if (!SocketUtil::FillTcpAddrInfo(currentTargetIp.c_str(), connectInfo->_targetPort, connectInfo->_family, connectPendingInfo->_remoteAddr._addr._data._sinV4))
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("FillTcpAddrInfo ipv4 fail ip[%s] port[%hu]"), connectInfo->_targetIp.ToString().c_str(), connectInfo->_targetPort);
            
            return NULL;
        }  
    }
    else
    {
        connectPendingInfo->_remoteAddr._addr._isIpv4 = false;
        if (!SocketUtil::FillTcpAddrInfo(currentTargetIp.c_str(), connectInfo->_targetPort, connectInfo->_family, connectPendingInfo->_remoteAddr._addr._data._sinV6))
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("FillTcpAddrInfo ipv6 fail ip[%s] port[%hu]"), connectInfo->_targetIp.ToString().c_str(), connectInfo->_targetPort);
            
            return NULL;
        }  
    }

    connectPendingInfo->_connectInfo = connectInfo;
    connectPendingInfo->_leftRetryTimes = leftTimes;
    connectPendingInfo->_currentTargetIp = currentTargetIp;

    return connectPendingInfo;
}

bool EpollTcpPoller::_TryGetNewTargetIp(const KERNEL_NS::AddrIpConfig &targetIp, std::set<KERNEL_NS::LibString> &filter, KERNEL_NS::LibString &currentIp)
{
    if(targetIp._isHostName)
    {
        // 初始创建使用第一个ip
        auto err = KERNEL_NS::IPUtil::GetIpByHostName(targetIp._ip, currentIp, filter, 0, false, true, targetIp._toIpv4);
        if(err != Status::Success)
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("GetIpByHostName fail targetIp:%s, filter:[%s]"), targetIp.ToString().c_str(), KERNEL_NS::StringUtil::ToString(filter, ',').c_str());
            return false;
        }

        return true;
    }

    if(filter.find(targetIp._ip) != filter.end())
        return false;

    currentIp = targetIp._ip;

    return true;
}


EpollTcpSession *EpollTcpPoller::_CreateSession(BuildSessionInfo *sessionInfo)
{
    g_Log->NetInfo(LOGFMT_OBJ_TAG("will create session by build session info:%s"), sessionInfo->ToString().c_str());

    if(UNLIKELY(sessionInfo->_serviceId == 0))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("must specify a belong service id： connect remote service id from LibConnectInfo, others from accept session service id sessionInfo:%s"), sessionInfo->ToString().c_str());
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
    auto newSession = EpollTcpSession::NewThreadLocal_EpollTcpSession(sessionInfo->_sessionId, sessionInfo->_isLinker, sessionInfo->_isFromConnect);
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
        EpollTcpSession::DeleteThreadLocal_EpollTcpSession(newSession);
        return NULL;
    }

    // pending 数量转真实数量
    _pollerMgr->ReduceSessionPending(1);
    _pollerMgr->AddSessionQuantity(1);
    
    _sessionIdRefSession.insert(std::make_pair(newSession->GetId(), newSession));

    auto iterIp = _ipRefSessions.find(sessionInfo->_remoteAddr._ip);
    if(iterIp == _ipRefSessions.end())
        iterIp = _ipRefSessions.insert(std::make_pair(sessionInfo->_remoteAddr._ip, std::set<EpollTcpSession *>())).first;
    iterIp->second.insert(newSession);

    _sessionCount.fetch_add(1, std::memory_order_release);
    if(newSession->IsConnectToRemote())
        _pollerMgr->AddConnectedSessionCount(1);
    else if(newSession->IsLinker())
        _pollerMgr->AddListenerSessionCount(1);
    else
        _pollerMgr->AddAcceptedSessionCount(1);

    g_Log->NetInfo(LOGFMT_OBJ_TAG("create session by build session info suc session info:%s"), newSession->ToString().c_str());
    return newSession;
}

EpollTcpSession *EpollTcpPoller::_CreateSession(LibListenInfo *listenInfo)
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
    auto newSession = EpollTcpSession::NewThreadLocal_EpollTcpSession(newSessionId, true, false);
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
        g_Log->NetError(LOGFMT_OBJ_TAG("init session fail listen info:%s, errCode:%d"), newSession->ToString().c_str(), errCode);
        EpollTcpSession::DeleteThreadLocal_EpollTcpSession(newSession);
        return NULL;
    }

    _epoll->AddEvent(sock->GetSock(), newSessionId, __ADD_LIB_EPOLL_ACCEPT_EVENT_FLAGS_DEF__);
    
    _sessionIdRefSession.insert(std::make_pair(newSession->GetId(), newSession));

    _sessionCount.fetch_add(1, std::memory_order_release);
    _pollerMgr->AddSessionQuantity(1);

    // 统计数量
    _pollerMgr->AddListenerSessionCount(1);

    g_Log->NetInfo(LOGFMT_OBJ_TAG("create session by listen info suc session info:%s"), newSession->ToString().c_str());
    return newSession;
}

void EpollTcpPoller::_CloseSession(EpollTcpSession *session, Int32 closeReasonEnum, UInt64 stub)
{
    g_Log->NetInfo(LOGFMT_OBJ_TAG("poller id:%llu, closeReasonEnum:%d,%s, stub:%llu, close session :%s")
                , GetPollerId(), closeReasonEnum, CloseSessionInfo::GetCloseReason(closeReasonEnum), stub, session->ToString().c_str());
    
    // 解除epoll监听
    if(session->IsLinker())
        _epoll->DelEvent(session->GetSock()->GetSock(), session->GetId(), __ADD_LIB_EPOLL_ACCEPT_EVENT_FLAGS_DEF__);
    else
        _epoll->DelEvent(session->GetSock()->GetSock(), session->GetId(), __ADD_LIB_EPOLL_EVENT_FLAGS_DEF__);
    _poller->GetDirtyHelper()->Clear(session);

    // 默认都是需要destroy event 不管stub是否有值 session关闭的消息
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
                auto sessions = _GetSessionsByIp(addr->GetRemoteIpStr());
                if(sessions)
                    sessions->erase(session);
            }
        }
    }

    session->Close();
    _sessionIdRefSession.erase(session->GetId());
    _sessionCount.fetch_sub(1, std::memory_order_release);

    if(session->IsLinker())
        _pollerMgr->ReduceListenerSessionCount(1);
    else if(session->IsConnectToRemote())
        _pollerMgr->ReduceConnectedSessionCount(1);
    else
        _pollerMgr->ReduceAcceptedSessionCount(1);
        
    _pollerMgr->ReduceSessionQuantity(1);

    EpollTcpSession::DeleteThreadLocal_EpollTcpSession(session);
}

bool EpollTcpPoller::_CanClose(EpollTcpSession *session) const
{
    if(UNLIKELY(session->IsLinker()))
        return true;

    return !session->CanRecv() && !session->CanSend();
}

void EpollTcpPoller::_TryCloseSession(EpollTcpSession *session, Int32 closeReasonEnum, UInt64 stub)
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

void EpollTcpPoller::_ControlCloseSession(EpollTcpSession *session, Int32 closeReason, Int64 closeMillisecondTime, UInt64 stub, bool forbidRead, bool forbidWrite)
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
        _poller->GetTimerMgr()->TakeOverLifeTime(newTimer, [](LibTimer *t){
            LibTimer::DeleteThreadLocal_LibTimer(t);
        });
        
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



KERNEL_END

#endif
