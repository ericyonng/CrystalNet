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
 * Date: 2022-04-21 18:08:01
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_TCP_EPOLL_TCP_POLLER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_TCP_EPOLL_TCP_POLLER_H__

#pragma once

#include <kernel/common/macro.h>

#if CRYSTAL_TARGET_PLATFORM_LINUX

#include <kernel/common/LibObject.h>
#include <kernel/comp/CompObject/CompHostObject.h>
#include <kernel/comp/LibList.h>

#include <vector>
#include <list>
#include <set>
#include <map>
#include <atomic>

KERNEL_BEGIN

class Variant;

struct PollerEvent;

class Poller;
class EpollTcpSession;
class TcpPollerMgr;
struct TcpPollerInstConfig;
class LibEpoll;
class LibThread;
struct LibConnectInfo;
struct LibConnectPendingInfo;
struct BuildSessionInfo;
struct LibListenInfo;
class LibPacket;
class IPollerMgr;
class IServiceProxy;
struct IpControlInfo;
class TlsMemoryCleanerComp;


template<typename KeyType, typename MaskValue>
class LibDirtyHelper;

class EpollTcpPoller : public CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, EpollTcpPoller);

public:
    EpollTcpPoller(TcpPollerMgr *pollerMgr, UInt64 pollerId, const TcpPollerInstConfig *cfg);
    virtual ~EpollTcpPoller();
    void Release() override;
    // 有多线程所以这个时候不能直接ready
    void DefaultMaskReady(bool isReady) override {}

public:
    void OnRegisterComps() override;
    void Clear() override;
    LibString ToString() const override;
    UInt64 CalcLoadScore() const;
    UInt64 GetPollerId() const;
    UInt64 GetSessionAmount() const;

    // 异步事件投递
public:
    void PostSend(Int32 level, UInt64 sessionId, LibPacket *packet);
    void PostSend(Int32 level, UInt64 sessionId, LibList<LibPacket *> *packets);
    void PostNewSession(Int32 level, BuildSessionInfo *buildSessionInfo);
    void PostAddlisten(Int32 level, LibListenInfo *listenInfo);
    void PostAddlistenList(Int32 level, std::vector<LibListenInfo *> &listenInfoList);
    void PostConnect(Int32 level, LibConnectInfo *connectInfo);
    void PostCloseSession(UInt64 fromServiceId, Int32 level, UInt64 sessionId, Int64 closeMillisecondTimeDelay, bool forbidRead, bool forbidWrite);
    void PostIpControl(Int32 level, const std::list<IpControlInfo *> &controlList);
    void PostQuitServiceSessionsEvent(UInt64 serviceId, Int32 level = 0);

private:
    EpollTcpSession *_GetSession(UInt64 sessionId);
    std::set<EpollTcpSession *> *_GetSessionsByIp(const LibString &ip);
    EpollTcpSession *_CreateSession(BuildSessionInfo *sessionInfo);
    EpollTcpSession *_CreateSession(LibListenInfo *listenInfo);
    void _CloseSession(EpollTcpSession *session, Int32 closeReasonEnum, UInt64 stub);
    bool _CanClose(EpollTcpSession *session) const;
    void _TryCloseSession(EpollTcpSession *session, Int32 closeReasonEnum, UInt64 stub);
    void _ControlCloseSession(EpollTcpSession *session, Int32 closeReason, Int64 closeMillisecondTime, UInt64 stub, bool forbidRead = true, bool forbidWrite = true);
    
private:
    void _SendData(EpollTcpSession *session, LibList<LibPacket *> *packets);
    bool _TryHandleConnecting(UInt64 sessionId, Int32 events);

private:
    // 优先级组件创建完成
    Int32 _OnPriorityLevelCompsCreated() override;
    // 所有组件创建完成
    Int32 _OnCompsCreated() override;
    // 在组件初始化前
    Int32 _OnHostInit() override;
    // 在组件启动之前
    Int32 _OnHostWillStart() override;
    // 组件启动之后
    Int32 _OnHostStart() override;
    // 在组件willclose之前
    void _OnHostBeforeCompsWillClose() override;
    // 在组件willclose之后
    void _OnHostWillClose() override;
    // 在组件Close之后
    void _OnHostClose() override;
    // 清理
    void _Clear();

    // poller组件回调
    bool _OnPollerPrepare(Poller *poller);
    void _OnPollerWillDestroy(Poller *poller);

    // poller事件
    void _OnWrite(PollerEvent *ev);
    void _OnAsynConnect(PollerEvent *ev);
    void _OnNewSession(PollerEvent *ev);
    void _OnMonitor(PollerEvent *ev);
    void _OnCloseSession(PollerEvent *ev);
    void _OnAddListen(PollerEvent *ev);
    void _OnIpRuleControl(PollerEvent *ev);
    void _OnQuitServiceSessionsEvent(PollerEvent *ev);
    void _OnRealDoQuitServiceSessionEvent(PollerEvent *ev);

    void _OnConnectSuc(LibConnectPendingInfo *&connectPendingInfo);
    void _OnConnectPending(LibConnectPendingInfo *&connectPendingInfo);
    void _OnConnectFailure(LibConnectInfo *connectInfo, LibConnectPendingInfo *connectPending, Int32 errCode);

    void _OnAccept(EpollTcpSession *session);
    Int32 _OnAcceptedNew(SOCKET sock, EpollTcpSession *session);

    // 脏事件
    void _OnDirtySessionAccept(LibDirtyHelper<void *, UInt32> *dirtyHelper, void *&session, Variant *params);
    void _OnDirtySessionWrite(LibDirtyHelper<void *, UInt32> *dirtyHelper, void *&session, Variant *params);
    void _OnDirtySessionRead(LibDirtyHelper<void *, UInt32> *dirtyHelper, void *&session, Variant *params);
    void _OnDirtySessionClose(LibDirtyHelper<void *, UInt32> *dirtyHelper, void *&session, Variant *params);

    // epoll 监控线程
    void _OnMonitorThread(LibThread *t);

    // 事件循环线程
    void _OnPollEventLoop(LibThread *t);

private:
    void _DestroyConnect(LibConnectPendingInfo *&connectPendingInfo, bool destroyConnectInfo);
    Int32 _CheckConnect(LibConnectPendingInfo *&connectPendingInfo, bool &giveup);
    LibConnectPendingInfo *_CreateNewConectPendingInfo(LibConnectInfo *connectInfo, Int32 leftTimes);

private:
    const UInt64 _pollerId;
    TcpPollerMgr *_tcpPollerMgr;
    IPollerMgr *_pollerMgr;
    IServiceProxy *_serviceProxy;
    Poller *_poller;
    TlsMemoryCleanerComp *_memoryCleaner;

    const TcpPollerInstConfig *_cfg;
    std::map<UInt64, EpollTcpSession *> _sessionIdRefSession;
    std::map<LibString, std::set<EpollTcpSession *>> _ipRefSessions;

    std::atomic<UInt64> _sessionCount;
    std::atomic<UInt64> _sessionPendingCount;
    std::map<UInt64, LibConnectPendingInfo *> _sessionIdRefAsynConnectPendingInfo;  // 连接中未完成

    LibEpoll *_epoll;               // epoll 对象
    UInt64 _wakeupSessionId;        // 事件唤醒sessionId
    Int32 _wakeupEventFd;           // 事件唤醒

    LibThread *_monitor;                    // 监听线程
    LibThread *_eventLoopThread;            // 事件循环线程
    Int32 _pollerInstMonitorPriorityLevel;  // 监听线程的消息优先级

};

ALWAYS_INLINE EpollTcpSession *EpollTcpPoller::_GetSession(UInt64 sessionId)
{
    auto iter = _sessionIdRefSession.find(sessionId);
    return iter == _sessionIdRefSession.end() ? NULL : iter->second;   
}

ALWAYS_INLINE std::set<EpollTcpSession *> *EpollTcpPoller::_GetSessionsByIp(const LibString &ip)
{
    auto iter = _ipRefSessions.find(ip);
    return iter == _ipRefSessions.end() ? NULL : &(iter->second);
}

ALWAYS_INLINE UInt64 EpollTcpPoller::GetPollerId() const
{
    return _pollerId;
}

ALWAYS_INLINE UInt64 EpollTcpPoller::GetSessionAmount() const
{
    return _sessionCount;
}

KERNEL_END

#endif

#endif
