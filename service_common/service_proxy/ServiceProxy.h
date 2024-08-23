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
 * Date: 2021-12-09 01:21:08
 * Author: Eric Yonng
 * Description: 
 *              session与服务的关系:鉴于连接与监控,session的消息要转发到哪个service或者哪组service上，应该看session的serviceMainType,或者serviceId决定
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_SERVICE_PROXY_SERVICE_PROXY_H__
#define __CRYSTAL_NET_SERVICE_COMMON_SERVICE_PROXY_SERVICE_PROXY_H__

#pragma once

#include <kernel/comp/LibString.h>
#include <kernel/comp/Service/IServiceProxy.h>
#include <kernel/comp/LibList.h>
#include <kernel/comp/Lock/Impl/SpinLock.h>

#include <service_common/common/common.h>
#include <service_common/service/ServiceOption.h>

#include <list>
#include <vector>
#include <unordered_map>
#include <atomic>

KERNEL_BEGIN

struct PollerEvent;
class LibSession;
class IProtocolStack;
class LibPacket;
struct SessionOption;
struct IpControlInfo;
class LibThread;
class Variant;
class TcpPollerMgr;
class IPollerMgr;

KERNEL_END

SERVICE_COMMON_BEGIN

class IService;
struct SimpleSessionInfo;
class IServiceFactory;
class Application;
struct ServiceProxyStatisticsInfo;

class ServiceProxy : public KERNEL_NS::IServiceProxy
{
    POOL_CREATE_OBJ_DEFAULT_P1(IServiceProxy, ServiceProxy);
public:
    ServiceProxy();
    virtual ~ServiceProxy();
    void Release() final;
    virtual void DefaultMaskReady(bool isReady){}

public:
    // kernel => service
    virtual void PostMsg(UInt64 serviceId, UInt32 priorityLevel, KERNEL_NS::PollerEvent *msg, Int64 packetsCount = 0) final;
    virtual void PostQuitService(UInt32 priorityLevel = 0) final;
    virtual KERNEL_NS::IProtocolStack *GetProtocolStack(KERNEL_NS::LibSession *session) final;

    // 设置服务创建工厂 在application init后主动调用
    void SetServiceFactory(IServiceFactory *serviceFactory);

    // 关闭app
    Application *GetApp();
    const Application *GetApp() const;
    void CloseApp(Int32 err = Status::Success);

    // 监控信息
    void OnMonitor(ServiceProxyStatisticsInfo &info);
    

private:
    virtual Int32 _OnInit() final;
    virtual Int32 _OnStart() final;
    virtual void _OnWillClose() final;
    virtual void _OnClose() final;
    virtual void Clear() final;

    // 清理资源
    void _Clear();

    // 拒绝服务
    void _RejectService(UInt64 serviceId);
    bool _IsRejectService(UInt64 serviceId);
    void _OnPrepareServiceThread(UInt64 serviceId, const KERNEL_NS::LibString &serviceName);

    IService *_GetService(UInt64 serviceId);
    const IService *_GetService(UInt64 serviceId) const;

    // params携带本线程归哪个服务独占
    void _OnServiceThread(KERNEL_NS::LibThread *t, KERNEL_NS::Variant *params);

private:
    KERNEL_NS::SpinLock _guard;                                     // 服务资源锁
    std::unordered_map<UInt64, IService *> _idRefService;                     // 服务
    std::vector<KERNEL_NS::LibThread *> _serviceThreads;            // 每个服务独立一个线程
    std::vector<KERNEL_NS::LibString> _activeServices;              // 激活的服务
    std::atomic<UInt64> _maxServiceId;                              // 分配serviceId
    std::atomic<UInt64> _closeServiceNum;                           // 服务关闭个数
    std::unordered_map<UInt64, std::atomic_bool> _serviceIdRefRejectServiceStatus;  // 拒绝服务标志

    IServiceFactory *_serviceFactory;
};

ALWAYS_INLINE void ServiceProxy::SetServiceFactory(IServiceFactory *serviceFactory)
{
    _serviceFactory = serviceFactory;
}

ALWAYS_INLINE Application *ServiceProxy::GetApp()
{
    return GetOwner()->CastTo<Application>();
}

ALWAYS_INLINE const Application *ServiceProxy::GetApp() const
{
    return GetOwner()->CastTo<Application>();
}

ALWAYS_INLINE void ServiceProxy::_RejectService(UInt64 serviceId)
{
    _guard.Lock();
    _serviceIdRefRejectServiceStatus[serviceId] = true;
    _guard.Unlock();
}

ALWAYS_INLINE bool ServiceProxy::_IsRejectService(UInt64 serviceId)
{
    _guard.Lock();
    bool isReject = false;
    auto iter = _serviceIdRefRejectServiceStatus.find(serviceId);
    if(iter != _serviceIdRefRejectServiceStatus.end())
        isReject = iter->second;
    _guard.Unlock();

    return isReject;
}

ALWAYS_INLINE IService *ServiceProxy::_GetService(UInt64 serviceId)
{
    auto iter = _idRefService.find(serviceId);
    return iter == _idRefService.end() ? NULL : iter->second;
}

ALWAYS_INLINE const IService *ServiceProxy::_GetService(UInt64 serviceId) const
{
    auto iter = _idRefService.find(serviceId);
    return iter == _idRefService.end() ? NULL : iter->second;
}

SERVICE_COMMON_END

#endif
