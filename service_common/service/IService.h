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
 * Date: 2021-12-09 02:03:36
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_SERVICE_ISERVICE_H__
#define __CRYSTAL_NET_SERVICE_COMMON_SERVICE_ISERVICE_H__

#pragma once

#include <kernel/kernel.h>
#include <service_common/common/common.h>
#include <service_common/service/ServiceOption.h>

SERVICE_COMMON_BEGIN

class ServiceProxy;
class Application;

class IService : public KERNEL_NS::CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, IService);

public:
    IService();
    virtual ~IService();
    // 有多线程所以这个时候不能直接ready
    void DefaultMaskReady(bool isReady) override {}

public:
    virtual void Clear() final;
    virtual void OnRegisterComps() final;  
    virtual KERNEL_NS::LibString ToString() const override;
    virtual KERNEL_NS::LibString IntroduceInfo() const;

    void SetServiceId(UInt64 serviceId);
    UInt64 GetServiceId() const;
    
    void SetServiceName(const KERNEL_NS::LibString &serviceName);
    const KERNEL_NS::LibString &GetServiceName() const;

    void SetServiceProxy(ServiceProxy *proxy);
    ServiceProxy *GetServiceProxy();
    const ServiceProxy *GetServiceProxy() const;

    KERNEL_NS::Poller *GetPoller();
    const KERNEL_NS::Poller *GetPoller() const;

    const KERNEL_NS::LibString &GetAppName() const;
    const KERNEL_NS::LibString &GetAppAliasName() const;
    const Application *GetApp() const;
    Application *GetApp();

    // 事件循环
    bool PrepareLoop();
    void EventLoop();
    void OnLoopEnd();
    void Push(Int32 level, KERNEL_NS::PollerEvent *ev);
    void Push(Int32 level, KERNEL_NS::LibList<KERNEL_NS::PollerEvent *> *evList);
    void AddRecvPackets(Int64 recvPackets);
    void AddConsumePackets(Int64 recvPackets);

    // 网络线程调用
    virtual KERNEL_NS::IProtocolStack *GetProtocolStack(KERNEL_NS::LibSession *session) = 0;
    virtual KERNEL_NS::IProtocolStack *GetProtocolStack(Int32 sessionType) = 0;

    // 获取定时器
    virtual KERNEL_NS::TimerMgr *GetTimerMgr() = 0;
    virtual const KERNEL_NS::TimerMgr *GetTimerMgr() const = 0;

    // 协议订阅 已经存在的订阅会被新的覆盖并报warn
    template<typename ObjType>
    void Subscribe(Int32 opcodeId, ObjType *obj, void (ObjType::*Handler)(KERNEL_NS::LibPacket *&));
    virtual void Subscribe(Int32 opcodeId, KERNEL_NS::IDelegate<void, KERNEL_NS::LibPacket *&> *deleg) = 0;

    virtual KERNEL_NS::EventManager *GetEventMgr() = 0;
    virtual const KERNEL_NS::EventManager *GetEventMgr() const = 0;

    // 初始化从poller传来的消息处理接口
    static void InitPollerEventHandler();

    // 监控信息
    virtual void OnMonitor(KERNEL_NS::LibString &info);

protected:
    // 在组件初始化前
    virtual Int32 _OnHostInit() final;
    // 所有组件创建完成
    virtual Int32 _OnCompsCreated() final;
    // 在组件启动之前
    virtual Int32 _OnHostWillStart() final;
    // 组件启动之后
    virtual Int32 _OnHostStart() final;
    // 在组件willclose之前
    virtual void _OnHostBeforeCompsWillClose() final;
    // 在组件willclose之后
    virtual void _OnHostWillClose() final;
    // 在组件Close之后
    virtual void _OnHostClose() final;

// 派生接口
protected:
    // 清理数据
    virtual void _OnServiceClear();
    // 注册组件
    virtual void _OnServiceRegisterComps() = 0;
    // 服务初始化 配置
    virtual Int32 _OnServiceInit() = 0;
    // 服务组件创建完成
    virtual Int32 _OnServiceCompsCreated();
    // 服务完全启动
    virtual Int32 _OnServiceStartup();
    // 服务即将关闭
    virtual void _OnServiceWillClose();
    // 服务完成关闭
    virtual void _OnServiceClosed();

    // 收到消息
    virtual void _OnMsg(KERNEL_NS::PollerEvent *msg) final;
    // 会话创建
    virtual void _OnSessionCreated(KERNEL_NS::PollerEvent *msg);
    // 会话销毁
    virtual void _OnSessionDestroy(KERNEL_NS::PollerEvent *msg);
    // 连接回调
    virtual void _OnAsynConnectRes(KERNEL_NS::PollerEvent *msg);
    // 监听回调
    virtual void _OnAddListenRes(KERNEL_NS::PollerEvent *msg);
    // 收到网络消息回调
    virtual void _OnRecvMsg(KERNEL_NS::PollerEvent *msg);
    // 退出服务消息回调
    virtual void _OnQuitServiceEvent(KERNEL_NS::PollerEvent *msg);

    // 初始化相关
    virtual bool _OnPollerPrepare(KERNEL_NS::Poller *poller);
    // 销毁相关
    virtual void _OnPollerWillDestroy(KERNEL_NS::Poller *poller);

private:
    void _Clear();

protected:
    UInt64 _serviceId;
    KERNEL_NS::Poller *_poller;
    ServiceProxy *_serviceProxy;

    KERNEL_NS::LibString _serviceName;
    UInt64 _maxPieceTimeInMicroseconds;
    Int32 _maxPriorityLevel;
    UInt64 _maxSleepMilliseconds;

    std::atomic<Int64> _recvPackets;
    std::atomic<Int64> _consumePackets;

    typedef void (IService::*PollerEventHandler)(KERNEL_NS::PollerEvent *msg);
    static PollerEventHandler _pollerEventHandler[KERNEL_NS::PollerEventType::EvMax];
};

ALWAYS_INLINE void IService::SetServiceId(UInt64 serviceId)
{
    _serviceId = serviceId;
}

ALWAYS_INLINE UInt64 IService::GetServiceId() const
{
    return _serviceId;
}

ALWAYS_INLINE void IService::SetServiceName(const KERNEL_NS::LibString &serviceName)
{
    _serviceName = serviceName;
}

ALWAYS_INLINE const KERNEL_NS::LibString &IService::GetServiceName() const
{
    return _serviceName;
}

ALWAYS_INLINE void IService::SetServiceProxy(ServiceProxy *proxy)
{
    _serviceProxy = proxy;
}

ALWAYS_INLINE ServiceProxy *IService::GetServiceProxy()
{
    return _serviceProxy;
}

ALWAYS_INLINE const ServiceProxy *IService::GetServiceProxy() const
{
    return _serviceProxy;
}

ALWAYS_INLINE KERNEL_NS::Poller *IService::GetPoller()
{
    return _poller;
}

ALWAYS_INLINE const KERNEL_NS::Poller *IService::GetPoller() const
{
    return _poller;
}

ALWAYS_INLINE void IService::EventLoop()
{
    _poller->EventLoop();
}

ALWAYS_INLINE void IService::OnLoopEnd()
{
    _poller->OnLoopEnd();

    MaskReady(false);
}

ALWAYS_INLINE bool IService::PrepareLoop()
{
    if(!_poller->PrepareLoop())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("poller prepare loop fail please check."));
        return false;
    }

    MaskReady(true);

    return true;
}

ALWAYS_INLINE void IService::Push(Int32 level, KERNEL_NS::PollerEvent *ev)
{
    _poller->Push(level, ev);
}

ALWAYS_INLINE void IService::Push(Int32 level, KERNEL_NS::LibList<KERNEL_NS::PollerEvent *> *evList)
{
    _poller->Push(level, evList);
}

ALWAYS_INLINE void IService::AddRecvPackets(Int64 recvPackets)
{
    _recvPackets += recvPackets;
}

ALWAYS_INLINE void IService::AddConsumePackets(Int64 recvPackets)
{
    _consumePackets += recvPackets;
}

template<typename ObjType>
ALWAYS_INLINE void IService::Subscribe(Int32 opcodeId, ObjType *obj, void (ObjType::*Handler)(KERNEL_NS::LibPacket *&))
{
    auto delg = KERNEL_NS::DelegateFactory::Create(obj, Handler);
    Subscribe(opcodeId, delg);
}

SERVICE_COMMON_END

#endif
