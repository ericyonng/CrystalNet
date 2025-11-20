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
#include <service_common/service_proxy/ServiceProxyStatisticsInfo.h>

SERVICE_COMMON_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(ServiceProxy);

ServiceProxy::ServiceProxy()
:IServiceProxy(KERNEL_NS::RttiUtil::GetTypeId<ServiceProxy>())
,_maxServiceId{0}
,_closeServiceNum{0}
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

void ServiceProxy::PostMsg(UInt64 serviceId, KERNEL_NS::PollerEvent *msg, Int64 packetsCount)
{
    auto iter = _serviceIdRefRejectServiceStatus.find(serviceId);
    if(UNLIKELY(iter->second))
    {
        if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
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
        
    service->Push(msg);

    if(packetsCount > 0)
        service->AddRecvPackets(packetsCount);
}

void ServiceProxy::PostQuitService()
{
    _guard.Lock();
    for(auto iter : _idRefService)
    {
        auto ev = KERNEL_NS::QuitServiceEvent::New_QuitServiceEvent();
        PostMsg(iter.second->GetServiceId(), ev);
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

void ServiceProxy::CloseApp(Int32 err)
{
    GetOwner()->CastTo<Application>()->SinalFinish(err);
}

void ServiceProxy::OnMonitor(ServiceProxyStatisticsInfo &info)
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
        {
            ServiceStatisticsInfo serviceInfo;
            iter.second->OnMonitor(serviceInfo);
            info._serviceStatistatics.push_back(serviceInfo);
        }

        _guard.Unlock();
    }
}

void ServiceProxy::Clear()
{
    _Clear();

    if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))
        g_Log->Info(LOGFMT_OBJ_TAG("service proxy clear"));
}

Int32 ServiceProxy::_OnInit() 
{
    // 读取服务相关配置创建服务的线程
    auto application = GetOwner()->CastTo<Application>();
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

    bool cleanWithClose = false;
    Int32 errCode = Status::Success;
    bool hasAbnormal = false;
    
    KERNEL_FINALLY_BEGIN(cleaner)
    {
        if(!hasAbnormal && errCode == Status::Success)
            return;

        // 必须不能再抛异常了
        try
        {
            if(cleanWithClose)
                service->WillClose();

            _guard.Lock();
            _idRefService.erase(serviceId);
            _guard.Unlock();
            ++_closeServiceNum;

            if(cleanWithClose)
                service->Close();
        }
        catch (std::exception &e)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("abnormal:%s, when clean service: %s"), e.what(), service->IntroduceInfo().c_str());
        }
        catch (...)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("unknown abnormal when clean service: %s"), service->IntroduceInfo().c_str());
        }
    }
    KERNEL_FINALLY_END();

    // 服务启动需要管控异常
    try
    {
        // 设置参数
        service->SetServiceProxy(this);
        service->SetServiceId(serviceId);
        service->SetServiceName(serviceName);

        _guard.Lock();
        _idRefService.insert(std::make_pair(serviceId, service));
        _guard.Unlock();

        // 1.获取配置,创建激活的服务t中需要带本服务线程所需要的相关服务信息，且本线程必须是某个服务独占的
        g_Log->Info(LOGFMT_OBJ_TAG("service %s init..."), service->IntroduceInfo().c_str());
        errCode = service->Init();
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

        cleanWithClose = true;
        g_Log->Info(LOGFMT_OBJ_TAG("service %s prepare loop..."), service->IntroduceInfo().c_str());
        if(!service->PrepareLoop())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("service %s prepare loop fail"), service->IntroduceInfo().c_str());
            SetErrCode(NULL, Status::Failed);
            return;
        }
    }
    catch (std::exception &e)
    {
        hasAbnormal = true;
        errCode = Status::Failed;
        g_Log->Error(LOGFMT_OBJ_TAG("service exception service:%s, exception:%s"), service->IntroduceInfo().c_str(), e.what());
    }
    catch (...)
    {
        hasAbnormal = true;
        errCode = Status::Failed;
        g_Log->Error(LOGFMT_OBJ_TAG("service unknown exception service:%s"), service->IntroduceInfo().c_str());
        throw;
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

    // 剔除所有service的会话
    g_Log->Info(LOGFMT_OBJ_TAG("service %s quit all sessions..."), service->IntroduceInfo().c_str());
    service->GetComp<KERNEL_NS::IPollerMgr>()->QuitAllSessions(service->GetServiceId());

    try
    {
        g_Log->Info(LOGFMT_OBJ_TAG("service %s will close..."), service->IntroduceInfo().c_str());
        service->WillClose();
    }
    catch (std::exception &e)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("abnormal:%s, when will close service: %s"), e.what(), service->IntroduceInfo().c_str());
    }
    catch (...)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("unknown abnormal when will close service: %s"), service->IntroduceInfo().c_str());
    }

    try
    {
        g_Log->Info(LOGFMT_OBJ_TAG("service %s close..."), service->IntroduceInfo().c_str());
        service->Close();
    }
    catch (std::exception &e)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("abnormal:%s, when close service: %s"), e.what(), service->IntroduceInfo().c_str());
    }
    catch (...)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("unknown abnormal when close service: %s"), service->IntroduceInfo().c_str());
    }

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