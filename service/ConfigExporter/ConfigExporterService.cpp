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
 * Date: 2022-06-26 19:11:12
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>

// TODO:业务组件
#include <service/ConfigExporter/Comps/Comps.h>

// #include <protocols/protocols.h>
#include <service/ConfigExporter/ServiceFactory.h>
#include <service/ConfigExporter/ConfigExporterService.h>

SERVICE_BEGIN

// 配置项

static const UInt64 s_maxPieceTimeInMicrosecondsDefault = 8*1000;   // 默认的poller事务超时时间片
static const UInt64 s_maxPollerSleepInMilliSecondsDefault = 20;      // 默认的poller扫描时间间隔(20ms)
static const UInt64 s_defaultFrameUpdateTimeMs = 50;      // 默认帧更新时间间隔(50ms)

POOL_CREATE_OBJ_DEFAULT_IMPL(ConfigExporterService);

ConfigExporterService::ConfigExporterService()
:_timerMgr(NULL)
,_updateTimer(NULL)
,_frameUpdateTimeMs(0)
,_eventMgr(NULL)
,_serviceConfig(NULL)
{
    ILogicSys::GetCurrentService() = this;
}

ConfigExporterService::~ConfigExporterService()
{
    _OnServiceClear();
}

void ConfigExporterService::Release()
{
    ConfigExporterService::DeleteByAdapter_ConfigExporterService(ServiceFactory::_buildType.V, this);
}

KERNEL_NS::IProtocolStack *ConfigExporterService::GetProtocolStack(KERNEL_NS::LibSession *session)
{
    auto iter = this->_sessionTypeRefProtocolStack.find(session->GetSessionType());
    return iter == this->_sessionTypeRefProtocolStack.end() ? NULL : iter->second;
}

KERNEL_NS::IProtocolStack *ConfigExporterService::GetProtocolStack(Int32 SessionType)
{
    auto iter = this->_sessionTypeRefProtocolStack.find(SessionType);
    return iter == this->_sessionTypeRefProtocolStack.end() ? NULL : iter->second;
}

Int32 ConfigExporterService::GetSessionTypeByPort(UInt16 port) const
{
    auto iter = _serviceConfig->_portRefSessionType.find(port);
    return iter == _serviceConfig->_portRefSessionType.end() ? SessionType::UNKNOWN : iter->second;
}

 const ServiceConfig *ConfigExporterService::GetServiceConfig() const
 {
    return _serviceConfig;
 }

void ConfigExporterService::Subscribe(Int32 opcodeId, KERNEL_NS::IDelegate<void, KERNEL_NS::LibPacket *&> *deleg)
{
}

void ConfigExporterService::_OnServiceClear()
{
    g_Log->Info(LOGFMT_OBJ_TAG("service %s service clear "), GetObjName().c_str());
    _Clear();
}

void ConfigExporterService::_OnServiceRegisterComps()
{
    // 会话管理
    RegisterComp<SessionMgrFactory>();
    
     // 系统逻辑管理
    RegisterComp<SysLogicMgrFactory>();
    // 导表
    RegisterComp<XlsxExporterMgrFactory>();
}

Int32 ConfigExporterService::_OnServiceInit()
{
    _eventMgr = KERNEL_NS::EventManager::New_EventManager();
    _serviceConfig = ServiceConfig::New_ServiceConfig();

    // 1.opcode 初始化
    // auto err = Opcodes::Init();
    // if(err != Status::Success)
    // {
    //     g_Log->Error(LOGFMT_OBJ_TAG("opcodes init fail err:%d"), err);
    //     return err;
    // }
    
    {// 2.读取配置
        auto application = _serviceProxy->GetOwner()->CastTo<SERVICE_COMMON_NS::Application>();
        auto ini = application->GetIni();
        ini->Lock();
        {// poller超时时间片
            UInt64 cache = 0;
            if(!ini->CheckReadNumber(GetServiceName().c_str(), "MaxPieceTimeInMicroseconds", cache))
            {
                cache = s_maxPieceTimeInMicrosecondsDefault;
                if(!ini->WriteNumber(GetServiceName().c_str(), "MaxPieceTimeInMicroseconds", cache))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write nubmer fail ini:%s, service name:%s, MaxPieceTimeInMicroseconds"), ini->GetPath().c_str(), GetServiceName().c_str());
                    ini->Unlock();
                    return Status::ConfigError;
                }
            }

            _maxPieceTimeInMicroseconds = cache;
        }

        {// 消息的最大优先级

            _maxPriorityLevel = application->GetKernelConfig()._maxPollerMsgPriorityLevel;
        }

        {// poller最大扫描时间间隔
            UInt64 cache = 0;
            if(!ini->CheckReadNumber(GetServiceName().c_str(), "PollerMaxSleepMilliseconds", cache))
            {
                cache = s_maxPollerSleepInMilliSecondsDefault;
                if(!ini->WriteNumber(GetServiceName().c_str(), "PollerMaxSleepMilliseconds", cache))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write nubmer fail ini:%s, service name:%s, PollerMaxSleepMilliseconds"), ini->GetPath().c_str(), GetServiceName().c_str());
                    ini->Unlock();
                    return Status::ConfigError;
                }
            }

            _maxSleepMilliseconds = cache;
        }

        {// 
            Int64 cache = 0;
            if(!ini->CheckReadNumber(GetServiceName().c_str(), "FrameUpdateTimeMs", cache))
            {
                cache = s_defaultFrameUpdateTimeMs;
                if(!ini->WriteNumber(GetServiceName().c_str(), "FrameUpdateTimeMs", cache))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write nubmer fail ini:%s, service name:%s, FrameUpdateTimeMs"), ini->GetPath().c_str(), GetServiceName().c_str());
                    ini->Unlock();
                    return Status::ConfigError;
                }
            }

            _frameUpdateTimeMs = cache;
        }

        if(!_serviceConfig->Parse(GetServiceName(), ini))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("parse service config fail seg:%s."), GetServiceName().c_str());
            return Status::CfgError;
        }

        ini->Unlock();
    }

    g_Log->Info(LOGFMT_OBJ_TAG("service %s init suc "), GetObjName().c_str());
    return Status::Success;
}

Int32 ConfigExporterService::_OnServiceCompsCreated()
{
    _timerMgr = _poller->GetTimerMgr();
    _updateTimer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer(_timerMgr);
    _updateTimer->SetTimeOutHandler(this, &ConfigExporterService::_OnFrameTimer);

    // 设置ip rule mgr
    auto serviceProxy = GetServiceProxy();
    auto application = serviceProxy->GetOwner()->CastTo<SERVICE_COMMON_NS::Application>();
    auto &config = application->GetKernelConfig();
    auto ipRuleMgr = GetComp<KERNEL_NS::IpRuleMgr>();
    if(!ipRuleMgr->SetBlackWhiteListFlag(config._blackWhiteListMode))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("SetBlackWhiteListFlag fail black white list flag:%u"), config._blackWhiteListMode);
        if(GetOwner())
            GetOwner()->SetErrCode(this, Status::Failed);
        return Status::Failed;
    }

    return Status::Success;
}

Int32 ConfigExporterService::_OnServiceStartup()
{
    CompObject *notReady = NULL;
    for(;!IsAllCompsReady(notReady);)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("%s not ready please check!"), notReady->ToString().c_str());
        if(GetErrCode() != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("error happen errCode:%d"), GetErrCode());
            break;
        }
    }

    auto errCode = GetErrCode();
    if(errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("start up errCode:%d"), errCode);
        return errCode;
    }

    // 启动定时器
    _updateTimer->Schedule(_frameUpdateTimeMs);

    // 创建监听
    // 连接center
    // 连接目标

    g_Log->Info(LOGFMT_OBJ_TAG("service %s startup with  "), GetObjName().c_str());
    return Status::Success;
}

void ConfigExporterService::_OnServiceWillClose() 
{
    if(_updateTimer)
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(_updateTimer);
    _updateTimer = NULL;

    g_Log->Info(LOGFMT_OBJ_TAG("service %s will close "), GetObjName().c_str());
}

void ConfigExporterService::_OnServiceClosed()
{
    CompObject *notDown = NULL;
    for(;!IsAllCompsDown(notDown);)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("%s not down please check!"), notDown->ToString().c_str());
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
    }

    _OnServiceClear();
    g_Log->Info(LOGFMT_OBJ_TAG("service %s closed "), GetObjName().c_str());
}

void ConfigExporterService::_OnSessionCreated(KERNEL_NS::PollerEvent *msg)
{
    auto sessionCreatedEv = msg->CastTo<KERNEL_NS::SessionCreatedEvent>();

    // 预创建
    {
        auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::SESSION_WILL_CREATED);
        ev->SetParam(Params::SESSION_ID, sessionCreatedEv->_sessionId);
        ev->SetParam(Params::LOCAL_ADDR, &sessionCreatedEv->_localAddr);
        ev->SetParam(Params::REMOTE_ADDR, &sessionCreatedEv->_targetAddr);
        ev->SetParam(Params::PROTOCOL_TYPE, sessionCreatedEv->_protocolType);
        ev->SetParam(Params::PRIORITY_LEVEL, sessionCreatedEv->_priorityLevel);
        ev->SetParam(Params::SESSION_TYPE, sessionCreatedEv->_sessionType);
        ev->SetParam(Params::SESSION_POLLER_ID, sessionCreatedEv->_sessionPollerId);
        ev->SetParam(Params::SERVICE_ID, sessionCreatedEv->_belongServiceId);
        ev->SetParam(Params::STUB, sessionCreatedEv->_stub);
        ev->SetParam(Params::IS_FROM_CONNECT, sessionCreatedEv->_isFromConnect);
        ev->SetParam(Params::IS_FROM_LINKER, sessionCreatedEv->_isLinker);
        _eventMgr->FireEvent(ev);
    }

    // 创建完成
    auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::SESSION_CREATED);
    ev->SetParam(Params::SESSION_ID, sessionCreatedEv->_sessionId);
    ev->SetParam(Params::LOCAL_ADDR, &sessionCreatedEv->_localAddr);
    ev->SetParam(Params::REMOTE_ADDR, &sessionCreatedEv->_targetAddr);
    ev->SetParam(Params::PROTOCOL_TYPE, sessionCreatedEv->_protocolType);
    ev->SetParam(Params::PRIORITY_LEVEL, sessionCreatedEv->_priorityLevel);
    ev->SetParam(Params::SESSION_TYPE, sessionCreatedEv->_sessionType);
    ev->SetParam(Params::SESSION_POLLER_ID, sessionCreatedEv->_sessionPollerId);
    ev->SetParam(Params::SERVICE_ID, sessionCreatedEv->_belongServiceId);
    ev->SetParam(Params::STUB, sessionCreatedEv->_stub);
    ev->SetParam(Params::IS_FROM_CONNECT, sessionCreatedEv->_isFromConnect);
    ev->SetParam(Params::IS_FROM_LINKER, sessionCreatedEv->_isLinker);
    _eventMgr->FireEvent(ev);
}

void ConfigExporterService::_OnSessionDestroy(KERNEL_NS::PollerEvent *msg)
{
    KERNEL_NS::SessionDestroyEvent *destroyEv = msg->CastTo<KERNEL_NS::SessionDestroyEvent>();

    // 预创建
    {
        auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::SESSION_WILL_DESTROY);
        ev->SetParam(Params::SESSION_ID, destroyEv->_sessionId);
        ev->SetParam(Params::SESSION_CLOSE_REASON, destroyEv->_closeReason);
        ev->SetParam(Params::SERVICE_ID, destroyEv->_serviceId);
        ev->SetParam(Params::PRIORITY_LEVEL, destroyEv->_priorityLevel);
        ev->SetParam(Params::STUB, destroyEv->_stub);

        _eventMgr->FireEvent(ev);
    }

    // 销毁完成
    auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::SESSION_DESTROY);
    ev->SetParam(Params::SESSION_ID, destroyEv->_sessionId);
    ev->SetParam(Params::SESSION_CLOSE_REASON, destroyEv->_closeReason);
    ev->SetParam(Params::SERVICE_ID, destroyEv->_serviceId);
    ev->SetParam(Params::PRIORITY_LEVEL, destroyEv->_priorityLevel);
    ev->SetParam(Params::STUB, destroyEv->_stub);

    _eventMgr->FireEvent(ev);
}

void ConfigExporterService::_OnAsynConnectRes(KERNEL_NS::PollerEvent *msg)
{
    g_Log->Info(LOGFMT_OBJ_TAG("asyn connect res:%s"), msg->ToString().c_str());

    auto connectRes = msg->CastTo<KERNEL_NS::AsynConnectResEvent>();

    auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::ASYN_CONNECT_RES);
    ev->SetParam(Params::ERROR_CODE, connectRes->_errCode);
    ev->SetParam(Params::LOCAL_ADDR, &connectRes->_localAddr);
    ev->SetParam(Params::REMOTE_ADDR, &connectRes->_targetAddr);
    ev->SetParam(Params::FAMILY, connectRes->_family);
    ev->SetParam(Params::PROTOCOL_TYPE, connectRes->_protocolType);
    ev->SetParam(Params::PRIORITY_LEVEL, connectRes->_priorityLevel);
    ev->SetParam(Params::SESSION_POLLER_ID, connectRes->_sessionPollerId);
    ev->SetParam(Params::SERVICE_ID, connectRes->_fromServiceId);
    ev->SetParam(Params::STUB, connectRes->_stub);
    ev->SetParam(Params::SESSION_ID, connectRes->_sessionId);
    GetEventMgr()->FireEvent(ev);
}

void ConfigExporterService::_OnAddListenRes(KERNEL_NS::PollerEvent *msg)
{
    g_Log->Debug(LOGFMT_OBJ_TAG("add listen res:%s"), msg->ToString().c_str());

    KERNEL_NS::AddListenResEvent *addListenEv = msg->CastTo<KERNEL_NS::AddListenResEvent>();
    
    // 抛事件
    auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::ADD_LISTEN_RES);
    ev->SetParam(Params::ERROR_CODE, addListenEv->_errCode);
    ev->SetParam(Params::LOCAL_ADDR, &addListenEv->_localAddr);
    ev->SetParam(Params::FAMILY, addListenEv->_family);
    ev->SetParam(Params::SERVICE_ID, addListenEv->_serviceId);
    ev->SetParam(Params::STUB, addListenEv->_stub);
    ev->SetParam(Params::PRIORITY_LEVEL, addListenEv->_priorityLevel);
    ev->SetParam(Params::PROTOCOL_TYPE, addListenEv->_protocolType);
    ev->SetParam(Params::SESSION_ID, addListenEv->_sessionId);
    _eventMgr->FireEvent(ev);
}

void ConfigExporterService::_OnRecvMsg(KERNEL_NS::PollerEvent *msg)
{
    auto event = msg->CastTo<KERNEL_NS::RecvMsgEvent>();
    auto packets = event->_packets;
    if(UNLIKELY(!packets))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("have no any packets msg:%s"), msg->ToString().c_str());
        return;
    }

    g_Log->Info(LOGFMT_OBJ_TAG("recieve a net message :%s"), event->ToString().c_str());

    for(auto node = packets->Begin(); node; node = node->_next)
    {
        auto packet = node->_data;
        if(UNLIKELY(!packet))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("packet cant be null session id:%llu, service id:%llu, priority level:%u.")
                        , event->_sessionId, event->_serviceId, event->_priorityLevel);
            continue;
        }

        auto handler = _GetMsgHandler(packet->GetOpcode());
        if(UNLIKELY(!handler))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("a packet with unknown opcode handler packet:%s"), packet->ToString().c_str());
            continue;
        }

        handler->Invoke(packet);
    }

    KERNEL_NS::LibList<KERNEL_NS::LibPacket *>::Delete_LibList(packets);
    event->_packets = NULL;
}

void ConfigExporterService::_OnQuitServiceEvent(KERNEL_NS::PollerEvent *msg)
{
    // 抛事件
    auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::QUIT_SERVICE_EVENT);
    _eventMgr->FireEvent(ev);

    // 退出
    _poller->QuitLoop();
}

bool ConfigExporterService::_OnPollerPrepare(KERNEL_NS::Poller *poller)
{
    g_Log->Info(LOGFMT_OBJ_TAG("service %s poller prepare "), GetObjName().c_str());
    return true;
}

void ConfigExporterService::_OnPollerWillDestroy(KERNEL_NS::Poller *poller) 
{
    g_Log->Info(LOGFMT_OBJ_TAG("service %s poller will destroy "), GetObjName().c_str());
}

void ConfigExporterService::_Clear()
{
    KERNEL_NS::ContainerUtil::DelContainer<Int32, KERNEL_NS::IProtocolStack *, KERNEL_NS::AutoDelMethods::Release>(_stackTypeRefProtocolStack);
    KERNEL_NS::ContainerUtil::DelContainer<Int32, KERNEL_NS::IDelegate<void, KERNEL_NS::LibPacket *> *, KERNEL_NS::AutoDelMethods::Release>(_opcodeRefHandler);
    
    _sessionTypeRefProtocolStack.clear();

    if(LIKELY(_eventMgr))
    {
        KERNEL_NS::EventManager::Delete_EventManager(_eventMgr);
        _eventMgr = NULL;
    }

    if(LIKELY(_serviceConfig))
    {
        ServiceConfig::Delete_ServiceConfig(_serviceConfig);
        _serviceConfig = NULL;
    }
}

void ConfigExporterService::_OnFrameTimer(KERNEL_NS::LibTimer *timer)
{
    g_Log->Debug(LOGFMT_OBJ_TAG("my test service frame timer."));
    OnUpdate();
}

SERVICE_END
