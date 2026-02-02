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

#include <service/common/common.h>
#include <service/TestService/Common/ServiceCommon.h>

#include <service/TestService/Comps/SysLogic/Impl/SysLogicMgr.h>
#include <service/TestService/Comps/SysLogic/Impl/SysLogicMgrFactory.h>

SERVICE_BEGIN


SysLogicMgr::SysLogicMgr()
:ISysLogicMgr(KERNEL_NS::RttiUtil::GetTypeId<SysLogicMgr>())
,_detectLink(NULL)
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

Int32 SysLogicMgr::AddTcpListen(const KERNEL_NS::AddrIpConfig &ip, UInt16 port
, UInt64 &stub, KERNEL_NS::IDelegate<void, UInt64, Int32, const KERNEL_NS::Variant *, bool &> *delg
, Int32 sessionCount
, Int32 sessionType, Int32 family, Int32 protocolStackType) const
{
    // 1.校验ip
    if(!ip._ip.empty())
    {
        if(!KERNEL_NS::SocketUtil::IsIp(ip._ip))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("illegal ip:%s"), ip.ToString().c_str());
            return Status::SockError_IllegalIp;
        }
    }

    auto service = IGlobalSys::GetCurrentService();
    auto stubHandleMgr = service->GetComp<IStubHandleMgr>();
    stub = stubHandleMgr->NewStub();
    if(stubHandleMgr->HasStub(stub))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("stub callback is already existes please check stub:%llu, ip:%s, port:%hu, sessionType:%d, family:%d")
                    , stub, ip.ToString().c_str(), port, sessionType, family);

        return Status::Repeat;
    }

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
    option._protocolStackType = protocolStackType;

    // 消息接收限制
    switch (option._sessionType)
    {
    case SessionType::INNER:
    case SessionType::OUTER_NO_LIMIT:
        option._sessionRecvPacketContentLimit = 0;
        break;
    case SessionType::OUTER:
        option._sessionRecvPacketContentLimit = GetApp()->GetKernelConfig()._sessionRecvPacketContentLimit;
        break;
    default:
        break;
    }

    // 消息发送限制
    switch (option._sessionType)
    {
    case SessionType::INNER:
    case SessionType::OUTER_NO_LIMIT:
        option._sessionSendPacketContentLimit = 0;
        break;
    case SessionType::OUTER:
        option._sessionSendPacketContentLimit = GetApp()->GetKernelConfig()._sessionSendPacketContentLimit;
        break;
    default:
        break;
    }

    auto listenInfo = KERNEL_NS::LibListenInfo::New_LibListenInfo();
    listenInfo->_ip = ip._ip;
    listenInfo->_port = port;
    listenInfo->_family = family;
    listenInfo->_serviceId = service->GetServiceId();
    listenInfo->_stub = stub;
    listenInfo->_protocolType = KERNEL_NS::ProtocolType::TCP;

    // windows下同一个端口只能一个session
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
    if(listenInfo->_port != 0)
        sessionCount = 1;
    #endif
    listenInfo->_sessionCount = sessionCount;
    listenInfo->_sessionOption = option;

    if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))
        g_Log->Info(LOGFMT_OBJ_TAG("add listen info:%s"), listenInfo->ToString().c_str());

    auto tcpPollerMgr = service->GetTcpPollerMgr();
    tcpPollerMgr->PostAddlisten(listenInfo);

    // 回调
    if(delg)
        stubHandleMgr->NewHandle(stub, delg);

    if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))
        g_Log->Info(LOGFMT_OBJ_TAG("post a new listen ip:%s, port:%hu, sessionType:%d, family:%d, stub:%llu")
                , ip.ToString().c_str(), port, sessionType, family, stub);

    return Status::Success;
}

Int32 SysLogicMgr::AsynTcpConnect(const KERNEL_NS::AddrIpConfig &remoteIp, UInt16 remotePort, UInt64 &stub
, KERNEL_NS::IDelegate<void, UInt64, Int32, const KERNEL_NS::Variant *, bool &> *callback
, const KERNEL_NS::AddrIpConfig &localIp
, UInt16 localPort
, KERNEL_NS::IProtocolStack *stack /* 指定协议栈 */
, Int32 retryTimes    /* 超时重试次数 */
, Int64 periodMs  /* 超时时间 */
, Int32 sessionType /* 会话类型 */
, Int32 family /* AF_INET:ipv4, AF_INET6:ipv6 */
, Int32 protocolStackType
) const
{
    // 1.校验参数
    if(!remoteIp._ip.empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("invalid remote ip:%s"), remoteIp.ToString().c_str());
        return Status::SockError_IllegalIp;
    }

    if(!localIp._ip.empty() && !KERNEL_NS::SocketUtil::IsIp(localIp._ip))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("invalid local ip:%s"), localIp.ToString().c_str());
        return Status::SockError_IllegalIp;
    }

    auto service = IGlobalSys::GetCurrentService();
    auto stubHandlMgr = service->GetComp<IStubHandleMgr>();
    auto st = stubHandlMgr->NewHandle(callback, stub);
    if(st != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("create new stub handler fail st:%d, remote ip:%s, port:%hu"), st, remoteIp.ToString().c_str(), remotePort);
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
    option._protocolStackType = protocolStackType;

    // 消息发送限速
    switch (option._sessionType)
    {
    case SessionType::INNER:
    case SessionType::OUTER_NO_LIMIT:
        option._sessionRecvPacketContentLimit = 0;
        break;
    case SessionType::OUTER:
        option._sessionRecvPacketContentLimit = GetApp()->GetKernelConfig()._sessionRecvPacketContentLimit;
        break;
    default:
        break;
    }

    // 消息发送限速
    switch (option._sessionType)
    {
    case SessionType::INNER:
    case SessionType::OUTER_NO_LIMIT:
        option._sessionSendPacketContentLimit = 0;
        break;
    case SessionType::OUTER:
        option._sessionSendPacketContentLimit = GetApp()->GetKernelConfig()._sessionSendPacketContentLimit;
        break;
    default:
        break;
    }

    auto connectInfo = KERNEL_NS::LibConnectInfo::New_LibConnectInfo();
    connectInfo->_localIp = localIp;
    connectInfo->_localPort = localPort;
    connectInfo->_targetIp = remoteIp;
    connectInfo->_targetPort = remotePort;
    connectInfo->_family = family;
    connectInfo->_protocolType = KERNEL_NS::ProtocolType::TCP;
    connectInfo->_pollerId = 0;
    connectInfo->_retryTimes = retryTimes;
    connectInfo->_periodMs = periodMs;
    connectInfo->_stub = stub;
    connectInfo->_fromServiceId = service->GetServiceId();
    connectInfo->_stack = stack;
    connectInfo->_sessionOption = option;
    // g_Log->Info(LOGFMT_OBJ_TAG("add connect info:%s"), connectInfo->ToString().c_str());
    auto tcpPollerMgr = service->GetTcpPollerMgr();
    tcpPollerMgr->PostConnect(connectInfo);

    return Status::Success;
}

bool SysLogicMgr::IsAllTaskFinish() const
{
    return _unhandledListenAddr.empty() && _unhandledContectAddr.empty();
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
                                , addrInfo->_listenSessionCount
                                , addrInfo->_sessionType
                                , addrInfo->_af
                                ,addrInfo->_protocolStackType
                                );

        if(st != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("add tcp listen fail addrInfo:%s, st:%d"), addrInfo->ToString().c_str(), st);
            return st;
        }

        #if CRYSTAL_TARGET_PLATFORM_WINDOWS
            if(addrInfo->_localPort != 0)
                _unhandledListenAddr.insert(std::make_pair(stub, std::make_pair(addrInfo, 1)));
            else
                _unhandledListenAddr.insert(std::make_pair(stub, std::make_pair(addrInfo, addrInfo->_listenSessionCount)));
        #else
            _unhandledListenAddr.insert(std::make_pair(stub, std::make_pair(addrInfo, addrInfo->_listenSessionCount)));
        #endif
    }

    // 2.连接中心服
    auto centerAddr = serviceConfig->_centerAddr;
    if(centerAddr && !centerAddr->_remoteIp._ip.empty())
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
        ,  SessionType::INNER
        , centerAddr->_remoteIp.GetAf()
        , centerAddr->_protocolStackType);
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
        if(!addr || addr->_remoteIp._ip.empty())
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
        ,  addr->_sessionType
        , addr->_remoteIp.GetAf()
        , addr->_protocolStackType);
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
        g_Log->Warn(LOGFMT_OBJ_TAG("listen addr %s, not success..."), iter.second.first->ToString().c_str());

    for(auto iter : _unhandledContectAddr)
    {
        auto addrConfig = iter.second;
        auto toIpv4 = addrConfig->_remoteIp._isHostName ? (addrConfig->_remoteIp._toIpv4 ? "to ipv4" : "to ipv6") : "None";
        g_Log->Warn(LOGFMT_OBJ_TAG("connect [%s:%hu => %s:%hu(%s)]...")
        , addrConfig->_localIp._ip.c_str(), addrConfig->_localPort
        , addrConfig->_remoteIp._ip.c_str(), addrConfig->_remotePort, toIpv4);
    }

    if(_unhandledListenAddr.empty() && _unhandledContectAddr.empty())
    {
        timer->Cancel();

        auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::SERVICE_COMMON_SESSION_READY);
        GetEventMgr()->FireEvent(ev);
    }
}

void SysLogicMgr::_OnAddListenRes(UInt64 stub, Int32 errCode, const KERNEL_NS::Variant *params, bool &doRemove)
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

    auto &addrInfo = iter->second;
    --addrInfo.second;
    g_Log->Warn(LOGFMT_OBJ_TAG("[ADD LISTEN SUCCESS]:%s, LeftCount:%d"), addrInfo.first->ToString().c_str(), addrInfo.second);

    if(addrInfo.second <= 0)
    {
        doRemove = true;
        _unhandledListenAddr.erase(iter);
    }
    else
    {
        doRemove = false;
    }
}

void SysLogicMgr::_OnConnectRes(UInt64 stub, Int32 errCode, const KERNEL_NS::Variant *params, bool &doRemove)
{
    auto failureIps = (*params)[Params::TARGET_ADDR_FAILURE_IP_SET].AsPtr<std::set<KERNEL_NS::LibString>>();
    auto targetAddrConfig = (*params)[Params::TARGET_ADDR_IP_CONFIG].AsPtr<KERNEL_NS::AddrIpConfig>();
    auto remoteAddr = (*params)[Params::REMOTE_ADDR].AsPtr<KERNEL_NS::BriefAddrInfo>();
    auto localAddr = (*params)[Params::LOCAL_ADDR].AsPtr<KERNEL_NS::BriefAddrInfo>();
    auto sessionId = (*params)[Params::SESSION_ID].AsUInt64();

    if(errCode != Status::Success)
    {
        if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))
            g_Log->Info(LOGFMT_OBJ_TAG("[connect res]fail, [%s:%hu => %s:%hu] fail stub:%llu, sessionId:%llu, errCode:%d, failure ip:%s")
            , localAddr->_ip.c_str(), localAddr->_port, targetAddrConfig->_ip.c_str(), remoteAddr->_port,  stub, sessionId, errCode
            , KERNEL_NS::StringUtil::ToString(*failureIps, ',').c_str());

        auto iter = _unhandledContectAddr.find(stub);
        if(iter == _unhandledContectAddr.end())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("[connect res]unhandled connect callback not exists stub:%llu"), stub);
            return;
        }

        // 失败要重新连接
        Int32 af = AF_INET;
        auto addr = iter->second;
        if(addr->_remoteIp._isHostName)
        {
            af = addr->_remoteIp._toIpv4 ? AF_INET : AF_INET6;
        }
        else
        {
            af = KERNEL_NS::SocketUtil::IsIpv4(addr->_remoteIp._ip) ? AF_INET : AF_INET6;
        }
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
        ,  SessionType::INNER
        , af
        , addr->_protocolStackType);
        if(st != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("[connect res]asyn connect fail st:%d, center addr:%s"), st, addr->ToString().c_str());
            return;
        }

        if(stub == 0)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("[connect res]asyn tcp connect stub fail addr:%s"), addr->ToString().c_str());
            return;
        }

        _unhandledContectAddr.erase(iter);
        _unhandledContectAddr.insert(std::make_pair(stub, addr));

        if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))
        {
            auto toIpv4 = addr->_remoteIp._isHostName ? (addr->_remoteIp._toIpv4 ? "to ipv4" : "to ipv6") : "None";
            g_Log->Info(LOGFMT_OBJ_TAG("[connect res]try reconnect [%s:%hu => %s:%hu(%s)]... \n addr:%s")
            , addr->_localIp._ip.c_str(), addr->_localPort, addr->_remoteIp._ip.c_str(), addr->_remotePort, toIpv4, addr->ToString().c_str());
        }

        return;
    }

    auto iter = _unhandledContectAddr.find(stub);
    if(iter == _unhandledContectAddr.end())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("[connect res]suc but have no callback stub:%llu"), stub);
        return;
    }

    if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))
    {
        auto addrInfo = iter->second;
        g_Log->Info(LOGFMT_OBJ_TAG("[connect res]success, stub:%llu, sessionId:%llu [%s:%hu => %s(%s):%hu]\naddr info:%s ")
        , stub, sessionId, localAddr->_ip.c_str(), localAddr->_port
        , addrInfo->_remoteIp._ip.c_str(), remoteAddr->_ip.c_str(), remoteAddr->_port
        , addrInfo->ToString().c_str());
    }

    _unhandledContectAddr.erase(iter);
}

void SysLogicMgr::_CloseServiceEvent(KERNEL_NS::LibEvent *ev)
{
    g_Log->Info(LOGFMT_OBJ_TAG("service will close."));
    GetService()->MaskServiceModuleQuitFlag(this);
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
