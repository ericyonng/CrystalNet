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
//  * Date: 2022-12-26 13:53:00
//  * Author: Eric Yonng
//  * Description: 
// */

// #ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_UDP_UDP_POLLER_MGR_H__
// #define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_UDP_UDP_POLLER_MGR_H__

// #pragma once

// #include <kernel/kernel_inc.h>
// #include <kernel/comp/CompObject/CompObjectInc.h>
// #include <kernel/comp/LibList.h>

// KERNEL_BEGIN

// class Poller;
// struct UdpPollerConfig;
// struct UdpPollerInstConfig;
// struct LibConnectInfo;
// class IServiceProxy;
// struct BuildSessionInfo;
// class LibPacket;
// struct LibListenInfo;
// struct IpControlInfo;

// #if CRYSTAL_TARGET_PLATFORM_WINDOWS
// class IocpUdpPoller;
// #endif

// #if CRYSTAL_TARGET_PLATFORM_LINUX
// class EpollUdpPoller;
// #endif

// class KERNEL_EXPORT UdpPollerMgr : public CompObject
// {
//     POOL_CREATE_OBJ_DEFAULT_P1(CompObject, UdpPollerMgr);

// public:
//     #if CRYSTAL_TARGET_PLATFORM_WINDOWS
//          using UdpPoller = IocpUdpPoller;
//     #endif
//     #if CRYSTAL_TARGET_PLATFORM_LINUX
//         using UdpPoller = EpollUdpPoller;
//     #endif

//     using PollerClusterType = std::vector<UdpPoller *>;        // 集群

// public:
//     UdpPollerMgr();
//     ~UdpPollerMgr();
//     virtual void Release() override;

//     void Clear() override;
//     LibString ToString() const override;

//     void SetConfig(const UdpPollerConfig *cfg);
//     const UdpPollerConfig *GetConfig() const;

//     void SetServiceProxy(IServiceProxy *serviceProxy);
//     IServiceProxy *GetServiceProxy();
//     const IServiceProxy *GetServiceProxy() const;
    
//     UdpPoller *GetPoller(UInt64 pollerId);
//     const UdpPoller *GetPoller(UInt64 pollerId) const;
//     const std::unordered_map<UInt64, UdpPoller *> &GetAllPollers() const;

// public:
//     void PostConnect(LibConnectInfo *connectInfo);
//     void PostAddlisten(Int32 level, LibListenInfo *listenInfo);
//     void PostAddlistenList(Int32 level, std::vector<LibListenInfo *> &listenInfoList);
//     void PostSend(UInt64 pollerId, Int32 level, UInt64 sessionId, LibPacket *packet);
//     void PostSend(UInt64 pollerId, Int32 level, UInt64 sessionId, LibList<LibPacket *> *packets);
//     void PostCloseSession(UInt64 pollerId, UInt64 fromeService, Int32 level, UInt64 sessionId, Int64 closeMillisecondTime, bool forbidRead, bool forbidWrite);
//     void PostIpControl(Int32 level, const std::list<IpControlInfo *> &controlList);

//     #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
//         void OnConnectRemoteSuc(BuildSessionInfo *newSessionInfo);
//     #endif
    
//     void OnAcceptedSuc(BuildSessionInfo *newSessionInfo);

//     void QuitAllSessions(UInt64 serviceId);

// protected:
//     Int32 _OnCreated() override;
//     Int32 _OnInit() override;
//     Int32 _OnStart() override;
//     void _OnWillClose() override;
//     void _OnClose() override;

// private:
//     UdpPoller *_CreatePoller(const UdpPollerInstConfig *cfg);
//     UdpPollerMgr::PollerClusterType *_GetPollerCluster(Int32 pollerFeature);
//     UdpPollerMgr::UdpPoller *_SelectLowerLoaderPoller(UdpPollerMgr::PollerClusterType *cluster);

//     bool IsAllReady() const;
//     bool IsAllDown() const;

// private:
//     void _Clear();

//     const UdpPollerConfig *_config;

//     UInt64 _maxPollerId;
//     std::unordered_map<Int32, PollerClusterType *> _pollerFeatureRefPollerCluster;
//     std::unordered_map<UInt64, UdpPoller *> _idRefPoller;

//     IServiceProxy *_serviceProxy;
// };

// ALWAYS_INLINE void UdpPollerMgr::SetConfig(const UdpPollerConfig *cfg)
// {
//     _config = cfg;
// }

// ALWAYS_INLINE const UdpPollerConfig *UdpPollerMgr::GetConfig() const
// {
//     return _config;
// }

// ALWAYS_INLINE void UdpPollerMgr::SetServiceProxy(IServiceProxy *serviceProxy)
// {
//     _serviceProxy = serviceProxy;
// }

// ALWAYS_INLINE IServiceProxy *UdpPollerMgr::GetServiceProxy()
// {
//     return _serviceProxy;
// }

// ALWAYS_INLINE const IServiceProxy *UdpPollerMgr::GetServiceProxy() const
// {
//     return _serviceProxy;
// }

// ALWAYS_INLINE UdpPollerMgr::UdpPoller *UdpPollerMgr::GetPoller(UInt64 pollerId)
// {
//     auto iter = _idRefPoller.find(pollerId);
//     return iter == _idRefPoller.end() ? NULL : iter->second;
// }

// ALWAYS_INLINE const UdpPollerMgr::UdpPoller *UdpPollerMgr::GetPoller(UInt64 pollerId) const
// {
//     auto iter = _idRefPoller.find(pollerId);
//     return iter == _idRefPoller.end() ? NULL : iter->second;
// }

// ALWAYS_INLINE const std::unordered_map<UInt64, UdpPollerMgr::UdpPoller *> &UdpPollerMgr::GetAllPollers() const
// {
//     return _idRefPoller;
// }

// KERNEL_END

// #endif
