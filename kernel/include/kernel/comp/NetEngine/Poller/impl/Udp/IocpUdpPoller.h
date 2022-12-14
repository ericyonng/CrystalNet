// /*!
//  *  MIT License
//  *  
//  *  Copyright (c) 2020 ericyonng<120453674@qq.com>
//  *  
//  *  Permission is hereby granted, free of charge, to any person obtaining a copy
//  *  of this software and associated documentation files (the "Software"), to deal
//  *  in the Software without restriction, including without limitation the rights
//  *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  *  copies of the Software, and to permit persons to whom the Software is
//  *  furnished to do so, subject to the following conditions:
//  *  
//  *  The above copyright notice and this permission notice shall be included in all
//  *  copies or substantial portions of the Software.
//  *  
//  *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  *  SOFTWARE.
//  * 
//  * Date: 2022-04-21 18:08:09
//  * Author: Eric Yonng
//  * Description: 
// */

// #ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_TCP_IOCP_TCP_POLLER_H__
// #define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_TCP_IOCP_TCP_POLLER_H__


// #pragma once

// #include <kernel/kernel_inc.h>
// #include <kernel/comp/CompObject/CompObjectInc.h>
// #include <kernel/comp/NetEngine/Poller/Defs/PollerEvent.h>
// #include <kernel/comp/LibList.h>
// #include <kernel/comp/Delegate/Delegate.h>
// #include <kernel/comp/LibDirtyHelper.h>

// #if CRYSTAL_TARGET_PLATFORM_WINDOWS

// KERNEL_BEGIN

// class Poller;
// class IocpTcpSession;
// class TcpPollerMgr;
// struct TcpPollerInstConfig;
// class LibIocp;
// class LibThread;
// struct LibConnectInfo;
// struct LibConnectPendingInfo;
// struct BuildSessionInfo;
// struct LibListenInfo;
// class LibPacket;
// class IPollerMgr;
// class IServiceProxy;
// struct IpControlInfo;

// class KERNEL_EXPORT IocpTcpPoller : public CompHostObject
// {
//     POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, IocpTcpPoller);

// public:
//     IocpTcpPoller(TcpPollerMgr *pollerMgr, UInt64 pollerId, const TcpPollerInstConfig * cfg);
//     ~IocpTcpPoller();
//     void Release() override;
//     // ??????????????????????????????????????????ready
//     void DefaultMaskReady(bool isReady) override {}
    
// public:
//     static void InitStatic();
//     void OnRegisterComps() override;
//     void Clear() override;
//     LibString ToString() const override;
//     UInt64 CalcLoadScore() const;
//     UInt64 GetPollerId() const;
//     UInt64 GetSessionAmount() const;

//     // ??????????????????
// public:
//     void PostSend(Int32 level, UInt64 sessionId, LibPacket *packet);
//     void PostSend(Int32 level, UInt64 sessionId, LibList<LibPacket *> *packets);
//     void PostNewSession(Int32 level, BuildSessionInfo *buildSessionInfo);
//     void PostAddlisten(Int32 level, LibListenInfo *listenInfo);
//     void PostAddlistenList(Int32 level, std::vector<LibListenInfo *> &listenInfoList);
//     void PostConnect(Int32 level, LibConnectInfo *connectInfo);
//     void PostCloseSession(UInt64 fromServiceId, Int32 level,  UInt64 sessionId, Int64 closeMillisecondTime, bool forbidRead, bool forbidWrite);
//     void PostIpControl(Int32 level, const std::list<IpControlInfo *> &controlList);
//     void PostQuitServiceSessionsEvent(UInt64 serviceId, Int32 level = 0);

// private:
//     IocpTcpSession *_GetSession(UInt64 sessionId);
//     std::set<IocpTcpSession *> &_GetSessionsByIp(const LibString &ip);
//     IocpTcpSession *_CreateSession(BuildSessionInfo *sessionInfo);
//     IocpTcpSession *_CreateSession(LibListenInfo *listenInfo);
//     void _CloseSession(IocpTcpSession *session, Int32 closeReasonEnum, UInt64 stub);
//     bool _CanClose(IocpTcpSession *session) const;
//     void _TryCloseSession(IocpTcpSession *session, Int32 closeReasonEnum, UInt64 stub);
//     void _ControlCloseSession(IocpTcpSession *session, Int32 closeReason, Int64 closeMillisecondTime, UInt64 stub, bool forbidRead = true, bool forbidWrite = true);

// private:
//     bool _TryHandleConnecting(IoEvent &io, Int32 ioErrCode);

// private:
//     // ?????????????????????
//     Int32 _OnHostInit() override;
//     // ????????????????????????
//     Int32 _OnCompsCreated() override;
//     // ?????????????????????
//     Int32 _OnHostWillStart() override;
//     // ??????????????????
//     Int32 _OnHostStart() override;
//     // ?????????willclose??????
//     void _OnHostBeforeCompsWillClose() override;
//     // ?????????willclose??????
//     void _OnHostWillClose() override;
//     // ?????????Close??????
//     void _OnHostClose() override;
//     // ??????
//     void _Clear();

//     // poller????????????
//     bool _OnPollerPrepare(Poller *poller);
//     void _OnPollerWillDestroy(Poller *poller);
//     void _OnPollerEvent(PollerEvent *ev);

//     // poller??????
//     void _OnWrite(PollerEvent *ev);
//     void _OnAsynConnect(PollerEvent *ev);
//     void _OnNewSession(PollerEvent *ev);
//     void _OnMonitor(PollerEvent *ev);
//     void _OnCloseSession(PollerEvent *ev);
//     void _OnAddListen(PollerEvent *ev);
//     void _OnIpRuleControl(PollerEvent *ev);
//     void _OnQuitServiceSessionsEvent(PollerEvent *ev);
//     void _OnRealDoQuitServiceSessionEvent(PollerEvent *ev);

//     void _OnConnectSuc(LibConnectPendingInfo *&connectPendingInfo);
//     void _OnConnectFailure(LibConnectInfo *connectInfo, LibConnectPendingInfo *connectPending, Int32 errCode);

//     void _OnAccept(IocpTcpSession *session, IoEvent &io);

//     // ?????????
//     void _OnDirtySessionAccept(LibDirtyHelper<void *, UInt32> *dirtyHelper, void *session, Variant *params);
//     void _OnDirtySessionWrite(LibDirtyHelper<void *, UInt32> *dirtyHelper, void *session, Variant *params);
//     void _OnDirtySessionRead(LibDirtyHelper<void *, UInt32> *dirtyHelper, void *session, Variant *params);
//     void _OnDirtySessionClose(LibDirtyHelper<void *, UInt32> *dirtyHelper, void *session, Variant *params);

//     void _OnMonitorThread(LibThread *t);

//     // ??????????????????
//     void _OnPollEventLoop(LibThread *t);
    
// private:
//     void _DestroyConnect(LibConnectPendingInfo *&connectPendingInfo, bool destroyConnectInfo);
//     Int32 _CheckConnect(LibConnectPendingInfo *&connectPendingInfo, bool &giveup);
//     LibConnectPendingInfo *_CreateNewConectPendingInfo(LibConnectInfo *connectInfo, Int32 leftTimes);

// private:
//     const UInt64 _pollerId;
//     TcpPollerMgr *_tcpPollerMgr;
//     IPollerMgr *_pollerMgr;
//     IServiceProxy *_serviceProxy;
//     Poller *_poller;

//     const TcpPollerInstConfig *_cfg;
//     std::map<UInt64, IocpTcpSession *> _sessionIdRefSession;
//     std::map<KERNEL_NS::LibString, std::set<IocpTcpSession *>> _ipRefSessions;

//     std::atomic<UInt64> _sessionCount;
//     std::atomic<UInt64> _sessionPendingCount;
//     std::map<UInt64, LibConnectPendingInfo *> _sessionIdRefAsynConnectPendingInfo;  // ??????????????????

//     LibIocp *_iocp;                // iocp ??????
//     UInt64 _wakeupSessionId;        // ????????????sessionId
//     IDelegate<void> *_quitIocpWorker;   // iocp worker????????????

//     LibThread *_monitor;                    // ????????????
//     LibThread *_eventLoopThread;            // ??????????????????
//     Int32 _pollerInstMonitorPriorityLevel;  // ??????????????????????????????

//     // ??????
//     typedef void (IocpTcpPoller::*PollerEventHandler)(PollerEvent *);
//     static PollerEventHandler _eventHandlerArray[PollerEventType::EvMax];
// };

// ALWAYS_INLINE UInt64 IocpTcpPoller::GetPollerId() const
// {
//     return _pollerId;
// }

// ALWAYS_INLINE UInt64 IocpTcpPoller::GetSessionAmount() const
// {
//     return _sessionCount;
// }

// KERNEL_END

// #endif

// #endif
