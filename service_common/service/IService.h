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

#include <kernel/comp/LibString.h>
#include <kernel/comp/CompObject/CompHostObject.h>
#include <kernel/comp/LibList.h>
#include <kernel/comp/Delegate/LibDelegate.h>

#include <service_common/common/common.h>
#include <service_common/service/ServiceOption.h>

#include <atomic>
#include <set>

KERNEL_BEGIN

class Poller;
struct PollerEvent;
class LibSession;
class IProtocolStack;
class TimerMgr;
class LibPacket;
class EventManager;
class IPollerMgr;
class TcpPollerMgr;

KERNEL_END

SERVICE_COMMON_BEGIN

class ServiceProxy;
class Application;

struct ServiceStatisticsInfo;

// 默认的servicestatus，也可以根据业务需要来设置
class ServiceStatus
{
public:
    enum ENUMS
    {
        SERVICE_NOT_ACTIVE = 0,
        SERVICE_INITING = 1,
        SERVICE_INITED = 2,
        SERVICE_STARTING = 3,
        SERVICE_STARTED = 4,
        SERVICE_WILL_QUIT = 5,
        SERVICE_QUITED = 6,
        SERVICE_CLOSING = 7,
        SERVICE_CLOSED = 8,

        SERVICE_STATUS_END,
    };

    static KERNEL_NS::LibString ToString(Int32 serviceStatus)
    {
        switch (serviceStatus)
        {
        case SERVICE_NOT_ACTIVE: return "SERVICE_NOT_ACTIVE";
        case SERVICE_INITING: return "SERVICE_INITING";
        case SERVICE_INITED: return "SERVICE_INITED";
        case SERVICE_STARTING: return "SERVICE_STARTING";
        case SERVICE_STARTED: return "SERVICE_STARTED";
        case SERVICE_WILL_QUIT: return "SERVICE_WILL_QUIT";
        case SERVICE_CLOSING: return "SERVICE_CLOSING";
        case SERVICE_CLOSED: return "SERVICE_CLOSED";
        default:
            break;
        }

        return KERNEL_NS::LibString().AppendFormat("unknown service status:%d", serviceStatus);
    }
};

// TODO:改变poller
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

    virtual UInt64 GetSessionAmount() const;
    virtual const KERNEL_NS::PollerConfig &GetPollerConfig() const = 0;

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
    virtual const KERNEL_NS::IProtocolStack *GetProtocolStack(KERNEL_NS::LibSession *session) const = 0;
    virtual KERNEL_NS::IProtocolStack *GetProtocolStack(Int32 prototalStackType) = 0;
    virtual const KERNEL_NS::IProtocolStack *GetProtocolStack(Int32 prototalStackType) const = 0;

    // 获取定时器
    virtual KERNEL_NS::TimerMgr *GetTimerMgr() = 0;
    virtual const KERNEL_NS::TimerMgr *GetTimerMgr() const = 0;

    // 协议订阅 已经存在的订阅会被新的覆盖并报warn 专门定制Protocol消息
    template<typename ObjType>
    void Subscribe(Int32 opcodeId, ObjType *obj, void (ObjType::*Handler)(KERNEL_NS::LibPacket *&));
    virtual void Subscribe(Int32 opcodeId, KERNEL_NS::IDelegate<void, KERNEL_NS::LibPacket *&> *deleg) = 0;

    virtual KERNEL_NS::EventManager *GetEventMgr() = 0;
    virtual const KERNEL_NS::EventManager *GetEventMgr() const = 0;

    // 初始化从poller传来的消息处理接口
    void InitPollerEventHandler();

    // 监控信息
    virtual void OnMonitor(ServiceStatisticsInfo &info);

    // service模块是否退出
    virtual bool CheckServiceModuleQuitEnd(KERNEL_NS::LibString &notEndInfo) const;
    // service模块退出
    virtual void MaskServiceModuleQuitFlag(const KERNEL_NS::CompObject *comp);
    // 是否退出
    virtual bool IsServiceModuleQuit(const KERNEL_NS::CompObject *comp) const;
    // 注册需要关注的模块
    virtual void RegisterFocusServiceModule(const KERNEL_NS::CompObject *comp);
    // service退出标志
    bool IsServiceWillQuit() const;
    // service状态
    Int32 GetServiceStatus() const;
    // 只能往高的状态设置,不可往低的状态设置，否则会报错处理
    void SetServiceStatus(Int32 serviceStatus);
    // service status 接口,可以根据业务需要来重写默认使用 ServiceStatus
    virtual KERNEL_NS::LibString ServiceStatusToString(Int32 serviceStatus) const;

    // 设置事件最大类型
    virtual void SetMaxEventType(Int32 maxEventType);
    virtual Int32 GetMaxEventType() const;

    const std::set<const KERNEL_NS::CompObject *> &GetALlFocusServiceModule() const;
    std::set<const KERNEL_NS::CompObject *> &GetALlFocusServiceModule();

    // 获得消息优先级
    Int32 GetMaxPriorityLevel() const;

    // 获取pollermgr
    KERNEL_NS::IPollerMgr *GetPollerMgr();
    const KERNEL_NS::IPollerMgr *GetPollerMgr() const;

    // 获取tcppollermgr
    KERNEL_NS::TcpPollerMgr *GetTcpPollerMgr();
    const KERNEL_NS::TcpPollerMgr *GetTcpPollerMgr() const;

protected:
    // 在组件初始化前
    virtual Int32 _OnHostInit() final;
    // 带优先级组件创建
    virtual Int32 _OnPriorityLevelCompsCreated() final;
    // 派生服务执行优先级组件创建完成
    virtual Int32 _OnServicePriorityLevelCompsCreated() { return Status::Success; }
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

    //事件循环开始
    virtual void _OnEventLoopStart() {}

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
    virtual void _OnQuitServiceEvent(KERNEL_NS::PollerEvent *msg) final;
    virtual void _OnQuitingService(KERNEL_NS::PollerEvent *msg){}

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

    Int32 _maxEventType;

    std::set<const KERNEL_NS::CompObject *> _quitEndComps;
    std::set<const KERNEL_NS::CompObject *> _forcusComps;
    Int32 _serviceStatus;
    KERNEL_NS::IPollerMgr *_pollerMgr;
    KERNEL_NS::TcpPollerMgr *_tcpPollerMgr;
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

ALWAYS_INLINE void IService::AddRecvPackets(Int64 recvPackets)
{
    _recvPackets.fetch_add(recvPackets, std::memory_order_release);
}

ALWAYS_INLINE void IService::AddConsumePackets(Int64 recvPackets)
{
    _consumePackets.fetch_add(recvPackets, std::memory_order_release);
}

template<typename ObjType>
ALWAYS_INLINE void IService::Subscribe(Int32 opcodeId, ObjType *obj, void (ObjType::*Handler)(KERNEL_NS::LibPacket *&))
{
    auto delg = KERNEL_NS::DelegateFactory::Create(obj, Handler);
    Subscribe(opcodeId, delg);
}

ALWAYS_INLINE const std::set<const KERNEL_NS::CompObject *> &IService::GetALlFocusServiceModule() const
{
    return _forcusComps;
}

ALWAYS_INLINE std::set<const KERNEL_NS::CompObject *> &IService::GetALlFocusServiceModule()
{
    return _forcusComps;
}

ALWAYS_INLINE Int32 IService::GetMaxPriorityLevel() const
{
    return _maxPriorityLevel;
}

// 获取pollermgr
ALWAYS_INLINE KERNEL_NS::IPollerMgr *IService::GetPollerMgr()
{
    return _pollerMgr;
}

ALWAYS_INLINE const KERNEL_NS::IPollerMgr *IService::GetPollerMgr() const
{
    return _pollerMgr;
}

// 获取tcppollermgr
ALWAYS_INLINE KERNEL_NS::TcpPollerMgr *IService::GetTcpPollerMgr()
{
    return _tcpPollerMgr;
}

ALWAYS_INLINE const KERNEL_NS::TcpPollerMgr *IService::GetTcpPollerMgr() const
{
    return _tcpPollerMgr;
}

ALWAYS_INLINE bool IService::IsServiceWillQuit() const
{
    return _serviceStatus == ServiceStatus::SERVICE_WILL_QUIT;
}

ALWAYS_INLINE Int32 IService::GetServiceStatus() const
{
    return _serviceStatus;
}

SERVICE_COMMON_END

#endif
