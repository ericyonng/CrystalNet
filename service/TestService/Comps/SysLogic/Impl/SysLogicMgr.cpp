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

#include <pch.h>
#include <service/TestService/Comps/StubHandle/StubHandle.h>
#include <service/TestService/MyTestService.h>

#include <service/TestService/Comps/SysLogic/Impl/SysLogicMgr.h>
#include <service/TestService/Comps/SysLogic/Impl/SysLogicMgrFactory.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(ISysLogicMgr);

POOL_CREATE_OBJ_DEFAULT_IMPL(SysLogicMgr);

SysLogicMgr::SysLogicMgr()
:_detectLink(NULL)
,_closeServiceStub(INVALID_LISTENER_STUB)
{

}

SysLogicMgr::~SysLogicMgr()
{
    _Clear();
}

void SysLogicMgr::Release()
{
    SysLogicMgr::DeleteByAdapter_SysLogicMgr(SysLogicMgrFactory::_buildType.V, this);
}

Int32 SysLogicMgr::AddTcpListen(const KERNEL_NS::LibString &ip, UInt16 port
, UInt64 &stub, KERNEL_NS::IDelegate<void, UInt64, Int32, const KERNEL_NS::Variant *> *delg
, UInt32 priorityLevel, Int32 sessionType, Int32 family) const
{
    // 1.校验ip
    if(!ip.empty())
    {
        if(!KERNEL_NS::SocketUtil::IsIp(ip))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("illegal ip:%s"), ip.c_str());
            return Status::SockError_IllegalIp;
        }
    }

    auto service = IGlobalSys::GetCurrentService();
    auto stubHandleMgr = service->GetComp<IStubHandleMgr>();
    stub = stubHandleMgr->NewStub();
    if(stubHandleMgr->HasStub(stub))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("stub callback is already existes please check stub:%llu, ip:%s, port:%hu, priorityLevel:%u, sessionType:%d, family:%d")
                    , stub, ip.c_str(), port, priorityLevel, sessionType, family);

        return Status::Repeat;
    }

    auto serviceProxy = service->GetServiceProxy();
    KERNEL_NS::SessionOption option;

    // 发送和接收缓冲区使用默认值
    auto &config = service->GetServiceProxy()->GetOwner()->CastTo<SERVICE_COMMON_NS::Application>()->GetKernelConfig();
    option._noDelay = true;
    option._sockSendBufferSize = 0;
    option._sockRecvBufferSize = 0;
    option._sessionBufferCapicity = config._sessionBufferCapicity;

    if(sessionType == SessionType::INNER)
    {
        option._sessionRecvPacketSpeedLimit = 0;
        option._sessionRecvPacketSpeedTimeUnitMs = 0;
    }
    else
    {
        option._sessionRecvPacketSpeedLimit = config._sessionRecvPacketSpeedLimit;
        option._sessionRecvPacketSpeedTimeUnitMs = config._sessionRecvPacketSpeedTimeUnitMs;
    }

    option._sessionRecvPacketStackLimit = config._sessionRecvPacketStackLimit;

    option._forbidRecv = true;
    option._sessionType = sessionType;
    serviceProxy->TcpAddListen(service->GetServiceId(), priorityLevel, family, ip, port, stub, option);

    // 回调
    if(delg)
        stubHandleMgr->NewHandle(stub, delg);

    g_Log->Info(LOGFMT_OBJ_TAG("post a new listen ip:%s, port:%hu, priorityLevel:%u, sessionType:%d, family:%d, stub:%llu")
                , ip.c_str(), port, priorityLevel, sessionType, family, stub);


    return Status::Success;
}

Int32 SysLogicMgr::AsynTcpConnect(const KERNEL_NS::LibString &remoteIp, UInt16 remotePort, UInt64 &stub
, KERNEL_NS::IDelegate<void, UInt64, Int32, const KERNEL_NS::Variant *> *callback
, const KERNEL_NS::LibString &localIp
, UInt16 localPort
, KERNEL_NS::IProtocolStack *stack /* 指定协议栈 */
, Int32 retryTimes    /* 超时重试次数 */
, Int64 periodMs  /* 超时时间 */
, UInt32 priorityLevel /* 消息队列优先级 */
, Int32 sessionType /* 会话类型 */
, Int32 family /* AF_INET:ipv4, AF_INET6:ipv6 */
) const
{
    // 1.校验参数
    if(!remoteIp.empty() && !KERNEL_NS::SocketUtil::IsIp(remoteIp))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("invalid remote ip:%s"), remoteIp.c_str());
        return Status::SockError_IllegalIp;
    }

    if(!localIp.empty() && !KERNEL_NS::SocketUtil::IsIp(localIp))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("invalid local ip:%s"), localIp.c_str());
        return Status::SockError_IllegalIp;
    }

    auto service = IGlobalSys::GetCurrentService();
    auto stubHandlMgr = service->GetComp<IStubHandleMgr>();
    auto st = stubHandlMgr->NewHandle(callback, stub);
    if(st != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("create new stub handler fail st:%d, remote ip:%s, port:%hu"), st, remoteIp.c_str(), remotePort);
        return Status::CreateNewStubFail;
    }

    auto serviceProxy = service->GetServiceProxy();
    auto &config = serviceProxy->GetOwner()->CastTo<SERVICE_COMMON_NS::Application>()->GetKernelConfig();
    KERNEL_NS::SessionOption option;
    option._noDelay = true;
    option._sockSendBufferSize = 0;
    option._sockRecvBufferSize = 0;
    option._sessionBufferCapicity = config._sessionBufferCapicity;

    
    if(sessionType == SessionType::INNER)
    {
        option._sessionRecvPacketSpeedLimit = 0;
        option._sessionRecvPacketSpeedTimeUnitMs = 0;
    }
    else
    {
        option._sessionRecvPacketSpeedLimit = config._sessionRecvPacketSpeedLimit;
        option._sessionRecvPacketSpeedTimeUnitMs = config._sessionRecvPacketSpeedTimeUnitMs;
    }
    
    option._sessionRecvPacketStackLimit = config._sessionRecvPacketStackLimit;

    option._forbidRecv = false;
    option._sessionType = sessionType;
    serviceProxy->TcpAsynConnect(service->GetServiceId(), stub, priorityLevel, family, remoteIp, remotePort, option, stack, retryTimes, periodMs, localIp, localPort);
    return Status::Success;
}

Int32 SysLogicMgr::_OnGlobalSysInit()
{
    _detectLink = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    _detectLink->SetTimeOutHandler(this, &SysLogicMgr::_OnDetectLinkTimer);


    _closeServiceStub = GetEventMgr()->AddListener(EventEnums::QUIT_SERVICE_EVENT, this, &SysLogicMgr::_CloseServiceEvent);

    return Status::Success;
}

Int32 SysLogicMgr::_OnHostStart()
{
    // 获取系统配置
    auto service = GetService()->CastTo<MyTestService>();
    auto serviceConfig = service->GetServiceConfig();

    // 1.添加监听
    auto &listenAddrs = serviceConfig->_listenAddrs;
    for(auto &addrInfo : listenAddrs)
    {
        UInt64 stub = 0;
        auto st = AddTcpListen(addrInfo->_localIp
                                , addrInfo->_localPort
                                , stub
                                , this
                                , &SysLogicMgr::_OnAddListenRes
                                , addrInfo->_priorityLevel
                                , addrInfo->_sessionType
                                , addrInfo->_af);

        if(st != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("add tcp listen fail addrInfo:%s, st:%d"), addrInfo->ToString().c_str(), st);
            return st;
        }

        _unhandledListenAddr.insert(std::make_pair(stub, addrInfo));
    }

    // 2.连接中心服
    auto centerAddr = serviceConfig->_centerAddr;
    if(centerAddr && !centerAddr->_remoteIp.empty())
    {
        UInt64 stub = 0;
        auto st = ISysLogicMgr::AsynTcpConnect(centerAddr->_remoteIp
        , centerAddr->_remotePort
        , stub
        , this
        , &SysLogicMgr::_OnConnectRes
        , centerAddr->_localIp
        , centerAddr->_localPort
        , NULL
        , 3
        , 12000
        ,  PriorityLevelDefine::INNER
        ,  SessionType::INNER
        , KERNEL_NS::SocketUtil::IsIpv4(centerAddr->_remoteIp) ? AF_INET : AF_INET6);
        if(st != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("asyn connect fail st:%d, center addr:%s"), st, centerAddr->ToString().c_str());
            return st;
        }

        _unhandledContectAddr.insert(std::make_pair(stub, centerAddr));
    }

    // 3.连接剩余的目标服
    const Int32 leftTargetCount = static_cast<Int32>(serviceConfig->_connectAddrGroup.size());
    for(Int32 idx = 0; idx < leftTargetCount; ++idx)
    {
        auto addr = serviceConfig->_connectAddrGroup[idx];
        if(!addr || addr->_remoteIp.empty())
            continue;

        UInt64 stub = 0;
        auto st = ISysLogicMgr::AsynTcpConnect(addr->_remoteIp
        , addr->_remotePort
        , stub
        , this
        , &SysLogicMgr::_OnConnectRes
        , addr->_localIp
        , addr->_localPort
        , NULL
        , 0
        , 0
        ,  addr->_priorityLevel
        ,  addr->_sessionType
        , KERNEL_NS::SocketUtil::IsIpv4(addr->_remoteIp) ? AF_INET : AF_INET6);
        if(st != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("asyn connect fail st:%d, center addr:%s"), st, centerAddr->ToString().c_str());
            return st;
        }

        _unhandledContectAddr.insert(std::make_pair(stub, addr));
    }

    // 4.启动定时器检测
    _detectLink->Schedule(1000);

    return Status::Success;
}

void SysLogicMgr::_OnGlobalSysClose()
{
    _Clear();

    if (_closeServiceStub != INVALID_LISTENER_STUB)
    {
        GetEventMgr()->RemoveListenerX(_closeServiceStub);
    }
}

void SysLogicMgr::_OnDetectLinkTimer(KERNEL_NS::LibTimer *timer)
{
    // 1.检测监听是否成功
    for(auto iter : _unhandledListenAddr)
        g_Log->Warn(LOGFMT_OBJ_TAG("listen addr %s, not success..."), iter.second->ToString().c_str());

    for(auto iter : _unhandledContectAddr)
        g_Log->Warn(LOGFMT_OBJ_TAG("connect to %s..."), iter.second->ToString().c_str());

    if(_unhandledListenAddr.empty() && _unhandledContectAddr.empty())
    {
        timer->Cancel();

        auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::SERVICE_COMMON_SESSION_READY);
        GetEventMgr()->FireEvent(ev);
    }
}

void SysLogicMgr::_OnAddListenRes(UInt64 stub, Int32 errCode, const KERNEL_NS::Variant *params)
{
    if(errCode != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("add listen fail stub:%llu, errCode:%d"), stub, errCode);

        auto iter = _unhandledListenAddr.find(stub);
        if(iter == _unhandledListenAddr.end())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("unhandled listen callback not exists stub:%llu"), stub);
            return;
        }

        return;
    }

    auto iter = _unhandledListenAddr.find(stub);
    if(iter == _unhandledListenAddr.end())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("add listen suc but have no callback stub:%llu"), stub);
        return;
    }

    auto addrInfo = iter->second;
    g_Log->Info(LOGFMT_OBJ_TAG("add listen suc:%s"), addrInfo->ToString().c_str());

    _unhandledListenAddr.erase(iter);
}

void SysLogicMgr::_OnConnectRes(UInt64 stub, Int32 errCode, const KERNEL_NS::Variant *params)
{
    if(errCode != Status::Success)
    {
        g_Log->Info(LOGFMT_OBJ_TAG("connect fail stub:%llu, errCode:%d"), stub, errCode);

        auto iter = _unhandledContectAddr.find(stub);
        if(iter == _unhandledContectAddr.end())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("unhandled connect callback not exists stub:%llu"), stub);
            return;
        }

        // 失败要重新连接
        auto addr = iter->second;
        stub = 0;
        auto st = ISysLogicMgr::AsynTcpConnect(addr->_remoteIp
        , addr->_remotePort
        , stub
        , this
        , &SysLogicMgr::_OnConnectRes
        , addr->_localIp
        , addr->_localPort
        , NULL
        , 0
        , 0
        ,  PriorityLevelDefine::INNER
        ,  SessionType::INNER
        , KERNEL_NS::SocketUtil::IsIpv4(addr->_remoteIp) ? AF_INET : AF_INET6);
        if(st != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("asyn connect fail st:%d, center addr:%s"), st, addr->ToString().c_str());
            return;
        }

        if(stub == 0)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("asyn tcp connect stub fail addr:%s"), addr->ToString().c_str());
            return;
        }

        _unhandledContectAddr.erase(iter);
        _unhandledContectAddr.insert(std::make_pair(stub, addr));

        g_Log->Info(LOGFMT_OBJ_TAG("reconnect addr:%s"), addr->ToString().c_str());

        return;
    }

    auto iter = _unhandledContectAddr.find(stub);
    if(iter == _unhandledContectAddr.end())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("connect suc but have no callback stub:%llu"), stub);
        return;
    }

    auto addrInfo = iter->second;
    g_Log->Info(LOGFMT_OBJ_TAG("connect suc:%s"), addrInfo->ToString().c_str());

    _unhandledContectAddr.erase(iter);
}

void SysLogicMgr::_CloseServiceEvent(KERNEL_NS::LibEvent *ev)
{
    g_Log->Info(LOGFMT_OBJ_TAG("service will close."));
}

void SysLogicMgr::_Clear()
{
    _unhandledListenAddr.clear();
    _unhandledContectAddr.clear();

    if(_detectLink)
    {
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(_detectLink);
        _detectLink = NULL;
    }
}

SERVICE_END
