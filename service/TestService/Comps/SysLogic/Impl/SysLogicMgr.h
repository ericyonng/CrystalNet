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
#include <service/TestService/Comps/SysLogic/Interface/ISysLogicMgr.h>
#include <kernel/kernel.h>
#include <service_common/ServiceCommon.h>

SERVICE_BEGIN

struct AddrConfig;

class SysLogicMgr : public ISysLogicMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(ISysLogicMgr, SysLogicMgr);

public:
    SysLogicMgr();
    ~SysLogicMgr();

    void Release() override;

  /*
  * 新增监听
  * @param(ip): 0.0.0.0或放空将监听所有网卡,
  * @param(port): 0将由系统分配一个端口
  * @param(sessionType): SessionType指定一个会话类型,连入的会话都将继承这个类型
  * @param(family): AF_INET:ipv4, AF_INET6 :ipv6
  * @param(family): AF_INET:ipv4, AF_INET6 :ipv6
  */
  template<typename ObjType>
  Int32 AddTcpListen(const KERNEL_NS::LibString &ip, UInt16 port
                    , UInt64 &stub, ObjType *obj
                    , void(ObjType::*handler)(UInt64 stub, Int32 errCode, const KERNEL_NS::Variant *params)
                    , Int32 sessionCount = 1
                    , UInt32 priorityLevel = PriorityLevelDefine::OUTER1
                    , Int32 sessionType = SessionType::OUTER
                    , Int32 family = AF_INET, Int32 protocolStackType = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL) const;

  Int32 AddTcpListen(const KERNEL_NS::LibString &ip, UInt16 port
  , UInt64 &stub, KERNEL_NS::IDelegate<void, UInt64, Int32, const KERNEL_NS::Variant *> *delg
  , Int32 sessionCount = 1
  , UInt32 priorityLevel = PriorityLevelDefine::OUTER1
  , Int32 sessionType = SessionType::OUTER
  , Int32 family = AF_INET, Int32 protocolStackType = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL) const;

  Int32 AddTcpListen(const KERNEL_NS::LibString &ip, UInt16 port, UInt64 &stub
  , Int32 sessionCount = 1
  , UInt32 priorityLevel = PriorityLevelDefine::OUTER1
  , Int32 sessionType = SessionType::OUTER
  , Int32 family = AF_INET, Int32 protocolStackType = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL) const;

  /*
  * 连接远程
  */

  Int32 AsynTcpConnect(const KERNEL_NS::LibString &remoteIp, UInt16 remotePort, UInt64 &stub
  , KERNEL_NS::IDelegate<void, UInt64, Int32, const KERNEL_NS::Variant *> *callback
  , const KERNEL_NS::LibString &localIp = ""
  , UInt16 localPort = 0
  , KERNEL_NS::IProtocolStack *stack = NULL /* 指定协议栈 */
  , Int32 retryTimes = 0    /* 超时重试次数 */
  , Int64 periodMs = 0  /* 超时时间 */
  , UInt32 priorityLevel = PriorityLevelDefine::INNER /* 消息队列优先级 */
  , Int32 sessionType = SessionType::INNER /* 会话类型 */
  , Int32 family = AF_INET /* AF_INET:ipv4, AF_INET6:ipv6 */
  , Int32 protocolStackType = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL
  ) const  override;

  virtual bool IsAllTaskFinish() const override;

    OBJ_GET_OBJ_TYPEID_DECLARE();

protected:
   Int32 _OnGlobalSysInit() override;
   Int32 _OnHostStart() override;
   void _OnGlobalSysClose() override;

    void _OnDetectLinkTimer(KERNEL_NS::LibTimer *timer);

    void _OnAddListenRes(UInt64 stub, Int32 errCode, const KERNEL_NS::Variant *params);
    void _OnConnectRes(UInt64 stub, Int32 errCode, const KERNEL_NS::Variant *params);
    void _CloseServiceEvent(KERNEL_NS::LibEvent *ev);
private:
    void _Clear();

private:
    std::unordered_map<UInt64, AddrConfig *> _unhandledListenAddr;
    std::unordered_map<UInt64, AddrConfig *> _unhandledContectAddr;
    KERNEL_NS::LibTimer *_detectLink;

    KERNEL_NS::ListenerStub _closeServiceStub;
};

template<typename ObjType>
ALWAYS_INLINE Int32 SysLogicMgr::AddTcpListen(const KERNEL_NS::LibString &ip, UInt16 port
                , UInt64 &stub, ObjType *obj
                , void(ObjType::*handler)(UInt64 stub, Int32 errCode, const KERNEL_NS::Variant *params)
                , Int32 sessionCount
                , UInt32 priorityLevel
                , Int32 sessionType
                , Int32 family, Int32 protocolStackType) const
{
    auto delg = KERNEL_NS::DelegateFactory::Create(obj, handler);
    auto st = AddTcpListen(ip, port, stub, delg, sessionCount, priorityLevel, sessionType, family, protocolStackType);
    if(st != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("add tcp listen fail st:%d, ip:%s, port:%hu, sessionType:%d, family:%d"), st, ip.c_str(), port, sessionType, family);
        delg->Release();
        return st;
    }

    return st;
}

ALWAYS_INLINE Int32 SysLogicMgr::AddTcpListen(const KERNEL_NS::LibString &ip, UInt16 port, UInt64 &stub, Int32 sessionCount, UInt32 priorityLevel, Int32 sessionType, Int32 family, Int32 protocolStackType) const
{
    return AddTcpListen(ip, port, stub, NULL, sessionCount, priorityLevel, sessionType, family, protocolStackType);
}

SERVICE_END
