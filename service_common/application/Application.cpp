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
#include <service_common/service_proxy/ServiceProxyInc.h>

#include <service_common/application/Application.h>

SERVICE_COMMON_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(Application);

Application::Application()
:_configIni(NULL)
,_monitor(NULL)
,_statisticsInfo(StatisticsInfo::New_StatisticsInfo())
,_statisticsInfoCache(StatisticsInfo::New_StatisticsInfo())
{

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
    RegisterComp<KERNEL_NS::PollerMgrFactory>();
    RegisterComp<ServiceProxyFactory>();
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
    _monitor->AddTask(this, &Application::_OnMonitor);

    _configIni = KERNEL_NS::LibIniFile::New_LibIniFile();
    if(!_memoryIni.empty())
        _configIni->SetMemoryIniContent(_memoryIni);
        
    if(!_configIni->Init(_ini.c_str()))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("config ini init fail ini file:%s"), _ini.c_str());
        return Status::ConfigError;
    }

    errCode = _ReadBaseConfigs();
    if(errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("read base configs fail errCode:%d"), errCode);
        return errCode;
    }

    // ??????apply id
    _GenerateMachineApplyId();

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

    // poller mgr ?????????
    auto pollerMgr = GetComp<KERNEL_NS::IPollerMgr>();
    pollerMgr->SetConfig(_pollerConfig);
    pollerMgr->SetServiceProxy(GetComp<SERVICE_COMMON_NS::ServiceProxy>());

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
}

void Application::_OnHostWillClose()
{
    KERNEL_NS::IApplication::_OnHostWillClose();
}

void Application::_OnHostClose()
{
    CompObject *notDownComp = NULL;
    for(;!IsAllCompsDown(notDownComp);)
    {
        g_Log->Info(LOGFMT_OBJ_TAG("app monitor wait for all comps down current not down comp:%s."), notDownComp->GetObjName().c_str());
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
    }

    if(LIKELY(_monitor))
    {
        if(LIKELY(_monitor->HalfClose()))
            _monitor->FinishClose();
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
}

Int32 Application::_ReadBaseConfigs()
{
    // ???????????????
    {// ??????????????????
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
        _pollerConfig._blackWhiteListFlag = static_cast<UInt32>(cache);
    }
    {// ????????????
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
        _pollerConfig._maxSessionQuantity = cache;
    }

    // tcp poller ????????????
    auto &tcpPollerConfig = _pollerConfig._tcpPollerConfig;
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

        {// ???????????????????????????
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
        {// ???????????????????????????
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
        {// ???????????????????????????
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
        {// ??????????????????
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
        {// ??????poller??????????????????
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
        {// ???????????????????????????
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
        {// ??????poller monitor??????????????????????????????
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

        {// session??????????????????
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

        {// session ??????
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

        {// session ??????????????????
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

        {// session ??????????????????
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
        
        {// linker????????????
            auto newPollerFeatureConfig = KERNEL_NS::TcpPollerFeatureConfig::New_TcpPollerFeatureConfig(&tcpPollerConfig, KERNEL_NS::PollerFeature::LINKER);
            newPollerFeatureConfig->_pollerInstConfigs.resize(_kernelConfig._linkInOutPollerAmount);
            tcpPollerConfig._pollerFeatureRefConfig.insert(std::make_pair(KERNEL_NS::PollerFeature::LINKER, newPollerFeatureConfig));

            // ????????????
            auto &pollerInstConfigs = newPollerFeatureConfig->_pollerInstConfigs;
            for(Int32 idx = 0; idx < _kernelConfig._linkInOutPollerAmount; ++idx)
            {
                auto newInstConfig = KERNEL_NS::TcpPollerInstConfig::New_TcpPollerInstConfig(newPollerFeatureConfig, static_cast<UInt32>(idx + 1));
                newInstConfig->_handleRecvBytesPerFrameLimit = _kernelConfig._maxRecvBytesPerFrame;
                newInstConfig->_handleSendBytesPerFrameLimit = _kernelConfig._maxSendBytesPerFrame;
                newInstConfig->_handleAcceptPerFrameLimit = _kernelConfig._maxAcceptCountPerFrame;
                newInstConfig->_maxPieceTimeInMicroseconds = _kernelConfig._maxPieceTimeInMicroSecPerFrame;
                newInstConfig->_maxSleepMilliseconds = _kernelConfig._maxPollerScanMilliseconds;
                newInstConfig->_maxPriorityLevel = _kernelConfig._maxPollerMsgPriorityLevel;
                newInstConfig->_pollerInstMonitorPriorityLevel = _kernelConfig._pollerMonitorEventPriorityLevel;
                newInstConfig->_bufferCapacity = _kernelConfig._sessionBufferCapicity;
                newInstConfig->_sessionRecvPacketSpeedLimit = _kernelConfig._sessionRecvPacketSpeedLimit;
                newInstConfig->_sessionRecvPacketSpeedTimeUnitMs = _kernelConfig._sessionRecvPacketSpeedTimeUnitMs;
                newInstConfig->_sessionRecvPacketStackLimit = _kernelConfig._sessionRecvPacketStackLimit;
                pollerInstConfigs[idx] = newInstConfig;
            }
        }
       
        {// transfer????????????
            auto newPollerFeatureConfig = KERNEL_NS::TcpPollerFeatureConfig::New_TcpPollerFeatureConfig(&tcpPollerConfig, KERNEL_NS::PollerFeature::DATA_TRANSFER);
            newPollerFeatureConfig->_pollerInstConfigs.resize(_kernelConfig._dataTransferPollerAmount);
            tcpPollerConfig._pollerFeatureRefConfig.insert(std::make_pair(KERNEL_NS::PollerFeature::DATA_TRANSFER, newPollerFeatureConfig));

            // ????????????
            auto &pollerInstConfigs = newPollerFeatureConfig->_pollerInstConfigs;
            for(Int32 idx = 0; idx < _kernelConfig._dataTransferPollerAmount; ++idx)
            {
                auto newInstConfig = KERNEL_NS::TcpPollerInstConfig::New_TcpPollerInstConfig(newPollerFeatureConfig,  static_cast<UInt32>(idx + 1));
                newInstConfig->_handleRecvBytesPerFrameLimit = _kernelConfig._maxRecvBytesPerFrame;
                newInstConfig->_handleSendBytesPerFrameLimit = _kernelConfig._maxSendBytesPerFrame;
                newInstConfig->_handleAcceptPerFrameLimit = _kernelConfig._maxAcceptCountPerFrame;
                newInstConfig->_maxPieceTimeInMicroseconds = _kernelConfig._maxPieceTimeInMicroSecPerFrame;
                newInstConfig->_maxSleepMilliseconds = _kernelConfig._maxPollerScanMilliseconds;
                newInstConfig->_maxPriorityLevel = _kernelConfig._maxPollerMsgPriorityLevel;
                newInstConfig->_pollerInstMonitorPriorityLevel = _kernelConfig._pollerMonitorEventPriorityLevel;
                newInstConfig->_bufferCapacity = _kernelConfig._sessionBufferCapicity;
                newInstConfig->_sessionRecvPacketSpeedLimit = _kernelConfig._sessionRecvPacketSpeedLimit;
                newInstConfig->_sessionRecvPacketSpeedTimeUnitMs = _kernelConfig._sessionRecvPacketSpeedTimeUnitMs;
                newInstConfig->_sessionRecvPacketStackLimit = _kernelConfig._sessionRecvPacketStackLimit;
                pollerInstConfigs[idx] = newInstConfig;
            }
        }
    }

    // udp poller ????????????
    // auto &udpPollerConfig = _pollerConfig._udpPollerConfig;
    // {        
    //     {// linker????????????
    //         auto newPollerFeatureConfig = KERNEL_NS::UdpPollerFeatureConfig::New_UdpPollerFeatureConfig(&udpPollerConfig, KERNEL_NS::PollerFeature::LINKER);
    //         newPollerFeatureConfig->_pollerInstConfigs.resize(_kernelConfig._linkInOutPollerAmount);
    //         udpPollerConfig._pollerFeatureRefConfig.insert(std::make_pair(KERNEL_NS::PollerFeature::LINKER, newPollerFeatureConfig));

    //         // ????????????
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
       
    //     {// transfer????????????
    //         auto newPollerFeatureConfig = KERNEL_NS::UdpPollerFeatureConfig::New_UdpPollerFeatureConfig(&udpPollerConfig, KERNEL_NS::PollerFeature::DATA_TRANSFER);
    //         newPollerFeatureConfig->_pollerInstConfigs.resize(_kernelConfig._dataTransferPollerAmount);
    //         udpPollerConfig._pollerFeatureRefConfig.insert(std::make_pair(KERNEL_NS::PollerFeature::DATA_TRANSFER, newPollerFeatureConfig));

    //         // ????????????
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
        {// ????????????
            KERNEL_NS::LibString cache;
            if(!_configIni->ReadStr(APPLICATION_CONFIG_SEG, PROJECT_ALIAS_NAME_KEY, cache))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("have no project alias name please check!"));
                return Status::ConfigError;
            }
            _appConfig._appAliasName = cache;
        }
        {// ??????????????????
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
        {// ??????id
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
            _appConfig._machineId = static_cast<UInt16>(cache);
        }
        {// ????????????????????????
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
        {// ???????????????????????????
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
        {// ?????????????????????id
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
        {// ??????????????????id???????????????????????????id
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

    // base64(sha256(???????????? + ??????????????? + ?????? + ??????id))
    KERNEL_NS::LibString str;
    str.AppendFormat("%s", _appConfig._registerPath.c_str())
        .AppendFormat("%s", _appConfig._projectMainServiceName.c_str())
        .AppendFormat("%s", KERNEL_NS::StringUtil::Num2Str(_appConfig._registerTime).c_str())
        .AppendFormat("%s", KERNEL_NS::StringUtil::Num2Str(_appConfig._registerProcessId).c_str())
        ;
    str = KERNEL_NS::LibDigest::MakeSha256(str);
    _appConfig._machineApplyId = KERNEL_NS::LibBase64::Encode(str);

    // ??????machine apply id
    g_Log->Info(LOGFMT_OBJ_TAG("%s \nmachine apply id:%s"), IntroduceStr().c_str(), _appConfig._machineApplyId.c_str());
}

void Application::_OnMonitorThreadFrame()
{
    // g_Log->Debug(LOGFMT_OBJ_TAG("\napplication motinotr thread frame app info:%s."), ToString().c_str());

    // 1.?????????????????????????????????
    _lck.Lock();
    auto sw = _statisticsInfo;
    _statisticsInfo = _statisticsInfoCache;
    _statisticsInfoCache = sw;
    _lck.Unlock();

    // 2.??????poller??????
    auto pollerMgr = GetComp<KERNEL_NS::IPollerMgr>();
    KERNEL_NS::LibString info;
    pollerMgr->OnMonitor(info);
    
    Double average = 0;
    if(_statisticsInfoCache->_resCount > 0)
    {
        average = static_cast<Double>(_statisticsInfoCache->_resTotalNs) / _statisticsInfoCache->_resCount / 1000000;
    }

    g_Log->Monitor("%s[- RESPONSE INFO BEGIN -]\nSampleNumber:%llu. Min:%lf(ms). Average:%lf(ms). Max:%lf(ms).\n[- RESPONSE INFO END -]\n"
                , info.c_str(), _statisticsInfoCache->_resCount, static_cast<Double>(_statisticsInfoCache->_minResNs) / 1000000, average, static_cast<Double>(_statisticsInfoCache->_maxResNs) / 1000000);

    _statisticsInfoCache->_minResNs = 0;
    _statisticsInfoCache->_maxResNs = 0;
    _statisticsInfoCache->_resCount = 0;
    _statisticsInfoCache->_resTotalNs = 0;
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
        {// ???????????????????????????
            g_Log->Error(LOGFMT_OBJ_TAG("comp start fail errCode:%d"), GetErrCode());
            break;
        }
    }

    if(!t->IsDestroy())
        MaskReady(true);

    if(LIKELY(GetErrCode() == Status::Success))
    {
        g_Log->Info(LOGFMT_OBJ_TAG("all comps are ready system info:%s."), ToString().c_str());

        while (!t->IsDestroy())
        {
            _OnMonitorThreadFrame();
            KERNEL_NS::SystemUtil::ThreadSleep(1000);
        }
    }
    else
    {
        g_Log->Error(LOGFMT_OBJ_TAG("application %s will close errCode:%d"), ToString().c_str(), GetErrCode());
        SinalFinish(GetErrCode());
    }

    g_Log->Info(LOGFMT_OBJ_TAG("monitor quik thread id:%llu."), KERNEL_NS::SystemUtil::GetCurrentThreadId());
}


SERVICE_COMMON_END
