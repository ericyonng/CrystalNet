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
 * Date: 2022-04-03 15:25:56
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_TCP_TCP_POLLER_MGR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_TCP_TCP_POLLER_MGR_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/CompObject/CompObjectInc.h>
#include <kernel/comp/LibList.h>

KERNEL_BEGIN

class Poller;
struct TcpPollerConfig;
struct TcpPollerInstConfig;
struct LibConnectInfo;
class IServiceProxy;
struct BuildSessionInfo;
class LibPacket;
struct LibListenInfo;
struct IpControlInfo;

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
class IocpTcpPoller;
#endif

#if CRYSTAL_TARGET_PLATFORM_LINUX
class EpollTcpPoller;
#endif

class KERNEL_EXPORT TcpPollerMgr : public CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, TcpPollerMgr);

public:
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
         using TcpPoller = IocpTcpPoller;
    #endif
    #if CRYSTAL_TARGET_PLATFORM_LINUX
        using TcpPoller = EpollTcpPoller;
    #endif

    using PollerClusterType = std::vector<TcpPoller *>;        // 集群

public:
    TcpPollerMgr();
    ~TcpPollerMgr();
    virtual void Release() override;

    void Clear() override;
    LibString ToString() const override;

    void SetConfig(const TcpPollerConfig *cfg);
    const TcpPollerConfig *GetConfig() const;

    void SetServiceProxy(IServiceProxy *serviceProxy);
    IServiceProxy *GetServiceProxy();
    const IServiceProxy *GetServiceProxy() const;
    
    TcpPoller *GetPoller(UInt64 pollerId);
    const TcpPoller *GetPoller(UInt64 pollerId) const;
    const std::unordered_map<UInt64, TcpPoller *> &GetAllPollers() const;

    virtual void DefaultMaskReady(bool isReady){}

public:
    void PostConnect(LibConnectInfo *connectInfo);
    void PostAddlisten(Int32 level, LibListenInfo *listenInfo);
    void PostAddlistenList(Int32 level, std::vector<LibListenInfo *> &listenInfoList);
    void PostSend(UInt64 pollerId, Int32 level, UInt64 sessionId, LibPacket *packet);
    void PostSend(UInt64 pollerId, Int32 level, UInt64 sessionId, LibList<LibPacket *> *packets);
    void PostCloseSession(UInt64 pollerId, UInt64 fromeService, Int32 level, UInt64 sessionId, Int64 closeMillisecondTime, bool forbidRead, bool forbidWrite);
    void PostIpControl(Int32 level, const std::list<IpControlInfo *> &controlList);

    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        void OnConnectRemoteSuc(BuildSessionInfo *newSessionInfo);
    #endif
    
    void OnAcceptedSuc(BuildSessionInfo *newSessionInfo);

    void QuitAllSessions(UInt64 serviceId);

protected:
    Int32 _OnCreated() override;
    Int32 _OnInit() override;
    Int32 _OnStart() override;
    void _OnWillClose() override;
    void _OnClose() override;

private:
    TcpPoller *_CreatePoller(const TcpPollerInstConfig *cfg);
    TcpPollerMgr::PollerClusterType *_GetPollerCluster(Int32 pollerFeature);
    TcpPollerMgr::TcpPoller *_SelectLowerLoaderPoller(TcpPollerMgr::PollerClusterType *cluster, const std::set<TcpPollerMgr::TcpPoller *> &excludes);

    bool IsAllReady() const;
    bool IsAllDown() const;

private:
    void _Clear();

    const TcpPollerConfig *_config;

    UInt64 _maxPollerId;
    std::unordered_map<Int32, PollerClusterType *> _pollerFeatureRefPollerCluster;
    std::unordered_map<UInt64, TcpPoller *> _idRefPoller;

    IServiceProxy *_serviceProxy;

    Int32 _trasferFeatureId;
    Int32 _linkerFeatureId;
};

ALWAYS_INLINE void TcpPollerMgr::SetConfig(const TcpPollerConfig *cfg)
{
    _config = cfg;
}

ALWAYS_INLINE const TcpPollerConfig *TcpPollerMgr::GetConfig() const
{
    return _config;
}

ALWAYS_INLINE void TcpPollerMgr::SetServiceProxy(IServiceProxy *serviceProxy)
{
    _serviceProxy = serviceProxy;
}

ALWAYS_INLINE IServiceProxy *TcpPollerMgr::GetServiceProxy()
{
    return _serviceProxy;
}

ALWAYS_INLINE const IServiceProxy *TcpPollerMgr::GetServiceProxy() const
{
    return _serviceProxy;
}

ALWAYS_INLINE TcpPollerMgr::TcpPoller *TcpPollerMgr::GetPoller(UInt64 pollerId)
{
    auto iter = _idRefPoller.find(pollerId);
    return iter == _idRefPoller.end() ? NULL : iter->second;
}

ALWAYS_INLINE const TcpPollerMgr::TcpPoller *TcpPollerMgr::GetPoller(UInt64 pollerId) const
{
    auto iter = _idRefPoller.find(pollerId);
    return iter == _idRefPoller.end() ? NULL : iter->second;
}

ALWAYS_INLINE const std::unordered_map<UInt64, TcpPollerMgr::TcpPoller *> &TcpPollerMgr::GetAllPollers() const
{
    return _idRefPoller;
}

KERNEL_END

#endif
