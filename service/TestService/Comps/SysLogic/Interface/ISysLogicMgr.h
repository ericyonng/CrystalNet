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

#include <service/TestService/ServiceCompHeader.h>
#include <service/common/BaseComps/GlobalSys/GlobalSys.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/Delegate/LibDelegate.h>
#include <kernel/common/statics.h>

KERNEL_BEGIN
class Variant;
class IProtocolStack;
KERNEL_END

SERVICE_BEGIN

class ISysLogicMgr : public IGlobalSys
{
    POOL_CREATE_OBJ_DEFAULT_P1(IGlobalSys, ISysLogicMgr);

public:
  ISysLogicMgr(UInt64 objTypeId) : IGlobalSys(objTypeId) {}

    
    /*
    * 新增监听
    * @param(ip): 0.0.0.0或放空将监听所有网卡,
    * @param(port): 0将由系统分配一个端口
    * @param(sessionType): SessionType指定一个会话类型,连入的会话都将继承这个类型
    * @param(family): AF_INET:ipv4, AF_INET6 :ipv6
    * @param(family): AF_INET:ipv4, AF_INET6 :ipv6
    */
    template<typename ObjType>
    Int32 AddTcpListen(const KERNEL_NS::AddrIpConfig &ip, UInt16 port
                      , UInt64 &stub, ObjType *obj
                      , void(ObjType::*handler)(UInt64 stub, Int32 errCode, const KERNEL_NS::Variant *params, bool &)
                      , Int32 sessionCount = 1
                      , const KERNEL_NS::PacketOptions &packetOptions = KERNEL_NS::PacketOptions()
                      , Int32 family = AF_INET, Int32 protocolStackType = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL) const;

    virtual Int32 AddTcpListen(const KERNEL_NS::AddrIpConfig &ip, UInt16 port
    , UInt64 &stub, KERNEL_NS::IDelegate<void, UInt64, Int32, const KERNEL_NS::Variant *, bool &> *delg
    , Int32 sessionCount = 1
    , const KERNEL_NS::PacketOptions &packetOptions = KERNEL_NS::PacketOptions()
    , Int32 family = AF_INET, Int32 protocolStackType = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL) const = 0;

    virtual  Int32 AddTcpListen(const KERNEL_NS::AddrIpConfig &ip, UInt16 port, UInt64 &stub
    , Int32 sessionCount = 1
    , const KERNEL_NS::PacketOptions &packetOptions = KERNEL_NS::PacketOptions()
    , Int32 family = AF_INET, Int32 protocolStackType = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL) const = 0;
    
  /*
  * 连接远程 bool:需要删除stub的响应回调么, 默认是要删除
  */
 template<typename ObjType>
  Int32 AsynTcpConnect(const KERNEL_NS::AddrIpConfig &remoteIp, UInt16 remotePort, UInt64 &stub
  , ObjType *obj, void(ObjType::*handler)(UInt64 stub, Int32 errCode, const KERNEL_NS::Variant *params, bool &) /* 连接成功回调 */
  , const KERNEL_NS::AddrIpConfig &localIp = KERNEL_NS::AddrIpConfig()
  , UInt16 localPort = 0
  , KERNEL_NS::IProtocolStack *stack = NULL /* 指定协议栈 */
  , Int32 retryTimes = 0    /* 超时重试次数 */
  , Int64 periodMs = 0  /* 超时时间 */
  , const KERNEL_NS::PacketOptions &packetOptions = KERNEL_NS::PacketOptions() /*包配置*/
  , Int32 family = AF_INET /* AF_INET:ipv4, AF_INET6:ipv6 */
  , Int32 protocolStackType = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL
  ) const;

  // bool:需要删除stub的响应回调么, 默认是要删除
  virtual Int32 AsynTcpConnect(const KERNEL_NS::AddrIpConfig &remoteIp, UInt16 remotePort, UInt64 &stub
  , KERNEL_NS::IDelegate<void, UInt64, Int32, const KERNEL_NS::Variant *, bool &> *callback
  , const KERNEL_NS::AddrIpConfig &localIp = KERNEL_NS::AddrIpConfig()
  , UInt16 localPort = 0
  , KERNEL_NS::IProtocolStack *stack = NULL /* 指定协议栈 */
  , Int32 retryTimes = 0    /* 超时重试次数 */
  , Int64 periodMs = 0  /* 超时时间 */
  , const KERNEL_NS::PacketOptions &packetOptions = KERNEL_NS::PacketOptions() /*包配置*/
  , Int32 family = AF_INET /* AF_INET:ipv4, AF_INET6:ipv6 */
  , Int32 protocolStackType = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL
  ) const = 0;

  virtual Int32 AsynTcpConnect(const KERNEL_NS::AddrIpConfig &remoteIp, UInt16 remotePort, UInt64 &stub
  , const KERNEL_NS::AddrIpConfig &localIp = KERNEL_NS::AddrIpConfig()
  , UInt16 localPort = 0
  , KERNEL_NS::IProtocolStack *stack = NULL /* 指定协议栈 */
  , Int32 retryTimes = 0    /* 超时重试次数 */
  , Int64 periodMs = 0  /* 超时时间 */
  , const KERNEL_NS::PacketOptions &packetOptions = KERNEL_NS::PacketOptions() /*包配置*/
  , Int32 family = AF_INET /* AF_INET:ipv4, AF_INET6:ipv6 */
  , Int32 protocolStackType = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL
  ) const;

    // 模块准备就绪
  virtual bool IsAllTaskFinish() const = 0;
};


template<typename ObjType>
ALWAYS_INLINE Int32 ISysLogicMgr::AddTcpListen(const KERNEL_NS::AddrIpConfig &ip, UInt16 port
                , UInt64 &stub, ObjType *obj
                , void(ObjType::*handler)(UInt64 stub, Int32 errCode, const KERNEL_NS::Variant *params, bool &)
                , Int32 sessionCount
                , const KERNEL_NS::PacketOptions &packetOptions
                , Int32 family, Int32 protocolStackType) const
{
    auto delg = KERNEL_NS::DelegateFactory::Create(obj, handler);
    auto st = AddTcpListen(ip, port, stub, delg, sessionCount, packetOptions, family, protocolStackType);
    if(st != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("add tcp listen fail st:%d, ip:%s, port:%hu, family:%d"), st, ip.ToString().c_str(), port, family);
        delg->Release();
        return st;
    }

    return st;
}

 template<typename ObjType>
ALWAYS_INLINE Int32 ISysLogicMgr::AsynTcpConnect(const KERNEL_NS::AddrIpConfig &remoteIp, UInt16 remotePort, UInt64 &stub
, ObjType *obj, void(ObjType::*handler)(UInt64 stub, Int32 errCode, const KERNEL_NS::Variant *params, bool &) /* 连接成功回调 */
, const KERNEL_NS::AddrIpConfig &localIp
, UInt16 localPort
, KERNEL_NS::IProtocolStack *stack /* 指定协议栈 */
, Int32 retryTimes   /* 超时重试次数 */
, Int64 periodMs  /* 超时时间 */
, const KERNEL_NS::PacketOptions &packetOptions /*包配置*/
, Int32 family /* AF_INET:ipv4, AF_INET6:ipv6 */
, Int32 protocolStackType
) const
{
    auto deleg = KERNEL_NS::DelegateFactory::Create(obj, handler);
    auto st = AsynTcpConnect(remoteIp, remotePort, stub, deleg, localIp, localPort, stack, retryTimes, periodMs, packetOptions, family, protocolStackType);
    if(st != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("connect fail remote ip:%s, remote port:%hu"), remoteIp.ToString().c_str(), remotePort);
        deleg->Release();
        return st;
    }

    return Status::Success;
}

ALWAYS_INLINE Int32 ISysLogicMgr::AsynTcpConnect(const KERNEL_NS::AddrIpConfig &remoteIp, UInt16 remotePort, UInt64 &stub
, const KERNEL_NS::AddrIpConfig &localIp
, UInt16 localPort
, KERNEL_NS::IProtocolStack *stack /* 指定协议栈 */
, Int32 retryTimes    /* 超时重试次数 */
, Int64 periodMs   /* 超时时间 */
, const KERNEL_NS::PacketOptions &packetOptions /*包配置*/
, Int32 family /* AF_INET:ipv4, AF_INET6:ipv6 */
, Int32 protocolStackType
) const
{
    return AsynTcpConnect(remoteIp, remotePort, stub, NULL, localIp, localPort, stack, retryTimes, periodMs, packetOptions, family, protocolStackType);
}

SERVICE_END
