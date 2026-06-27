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
 * Date: 2022-06-21 12:54:08
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/kernel.h>

KERNEL_BEGIN

ALWAYS_HIDDEN UInt64 GetCrystalModuleId()
{
    static const UInt64 id = GetGlobalIdSrc().fetch_add(1, std::memory_order_release) + 1;

// #if _DEBUG
//     if(g_Log)
//     {
//         CLOG_DEBUG_GLOBAL(SystemUtil, "Application - GetCrystalModuleId:%llu", id);
//     }
// #endif
    
    return id;
}

KERNEL_END

#include <service_common/application/ResponseInfo.h>
#include <service_common/service_proxy/ServiceProxyInc.h>

#include <service_common/KillMonitor/KillMonitor.h>
#include <service_common/application/Application.h>

#ifndef DISABLE_OPCODES
 #include <protocols/protocols.h>
#endif

SERVICE_COMMON_BEGIN

Application::Application()
:IApplication(KERNEL_NS::RttiUtil::GetTypeId<Application>())
,_appCfg(KERNEL_NS::FileMonitor<ApplicationConfig, KERNEL_NS::YamlDeserializer>::New_FileMonitor())
,_startScanTimer(NULL)
,_monitorTimer(NULL)
,_memoryTimer(NULL)
,_killMonitorTimer(NULL)
,_statisticsInfo(StatisticsInfo::New_StatisticsInfo())
,_statisticsInfoCache(StatisticsInfo::New_StatisticsInfo())
,_maxEventType(KERNEL_NS::PollerEventType::EvMax)
{
    SetInterfaceTypeId(KERNEL_NS::RttiUtil::GetTypeId<IApplication>());
}

Application::~Application()
{
    _Clear();
}

void Application::Release()
{
    Application::Delete_Application(this);
}

UInt64 Application::GetAppModuleId() const
{
    return KERNEL_NS::GetCrystalModuleId();
}

const KERNEL_NS::LibString &Application::GetAppAliasName() const 
{
    return _appConfig.AliasName;
}

void Application::Clear()
{
    _Clear();
    KERNEL_NS::IApplication::Clear();
}

KERNEL_NS::LibString Application::ToString() const
{
    KERNEL_NS::LibString info;
    info.AppendFormat("iapplication:%s, alias:%s", KERNEL_NS::IApplication::ToString().c_str(), _appConfig.AliasName.c_str());
    return info;
}

void Application::OnRegisterComps()
{
    IApplication::OnRegisterComps();

    // 监控程序结束
    RegisterComp<KillMonitorMgrFactory>();

    RegisterComp<ServiceProxyFactory>();
}

void Application::SinalFinish(Int32 err)
{
    _runErr.store(err, std::memory_order_release);

    auto ev = KERNEL_NS::QuitApplicationEvent::New_QuitApplicationEvent();
    _poller->Push(ev);

    // _lck.Sinal();
}

void Application::PushResponceNs(UInt64 costNs)
{
    _guard.Lock();
    if(_statisticsInfo->_minResNs == 0)
        _statisticsInfo->_minResNs = costNs;
    if(_statisticsInfo->_maxResNs < costNs)
        _statisticsInfo->_maxResNs = costNs;

    ++_statisticsInfo->_resCount;
    _statisticsInfo->_resTotalNs += costNs;
    _guard.Unlock();
}

Int32 Application::_OnHostInit()
{
    Int32 errCode = KERNEL_NS::IApplication::_OnHostInit();
    if(errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("iapplication on houst init fail errcode:%d"), errCode);
        return errCode;
    }
    
    #ifndef DISABLE_OPCODES
    errCode = Opcodes::Init();
    if(errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("opcodes init fail errCode:%d"), errCode);
        return errCode;
    }
    #endif

    errCode = _ReadBaseConfigs();
    if(errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("read base configs fail errCode:%d"), errCode);
        return errCode;
    }

    // 设置内核先关参数
    KERNEL_NS::GarbageThread::GetInstence()->SetIntervalMs(_kernelConfig.GCIntervalMs);
    KERNEL_NS::CenterMemoryCollector::GetInstance()->SetAllThreadMemoryAllocUpperLimit(_kernelConfig.AllMemoryAlloctorTotalUpper);
    KERNEL_NS::CenterMemoryCollector::GetInstance()->SetWorkerIntervalMs(_kernelConfig.CenterCollectorIntervalMs);
    KERNEL_NS::CenterMemoryCollector::GetInstance()->SetBlockNumForPurgeLimit(_kernelConfig.WakeupCenterCollectorMinBlockNum);
    KERNEL_NS::CenterMemoryCollector::GetInstance()->SetRecycleForPurgeLimit(_kernelConfig.WakeupCenterCollectorMinMergeBufferInfoNum);

    // 信号处理
    auto &args = GetAppArgs();

    for(auto &arg : args)
    {
        g_Log->Info(LOGFMT_OBJ_TAG("arg:%s"), arg.c_str());
        auto seps = arg.Split("=");
        if(seps.empty())
            continue;

        if(seps.size() != 2)
        {
            g_Log->Debug(LOGFMT_OBJ_TAG("param format error, arg:%s"), arg.c_str());
            continue;
        }

        auto key = seps[0].strip();
        auto value = seps[1].strip();

        if(key == "--memory_log_signo")
        {
            _memoryLogSigno = KERNEL_NS::StringUtil::StringToInt32(value.c_str());
        }
        else
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("unknown key:%s, value:%s"), key.c_str(), value.c_str());
        }
    }

    KERNEL_NS::SignalHandleUtil::SetSignoIgnore(_memoryLogSigno);

    return Status::Success;
}

Int32 Application::_OnCompsCreated()
{
    Int32 errCode = KERNEL_NS::IApplication::_OnCompsCreated();
    if(errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("iapplication _OnCompsCreated fail errcode:%d"), errCode);
        return errCode;
    }

    _poller->Subscribe(KERNEL_NS::PollerEventType::QuitApplicationEvent, this, &Application::_OnQuitApplicationEvent);

    // 设置关闭监控
    auto killMonitor = GetComp<IKillMonitorMgr>();
    killMonitor->SetTimerMgr(_poller->GetTimerMgr());
    killMonitor->SetDeadthDetectionFile(_path + KERNEL_NS::LibString().AppendFormat(".kill_%d", _processId));
    
    _killMonitorTimer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    _killMonitorTimer->SetTimeOutHandler(this, &Application::_OnKillMonitorTimeOut);
    _killMonitorTimer->Schedule(2000);

    _startScanTimer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    _startScanTimer->SetTimeOutHandler(this, &Application::_OnMonitorTimeOut);
    _startScanTimer->Schedule(1000);
    

    // 内存清理设置
    auto tlsComps = KERNEL_NS::TlsUtil::GetTlsCompsOwner();
    auto memoryCleaner = tlsComps->GetComp<KERNEL_NS::TlsMemoryCleanerComp>();
    memoryCleaner->SetIntervalMs(_kernelConfig.MergeTlsMemoryBlockIntervalMs);

    // 设置serviceProxy
    auto pollerMgr = GetComp<KERNEL_NS::IPollerMgr>();
    auto serviceProxy = GetComp<SERVICE_COMMON_NS::ServiceProxy>();
    pollerMgr->SetServiceProxy(serviceProxy);
    
    return Status::Success;
}

Int32 Application::_OnHostWillStart()
{
    Int32 errCode = KERNEL_NS::IApplication::_OnHostWillStart();
    if(errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("iapplication _OnHostWillStart fail errcode:%d"), errCode);
        return errCode;
    }

    return Status::Success;
}

Int32 Application::_OnHostStart()
{
    Int32 errCode = KERNEL_NS::IApplication::_OnHostStart();
    if(errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("iapplication _OnHostStart fail errcode:%d"), errCode);
        return errCode;
    }

    g_Log->Info(LOGFMT_OBJ_TAG("application started."));
    return Status::Success;
}

void Application::_OnHostBeforeCompsWillClose()
{
    g_Log->Info(LOGFMT_OBJ_TAG("app will close"));

    // 依赖组件的需要先释放资源
    if(_killMonitorTimer)
    {
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(_killMonitorTimer);
        _killMonitorTimer = NULL;
    }
}

void Application::_OnHostWillClose()
{
    KERNEL_NS::IApplication::_OnHostWillClose();
}

void Application::_OnHostClose()
{
    if(_startScanTimer)
        _startScanTimer->Cancel();
    if(_monitorTimer)
        _monitorTimer->Cancel();
    if(_memoryTimer)
        _memoryTimer->Cancel();
    
    CompObject *notDownComp = NULL;
    for(;!IsAllCompsDown(notDownComp);)
    {
        g_Log->Info(LOGFMT_OBJ_TAG("app monitor wait for all comps down current not down comp:%s."), notDownComp->GetObjName().c_str());
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
    }

    MaskReady(false);

    _Clear();
    KERNEL_NS::IApplication::_OnHostClose();

    // 退出先打一次内存
    KERNEL_NS::SmartPtr<KERNEL_NS::IDelegate<void>, KERNEL_NS::AutoDelMethods::Release> workHandler = KERNEL_NS::MemoryMonitor::GetInstance()->MakeWorkTask();
    workHandler->Invoke();
}

void Application::_Clear()
{
    if(_startScanTimer)
    {
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(_startScanTimer);
        _startScanTimer = NULL;
    }

    if(_monitorTimer)
    {
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(_monitorTimer);
        _monitorTimer = NULL;
    }

    if(_memoryTimer)
    {
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(_memoryTimer);
        _memoryTimer = NULL;
    }
    
    if(_statisticsInfoCache)
    {
        StatisticsInfo::Delete_StatisticsInfo(_statisticsInfoCache);
        _statisticsInfoCache = NULL;
    }

    if(_statisticsInfo)
    {
        StatisticsInfo::Delete_StatisticsInfo(_statisticsInfo);
        _statisticsInfo = NULL;
    }

    if(_killMonitorTimer)
    {
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(_killMonitorTimer);
        _killMonitorTimer = NULL;
    }

    // 销毁协议信息
#ifndef DISABLE_OPCODES
    Opcodes::Destroy();
#endif

    if (_appCfg)
    {
        KERNEL_NS::FileMonitor<ApplicationConfig, KERNEL_NS::YamlDeserializer>::Delete_FileMonitor(_appCfg);
        _appCfg = NULL;
    }
}

Int32 Application::_ReadBaseConfigs()
{
    
    try
    {
        if(_yamlContent.empty())
        {
            auto config = YAML::LoadFile(_yamlPath.c_str());
            _appConfig = config["ApplicationConfig"].as<SERVICE_COMMON_NS::ApplicationConfig>();

            _appConfigSource.Path = _yamlPath;
        }
        else
        {
            auto config = YAML::Load(_yamlContent.GetRaw());
            _appConfig = config["ApplicationConfig"].as<SERVICE_COMMON_NS::ApplicationConfig>();
            _appConfigSource.FromMemory = const_cast<Byte8 *>(_yamlContent.data());
        }

        if (!_appCfg->Init(&_appConfigSource))
        {
            CLOG_ERROR("app config file monitor init fail");
            return Status::ConfigError;
        }

    }
    catch (std::exception &e)
    {
        CLOG_ERROR("app config load fail:%s", e.what());
        return Status::ConfigError;
    }
    catch (...)
    {
        CLOG_ERROR("app config init exception fail");
        return Status::ConfigError;
    }

    CLOG_INFO("app config load success.");
    return Status::Success;
}

void Application::_OnMonitorThreadFrame()
{
    // g_Log->Debug(LOGFMT_OBJ_TAG("\napplication motinotr thread frame app info:%s."), ToString().c_str());

    // 1.获取此刻的响应时间列表
    _guard.Lock();
    auto sw = _statisticsInfo;
    _statisticsInfo = _statisticsInfoCache;
    _statisticsInfoCache = sw;
    _guard.Unlock();

    // 2.网络信息
    KERNEL_NS::PollerMgrStatisticsInfo pollerMgrInfo;
    auto pollerMgr = GetComp<KERNEL_NS::IPollerMgr>();
    if(pollerMgr && pollerMgr->IsStarted())
        pollerMgr->OnMonitor(pollerMgrInfo);
    
    // 3.获取service信息
    auto serviceProxy = GetComp<ServiceProxy>();
    ServiceProxyStatisticsInfo serviceProxyInfo;
    if(serviceProxy && serviceProxy->IsStarted())
        serviceProxy->OnMonitor(serviceProxyInfo);

    auto curCfg = _appCfg->Current();
    if(!curCfg->DisableConsoleMonitorInfo)
    {
        KERNEL_NS::LibString info;
        info.AppendFormat("\n%s\n", serviceProxyInfo.ToString().c_str());

        auto pid = GetProcessId();
        info.AppendFormat("[PROC:%d SUMMARY INFO BEGIN]\n", pid);
        info.AppendFormat("%s\n", serviceProxyInfo.ToSummaryInfo().c_str());
        info.AppendFormat("[PROC:%d SUMMARY INFO END]\n", pid);

        info.AppendFormat("\n[PROC:%d KERNEL INFO BEGIN]\n", pid);
        info.AppendFormat("%s\n", pollerMgrInfo.ToString().c_str());
        info.AppendFormat("[PROC:%d KERNEL INFO END]\n", pid);

        Double average = 0;
        if(_statisticsInfoCache->_resCount > 0)
        {
            average = static_cast<Double>(_statisticsInfoCache->_resTotalNs) / _statisticsInfoCache->_resCount / 1000000;
        }

        CLOG_MONITOR("%s\n[- RESPONSE INFO BEGIN -]\nSampleNumber:%llu. Min:%lf(ms). Average:%lf(ms). Max:%lf(ms).\n[- RESPONSE INFO END -]\n", info.c_str(), _statisticsInfoCache->_resCount, static_cast<Double>(_statisticsInfoCache->_minResNs) / 1000000, average, static_cast<Double>(_statisticsInfoCache->_maxResNs) / 1000000);
    }

    _statisticsInfoCache->_minResNs = 0;
    _statisticsInfoCache->_maxResNs = 0;
    _statisticsInfoCache->_resCount = 0;
    _statisticsInfoCache->_resTotalNs = 0;
}

void Application::_OnQuitApplicationEvent(KERNEL_NS::PollerEvent *ev)
{
    _DoCloseApp();
}

void Application::_DoCloseApp()
{
    g_Log->Info(LOGFMT_OBJ_TAG("application will quit app:\n%s"), IntroduceStr().c_str());

    if(_startScanTimer)
        _startScanTimer->Cancel();
    if(_monitorTimer)
        _monitorTimer->Cancel();
    if(_memoryTimer)
        _memoryTimer->Cancel();
    
    _poller->Disable();
    _poller->QuitLoop();

    // 退出先打一次内存
    KERNEL_NS::SmartPtr<KERNEL_NS::IDelegate<void>, KERNEL_NS::AutoDelMethods::Release> workHandler = KERNEL_NS::MemoryMonitor::GetInstance()->MakeWorkTask();
    workHandler->Invoke();
}


void Application::_OnKillMonitorTimeOut(KERNEL_NS::LibTimer *timer)
{
    auto killMonitor = GetComp<IKillMonitorMgr>();
    if(killMonitor->IsReadyToDie())
    {
        _killMonitorTimer->Cancel();

        _DoCloseApp();
    }
}

void Application::_OnMonitorTimeOut(KERNEL_NS::LibTimer *timer)
{
    if(GetErrCode() != Status::Success)
    {// 某些组件初始化失败
        CLOG_ERROR_GLOBAL(Application,"comp start fail errCode:%d", GetErrCode());
        timer->Cancel();

        // 关服
        SinalFinish(GetErrCode());
        return;
    }
    
    // 等待所有组件启动
    auto poller = KERNEL_NS::TlsUtil::GetPoller();
    KERNEL_NS::CompObject *notReadyComp = NULL;
    if(!IsAllCompsReady(notReadyComp))
    {
        CLOG_INFO_GLOBAL(Application, "app monitor wait for all comps ready, current not ready comp:%s.", notReadyComp->GetObjName().c_str());
        return;
    }

    if(poller->IsQuit())
    {
        CLOG_INFO_GLOBAL(Application, "app quit");
        timer->Cancel();
        return;
    }

    // 所有都启动了, 设置ready
    if(!poller->IsQuit())
        MaskReady(true);

    // 没问题开启监控
    if(LIKELY(GetErrCode() == Status::Success))
    {
        CLOG_INFO_GLOBAL(Application, "all comps are ready system info:%s.", ToString().c_str());

        // 性能监控 1秒一次
        _monitorTimer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
        _monitorTimer->SetTimeOutHandler([this](KERNEL_NS::LibTimer *t){
            _OnMonitorThreadFrame();
        });
        _monitorTimer->Schedule(1000);

        // 内存监控日志
        _memoryTimer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
        KERNEL_NS::SmartPtr<KERNEL_NS::IDelegate<void>, KERNEL_NS::AutoDelMethods::Release> workHandler = KERNEL_NS::MemoryMonitor::GetInstance()->MakeWorkTask();
        _memoryTimer->SetTimeOutHandler([workHandler](KERNEL_NS::LibTimer *t){
            workHandler->Invoke();
        });
        _memoryTimer->Schedule(KERNEL_NS::MemoryMonitor::GetInstance()->GetMilliSecInterval());

        // 监控内存日志信号触发立即响应输出日志
        auto responseMemorySignal = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
        responseMemorySignal->GetMgr()->TakeOverLifeTime(responseMemorySignal, [](KERNEL_NS::LibTimer *t)
        {
            KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
        });
        auto memoryLogSigno = _memoryLogSigno;
        responseMemorySignal->SetTimeOutHandler([workHandler, memoryLogSigno](KERNEL_NS::LibTimer *t)
        {
            // 内存日志信号触发则立即答应日志
            if(KERNEL_NS::SignalHandleUtil::ExchangeSignoTriggerFlag(memoryLogSigno, false))
                workHandler->Invoke();
        });
        responseMemorySignal->Schedule(100);

        // 起服先打一次
        workHandler->Invoke();
    }
    else
    {
        // 启动失败则退出
        CLOG_INFO_GLOBAL(Application, "application %s will close errCode:%d", ToString().c_str(), GetErrCode());
        SinalFinish(GetErrCode());
    }

    timer->Cancel();
}



SERVICE_COMMON_END
