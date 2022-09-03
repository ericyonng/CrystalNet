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
#include <service_common/service_proxy/ServiceProxy.h>
#include <service_common/service/service.h>
#include <service_common/application/Application.h>
#include <service_common/service_proxy/ServiceProxyFactory.h>

SERVICE_COMMON_BEGIN

ServiceProxy::ServiceProxy()
:_maxServiceId{0}
,_closeServiceNum{0}
,_tcpPollerMgr(NULL)
,_pollerMgr(NULL)
,_ipRuleMgr(NULL)
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

void ServiceProxy::PostMsg(UInt64 serviceId, UInt32 priorityLevel, KERNEL_NS::PollerEvent *msg)
{
    auto service = _GetService(serviceId);
    if(UNLIKELY(!service))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("post msg fail service not exists serviceId:%llu"), serviceId);
        msg->Release();
        return;
    }

    service->Push(priorityLevel, msg);
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

void ServiceProxy::TcpAddListen(UInt64 fromServiceId, Int32 level, Int32 family, const KERNEL_NS::LibString &ip, UInt16 port, UInt64 stub, const KERNEL_NS::SessionOption &sessionOption)
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
    g_Log->Info(LOGFMT_OBJ_TAG("add connect info:%s"), connectInfo->ToString().c_str());
    _tcpPollerMgr->PostConnect(connectInfo);
}

void ServiceProxy::TcpSendMsg(UInt64 pollerId, Int32 level, UInt64 sessionId, KERNEL_NS::LibPacket *packet)
{
    _tcpPollerMgr->PostSend(pollerId, level, sessionId, packet);
    g_Log->Debug(LOGFMT_OBJ_TAG("send msg packet info:%s"), packet->ToString().c_str());
}

void ServiceProxy::TcpSendMsg(UInt64 pollerId, Int32 level, UInt64 sessionId, KERNEL_NS::LibList<KERNEL_NS::LibPacket *> *packets)
{
    _tcpPollerMgr->PostSend(pollerId, level, sessionId, packets);
    g_Log->Debug(LOGFMT_OBJ_TAG("send msg packets count:%llu"), packets->GetAmount());
}

void ServiceProxy::TcpCloseSession(UInt64 pollerId, Int32 level, UInt64 sessionId)
{
    _tcpPollerMgr->PostCloseSession(pollerId, level, sessionId);
    g_Log->Info(LOGFMT_OBJ_TAG("post close session sessionId:%llu"), sessionId);
}

void ServiceProxy::AddWhite(const KERNEL_NS::LibString &ip)
{
    _ipRuleMgr->Lock();
    _ipRuleMgr->PushWhite(ip);
    _ipRuleMgr->Unlock();
}

void ServiceProxy::AddBlack(const KERNEL_NS::LibString &ip)
{
    _ipRuleMgr->Lock();
    _ipRuleMgr->PushBlack(ip);
    _ipRuleMgr->Unlock();
}

void ServiceProxy::EraseWhite(const KERNEL_NS::LibString &ip)
{
    _ipRuleMgr->Lock();
    _ipRuleMgr->EraseWhite(ip);
    _ipRuleMgr->Unlock();
}

void ServiceProxy::EraseBlack(const KERNEL_NS::LibString &ip)
{
    _ipRuleMgr->Lock();
    _ipRuleMgr->EraseBlack(ip);
    _ipRuleMgr->Unlock();
}

void ServiceProxy::Clear()
{
    _Clear();
    g_Log->Info(LOGFMT_OBJ_TAG("service proxy clear"));
}

Int32 ServiceProxy::_OnInit() 
{
    // 读取服务相关配置创建服务的线程
    auto application = GetOwner()->CastTo<Application>();
    _pollerMgr = application->GetComp<KERNEL_NS::IPollerMgr>();
    _tcpPollerMgr = _pollerMgr->GetComp<KERNEL_NS::TcpPollerMgr>();
    _ipRuleMgr = _pollerMgr->GetComp<KERNEL_NS::IpRuleMgr>();
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
        _activeServices = cache.Split(',', -1, false);
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
        newVar->BecomeStr() = serviceName;
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
    g_Log->NetInfo(LOGFMT_OBJ_TAG("service proxy quit service loop..."));
    for(auto iter:_idRefService)
        iter.second->QuitLoop();

    g_Log->NetInfo(LOGFMT_OBJ_TAG("service proxy half close service thread..."));
    for(auto thread:_serviceThreads)
        thread->HalfClose();

    g_Log->NetInfo(LOGFMT_OBJ_TAG("service proxy finish close service thread..."));
    for(auto thread:_serviceThreads)
        thread->FinishClose();

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
}

 void ServiceProxy::_OnServiceThread(KERNEL_NS::LibThread *t, KERNEL_NS::Variant *params)
 {
    const auto &serviceName = params->AsStr();
    g_Log->Info(LOGFMT_OBJ_TAG("service %s thread will start."), serviceName.c_str());
    KERNEL_NS::SmartPtr<IService, KERNEL_NS::_Build::TL, KERNEL_NS::AutoDelMethods::ReleaseSafe> service = _serviceFactory->Create(serviceName);
    
    // 设置参数
    service->SetServiceProxy(this);
    const UInt64 serviceId = ++_maxServiceId;
    service->SetServiceId(serviceId);
    service->SetServiceName(serviceName);

    // 1.获取配置,创建激活的服务t中需要带本服务线程所需要的相关服务信息，且本线程必须是某个服务独占的
    g_Log->Info(LOGFMT_OBJ_TAG("service %s init..."), service->IntroduceInfo().c_str());
    auto errCode = service->Init();
    if(errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("service %s init fail errCode:%d "), service->IntroduceInfo().c_str(), errCode);
        SetErrCode(NULL, errCode);
        return;
    }

    // 2.启动服务
    g_Log->Info(LOGFMT_OBJ_TAG("service %s start..."), service->IntroduceInfo().c_str());
    errCode = service->Start();
    if(errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("service %s start fail errCode:%d"), service->IntroduceInfo().c_str(), errCode);
        SetErrCode(NULL, errCode);
        return;
    }

    g_Log->Info(LOGFMT_OBJ_TAG("service %s prepare loop..."), service->IntroduceInfo().c_str());
    if(!service->PrepareLoop())
    {
        ++_closeServiceNum;
        g_Log->Error(LOGFMT_OBJ_TAG("service %s prepare loop fail"), service->IntroduceInfo().c_str());
        SetErrCode(NULL, errCode);
        return;
    }

    // 判断所有服务是否启动完成
    _guard.Lock();
    _idRefService.insert(std::make_pair(serviceId, service));
    if(_idRefService.size() >= _serviceThreads.size())
    {
        g_Log->Info(LOGFMT_OBJ_TAG("all service start suc. service count:%llu, final suc service:%s"), static_cast<UInt64>(_idRefService.size()), service->IntroduceInfo().c_str());
        MaskReady(true);
    }
    _guard.Unlock();

    // 3.服务的事件循环
    g_Log->Info(LOGFMT_OBJ_TAG("service %s event loop begin."), service->IntroduceInfo().c_str());
    service->EventLoop();
    
    // 4.事件循环结束,销毁
    g_Log->Info(LOGFMT_OBJ_TAG("service %s on event loop end..."), service->IntroduceInfo().c_str());
    service->OnLoopEnd();

    g_Log->Info(LOGFMT_OBJ_TAG("service %s will close..."), service->IntroduceInfo().c_str());
    service->WillClose();
    g_Log->Info(LOGFMT_OBJ_TAG("service %s close..."), service->IntroduceInfo().c_str());
    service->Close();
    g_Log->Info(LOGFMT_OBJ_TAG("service %s close finish"), service->IntroduceInfo().c_str());

    // 判断所有服务是否结束
    const UInt64 closeNum = ++_closeServiceNum;
    if(closeNum >= _serviceThreads.size())
    {
        g_Log->Info(LOGFMT_OBJ_TAG("all service close suc, final service:%s, closeNum:%llu."), service->IntroduceInfo().c_str(), closeNum);
        MaskReady(false);
    }
 }

SERVICE_COMMON_END