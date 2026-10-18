// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2026-09-24 22:09:26
// Author: Eric Yonng
// Description:

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_SESSION_BRIDGE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_SESSION_BRIDGE_H__

#pragma once

#include <kernel/comp/CompObject/CompObject.h>
#include <kernel/comp/Delegate/LibDelegate.h>
#include <kernel/comp/Coroutines/CoTask.h>

#include "kernel/comp/NetEngine/Defs/AddrIpConfig.h"
#include "kernel/comp/NetEngine/Poller/Defs/AsyncResult.h"

KERNEL_BEGIN

template<typename ObjType, typename BuildType = _Build::MT>
class LibList;

class LibSession;
class IProtocolStack;
class Poller;
struct PollerEvent;
struct NetConfig;
class TcpPollerMgr;

// session 桥接器, 如果是Listener 那么给Accept产生的SessionBridge, 如果是Connect创建的，由发起Connect方提供
class KERNEL_EXPORT ISessionBridge : public CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, ISessionBridge);

public:
    ISessionBridge(UInt64 objTypeId);
    
    ~ISessionBridge() override;

    // 发消息
    virtual void PostMsg(PollerEvent *msg, Int64 packetsCount = 0);
    // 获取协议栈
    virtual IProtocolStack *GetProtocolStack(LibSession *session) = 0;

    // ISessionBridge所在的线程Poller才可调用
    void DoEvents(LibList<PollerEvent *> *&events);

    // 订阅消息处理
    template<typename ObjType>
    void Subscribe(Int32 eventType, ObjType *obj, void (ObjType::*handler)(KERNEL_NS::PollerEvent *&));
    void Subscribe(Int32 eventType, void (*handler)(KERNEL_NS::PollerEvent *&));
    void Subscribe(Int32 eventType, KERNEL_NS::IDelegate<void, KERNEL_NS::PollerEvent *&> *deleg);
    void UnSubscribe(Int32 eventType);

    void SetPoller(Poller *poller);
    void SetNetConfig(const KERNEL_NS::NetConfig &config);
    
    
    /*
     * 监听
     * @param(protocolStackType): 协议类型
     * @param(port): 端口
     * @param(addr): ipv6/ipv4, 不支持域名
     * @param(sessionCount): 创建多少个会话监听该端口, 端口的复用, 控制连入负载均衡
     * @param(packetOptions): 套接字包选项
     * @return(AsyncResult):监听返回的错误码和sessionId, 成功就通过sessionId去处理会话
     */
    CoTask<KERNEL_NS::AsyncMultiResult> AddTcpListen(TcpPollerMgr *tcpMgr, Int32 protocolStackType, UInt16 port, const LibString addr = "0.0.0.0", Int32 sessionCount = 1, const KERNEL_NS::PacketOptions packetOptions = KERNEL_NS::PacketOptions());
    
    /*
     * 连接远程
     * @param(remoteAddr): 远程地址
     * @param(remotePort): 远程端口
     * @param(bindLocalAddr): 绑定本地地址
     * @param()
     */
    CoTask<Int32> AsyncConnect(const KERNEL_NS::AddrIpConfig remoteAddr, UInt16 remotePort, const AddrIpConfig bindLocalAddr = KERNEL_NS::AddrIpConfig(), UInt16 localPort = 0)
    
protected:
    Int32 _OnInit() override;
    Int32 _OnStart() override;

    // 派生类重写接口来定制PollerEvent回调, 纯虚函数以便提醒使用者实现该接口
    virtual void _OnSessionCreated(PollerEvent *&ev) = 0;
    virtual void _OnAsynConnectRes(PollerEvent *&ev) = 0;
    virtual void _OnAddListenRes(PollerEvent *&ev) final;
    virtual void _OnSessionDestroy(PollerEvent *&ev) = 0;
    virtual void _OnRecvMsg(PollerEvent *&ev) = 0;

    // 监听处理
    virtual void _OnBeforeAddListen(PollerEvent *&ev) = 0;

protected:
    Poller *_bridgePoller;
    NetConfig *_netConfig;
    std::unordered_map<Int32, KERNEL_NS::IDelegate<void, KERNEL_NS::PollerEvent *&> *> _pollerEventHandler;
};

template<typename ObjType>
ALWAYS_INLINE void ISessionBridge::Subscribe(Int32 eventType, ObjType *obj, void (ObjType::*handler)(KERNEL_NS::PollerEvent *&))
{
    auto delg = DelegateFactory::Create(obj, handler);
    Subscribe(eventType, delg);
}

ALWAYS_INLINE void ISessionBridge::Subscribe(Int32 eventType, void (*handler)(KERNEL_NS::PollerEvent *&))
{
    auto delg = DelegateFactory::Create(handler);
    Subscribe(eventType, delg);
}

ALWAYS_INLINE void ISessionBridge::UnSubscribe(Int32 eventType)
{
    auto iter = _pollerEventHandler.find(eventType);
    if(UNLIKELY(iter == _pollerEventHandler.end()))
        return;

    iter->second->Release();
    _pollerEventHandler.erase(iter);
}

ALWAYS_INLINE void ISessionBridge::SetPoller(Poller *poller)
{
    _bridgePoller = poller;
}


KERNEL_END

#endif
