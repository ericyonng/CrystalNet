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
 * Date: 2022-09-22 15:03:11
 * Author: Eric Yonng
 * Description: 系统的业务,如连接，监听等
*/

#pragma once

#include <service/common/BaseComps/SysLogic/Interface/ISysLogicMgr.h>
#include <kernel/kernel.h>
#include <service_common/ServiceCommon.h>
#include <service/common/ServiceConfig.h>
#include <service/common/Configs/SysLogicOptions.h>

SERVICE_BEGIN

struct AddrConfig;

class SysLogicMgr : public ISysLogicMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(ISysLogicMgr, SysLogicMgr);

public:
    SysLogicMgr();
    ~SysLogicMgr();

    void Release() override;
    
    Int32 AddTcpListen(const KERNEL_NS::AddrIpConfig &ip, UInt16 port
    , UInt64 &stub, KERNEL_NS::IDelegate<void, UInt64, Int32, const KERNEL_NS::Variant *, bool &> *delg
    , Int32 sessionCount = 1
    , const KERNEL_NS::PacketOptions &packetOptions = KERNEL_NS::PacketOptions()
    , Int32 family = AF_INET, Int32 protocolStackType = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL) const override;

    Int32 AddTcpListen(const KERNEL_NS::AddrIpConfig &ip, UInt16 port, UInt64 &stub
    , Int32 sessionCount = 1
    , const KERNEL_NS::PacketOptions &packetOptions = KERNEL_NS::PacketOptions()
    , Int32 family = AF_INET, Int32 protocolStackType = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL) const override;
  /*
  * 连接远程 bool &:是否移除stub对应的响应回调
  */
  Int32 AsynTcpConnect(const KERNEL_NS::AddrIpConfig &remoteIp, UInt16 remotePort, UInt64 &stub
  , KERNEL_NS::IDelegate<void, UInt64, Int32, const KERNEL_NS::Variant *, bool &> *callback
  , const KERNEL_NS::AddrIpConfig &localIp = KERNEL_NS::AddrIpConfig()
  , UInt16 localPort = 0
  , KERNEL_NS::IProtocolStack *stack = NULL /* 指定协议栈 */
  , Int32 retryTimes = 0    /* 超时重试次数 */
  , Int64 periodMs = 0  /* 超时时间 */
  , const KERNEL_NS::PacketOptions &packetOptions = KERNEL_NS::PacketOptions() /*包配置*/
  , Int32 family = AF_INET /* AF_INET:ipv4, AF_INET6:ipv6 */
  , Int32 protocolStackType = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL
  ) const  override;

  virtual bool IsAllTaskFinish() const override;

protected:
   Int32 _OnGlobalSysInit() override;
   Int32 _OnHostStart() override;
   void _OnGlobalSysClose() override;
    void OnStartup() override;

    void _OnDetectLinkTimer(KERNEL_NS::LibTimer *timer);

    void _OnAddListenRes(UInt64 stub, Int32 errCode, const KERNEL_NS::Variant *params, bool &doRemove);
    void _OnConnectRes(UInt64 stub, Int32 errCode, const KERNEL_NS::Variant *params, bool &doRemove);
    void _CloseServiceEvent(KERNEL_NS::LibEvent *ev);
private:
    void _Clear();

private:
    // 可能多个session负载均衡
    std::unordered_map<UInt64, std::pair<KERNEL_NS::AddrIpConfig, Int32>> _unhandledListenAddr;
    std::unordered_map<UInt64, AddrConfig *> _unhandledContectAddr;
    KERNEL_NS::LibTimer *_detectLink;

    KERNEL_NS::ListenerStub _closeServiceStub;
    KERNEL_NS::FileMonitor<SysLogicOptions, KERNEL_NS::YamlDeserializer> *_options;
};

SERVICE_END
