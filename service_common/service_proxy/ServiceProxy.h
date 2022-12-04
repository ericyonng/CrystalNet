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

#include <service_common/common/common.h>
#include <kernel/kernel.h>
#include <service_common/service/ServiceOption.h>

SERVICE_COMMON_BEGIN

class IService;
struct SimpleSessionInfo;
class IServiceFactory;
class Application;

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
    virtual void PostMsg(UInt64 serviceId, UInt32 priorityLevel, KERNEL_NS::PollerEvent *msg) final;
    virtual void PostQuitService(UInt32 priorityLevel = 0) final;
    virtual KERNEL_NS::IProtocolStack *GetProtocolStack(KERNEL_NS::LibSession *session) final;

    // service => kernel
    virtual void TcpAddListen(UInt64 fromServiceId, Int32 level, Int32 family, const KERNEL_NS::LibString &ip, UInt16 port, UInt64 stub, const KERNEL_NS::SessionOption &sessionOption) final;
    virtual void TcpAsynConnect(UInt64 fromServiceId, UInt64 stub, Int32 level, UInt16 family, const KERNEL_NS::LibString &remoteIp, UInt16 remotePort, const KERNEL_NS::SessionOption &sessionOption, KERNEL_NS::IProtocolStack *protocolStack = NULL, Int32 retryTimes = 3, Int64 periodMs = 5000, const KERNEL_NS::LibString &localIp = "", UInt16 localPort = 0) final;
    virtual void TcpSendMsg(UInt64 pollerId, Int32 level, UInt64 sessionId, KERNEL_NS::LibPacket *packet) final;
    virtual void TcpSendMsg(UInt64 pollerId, Int32 level, UInt64 sessionId, KERNEL_NS::LibList<KERNEL_NS::LibPacket *> *packets) final;
    virtual void TcpSendMsg(UInt64 pollerId, Int32 level, UInt64 sessionId, const std::list<KERNEL_NS::LibPacket *> &packets) final;
    virtual void TcpCloseSession(UInt64 pollerId, Int32 level, UInt64 sessionId) final;

    // ip
    void AddWhite(const KERNEL_NS::LibString &ip, Int32 level = 0);
    void AddBlack(const KERNEL_NS::LibString &ip, Int32 level = 0);
    void EraseWhite(const KERNEL_NS::LibString &ip, Int32 level = 0);
    void EraseBlack(const KERNEL_NS::LibString &ip, Int32 level = 0);
    void AddWhite(const std::list<KERNEL_NS::LibString> &ips, Int32 level = 0);
    void AddBlack(const std::list<KERNEL_NS::LibString> &ips, Int32 level = 0);
    void EraseWhite(const std::list<KERNEL_NS::LibString> &ips, Int32 level = 0);
    void EraseBlack(const std::list<KERNEL_NS::LibString> &ips, Int32 level = 0);
    void ControlIpPipline(const std::list<KERNEL_NS::IpControlInfo *> &controlInfoList, Int32 level = 0);

    // 设置服务创建工厂 在application init后主动调用
    void SetServiceFactory(IServiceFactory *serviceFactory);

    // 关闭app
    Application *GetApp();
    const Application *GetApp() const;
    void CloseApp(Int32 err = Status::Success);

private:
    virtual Int32 _OnInit() final;
    virtual Int32 _OnStart() final;
    virtual void _OnWillClose() final;
    virtual void _OnClose() final;
    virtual void Clear() final;

    // 清理资源
    void _Clear();

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

    KERNEL_NS::TcpPollerMgr *_tcpPollerMgr;
    KERNEL_NS::IPollerMgr *_pollerMgr;
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

ALWAYS_INLINE void ServiceProxy::AddWhite(const KERNEL_NS::LibString &ip, Int32 level)
{
    std::list<KERNEL_NS::IpControlInfo *> newList;
    auto newInfo = KERNEL_NS::IpControlInfo::Create();
    newInfo->_ip = ip;
    newInfo->_controlFlow.push_back(KERNEL_NS::IpControlInfo::ADD_WHITE);
    ControlIpPipline(newList, level);
}

ALWAYS_INLINE void ServiceProxy::AddBlack(const KERNEL_NS::LibString &ip, Int32 level)
{
    std::list<KERNEL_NS::IpControlInfo *> newList;
    auto newInfo = KERNEL_NS::IpControlInfo::Create();
    newInfo->_ip = ip;
    newInfo->_controlFlow.push_back(KERNEL_NS::IpControlInfo::ADD_BLACK);
    ControlIpPipline(newList, level);
}

ALWAYS_INLINE void ServiceProxy::EraseWhite(const KERNEL_NS::LibString &ip, Int32 level)
{
    std::list<KERNEL_NS::IpControlInfo *> newList;
    auto newInfo = KERNEL_NS::IpControlInfo::Create();
    newInfo->_ip = ip;
    newInfo->_controlFlow.push_back(KERNEL_NS::IpControlInfo::ERASE_WHITE);
    ControlIpPipline(newList, level);
}

ALWAYS_INLINE void ServiceProxy::EraseBlack(const KERNEL_NS::LibString &ip, Int32 level)
{
    std::list<KERNEL_NS::IpControlInfo *> newList;
    auto newInfo = KERNEL_NS::IpControlInfo::Create();
    newInfo->_ip = ip;
    newInfo->_controlFlow.push_back(KERNEL_NS::IpControlInfo::ERASE_BLACK);
    ControlIpPipline(newList, level);
}

ALWAYS_INLINE void ServiceProxy::AddWhite(const std::list<KERNEL_NS::LibString> &ips, Int32 level)
{
    std::list<KERNEL_NS::IpControlInfo *> newList;
    for(auto &ip : ips)
    {
        auto newInfo = KERNEL_NS::IpControlInfo::Create();
        newInfo->_ip = ip;
        newInfo->_controlFlow.push_back(KERNEL_NS::IpControlInfo::ADD_WHITE);
    }
    ControlIpPipline(newList, level);
}

ALWAYS_INLINE void ServiceProxy::AddBlack(const std::list<KERNEL_NS::LibString> &ips, Int32 level)
{
    std::list<KERNEL_NS::IpControlInfo *> newList;
    for(auto &ip : ips)
    {
        auto newInfo = KERNEL_NS::IpControlInfo::Create();
        newInfo->_ip = ip;
        newInfo->_controlFlow.push_back(KERNEL_NS::IpControlInfo::ADD_BLACK);
    }
    ControlIpPipline(newList, level);
}

ALWAYS_INLINE void ServiceProxy::EraseWhite(const std::list<KERNEL_NS::LibString> &ips, Int32 level)
{
    std::list<KERNEL_NS::IpControlInfo *> newList;
    for(auto &ip : ips)
    {
        auto newInfo = KERNEL_NS::IpControlInfo::Create();
        newInfo->_ip = ip;
        newInfo->_controlFlow.push_back(KERNEL_NS::IpControlInfo::ERASE_WHITE);
    }
    ControlIpPipline(newList, level);
}

ALWAYS_INLINE void ServiceProxy::EraseBlack(const std::list<KERNEL_NS::LibString> &ips, Int32 level)
{
    std::list<KERNEL_NS::IpControlInfo *> newList;
    for(auto &ip : ips)
    {
        auto newInfo = KERNEL_NS::IpControlInfo::Create();
        newInfo->_ip = ip;
        newInfo->_controlFlow.push_back(KERNEL_NS::IpControlInfo::ERASE_BLACK);
    }
    ControlIpPipline(newList, level);
}

ALWAYS_INLINE void ServiceProxy::ControlIpPipline(const std::list<KERNEL_NS::IpControlInfo *> &controlInfoList, Int32 level)
{
    _tcpPollerMgr->PostIpControl(level, controlInfoList);
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
