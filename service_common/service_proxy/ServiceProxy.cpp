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
 * Date: 2021-12-09 01:36:02
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/kernel.h>
#include <service_common/service_proxy/ServiceProxy.h>
#include <service_common/service/service.h>
#include <service_common/application/Application.h>
#include <service_common/service_proxy/ServiceProxyFactory.h>

SERVICE_COMMON_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(ServiceProxy);

ServiceProxy::ServiceProxy()
:_maxServiceId{0}
,_closeServiceNum{0}
,_tcpPollerMgr(NULL)
,_pollerMgr(NULL)
,_serviceFactory(NULL)
{

}
    
ServiceProxy::~ServiceProxy()
{
    _Clear();
}

void ServiceProxy::Release()
{
    ServiceProxy::DeleteByAdapter_ServiceProxy(ServiceProxyFactory::_buildType.V, this);
}

void ServiceProxy::PostMsg(UInt64 serviceId, UInt32 priorityLevel, KERNEL_NS::PollerEvent *msg, Int64 packetsCount)
{
    auto iter = _serviceIdRefRejectServiceStatus.find(serviceId);
    if(UNLIKELY(iter->second))
    {
        g_Log->Debug(LOGFMT_OBJ_TAG("reject post msg serviceId:%llu, msg:%s"), serviceId, msg->ToString().c_str());
        msg->Release();
        return;
    }

    // service一定会等session全部退出
    auto service = _GetService(serviceId);
    if(UNLIKELY(!service))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("post msg fail service not exists serviceId:%llu, msg:%s"), serviceId, msg->ToString().c_str());
        msg->Release();
        return;
    }
        
    service->Push(priorityLevel, msg);

    if(packetsCount > 0)
        service->AddRecvPackets(packetsCount);
}

void ServiceProxy::PostQuitService(UInt32 priorityLevel)
{
    _guard.Lock();
    for(auto iter : _idRefService)
    {
        auto ev = KERNEL_NS::QuitServiceEvent::New_QuitServiceEvent();
        PostMsg(iter.second->GetServiceId(), priorityLevel, ev);
    }
    _guard.Unlock();
}

KERNEL_NS::IProtocolStack *ServiceProxy::GetProtocolStack(KERNEL_NS::LibSession *session)
{
    const auto serviceId = session->GetServiceId();
    auto service = _GetService(serviceId);
    if(UNLIKELY(!service))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("get protocol stack fail service not exists serviceId:%llu, sessionId:%llu"), serviceId, session->GetId());
        return NULL;
    }

    return service->GetProtocolStack(session);
}

void ServiceProxy::TcpAddListen(UInt64 fromServiceId, Int32 level, Int32 family, const KERNEL_NS::LibString &ip, UInt16 port, Int32 sessionCount, UInt64 stub, const KERNEL_NS::SessionOption &sessionOption)
{
    auto service = _GetService(fromServiceId);
    if(UNLIKELY(!service))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("tcp add listen fail service not exists serviceId:%llu, ip:%s, port:%hu, stub:%llu"), fromServiceId, ip.c_str(), port, stub);
        return;
    }

    auto listenInfo = KERNEL_NS::LibListenInfo::New_LibListenInfo();
    listenInfo->_ip = ip;
    listenInfo->_port = port;
    listenInfo->_family = family;
    listenInfo->_serviceId = fromServiceId;
    listenInfo->_stub = stub;
    listenInfo->_priorityLevel = level;
    listenInfo->_protocolType = KERNEL_NS::ProtocolType::TCP;
    listenInfo->_sessionCount = sessionCount;
    listenInfo->_sessionOption = sessionOption;
    g_Log->Info(LOGFMT_OBJ_TAG("add listen info:%s"), listenInfo->ToString().c_str());
    _tcpPollerMgr->PostAddlisten(level, listenInfo);
}

void ServiceProxy::TcpAsynConnect(UInt64 fromServiceId, UInt64 stub, Int32 level, UInt16 family, const KERNEL_NS::LibString &remoteIp, UInt16 remotePort,
const KERNEL_NS::SessionOption &sessionOption, KERNEL_NS::IProtocolStack *protocolStack, Int32 retryTimes, Int64 periodMs,
const KERNEL_NS::LibString &localIp, UInt16 localPort)
{
    auto service = _GetService(fromServiceId);
    if(UNLIKELY(!service))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("tcp asyn connect fail service not exists serviceId:%llu, remoteIp:%s, remotePort:%hu, stub:%llu, localIp:%s, localPort:%hu")
                    , fromServiceId, remoteIp.c_str(), remotePort, stub, localIp.c_str(), localPort);
        return;
    }

    auto connectInfo = KERNEL_NS::LibConnectInfo::New_LibConnectInfo();
    connectInfo->_localIp = localIp;
    connectInfo->_localPort = localPort;
    connectInfo->_targetIp = remoteIp;
    connectInfo->_targetPort = remotePort;
    connectInfo->_family = family;
    connectInfo->_protocolType = KERNEL_NS::ProtocolType::TCP;
    connectInfo->_priorityLevel = level;
    connectInfo->_pollerId = 0;
    connectInfo->_retryTimes = retryTimes;
    connectInfo->_periodMs = periodMs;
    connectInfo->_stub = stub;
    connectInfo->_fromServiceId = fromServiceId;
    connectInfo->_stack = protocolStack;
    connectInfo->_sessionOption = sessionOption;
    // g_Log->Info(LOGFMT_OBJ_TAG("add connect info:%s"), connectInfo->ToString().c_str());
    _tcpPollerMgr->PostConnect(connectInfo);
}

void ServiceProxy::TcpSendMsg(UInt64 pollerId, Int32 level, UInt64 sessionId, KERNEL_NS::LibPacket *packet)
{
    // g_Log->Info(LOGFMT_OBJ_TAG("service send msg session id:%llu, packet:%s"), sessionId, packet->ToString().c_str());
    _tcpPollerMgr->PostSend(pollerId, level, sessionId, packet);
    // g_Log->Debug(LOGFMT_OBJ_TAG("send msg packet info:%s"), packet->ToString().c_str());
}

void ServiceProxy::TcpSendMsg(UInt64 pollerId, Int32 level, UInt64 sessionId, KERNEL_NS::LibList<KERNEL_NS::LibPacket *> *packets)
{
    auto stackLimit = GetApp()->GetKernelConfig()._sessionRecvPacketStackLimit;

    if(LIKELY((stackLimit == 0) || packets->GetAmount() <= stackLimit))
    {
        _tcpPollerMgr->PostSend(pollerId, level, sessionId, packets);
        return;
    }

    KERNEL_NS::LibList<KERNEL_NS::LibPacket *> *packetList = NULL;
    for(auto node = packets->Begin(); node;)
    {
        if(UNLIKELY(packetList == NULL))
            packetList = KERNEL_NS::LibList<KERNEL_NS::LibPacket *>::New_LibList();

        packetList->PushBack(node->_data);
        if(UNLIKELY((stackLimit != 0) && (packetList->GetAmount() >= stackLimit)))
        {
            _tcpPollerMgr->PostSend(pollerId, level, sessionId, packetList);
            packetList = NULL;
        }

        node = packets->Erase(node);
    }

    if(LIKELY(packetList))
        _tcpPollerMgr->PostSend(pollerId, level, sessionId, packetList);

    // g_Log->Debug(LOGFMT_OBJ_TAG("send msg packets count:%llu"), packets->GetAmount());
}

void ServiceProxy::TcpSendMsg(UInt64 pollerId, Int32 level, UInt64 sessionId, const std::list<KERNEL_NS::LibPacket *> &packets)
{
    KERNEL_NS::LibList<KERNEL_NS::LibPacket *> *packetList = NULL;
    auto stackLimit = GetApp()->GetKernelConfig()._sessionRecvPacketStackLimit;
    for(auto packet : packets)
    {
        if(UNLIKELY(packetList == NULL))
            packetList = KERNEL_NS::LibList<KERNEL_NS::LibPacket *>::New_LibList();
        
        packetList->PushBack(packet);

        if(UNLIKELY((stackLimit != 0) && (packetList->GetAmount() >= stackLimit)))
        {
            _tcpPollerMgr->PostSend(pollerId, level, sessionId, packetList);
            packetList = NULL;
        }
    }

    if(LIKELY(packetList))
        _tcpPollerMgr->PostSend(pollerId, level, sessionId, packetList);

    // g_Log->Debug(LOGFMT_OBJ_TAG("send msg packets count:%llu"), static_cast<UInt64>(packets.size()));
}


void ServiceProxy::TcpCloseSession(UInt64 pollerId, UInt64 fromeService, Int32 level, UInt64 sessionId, Int64 closeMillisecondTimeDelay, bool forbidRead, bool forbidWrite)
{
    _tcpPollerMgr->PostCloseSession(pollerId, fromeService, level, sessionId, closeMillisecondTimeDelay, forbidRead, forbidWrite);
    // g_Log->Info(LOGFMT_OBJ_TAG("post close session sessionId:%llu"), sessionId);
}

void ServiceProxy::CloseApp(Int32 err)
{
    GetOwner()->CastTo<Application>()->SinalFinish(err);
}

void ServiceProxy::OnMonitor(KERNEL_NS::LibString &info)
{
    for(auto iter : _idRefService)
    {
        const auto serviceId = iter.first;

        _guard.Lock();
        bool isReject = false;
        auto iterStatus = _serviceIdRefRejectServiceStatus.find(serviceId);
        if(iterStatus !=  _serviceIdRefRejectServiceStatus.end())
            isReject = iterStatus->second;

        if(!isReject)
            iter.second->OnMonitor(info);

        _guard.Unlock();
    }
}

void ServiceProxy::Clear()
{
    _Clear();
    g_Log->Info(LOGFMT_OBJ_TAG("service proxy clear"));
}


void ServiceProxy::AddWhite(const KERNEL_NS::LibString &ip, Int32 level)
{
    std::list<KERNEL_NS::IpControlInfo *> newList;
    auto newInfo = KERNEL_NS::IpControlInfo::Create();
    newInfo->_ip = ip;
    newInfo->_controlFlow.push_back(KERNEL_NS::IpControlInfo::ADD_WHITE);
    ControlIpPipline(newList, level);
}

void ServiceProxy::AddBlack(const KERNEL_NS::LibString &ip, Int32 level)
{
    std::list<KERNEL_NS::IpControlInfo *> newList;
    auto newInfo = KERNEL_NS::IpControlInfo::Create();
    newInfo->_ip = ip;
    newInfo->_controlFlow.push_back(KERNEL_NS::IpControlInfo::ADD_BLACK);
    ControlIpPipline(newList, level);
}

void ServiceProxy::EraseWhite(const KERNEL_NS::LibString &ip, Int32 level)
{
    std::list<KERNEL_NS::IpControlInfo *> newList;
    auto newInfo = KERNEL_NS::IpControlInfo::Create();
    newInfo->_ip = ip;
    newInfo->_controlFlow.push_back(KERNEL_NS::IpControlInfo::ERASE_WHITE);
    ControlIpPipline(newList, level);
}

void ServiceProxy::EraseBlack(const KERNEL_NS::LibString &ip, Int32 level)
{
    std::list<KERNEL_NS::IpControlInfo *> newList;
    auto newInfo = KERNEL_NS::IpControlInfo::Create();
    newInfo->_ip = ip;
    newInfo->_controlFlow.push_back(KERNEL_NS::IpControlInfo::ERASE_BLACK);
    ControlIpPipline(newList, level);
}

void ServiceProxy::AddWhite(const std::list<KERNEL_NS::LibString> &ips, Int32 level)
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

void ServiceProxy::AddBlack(const std::list<KERNEL_NS::LibString> &ips, Int32 level)
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

void ServiceProxy::EraseWhite(const std::list<KERNEL_NS::LibString> &ips, Int32 level)
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

void ServiceProxy::EraseBlack(const std::list<KERNEL_NS::LibString> &ips, Int32 level)
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

void ServiceProxy::ControlIpPipline(const std::list<KERNEL_NS::IpControlInfo *> &controlInfoList, Int32 level)
{
    _tcpPollerMgr->PostIpControl(level, controlInfoList);
}

Int32 ServiceProxy::_OnInit() 
{
    // 读取服务相关配置创建服务的线程
    auto application = GetOwner()->CastTo<Application>();
    _pollerMgr = application->GetComp<KERNEL_NS::IPollerMgr>();
    _tcpPollerMgr = _pollerMgr->GetComp<KERNEL_NS::TcpPollerMgr>();
    _closeServiceNum = 0;

    auto ini = application->GetIni();
    {
        KERNEL_NS::LibString cache;
        if(!ini->ReadStr(SERVICE_COMMON_CONFIG_SEG, ACTIVE_SERVICE_ITEM_KEY, cache))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("have no active service, SERVICE_COMMON_CONFIG_SEG:%s, ACTIVE_SERVICE_ITEM_KEY:%s")
                        , SERVICE_COMMON_CONFIG_SEG, ACTIVE_SERVICE_ITEM_KEY);

            SetErrCode(NULL, Status::ConfigError);
            return Status::ConfigError;
        }

        cache.strip();
        _activeServices = cache.Split(',', -1, false, false);
        if(_activeServices.empty())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("have no active service SERVICE_COMMON_CONFIG_SEG:%s, ACTIVE_SERVICE_ITEM_KEY:%s")
                        , SERVICE_COMMON_CONFIG_SEG, ACTIVE_SERVICE_ITEM_KEY);
            SetErrCode(NULL, Status::ConfigError);
            return Status::ConfigError;
        }
    }

    const Int32 serviceCount = static_cast<Int32>(_activeServices.size());
    _serviceThreads.resize(serviceCount);
    for(Int32 idx = 0; idx < serviceCount; ++idx)
    {
        auto &serviceName = _activeServices[idx];
        auto newThread = CRYSTAL_NEW(KERNEL_NS::LibThread);
        _serviceThreads[idx] = newThread;
        auto newVar = KERNEL_NS::Variant::New_Variant();
        const UInt64 serviceId = ++_maxServiceId;
        (*newVar)["ServiceName"] = serviceName;
        (*newVar)["ServiceId"] = serviceId;

        newThread->SetThreadName(KERNEL_NS::LibString().AppendFormat("service-%llu-%s", serviceId, serviceName.c_str()));

        // 初始化服务的设施
        _OnPrepareServiceThread(serviceId, serviceName);

        newThread->AddTask2(this, &ServiceProxy::_OnServiceThread, newVar);
    }

    g_Log->Info(LOGFMT_OBJ_TAG("init service proxy suc"));
    return Status::Success;
}

Int32 ServiceProxy::_OnStart()
{
    for(auto thread:_serviceThreads)
        thread->Start();

    g_Log->Info(LOGFMT_OBJ_TAG("service proxy start."));
    return Status::Success;
}

void ServiceProxy::_OnWillClose()
{
    PostQuitService();

    g_Log->NetInfo(LOGFMT_OBJ_TAG("service proxy quit service loop..."));

    g_Log->NetInfo(LOGFMT_OBJ_TAG("service proxy half close service thread..."));
    for(auto thread:_serviceThreads)
        thread->HalfClose();

    g_Log->NetInfo(LOGFMT_OBJ_TAG("service proxy finish close service thread..."));
    for(auto thread:_serviceThreads)
        thread->FinishClose();

    MaskReady(false);

    g_Log->Info(LOGFMT_OBJ_TAG("service proxy will close."));
}

void ServiceProxy::_OnClose()
{
    g_Log->Info(LOGFMT_OBJ_TAG("service proxy close"));
    _Clear();
    CompObject::_OnClose();
}

void ServiceProxy::_Clear()
{
    KERNEL_NS::ContainerUtil::DelContainer(_serviceThreads);

    if(_serviceFactory)
        _serviceFactory->Release();
    _serviceFactory = NULL;

    _idRefService.clear();
    _activeServices.clear();

    _serviceIdRefRejectServiceStatus.clear();
}

void ServiceProxy::_OnPrepareServiceThread(UInt64 serviceId, const KERNEL_NS::LibString &serviceName)
{
    _guard.Lock();

    _serviceIdRefRejectServiceStatus.insert(std::make_pair(serviceId, false));

    _guard.Unlock();
}

 void ServiceProxy::_OnServiceThread(KERNEL_NS::LibThread *t, KERNEL_NS::Variant *params)
 {
    const auto &serviceName = (*params)["ServiceName"].AsStr();
    const auto serviceId = (*params)["ServiceId"].AsUInt64();

    g_Log->Info(LOGFMT_OBJ_TAG("service %s serviceId:%llu thread will start thread id:%llu.")
                    , serviceName.c_str(), serviceId, t->GetTheadId());
    KERNEL_NS::SmartPtr<IService, KERNEL_NS::AutoDelMethods::ReleaseSafe> service = _serviceFactory->Create(serviceName);
    
    // 设置参数
    service->SetServiceProxy(this);
    service->SetServiceId(serviceId);
    service->SetServiceName(serviceName);

    _guard.Lock();
    _idRefService.insert(std::make_pair(serviceId, service));
    _guard.Unlock();

    // 1.获取配置,创建激活的服务t中需要带本服务线程所需要的相关服务信息，且本线程必须是某个服务独占的
    g_Log->Info(LOGFMT_OBJ_TAG("service %s init..."), service->IntroduceInfo().c_str());
    auto errCode = service->Init();
    if(errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("service %s init fail errCode:%d "), service->IntroduceInfo().c_str(), errCode);
        SetErrCode(NULL, errCode);
        _guard.Lock();
        _idRefService.erase(serviceId);
        _guard.Unlock();
        ++_closeServiceNum;
        return;
    }

    // 2.启动服务
    g_Log->Info(LOGFMT_OBJ_TAG("service %s start..."), service->IntroduceInfo().c_str());
    errCode = service->Start();
    if(errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("service %s start fail errCode:%d"), service->IntroduceInfo().c_str(), errCode);
        SetErrCode(NULL, errCode);
        _guard.Lock();
        _idRefService.erase(serviceId);
        _guard.Unlock();
        ++_closeServiceNum;

        return;
    }

    g_Log->Info(LOGFMT_OBJ_TAG("service %s prepare loop..."), service->IntroduceInfo().c_str());
    if(!service->PrepareLoop())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("service %s prepare loop fail"), service->IntroduceInfo().c_str());
        SetErrCode(NULL, Status::Failed);
        service->WillClose();

        _guard.Lock();
        _idRefService.erase(serviceId);
        _guard.Unlock();
        ++_closeServiceNum;

        service->Close();
        return;
    }

    // 判断所有服务是否启动完成
    _guard.Lock();
    if(_idRefService.size() >= _serviceThreads.size())
    {
        g_Log->Info(LOGFMT_OBJ_TAG("all service start suc. service count:%llu, final suc service:%s"), static_cast<UInt64>(_idRefService.size()), service->IntroduceInfo().c_str());
        MaskReady(true);
    }
    _guard.Unlock();

    // 3.服务的事件循环
    g_Log->Info(LOGFMT_OBJ_TAG("service %s safty event loop begin."), service->IntroduceInfo().c_str());
    service->EventLoop();

    // 停止服务
    g_Log->Info(LOGFMT_OBJ_TAG("service %s reject service."), service->IntroduceInfo().c_str());
    _RejectService(serviceId);

    // 4.事件循环结束,销毁
    service->OnLoopEnd();
    g_Log->Info(LOGFMT_OBJ_TAG("service %s on event loop end..."), service->IntroduceInfo().c_str());

    g_Log->Info(LOGFMT_OBJ_TAG("service %s will close..."), service->IntroduceInfo().c_str());
    service->WillClose();

    // 剔除所有service的会话
    g_Log->Info(LOGFMT_OBJ_TAG("service %s quit all sessions..."), service->IntroduceInfo().c_str());
    _pollerMgr->QuitAllSessions(service->GetServiceId());

    g_Log->Info(LOGFMT_OBJ_TAG("service %s close..."), service->IntroduceInfo().c_str());

    service->Close();
    ++_closeServiceNum;

    g_Log->Info(LOGFMT_OBJ_TAG("service %s close finish"), service->IntroduceInfo().c_str());

    // // 判断所有服务是否结束
    // const UInt64 closeNum = ++_closeServiceNum;
    // if(closeNum >= _serviceThreads.size())
    // {
    //     g_Log->Info(LOGFMT_OBJ_TAG("all service close suc, final service:%s, closeNum:%llu."), service->IntroduceInfo().c_str(), closeNum);
    //     MaskReady(false);
    // }
 }

SERVICE_COMMON_END