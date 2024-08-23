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
#include <service_common/application/ResponseInfo.h>
#include <service_common/service_proxy/ServiceProxyInc.h>

#include <service_common/KillMonitor/KillMonitor.h>
#include <service_common/application/Application.h>

#ifndef DISABLE_OPCODES
 #include <protocols/protocols.h>
#endif

SERVICE_COMMON_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(Application);

Application::Application()
:IApplication(KERNEL_NS::RttiUtil::GetTypeId<Application>())
,_configIni(NULL)
,_monitor(NULL)
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

const KERNEL_NS::LibString &Application::GetAppAliasName() const 
{
    return _appConfig._appAliasName;
}

void Application::Clear()
{
    _Clear();
    KERNEL_NS::IApplication::Clear();
}

KERNEL_NS::LibString Application::ToString() const
{
    KERNEL_NS::LibString info;
    info.AppendFormat("iapplication:%s, alias:%s", KERNEL_NS::IApplication::ToString().c_str(), _appConfig._appAliasName.c_str());
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
    _runErr = err;

    auto ev = KERNEL_NS::QuitApplicationEvent::New_QuitApplicationEvent();
    _poller->Push(0, ev);

    // _lck.Sinal();
}

void Application::SetMachineId(UInt32 machineId)
{
    _appConfig._machineId = machineId;

    _configIni->Lock();
    _configIni->WriteNumber(APPLICATION_CONFIG_SEG, MACHINE_ID_KEY, static_cast<UInt64>(machineId));
    _configIni->Unlock();
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

    _monitor = CRYSTAL_NEW(KERNEL_NS::LibThread);
    _monitor->SetThreadName("AppMonitor");
    _monitor->AddTask(this, &Application::_OnMonitor);

    _configIni = KERNEL_NS::LibIniFile::New_LibIniFile();
    if(!_memoryIni.empty())
        _configIni->SetMemoryIniContent(_memoryIni);
        
    if(!_configIni->Init(_ini.c_str()))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("config ini init fail ini file:%s"), _ini.c_str());
        return Status::ConfigError;
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
    KERNEL_NS::GarbageThread::GetInstence()->SetIntervalMs(_kernelConfig._gCIntervalMs);
    KERNEL_NS::CenterMemoryCollector::GetInstance()->SetAllThreadMemoryAllocUpperLimit(_kernelConfig._allMemoryAlloctorTotalUpper);
    KERNEL_NS::CenterMemoryCollector::GetInstance()->SetWorkerIntervalMs(_kernelConfig._centerCollectorIntervalMs);
    KERNEL_NS::CenterMemoryCollector::GetInstance()->SetBlockNumForPurgeLimit(_kernelConfig._wakeupCenterCollectorMinBlockNum);
    KERNEL_NS::CenterMemoryCollector::GetInstance()->SetRecycleForPurgeLimit(_kernelConfig._wakeupCenterCollectorMinMergeBufferInfoNum);

    // 生成apply id
    _GenerateMachineApplyId();

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

    // 内存清理设置
    auto tlsComps = KERNEL_NS::TlsUtil::GetTlsCompsOwner();
    auto memoryCleaner = tlsComps->GetComp<KERNEL_NS::TlsMemoryCleanerComp>();
    memoryCleaner->SetIntervalMs(_kernelConfig._mergeTlsMemoryBlockIntervalMs);

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

    _monitor->Start();

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
    if(LIKELY(_monitor))
    {
        if(LIKELY(_monitor->HalfClose()))
            _monitor->FinishClose();
    }

    CompObject *notDownComp = NULL;
    for(;!IsAllCompsDown(notDownComp);)
    {
        g_Log->Info(LOGFMT_OBJ_TAG("app monitor wait for all comps down current not down comp:%s."), notDownComp->GetObjName().c_str());
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
    }

    MaskReady(false);

    _Clear();
    KERNEL_NS::IApplication::_OnHostClose();
}

void Application::_Clear()
{
    if(_configIni)
        KERNEL_NS::LibIniFile::Delete_LibIniFile(_configIni);
    _configIni = NULL;

    CRYSTAL_DELETE_SAFE(_monitor);

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
}

Int32 Application::_ReadBaseConfigs()
{
    // 内核配置项
    {// 黑白名单规则
        UInt64 cache = 0;
        if(!_configIni->CheckReadUInt(APPLICATION_KERNEL_CONFIG_SEG, KERNEL_OPTION_BLACK_WHITE_LIST_FLAG_KEY, cache))
        {
            cache = KERNEL_OPTION_BLACK_WHITE_LIST_FLAG_DEFAULT_VALUE;
            const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
            if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, KERNEL_OPTION_BLACK_WHITE_LIST_FLAG_KEY, value.c_str())))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                            , APPLICATION_KERNEL_CONFIG_SEG, KERNEL_OPTION_BLACK_WHITE_LIST_FLAG_KEY, value.c_str());
                return Status::ConfigError;
            }
        }

        _kernelConfig._blackWhiteListMode = static_cast<UInt32>(cache);
    }
    {// 会话数量
        UInt64 cache = 0;
        if(!_configIni->CheckReadUInt(APPLICATION_KERNEL_CONFIG_SEG, MAX_SESSION_QUANTITY_KEY, cache))
        {
            cache = MAX_SESSION_QUANTITY_DEFAULT_VALUE;
            const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
            if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, MAX_SESSION_QUANTITY_KEY, value.c_str())))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                            , APPLICATION_KERNEL_CONFIG_SEG, MAX_SESSION_QUANTITY_KEY, value.c_str());
                return Status::ConfigError;
            }
        }
        _kernelConfig._maxSessionQuantity = cache;
    }

    // tcp poller 相关配置
    {
        UInt64 linkInOutPollerAmount = 0;
        if(!_configIni->CheckReadUInt(APPLICATION_KERNEL_CONFIG_SEG, LINK_IN_OUT_POLLER_AMOUNT_KEY, linkInOutPollerAmount))
        {
            linkInOutPollerAmount = LINK_IN_OUT_POLLER_AMOUNT_DEFAULT_VALUE;
            const auto &value = KERNEL_NS::StringUtil::Num2Str(linkInOutPollerAmount);
            if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, LINK_IN_OUT_POLLER_AMOUNT_KEY, value.c_str())))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                            , APPLICATION_KERNEL_CONFIG_SEG, LINK_IN_OUT_POLLER_AMOUNT_KEY, value.c_str());
                return Status::ConfigError;
            }
        }
//         if(linkInOutPollerAmount == 0)
//         {
//             linkInOutPollerAmount = LINK_IN_OUT_POLLER_AMOUNT_DEFAULT_VALUE;
//             g_Log->Warn(LOGFMT_OBJ_TAG("linkin out poller cant be zero will fix it to default value:%llu"), linkInOutPollerAmount);
//         }
        _kernelConfig._linkInOutPollerAmount = static_cast<Int32>(linkInOutPollerAmount);

        UInt64 dataTransferPollerAmount = 0;
        if(!_configIni->CheckReadUInt(APPLICATION_KERNEL_CONFIG_SEG, DATA_TRANSFER_POLLER_AMOUNT_KEY, dataTransferPollerAmount))
        {
            dataTransferPollerAmount = DATA_TRANSFER_POLLER_AMOUNT_DEFAULT_VALUE;
            const auto &value = KERNEL_NS::StringUtil::Num2Str(dataTransferPollerAmount);
            if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, DATA_TRANSFER_POLLER_AMOUNT_KEY, value.c_str())))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                            , APPLICATION_KERNEL_CONFIG_SEG, DATA_TRANSFER_POLLER_AMOUNT_KEY, value.c_str());
                return Status::ConfigError;
            }
        }
//         if(dataTransferPollerAmount == 0)
//         {
//             dataTransferPollerAmount = DATA_TRANSFER_POLLER_AMOUNT_DEFAULT_VALUE;
//             g_Log->Warn(LOGFMT_OBJ_TAG("data transfer poller cant be zero will fix it to default value:%llu"), dataTransferPollerAmount);
//         }
        _kernelConfig._dataTransferPollerAmount = static_cast<Int32>(dataTransferPollerAmount);

        {// 单帧最大接收数据量
            UInt64 cache = 0;
            if(!_configIni->CheckReadUInt(APPLICATION_KERNEL_CONFIG_SEG, MAX_RECV_BYTES_PER_FRAME_KEY, cache))
            {
                cache = MAX_RECV_BYTES_PER_FRAME_DEFAULT_VALUE;
                const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, MAX_RECV_BYTES_PER_FRAME_KEY, value.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_KERNEL_CONFIG_SEG, MAX_RECV_BYTES_PER_FRAME_KEY, value.c_str());
                    return Status::ConfigError;
                }
            }
            _kernelConfig._maxRecvBytesPerFrame = cache;
        }
        {// 单帧最大发送数据量
            UInt64 cache = 0;
            if(!_configIni->CheckReadUInt(APPLICATION_KERNEL_CONFIG_SEG, MAX_SEND_BYTES_PER_FRAME_KEY, cache))
            {
                cache = MAX_SEND_BYTES_PER_FRAME_DEFAULT_VALUE;
                const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, MAX_SEND_BYTES_PER_FRAME_KEY, value.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_KERNEL_CONFIG_SEG, MAX_SEND_BYTES_PER_FRAME_KEY, value.c_str());
                    return Status::ConfigError;
                }
            }
            _kernelConfig._maxSendBytesPerFrame = cache;
        }
        {// 单帧最大处理连接数
            UInt64 cache = 0;
            if(!_configIni->CheckReadUInt(APPLICATION_KERNEL_CONFIG_SEG, MAX_ACCEPT_COUNT_PER_FRAME_KEY, cache))
            {
                cache = MAX_ACCEPT_COUNT_PER_FRAME_DEFAULT_VALUE;
                const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, MAX_ACCEPT_COUNT_PER_FRAME_KEY, value.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_KERNEL_CONFIG_SEG, MAX_ACCEPT_COUNT_PER_FRAME_KEY, value.c_str());
                    return Status::ConfigError;
                }
            }
            _kernelConfig._maxAcceptCountPerFrame = cache;
        }
        {// 最大帧时间片
            UInt64 cache = 0;
            if(!_configIni->CheckReadUInt(APPLICATION_KERNEL_CONFIG_SEG, MAX_PIECE_TIME_IN_MICRO_SEC_PER_FRAME_KEY, cache))
            {
                cache = MAX_PIECE_TIME_IN_MICRO_SEC_PER_FRAME_DEFAULT_VALUE;
                const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, MAX_PIECE_TIME_IN_MICRO_SEC_PER_FRAME_KEY, value.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_KERNEL_CONFIG_SEG, MAX_PIECE_TIME_IN_MICRO_SEC_PER_FRAME_KEY, value.c_str());
                    return Status::ConfigError;
                }
            }
            _kernelConfig._maxPieceTimeInMicroSecPerFrame = cache;
        }
        {// 最大poller扫描时间间隔
            UInt64 cache = 0;
            if(!_configIni->CheckReadUInt(APPLICATION_KERNEL_CONFIG_SEG, MAX_POLLER_SCAN_MILLISECONDS_KEY, cache))
            {
                cache = MAX_POLLER_SCAN_MILLISECONDS_DEFAULT_VALUE;
                const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, MAX_POLLER_SCAN_MILLISECONDS_KEY, value.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_KERNEL_CONFIG_SEG, MAX_POLLER_SCAN_MILLISECONDS_KEY, value.c_str());
                    return Status::ConfigError;
                }
            }
            _kernelConfig._maxPollerScanMilliseconds = cache;
        }
        {// 最大消息优先级等级
            Int64 cache = 0;
            if(!_configIni->CheckReadInt(APPLICATION_KERNEL_CONFIG_SEG, MAX_POLLER_MSG_PRIORITY_LEVEL_KEY, cache))
            {
                cache = MAX_POLLER_MSG_PRIORITY_LEVEL_DEFAULT_VALUE;
                const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, MAX_POLLER_MSG_PRIORITY_LEVEL_KEY, value.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_KERNEL_CONFIG_SEG, MAX_POLLER_MSG_PRIORITY_LEVEL_KEY, value.c_str());
                    return Status::ConfigError;
                }
            }
            _kernelConfig._maxPollerMsgPriorityLevel = static_cast<Int32>(cache);
        }
        {// 指定poller monitor事件的消息优先级等级
            Int64 cache = 0;
            if(!_configIni->CheckReadInt(APPLICATION_KERNEL_CONFIG_SEG, POLLER_MONITOR_EVENT_PRIORITY_LEVEL_KEY, cache))
            {
                cache = POLLER_MONITOR_EVENT_PRIORITY_LEVEL_DEFAULT_VALUE;
                const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, POLLER_MONITOR_EVENT_PRIORITY_LEVEL_KEY, value.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_KERNEL_CONFIG_SEG, POLLER_MONITOR_EVENT_PRIORITY_LEVEL_KEY, value.c_str());
                    return Status::ConfigError;
                }
            }
            _kernelConfig._pollerMonitorEventPriorityLevel = static_cast<Int32>(cache);
        }

        {// session缓冲大小设置
            UInt64 cache = 0;
            if(!_configIni->CheckReadUInt(APPLICATION_KERNEL_CONFIG_SEG, SESSION_BUFFER_CAPACITY_KEY, cache))
            {
                cache = SESSION_BUFFER_CAPACITY_DEFAULT_VALUE;
                const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, SESSION_BUFFER_CAPACITY_KEY, value.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_KERNEL_CONFIG_SEG, SESSION_BUFFER_CAPACITY_KEY, value.c_str());
                    return Status::ConfigError;
                }
            }
            _kernelConfig._sessionBufferCapicity = cache;
        }

        {// session 限速
            UInt64 cache = 0;
            if(!_configIni->CheckReadUInt(APPLICATION_KERNEL_CONFIG_SEG, SESSION_RECV_PACKET_SPEED_LIMIT_KEY, cache))
            {
                cache = SESSION_RECV_PACKET_SPEED_LIMIT_DEFAULT_VALUE;
                const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, SESSION_RECV_PACKET_SPEED_LIMIT_KEY, value.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_KERNEL_CONFIG_SEG, SESSION_RECV_PACKET_SPEED_LIMIT_KEY, value.c_str());
                    return Status::ConfigError;
                }
            }
            _kernelConfig._sessionRecvPacketSpeedLimit = cache;
        }

        {// session 限速时间单位
            UInt64 cache = 0;
            if(!_configIni->CheckReadUInt(APPLICATION_KERNEL_CONFIG_SEG, SESSION_RECV_PACKET_SPEED_TIME_UNIT_MS_KEY, cache))
            {
                cache = SESSION_RECV_PACKET_SPEED_TIME_UNIT_DEFAULT_VALUE;
                const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, SESSION_RECV_PACKET_SPEED_TIME_UNIT_MS_KEY, value.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_KERNEL_CONFIG_SEG, SESSION_RECV_PACKET_SPEED_TIME_UNIT_MS_KEY, value.c_str());
                    return Status::ConfigError;
                }
            }
            _kernelConfig._sessionRecvPacketSpeedTimeUnitMs = cache;
        }

        {// session 收包堆叠上限
            UInt64 cache = 0;
            if(!_configIni->CheckReadUInt(APPLICATION_KERNEL_CONFIG_SEG, SESSION_RECV_PACKET_STACK_LIMIT_KEY, cache))
            {
                cache = SESSION_RECV_PACKET_STACK_LIMIT_DEFAULT_VALUE;
                const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, SESSION_RECV_PACKET_STACK_LIMIT_KEY, value.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_KERNEL_CONFIG_SEG, SESSION_RECV_PACKET_STACK_LIMIT_KEY, value.c_str());
                    return Status::ConfigError;
                }
            }
            _kernelConfig._sessionRecvPacketStackLimit = cache;
        }

        {// session 收包单包限制
            UInt64 cache = 0;
            if(!_configIni->CheckReadUInt(APPLICATION_KERNEL_CONFIG_SEG, SESSION_RECV_PACKET_CONTENT_LIMIT_KEY, cache))
            {
                cache = SESSION_RECV_PACKET_CONTENT_LIMIT_DEFAULT_VALUE;
                const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, SESSION_RECV_PACKET_CONTENT_LIMIT_KEY, value.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_KERNEL_CONFIG_SEG, SESSION_RECV_PACKET_CONTENT_LIMIT_KEY, value.c_str());
                    return Status::ConfigError;
                }
            }
            _kernelConfig._sessionRecvPacketContentLimit = cache;
        }

        {// session 发包单包限制
            UInt64 cache = 0;
            if(!_configIni->CheckReadUInt(APPLICATION_KERNEL_CONFIG_SEG, SESSION_SEND_PACKET_CONTENT_LIMIT_KEY, cache))
            {
                cache = SESSION_SEND_PACKET_CONTENT_LIMIT_DEFAULT_VALUE;
                const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, SESSION_SEND_PACKET_CONTENT_LIMIT_KEY, value.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_KERNEL_CONFIG_SEG, SESSION_SEND_PACKET_CONTENT_LIMIT_KEY, value.c_str());
                    return Status::ConfigError;
                }
            }
            _kernelConfig._sessionSendPacketContentLimit = cache;
        }

        {// poller feature定义
            KERNEL_NS::LibString cache;
            if(!_configIni->ReadStr(APPLICATION_KERNEL_CONFIG_SEG, POLLER_FEATURE_TYPE_KEY, cache))
            {
                cache = POLLER_FEATURE_TYPE_DEFAULT_VALUE;
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, POLLER_FEATURE_TYPE_KEY, cache.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_KERNEL_CONFIG_SEG, POLLER_FEATURE_TYPE_KEY, cache.c_str());
                    return Status::ConfigError;
                }
            }

            const auto &featurePairParts = cache.Split(',');
            if(featurePairParts.size() < 2)
            {
                g_Log->Error(LOGFMT_OBJ_TAG("have no poller feature please check seg:%s, key:%s, value:%s")
                            , APPLICATION_KERNEL_CONFIG_SEG, POLLER_FEATURE_TYPE_KEY, cache.c_str());
                return Status::ConfigError;
            }

            for(auto &part : featurePairParts)
            {
                const auto &items = part.Split(':');
                if(items.size() < 2)
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("feature format error please check seg:%s, key:%s, value:%s, part:%s")
                            , APPLICATION_KERNEL_CONFIG_SEG, POLLER_FEATURE_TYPE_KEY, cache.c_str(), part.c_str());
                    return Status::ConfigError;
                }

                const auto &featureString = items[0];
                const auto &featureIdString = items[1];
                if(!featureIdString.isdigit())
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("feature id format error please check seg:%s, key:%s, value:%s, part:%s, featureIdString:%s")
                            , APPLICATION_KERNEL_CONFIG_SEG, POLLER_FEATURE_TYPE_KEY, cache.c_str(), part.c_str(), featureIdString.c_str());
                    return Status::ConfigError;
                }

                if(_kernelConfig._pollerFeatureStringRefId.find(featureString) != _kernelConfig._pollerFeatureStringRefId.end())
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("repeate feature please check seg:%s, key:%s, value:%s, part:%s, featureString:%s")
                            , APPLICATION_KERNEL_CONFIG_SEG, POLLER_FEATURE_TYPE_KEY, cache.c_str(), part.c_str(), featureString.c_str());
                    return Status::ConfigError;
                }
                
                auto featureId = KERNEL_NS::StringUtil::StringToInt32(featureIdString.c_str());
                _kernelConfig._pollerFeatureStringRefId.insert(std::make_pair(featureString, featureId));
                
                auto iterFeatureString = _kernelConfig._pollerFeatureIdRefString.find(featureId);
                if(iterFeatureString == _kernelConfig._pollerFeatureIdRefString.end())
                    iterFeatureString = _kernelConfig._pollerFeatureIdRefString.insert(std::make_pair(featureId, std::set<KERNEL_NS::LibString>())).first;
                iterFeatureString->second.insert(featureString);
            }

            if(_kernelConfig._pollerFeatureStringRefId.empty())
            {
                g_Log->Error(LOGFMT_OBJ_TAG("lack of poller feature config please check seg:%s, key:%s")
                        , APPLICATION_KERNEL_CONFIG_SEG, POLLER_FEATURE_TYPE_KEY);
                return Status::ConfigError;
            }
        }
    }

    {// gc 中央收集器等配置
        {
            UInt64 cache = 0;
            if(!_configIni->CheckReadUInt(APPLICATION_KERNEL_CONFIG_SEG, "AllMemoryAlloctorTotalUpper", cache))
            {
                cache = 4LLU * 1024LLU * 1024LLU * 1024LLU;
                const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, "AllMemoryAlloctorTotalUpper", value.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_KERNEL_CONFIG_SEG, "AllMemoryAlloctorTotalUpper", value.c_str());
                    return Status::ConfigError;
                }
            }
            _kernelConfig._allMemoryAlloctorTotalUpper = cache;
        }
        {
            UInt64 cache = 0;
            if(!_configIni->CheckReadUInt(APPLICATION_KERNEL_CONFIG_SEG, "GCIntervalMs", cache))
            {
                cache = 5000;
                const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, "GCIntervalMs", value.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_KERNEL_CONFIG_SEG, "GCIntervalMs", value.c_str());
                    return Status::ConfigError;
                }
            }
            _kernelConfig._gCIntervalMs = cache;
        }
        {
            Int64 cache = 0;
            if(!_configIni->CheckReadNumber(APPLICATION_KERNEL_CONFIG_SEG, "CenterCollectorIntervalMs", cache))
            {
                cache = 100;
                const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, "CenterCollectorIntervalMs", value.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_KERNEL_CONFIG_SEG, "CenterCollectorIntervalMs", value.c_str());
                    return Status::ConfigError;
                }
            }
            _kernelConfig._centerCollectorIntervalMs = cache;
        }
        {
            UInt64 cache = 0;
            if(!_configIni->CheckReadNumber(APPLICATION_KERNEL_CONFIG_SEG, "WakeupCenterCollectorMinBlockNum", cache))
            {
                cache = 100;
                const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, "WakeupCenterCollectorMinBlockNum", value.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_KERNEL_CONFIG_SEG, "WakeupCenterCollectorMinBlockNum", value.c_str());
                    return Status::ConfigError;
                }
            }
            _kernelConfig._wakeupCenterCollectorMinBlockNum = cache;
        }
        {
            UInt64 cache = 0;
            if(!_configIni->CheckReadNumber(APPLICATION_KERNEL_CONFIG_SEG, "WakeupCenterCollectorMinMergeBufferInfoNum", cache))
            {
                cache = 100;
                const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, "WakeupCenterCollectorMinMergeBufferInfoNum", value.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_KERNEL_CONFIG_SEG, "WakeupCenterCollectorMinMergeBufferInfoNum", value.c_str());
                    return Status::ConfigError;
                }
            }
            _kernelConfig._wakeupCenterCollectorMinMergeBufferInfoNum = cache;
        }

        {
            Int64 cache = 0;
            if(!_configIni->CheckReadNumber(APPLICATION_KERNEL_CONFIG_SEG, "MergeTlsMemoryBlockIntervalMs", cache))
            {
                cache = 60000;
                const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_KERNEL_CONFIG_SEG, "MergeTlsMemoryBlockIntervalMs", value.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_KERNEL_CONFIG_SEG, "MergeTlsMemoryBlockIntervalMs", value.c_str());
                    return Status::ConfigError;
                }
            }
            _kernelConfig._mergeTlsMemoryBlockIntervalMs = cache;
        }
    }

    KERNEL_NS::g_LinkerPollerName = POLLER_FEATURE_LINKER;
    KERNEL_NS::g_TransferPollerName = POLLER_FEATURE_DATA_TRANSFER;

    // udp poller 相关配置
    // auto &udpPollerConfig = _pollerConfig._udpPollerConfig;
    // {        
    //     {// linker相关配置
    //         auto newPollerFeatureConfig = KERNEL_NS::UdpPollerFeatureConfig::New_UdpPollerFeatureConfig(&udpPollerConfig, KERNEL_NS::PollerFeature::LINKER);
    //         newPollerFeatureConfig->_pollerInstConfigs.resize(_kernelConfig._linkInOutPollerAmount);
    //         udpPollerConfig._pollerFeatureRefConfig.insert(std::make_pair(KERNEL_NS::PollerFeature::LINKER, newPollerFeatureConfig));

    //         // 创建配置
    //         auto &pollerInstConfigs = newPollerFeatureConfig->_pollerInstConfigs;
    //         for(Int32 idx = 0; idx < _kernelConfig._linkInOutPollerAmount; ++idx)
    //         {
    //             auto newInstConfig = KERNEL_NS::UdpPollerInstConfig::New_UdpPollerInstConfig(newPollerFeatureConfig, static_cast<UInt32>(idx + 1));
    //             newInstConfig->_handleRecvBytesPerFrameLimit = _kernelConfig._maxRecvBytesPerFrame;
    //             newInstConfig->_handleSendBytesPerFrameLimit = _kernelConfig._maxSendBytesPerFrame;
    //             newInstConfig->_handleAcceptPerFrameLimit = _kernelConfig._maxAcceptCountPerFrame;
    //             newInstConfig->_maxPieceTimeInMicroseconds = _kernelConfig._maxPieceTimeInMicroSecPerFrame;
    //             newInstConfig->_maxSleepMilliseconds = _kernelConfig._maxPollerScanMilliseconds;
    //             newInstConfig->_maxPriorityLevel = _kernelConfig._maxPollerMsgPriorityLevel;
    //             newInstConfig->_pollerInstMonitorPriorityLevel = _kernelConfig._pollerMonitorEventPriorityLevel;
    //             newInstConfig->_bufferCapacity = _kernelConfig._sessionBufferCapicity;
    //             newInstConfig->_sessionRecvPacketSpeedLimit = _kernelConfig._sessionRecvPacketSpeedLimit;
    //             newInstConfig->_sessionRecvPacketSpeedTimeUnitMs = _kernelConfig._sessionRecvPacketSpeedTimeUnitMs;
    //             newInstConfig->_sessionRecvPacketStackLimit = _kernelConfig._sessionRecvPacketStackLimit;
    //             pollerInstConfigs[idx] = newInstConfig;
    //         }
    //     }
       
    //     {// transfer相关配置
    //         auto newPollerFeatureConfig = KERNEL_NS::UdpPollerFeatureConfig::New_UdpPollerFeatureConfig(&udpPollerConfig, KERNEL_NS::PollerFeature::DATA_TRANSFER);
    //         newPollerFeatureConfig->_pollerInstConfigs.resize(_kernelConfig._dataTransferPollerAmount);
    //         udpPollerConfig._pollerFeatureRefConfig.insert(std::make_pair(KERNEL_NS::PollerFeature::DATA_TRANSFER, newPollerFeatureConfig));

    //         // 创建配置
    //         auto &pollerInstConfigs = newPollerFeatureConfig->_pollerInstConfigs;
    //         for(Int32 idx = 0; idx < _kernelConfig._dataTransferPollerAmount; ++idx)
    //         {
    //             auto newInstConfig = KERNEL_NS::UdpPollerInstConfig::New_UdpPollerInstConfig(newPollerFeatureConfig,  static_cast<UInt32>(idx + 1));
    //             newInstConfig->_handleRecvBytesPerFrameLimit = _kernelConfig._maxRecvBytesPerFrame;
    //             newInstConfig->_handleSendBytesPerFrameLimit = _kernelConfig._maxSendBytesPerFrame;
    //             newInstConfig->_handleAcceptPerFrameLimit = _kernelConfig._maxAcceptCountPerFrame;
    //             newInstConfig->_maxPieceTimeInMicroseconds = _kernelConfig._maxPieceTimeInMicroSecPerFrame;
    //             newInstConfig->_maxSleepMilliseconds = _kernelConfig._maxPollerScanMilliseconds;
    //             newInstConfig->_maxPriorityLevel = _kernelConfig._maxPollerMsgPriorityLevel;
    //             newInstConfig->_pollerInstMonitorPriorityLevel = _kernelConfig._pollerMonitorEventPriorityLevel;
    //             newInstConfig->_bufferCapacity = _kernelConfig._sessionBufferCapicity;
    //             newInstConfig->_sessionRecvPacketSpeedLimit = _kernelConfig._sessionRecvPacketSpeedLimit;
    //             newInstConfig->_sessionRecvPacketSpeedTimeUnitMs = _kernelConfig._sessionRecvPacketSpeedTimeUnitMs;
    //             newInstConfig->_sessionRecvPacketStackLimit = _kernelConfig._sessionRecvPacketStackLimit;
    //             pollerInstConfigs[idx] = newInstConfig;
    //         }
    //     }
    // }

    {// application config
        {// 程序别名
            KERNEL_NS::LibString cache;
            if(!_configIni->ReadStr(APPLICATION_CONFIG_SEG, PROJECT_ALIAS_NAME_KEY, cache))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("have no project alias name please check!"));
                return Status::ConfigError;
            }
            _appConfig._appAliasName = cache;
        }
        {// 项目主服务名
            KERNEL_NS::LibString cache;
            if(!_configIni->ReadStr(APPLICATION_CONFIG_SEG, PROJECT_MAIN_SERVICE_NAME_KEY, cache))
            {
                cache = PROJECT_MAIN_SERVICE_NAME_DEFAULT_VALUE;
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_CONFIG_SEG, PROJECT_MAIN_SERVICE_NAME_KEY, cache.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_CONFIG_SEG, PROJECT_MAIN_SERVICE_NAME_KEY, cache.c_str());
                    return Status::ConfigError;
                }
            }
            _appConfig._projectMainServiceName = cache;
        }
        {// 机器id
            UInt64 cache;
            if(!_configIni->CheckReadUInt(APPLICATION_CONFIG_SEG, MACHINE_ID_KEY, cache))
            {
                cache = MACHINE_ID_DEFAULT_VALUE;
                const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_CONFIG_SEG, MACHINE_ID_KEY, value.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_CONFIG_SEG, MACHINE_ID_KEY, value.c_str());
                    return Status::ConfigError;
                }
            }
            _appConfig._machineId = static_cast<UInt32>(cache);
        }
        {// 注册成功机器时间
            UInt64 cache;
            if(!_configIni->CheckReadUInt(APPLICATION_CONFIG_SEG, REGISTER_TIME_KEY, cache))
            {
                cache = REGISTER_TIME_DEFAULT_VALUE;
                const auto &value = KERNEL_NS::StringUtil::Num2Str(cache);
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_CONFIG_SEG, REGISTER_TIME_KEY, value.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_CONFIG_SEG, REGISTER_TIME_KEY, value.c_str());
                    return Status::ConfigError;
                }
            }
            _appConfig._registerTime = cache;
        }
        {// 注册成功的进程路径
            KERNEL_NS::LibString cache;
            if(!_configIni->ReadStr(APPLICATION_CONFIG_SEG, REGISTER_PATH_KEY, cache))
            {
                cache = REGISTER_PATH_DEFAULT_VALUE;
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_CONFIG_SEG, REGISTER_PATH_KEY, cache.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_CONFIG_SEG, REGISTER_PATH_KEY, cache.c_str());
                    return Status::ConfigError;
                }
            }
            _appConfig._registerPath = cache;
        }
        {// 注册成功的进程id
            UInt64 cache;
            if(!_configIni->CheckReadNumber(APPLICATION_CONFIG_SEG, REGISTER_PROCESS_ID_KEY, cache))
            {
                cache = REGISTER_PROCESS_ID_DEFAULT_VALUE;
                if(UNLIKELY(!_configIni->WriteNumber(APPLICATION_CONFIG_SEG, REGISTER_PROCESS_ID_KEY, cache)))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%llu")
                                , APPLICATION_CONFIG_SEG, REGISTER_PROCESS_ID_KEY, cache);
                    return Status::ConfigError;
                }
            }
            _appConfig._registerProcessId = cache;
        }
        {// 程序申请机器id时的生成的唯一标识id
            KERNEL_NS::LibString cache;
            if(!_configIni->ReadStr(APPLICATION_CONFIG_SEG, MACHINE_APPLY_ID_KEY, cache))
            {
                cache = MACHINE_APPLY_ID_DEFAULT_VALUE;
                if(UNLIKELY(!_configIni->WriteStr(APPLICATION_CONFIG_SEG, MACHINE_APPLY_ID_KEY, cache.c_str())))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%s")
                                , APPLICATION_CONFIG_SEG, MACHINE_APPLY_ID_KEY, cache.c_str());
                    return Status::ConfigError;
                }
            }
            _appConfig._machineApplyId = cache;
        }

        {// DisableConsoleMonitorInfo
            Int32 cache;
            if(!_configIni->CheckReadNumber(APPLICATION_CONFIG_SEG, "DisableConsoleMonitorInfo", cache))
            {
                cache = 0;
                if(UNLIKELY(!_configIni->WriteNumber(APPLICATION_CONFIG_SEG, "DisableConsoleMonitorInfo", cache)))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s, value:%d")
                                , APPLICATION_CONFIG_SEG, "DisableConsoleMonitorInfo", cache);
                    return Status::ConfigError;
                }
            }
            _appConfig._disableConsoleMonitorInfo = cache != 0;
        }
    }

    return Status::Success;
}

void Application::_GenerateMachineApplyId()
{
    if(_appConfig._machineId != 0)
        return;

    _appConfig._registerTime = static_cast<UInt64>(GetAppStartTime().GetMicroTimestamp());
    _appConfig._registerPath = GetAppPath();
    _appConfig._registerProcessId = GetProcessId();

    // base64(sha256(进程路径 + 项目类型名 + 时间 + 进程id))
    KERNEL_NS::LibString str;
    str.AppendFormat("%s", _appConfig._registerPath.c_str())
        .AppendFormat("%s", _appConfig._projectMainServiceName.c_str())
        .AppendFormat("%s", KERNEL_NS::StringUtil::Num2Str(_appConfig._registerTime).c_str())
        .AppendFormat("%s", KERNEL_NS::StringUtil::Num2Str(_appConfig._registerProcessId).c_str())
        ;
    str = KERNEL_NS::LibDigest::MakeSha256(str);
    _appConfig._machineApplyId = KERNEL_NS::LibBase64::Encode(str);

    // 打印machine apply id
    g_Log->Info(LOGFMT_OBJ_TAG("%s \nmachine apply id:%s"), IntroduceStr().c_str(), _appConfig._machineApplyId.c_str());
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

    // 3.获取service信息
    auto serviceProxy = GetComp<ServiceProxy>();
    ServiceProxyStatisticsInfo serviceProxyInfo;
    if(serviceProxy && serviceProxy->IsStarted())
        serviceProxy->OnMonitor(serviceProxyInfo);
    
    KERNEL_NS::LibString info;
    info.AppendFormat("\n%s\n", serviceProxyInfo.ToString().c_str());

    auto pid = GetProcessId();
    info.AppendFormat("[PROC:%d SUMMARY INFO BEGIN]\n", pid);
    info.AppendFormat("%s\n", serviceProxyInfo.ToSummaryInfo().c_str());
    info.AppendFormat("[PROC:%d SUMMARY INFO END]\n", pid);

    Double average = 0;
    if(_statisticsInfoCache->_resCount > 0)
    {
        average = static_cast<Double>(_statisticsInfoCache->_resTotalNs) / _statisticsInfoCache->_resCount / 1000000;
    }

    if(!_appConfig._disableConsoleMonitorInfo)
    {
        g_Log->Monitor("%s\n[- RESPONSE INFO BEGIN -]\nSampleNumber:%llu. Min:%lf(ms). Average:%lf(ms). Max:%lf(ms).\n[- RESPONSE INFO END -]\n"
                    ,info.c_str(), _statisticsInfoCache->_resCount, static_cast<Double>(_statisticsInfoCache->_minResNs) / 1000000, average, static_cast<Double>(_statisticsInfoCache->_maxResNs) / 1000000);
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
    _poller->Disable();
    _poller->QuitLoop();

    // 先关闭监控
    if(LIKELY(_monitor))
    {
        if(LIKELY(_monitor->HalfClose()))
            _monitor->FinishClose();
    }
}

void Application::_OnMonitor(KERNEL_NS::LibThread *t)
{
    KERNEL_NS::CompObject *notReadyComp = NULL;
    for(;!IsAllCompsReady(notReadyComp);)
    {
        g_Log->Info(LOGFMT_OBJ_TAG("app monitor wait for all comps ready, current not ready comp:%s."), notReadyComp->GetObjName().c_str());
        KERNEL_NS::SystemUtil::ThreadSleep(1000);

        if(t->IsDestroy())
            break;

        if(GetErrCode() != Status::Success)
        {// 某些组件初始化失败
            g_Log->Error(LOGFMT_OBJ_TAG("comp start fail errCode:%d"), GetErrCode());
            break;
        }
    }

    if(!t->IsDestroy())
        MaskReady(true);

    auto poller = KERNEL_NS::TlsUtil::GetPoller();
    poller->PrepareLoop();

    auto timerMgr = poller->GetTimerMgr();

    if(LIKELY(GetErrCode() == Status::Success))
    {
        g_Log->Info(LOGFMT_OBJ_TAG("all comps are ready system info:%s."), ToString().c_str());

        // 性能监控 1秒一次
        KERNEL_NS::SmartPtr<KERNEL_NS::LibTimer, KERNEL_NS::AutoDelMethods::CustomDelete> monitorTimer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
        monitorTimer.SetClosureDelegate([](void *p){
            auto timer = reinterpret_cast<KERNEL_NS::LibTimer *>(p);
            KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(timer);
        });
        monitorTimer->SetTimeOutHandler([this](KERNEL_NS::LibTimer *t){
            _OnMonitorThreadFrame();
        });
        monitorTimer->Schedule(1000);

        // 内存监控 60秒一次
        KERNEL_NS::SmartPtr<KERNEL_NS::LibTimer, KERNEL_NS::AutoDelMethods::CustomDelete> memoryMonitorTimer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
        memoryMonitorTimer.SetClosureDelegate([](void *p){
            auto timer = reinterpret_cast<KERNEL_NS::LibTimer *>(p);
            KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(timer);
        });
        KERNEL_NS::SmartPtr<KERNEL_NS::IDelegate<void>, KERNEL_NS::AutoDelMethods::Release> workHandler = KERNEL_NS::MemoryMonitor::GetInstance()->MakeWorkTask();
        memoryMonitorTimer->SetTimeOutHandler([&workHandler](KERNEL_NS::LibTimer *t){
            workHandler->Invoke();
        });
        memoryMonitorTimer->Schedule(KERNEL_NS::MemoryMonitor::GetInstance()->GetMilliSecInterval());

        while (!t->IsDestroy())
        {
            timerMgr->Drive();
            KERNEL_NS::SystemUtil::ThreadSleep(100);

            // 内存日志信号触发则立即答应日志
            if(KERNEL_NS::SignalHandleUtil::ExchangeSignoTriggerFlag(_memoryLogSigno, false))
                workHandler->Invoke();
        }

        workHandler->Invoke();
    }
    else
    {
        g_Log->Error(LOGFMT_OBJ_TAG("application %s will close errCode:%d"), ToString().c_str(), GetErrCode());
        SinalFinish(GetErrCode());
    }

    poller->OnLoopEnd();

    g_Log->Info(LOGFMT_OBJ_TAG("monitor quik thread id:%llu."), KERNEL_NS::SystemUtil::GetCurrentThreadId());
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


SERVICE_COMMON_END
