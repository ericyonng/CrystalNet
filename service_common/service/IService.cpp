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
 * Date: 2022-06-22 01:50:54
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/kernel.h>
#include <service_common/service/IService.h>
#include <service_common/service_proxy/ServiceProxyCompType.h>
#include <service_common/service_proxy/ServiceProxy.h>
#include <service_common/application/Application.h>
#include <service_common/service/ServiceStatisticsInfo.h>

SERVICE_COMMON_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IService);

IService::IService()
:_serviceId(0)
,_poller(NULL)
,_serviceProxy(NULL)
,_maxPieceTimeInMicroseconds(0)
,_maxPriorityLevel(0)
,_maxSleepMilliseconds(0)
,_recvPackets{0}
,_consumePackets{0}
,_maxEventType(KERNEL_NS::PollerEventType::EvMax)
,_serviceStatus(ServiceStatus::SERVICE_NOT_ACTIVE)
,_pollerMgr(NULL)
,_tcpPollerMgr(NULL)
{
    _SetType(ServiceProxyCompType::COMP_SERVICE);
    
}

IService::~IService()
{
    _Clear();
}

void IService::Clear()
{
    _OnServiceClear();
    _Clear();
    CompHostObject::Clear();
}

void IService::OnRegisterComps()
{
    // 网络模块
    RegisterComp<KERNEL_NS::PollerMgrFactory>();
    // 基础组件 使用当前线程的poller
    // RegisterComp<KERNEL_NS::PollerFactory>();
    // ip规则
    RegisterComp<KERNEL_NS::IpRuleMgrFactory>();

    _OnServiceRegisterComps();
}  

KERNEL_NS::LibString IService::ToString() const
{
    return KERNEL_NS::LibString().AppendFormat("%s, id:%llu, status:%d,%s", _serviceName.c_str(), _serviceId, _serviceStatus, ServiceStatusToString(_serviceStatus).c_str());
}

KERNEL_NS::LibString IService::IntroduceInfo() const
{
    return KERNEL_NS::LibString().AppendFormat("service id:%llu, name:%s, status:%d,%s"
                , _serviceId, _serviceName.c_str(), _serviceStatus, ServiceStatusToString(_serviceStatus).c_str());
}

void IService::InitPollerEventHandler()
{
    _poller->Subscribe(KERNEL_NS::PollerEventType::SessionCreated, this, &IService::_OnSessionCreated);
    _poller->Subscribe(KERNEL_NS::PollerEventType::AsynConnectRes, this, &IService::_OnAsynConnectRes);
    _poller->Subscribe(KERNEL_NS::PollerEventType::AddListenRes, this, &IService::_OnAddListenRes);
    _poller->Subscribe(KERNEL_NS::PollerEventType::SessionDestroy, this, &IService::_OnSessionDestroy);
    _poller->Subscribe(KERNEL_NS::PollerEventType::RecvMsg, this, &IService::_OnRecvMsg);
    _poller->Subscribe(KERNEL_NS::PollerEventType::QuitServiceEvent, this, &IService::_OnQuitServiceEvent);
}

void IService::OnMonitor(ServiceStatisticsInfo &info)
{
    info._serviceId = _serviceId;

    // 2.获取poller信息
    auto pollerMgr = GetComp<KERNEL_NS::IPollerMgr>();
    if(pollerMgr && pollerMgr->IsStarted())
        pollerMgr->OnMonitor(info._pollerMgrStatisticsInfo);

    info._recvPackets = _recvPackets.exchange(0, std::memory_order_acquire);
    info._consumePackets = _consumePackets.exchange(0, std::memory_order_acquire);
    info._sessionAmount = GetSessionAmount();
    GetComp<KERNEL_NS::Poller>()->OnMonitor(info._servicePollerInfo);
}

// service模块是否退出
bool IService::CheckServiceModuleQuitEnd(KERNEL_NS::LibString &notEndInfo) const
{
    std::set<const KERNEL_NS::CompObject *> notEndComps;
    for(auto comp : _forcusComps)
    {
        // 过滤结束的组件
        if(_quitEndComps.find(comp) != _quitEndComps.end())
            continue;

        notEndComps.insert(comp);
        notEndInfo.AppendFormat("focus service module [%s] not quit end.\n", comp->GetObjName().c_str());
    }

    return notEndComps.empty();
}

// service模块退出
void IService::MaskServiceModuleQuitFlag(const KERNEL_NS::CompObject *comp)
{
    _quitEndComps.insert(comp);
}

bool IService::IsServiceModuleQuit(const KERNEL_NS::CompObject *comp) const
{
    return _quitEndComps.find(comp) != _quitEndComps.end();
}

void IService::RegisterFocusServiceModule(const KERNEL_NS::CompObject *comp)
{
    _forcusComps.insert(comp);
}

KERNEL_NS::LibString IService::ServiceStatusToString(Int32 serviceStatus) const
{
    return ServiceStatus::ToString(serviceStatus);
}

void IService::SetMaxEventType(Int32 maxEventType)
{
    if(maxEventType >= _maxEventType)
        _maxEventType = maxEventType;
}

Int32 IService::GetMaxEventType() const
{
    return _maxEventType;
}

const KERNEL_NS::LibString &IService::GetAppName() const
{
    auto app = _serviceProxy->GetOwner()->CastTo<Application>();
    return app->GetAppName();
}

const KERNEL_NS::LibString &IService::GetAppAliasName() const
{
    return GetApp()->GetAppAliasName();
}

const Application *IService::GetApp() const
{
    return _serviceProxy->GetOwner()->CastTo<Application>();
}

Application *IService::GetApp()
{
    return _serviceProxy->GetOwner()->CastTo<Application>();
}

UInt64 IService::GetSessionAmount() const
{
    return 0;
}

void IService::SetServiceStatus(Int32 serviceStatus)
{
    const auto oldServiceStatus = _serviceStatus;
    if(_serviceStatus >= serviceStatus)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("error service status:%d,%s please check, current service status:%d,%s")
                , serviceStatus, ServiceStatusToString(serviceStatus).c_str(), _serviceStatus, ServiceStatusToString(_serviceStatus).c_str());
        return;
    }

    _serviceStatus = serviceStatus;
    g_Log->Info(LOGFMT_OBJ_TAG("service status changed [%d,%s] => [%d,%s].")
                , oldServiceStatus, ServiceStatusToString(oldServiceStatus).c_str(), _serviceStatus, ServiceStatusToString(_serviceStatus).c_str());
}

void IService::EventLoop()
{
    _OnEventLoopStart();
    _poller->EventLoop();
}

void IService::OnLoopEnd()
{
    _poller->OnLoopEnd();

    MaskReady(false);
}

bool IService::PrepareLoop()
{
    if(!_poller->PrepareLoop())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("poller prepare loop fail please check."));
        return false;
    }

    MaskReady(true);

    return true;
}

void IService::Push(Int32 level, KERNEL_NS::PollerEvent *ev)
{
    _poller->Push(level, ev);
}

void IService::Push(Int32 level, KERNEL_NS::LibList<KERNEL_NS::PollerEvent *> *evList)
{
    _poller->Push(level, evList);
}

Int32 IService::_OnHostInit()
{
    SetServiceStatus(ServiceStatus::SERVICE_INITING);
    auto errCode = _OnServiceInit();
    if(errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("service %s init fail suc errCode:%d."), IntroduceInfo().c_str(), errCode);
        return errCode;
    }

    SetServiceStatus(ServiceStatus::SERVICE_INITED);
    g_Log->Info(LOGFMT_OBJ_TAG("service %s init suc."), IntroduceInfo().c_str());
    return Status::Success;
}

Int32 IService::_OnPriorityLevelCompsCreated()
{
    // todo:
    _poller = KERNEL_NS::TlsUtil::GetDefTls()->_tlsComps->GetPoller();

    InitPollerEventHandler();

    // poller 设置
    KERNEL_NS::TimeSlice span(0, 0, _maxPieceTimeInMicroseconds);
    _poller->SetMaxPriorityLevel(_maxPriorityLevel);
    _poller->SetMaxPieceTime(span);
    _poller->SetMaxSleepMilliseconds(_maxSleepMilliseconds);
    _poller->SetPepareEventWorkerHandler(this, &IService::_OnPollerPrepare);
    _poller->SetEventWorkerCloseHandler(this, &IService::_OnPollerWillDestroy);

    _pollerMgr = GetComp<KERNEL_NS::IPollerMgr>();
    auto st = _OnServicePriorityLevelCompsCreated();
    if(st != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("_OnServicePriorityLevelCompsCreated fail st:%d"), st);
        return st;
    }

    return Status::Success;
}

Int32 IService::_OnCompsCreated()
{
    Int32 errCode = _OnServiceCompsCreated();
    if(errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("service %s handle _OnServiceCompsCreated fail errCode:%d.")
                    , IntroduceInfo().c_str(), errCode);
        return errCode;
    }

    g_Log->Info(LOGFMT_OBJ_TAG("service %s comps created suc."), IntroduceInfo().c_str());
    return Status::Success;
}

Int32 IService::_OnHostWillStart()
{
    _tcpPollerMgr = _pollerMgr->GetComp<KERNEL_NS::TcpPollerMgr>();
    SetServiceStatus(ServiceStatus::SERVICE_STARTING);

    g_Log->Info(LOGFMT_OBJ_TAG("service %s will start suc."), IntroduceInfo().c_str());
    return Status::Success;
}

Int32 IService::_OnHostStart()
{
    Int32 errCode = _OnServiceStartup();
    if(errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("service:%s, _OnServiceStartup fail errCode:%d"), IntroduceInfo().c_str(), errCode);
        return errCode;
    }

    SetServiceStatus(ServiceStatus::SERVICE_STARTED);
    g_Log->Info(LOGFMT_OBJ_TAG("service %s start suc."), IntroduceInfo().c_str());
    return GetErrCode();
}

void IService::_OnHostBeforeCompsWillClose() 
{
    g_Log->Info(LOGFMT_OBJ_TAG("service %s before comps will close suc."), IntroduceInfo().c_str());
}

void IService::_OnHostWillClose()
{
    SetServiceStatus(ServiceStatus::SERVICE_CLOSING);
    _OnServiceWillClose();
    g_Log->Info(LOGFMT_OBJ_TAG("service %s will close suc."), IntroduceInfo().c_str());
}

void IService::_OnHostClose() 
{
    _OnServiceClosed();
    g_Log->Info(LOGFMT_OBJ_TAG("service %s close suc."), IntroduceInfo().c_str());
    SetServiceStatus(ServiceStatus::SERVICE_CLOSED);
}

void IService::_OnServiceClear()
{
    g_Log->Info(LOGFMT_OBJ_TAG("service %s on service clear default."), IntroduceInfo().c_str());
}

Int32 IService::_OnServiceCompsCreated()
{
    g_Log->Info(LOGFMT_OBJ_TAG("service %s on service comps created default."), IntroduceInfo().c_str());
    return Status::Success;
}

Int32 IService::_OnServiceStartup()
{
    g_Log->Info(LOGFMT_OBJ_TAG("service %s on service start up default."), IntroduceInfo().c_str());
    return Status::Success;
}

void IService::_OnServiceWillClose()
{
    g_Log->Info(LOGFMT_OBJ_TAG("service %s on service will close default."), IntroduceInfo().c_str());
}

void IService::_OnServiceClosed()
{
    g_Log->Info(LOGFMT_OBJ_TAG("service %s on service closed default."), IntroduceInfo().c_str());
}

void IService::_OnSessionCreated(KERNEL_NS::PollerEvent *msg)
{
    g_Log->Info(LOGFMT_OBJ_TAG("session created:%s"), msg->ToString().c_str());
}

void IService::_OnSessionDestroy(KERNEL_NS::PollerEvent *msg) 
{
    g_Log->Info(LOGFMT_OBJ_TAG("session destroyed:%s"), msg->ToString().c_str());
}

void IService::_OnAsynConnectRes(KERNEL_NS::PollerEvent *msg)
{
    g_Log->Info(LOGFMT_OBJ_TAG("asyn connect response:%s"), msg->ToString().c_str());
}

void IService::_OnAddListenRes(KERNEL_NS::PollerEvent *msg) 
{
    g_Log->Info(LOGFMT_OBJ_TAG("add listen response:%s"), msg->ToString().c_str());
}

void IService::_OnRecvMsg(KERNEL_NS::PollerEvent *msg) 
{
    g_Log->Info(LOGFMT_OBJ_TAG("recieve message from net:%s"), msg->ToString().c_str());
}

void IService::_OnQuitServiceEvent(KERNEL_NS::PollerEvent *msg)
{
    SetServiceStatus(ServiceStatus::SERVICE_WILL_QUIT);

    g_Log->Info(LOGFMT_OBJ_TAG("will quit service [%s] msg:%s"), IntroduceInfo().c_str(),  msg->ToString().c_str());
    _OnQuitingService(msg);

    // 启动定时器检测service的所有模块是否已经完全退出
    auto timerMgr = GetTimerMgr();
    if(UNLIKELY(!timerMgr))
    {
        if(LIKELY(_poller))
        {
            _poller->Disable();
            _poller->QuitLoop();
        }

        g_Log->Warn(LOGFMT_OBJ_TAG("have no timer mgr when quit service service info:%s."), ToString().c_str());
        return;
    }

    // 每秒检测一次模块是否退出
    auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    timer->GetMgr()->TakeOverLifeTime(timer, [](KERNEL_NS::LibTimer *t){
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
    });
    timer->SetTimeOutHandler([this](KERNEL_NS::LibTimer *t){
        
        KERNEL_NS::LibString notEndInfo;
        if(!CheckServiceModuleQuitEnd(notEndInfo))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("service module not end info:\n%s"), notEndInfo.c_str());
            return;
        }

        _poller->Disable();
        _poller->QuitLoop();
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
    });
    timer->Schedule(1000);
}

bool IService::_OnPollerPrepare(KERNEL_NS::Poller *poller)
{
    g_Log->Info(LOGFMT_OBJ_TAG("service poller prepare"));
    return true;
}

void IService::_OnPollerWillDestroy(KERNEL_NS::Poller *poller)
{
    g_Log->Info(LOGFMT_OBJ_TAG("service will destroy."));
}

void IService::_Clear()
{
    g_Log->Info(LOGFMT_OBJ_TAG("service will clear."));
}


SERVICE_COMMON_END
