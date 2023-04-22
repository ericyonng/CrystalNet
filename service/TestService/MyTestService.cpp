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
#include <service/TestService/Comps/Comps.h>

#include <protocols/protocols.h>
#include <service/TestService/ServiceFactory.h>
#include <service/TestService/MyTestService.h>

SERVICE_BEGIN

// 配置项

static const UInt64 s_maxPieceTimeInMicrosecondsDefault = 8*1000;   // 默认的poller事务超时时间片
static const UInt64 s_maxPollerSleepInMilliSecondsDefault = 20;      // 默认的poller扫描时间间隔(20ms)
static const UInt64 s_defaultFrameUpdateTimeMs = 50;      // 默认帧更新时间间隔(50ms)

POOL_CREATE_OBJ_DEFAULT_IMPL(MyTestService);

MyTestService::MyTestService()
:_timerMgr(NULL)
,_updateTimer(NULL)
,_frameUpdateTimeMs(0)
,_eventMgr(NULL)
,_serviceConfig(NULL)
{
    ILogicSys::GetCurrentService() = this;
}

MyTestService::~MyTestService()
{
    _OnServiceClear();
}

void MyTestService::Release()
{
    MyTestService::DeleteByAdapter_MyTestService(ServiceFactory::_buildType.V, this);
}

KERNEL_NS::IProtocolStack *MyTestService::GetProtocolStack(KERNEL_NS::LibSession *session)
{
    auto iter = this->_sessionTypeRefProtocolStack.find(session->GetSessionType());
    return iter == this->_sessionTypeRefProtocolStack.end() ? NULL : iter->second;
}

KERNEL_NS::IProtocolStack *MyTestService::GetProtocolStack(Int32 sessionType)
{
    auto iter = this->_sessionTypeRefProtocolStack.find(sessionType);
    return iter == this->_sessionTypeRefProtocolStack.end() ? NULL : iter->second;
}

Int32 MyTestService::GetSessionTypeByPort(UInt16 port) const
{
    auto iter = _serviceConfig->_portRefSessionType.find(port);
    return iter == _serviceConfig->_portRefSessionType.end() ? SessionType::UNKNOWN : iter->second;
}

 const ServiceConfig *MyTestService::GetServiceConfig() const
 {
    return _serviceConfig;
 }

UInt64 MyTestService::GetSessionAmount() const
{
    auto sessionMgr = GetComp<ISessionMgr>();
    return sessionMgr->GetSessionAmount();
}

void MyTestService::Subscribe(Int32 opcodeId, KERNEL_NS::IDelegate<void, KERNEL_NS::LibPacket *&> *deleg)
{
    auto msgHandler = _GetMsgHandler(opcodeId);
    if(UNLIKELY(msgHandler))
    {
        KERNEL_NS::LibString opcodeInfo;
        _GetOpcodeInfo(opcodeId, opcodeInfo);
        g_Log->Warn(LOGFMT_OBJ_TAG("repeate msg handler opcodeInfo:%s"), opcodeInfo.c_str());
        
        msgHandler->Release();
        _opcodeRefHandler.erase(opcodeId);
    }

    if(UNLIKELY(!_CheckOpcodeEnable(opcodeId)))
    {
        KERNEL_NS::LibString opcodeInfo;
        _GetOpcodeInfo(opcodeId, opcodeInfo);

        g_Log->Warn(LOGFMT_OBJ_TAG("subscribe a disable opcode opcode info:%s"), opcodeInfo.c_str());
        return;
    }

    _opcodeRefHandler.insert(std::make_pair(opcodeId, deleg));
}

void MyTestService::_OnServiceClear()
{
    g_Log->Info(LOGFMT_OBJ_TAG("service %s service clear "), GetObjName().c_str());
    _Clear();
}

void MyTestService::_OnServiceRegisterComps()
{
    // 会话管理
    RegisterComp<SessionMgrFactory>();
    
     // 系统逻辑管理
    RegisterComp<SysLogicMgrFactory>();
    // 存根系统
    RegisterComp<StubHandleMgrFactory>();

    // 测试组件
    RegisterComp<MyServiceCompFactory>();

    // 测试
    RegisterComp<TestMgrFactory>();
}

Int32 MyTestService::_OnServiceInit()
{
    _eventMgr = KERNEL_NS::EventManager::New_EventManager();
    _serviceConfig = ServiceConfig::New_ServiceConfig();

    Int32 err = Status::Success;
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

    // 3.协议栈初始化
    err = _InitProtocolStack();
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("init protocol stack fail err:%d"), err);
        return err;
    }

    g_Log->Info(LOGFMT_OBJ_TAG("service %s init suc "), GetObjName().c_str());
    return Status::Success;
}

Int32 MyTestService::_OnServiceCompsCreated()
{
    _timerMgr = _poller->GetTimerMgr();
    _updateTimer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer(_timerMgr);
    _updateTimer->SetTimeOutHandler(this, &MyTestService::_OnFrameTimer);

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

Int32 MyTestService::_OnServiceStartup()
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

void MyTestService::_OnServiceWillClose() 
{
    if(_updateTimer)
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(_updateTimer);
    _updateTimer = NULL;

    g_Log->Info(LOGFMT_OBJ_TAG("service %s will close "), GetObjName().c_str());
}

void MyTestService::_OnServiceClosed()
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

void MyTestService::_OnSessionCreated(KERNEL_NS::PollerEvent *msg)
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

void MyTestService::_OnSessionDestroy(KERNEL_NS::PollerEvent *msg)
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

void MyTestService::_OnAsynConnectRes(KERNEL_NS::PollerEvent *msg)
{
    // g_Log->Info(LOGFMT_OBJ_TAG("asyn connect res:%s"), msg->ToString().c_str());

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

void MyTestService::_OnAddListenRes(KERNEL_NS::PollerEvent *msg)
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

void MyTestService::_OnRecvMsg(KERNEL_NS::PollerEvent *msg)
{
    auto event = msg->CastTo<KERNEL_NS::RecvMsgEvent>();
    auto packets = event->_packets;
    if(UNLIKELY(!packets))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("have no any packets msg:%s"), msg->ToString().c_str());
        return;
    }

    // g_Log->Info(LOGFMT_OBJ_TAG("recieve a net message :%s"), event->ToString().c_str());

    for(auto node = packets->Begin(); node;)
    {
        auto packet = node->_data;
        node = packets->Erase(node);

        if(UNLIKELY(!packet))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("packet cant be null session id:%llu, service id:%llu, priority level:%u.")
                        , event->_sessionId, event->_serviceId, event->_priorityLevel);
            continue;
        }

        const auto opcode = packet->GetOpcode();
        auto handler = _GetMsgHandler(opcode);
        if(UNLIKELY(!handler))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("a packet with unknown opcode handler packet:%s"), packet->ToString().c_str());
            packet->ReleaseUsingPool();
            continue;
        }

        const auto sessionId = packet->GetSessionId();
        const auto packetId = packet->GetPacketId();
        const auto getContent = [sessionId, packetId, opcode](){
            const auto opcodeInfo = Opcodes::GetOpcodeInfo(opcode);
            return KERNEL_NS::LibString().AppendFormat(PR_FMT("sessionId:%llu, packetid:%lld, opcode:%d,[%s],  ")
                , sessionId, packetId, opcode, opcodeInfo ? opcodeInfo->_opcodeName.c_str() : "Unknown Opcode.");
        };
        PERFORMANCE_RECORD_DEF(pr, getContent, 5);

        handler->Invoke(packet);
        if(LIKELY(packet))
            packet->ReleaseUsingPool();

        // 消费消息数量统计
        AddConsumePackets(1);
    }

    KERNEL_NS::LibList<KERNEL_NS::LibPacket *>::Delete_LibList(packets);
    event->_packets = NULL;
}

void MyTestService::_OnQuitingService(KERNEL_NS::PollerEvent *msg)
{
    // 抛事件
    auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::QUIT_SERVICE_EVENT);
    _eventMgr->FireEvent(ev);
}

bool MyTestService::_OnPollerPrepare(KERNEL_NS::Poller *poller)
{
    g_Log->Info(LOGFMT_OBJ_TAG("service %s poller prepare "), GetObjName().c_str());
    return true;
}

void MyTestService::_OnPollerWillDestroy(KERNEL_NS::Poller *poller) 
{
    g_Log->Info(LOGFMT_OBJ_TAG("service %s poller will destroy "), GetObjName().c_str());
}

void MyTestService::_Clear()
{
    KERNEL_NS::ContainerUtil::DelContainer<Int32, KERNEL_NS::IProtocolStack *, KERNEL_NS::AutoDelMethods::Release>(_stackTypeRefProtocolStack);
    KERNEL_NS::ContainerUtil::DelContainer<Int32, KERNEL_NS::IDelegate<void, KERNEL_NS::LibPacket *&> *, KERNEL_NS::AutoDelMethods::Release>(_opcodeRefHandler);
    
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

void MyTestService::_OnFrameTimer(KERNEL_NS::LibTimer *timer)
{
    // g_Log->Debug(LOGFMT_OBJ_TAG("my test service frame timer."));
    OnUpdate();
}

Int32 MyTestService::_InitProtocolStack()
{
    const auto limit = GetApp()->GetKernelConfig()._sessionRecvPacketContentLimit;
    for(Int32 idx = SERVICE_COMMON_NS::CrystalProtocolStackType::BEGIN; idx < SERVICE_COMMON_NS::CrystalProtocolStackType::END; ++idx)
    {
        auto stack = SERVICE_COMMON_NS::CrystalProtocolStackFactory::Create(idx, limit);
        if(stack)
        {
            // opcode解析
            auto delg = KERNEL_NS::DelegateFactory::Create(&Opcodes::ToString);
            stack->SetOpcodeNameParser(delg);

            _stackTypeRefProtocolStack.insert(std::make_pair(idx, stack));

            auto sessionType = SessionType::TurnFromProtocolStackType(idx);
            if(UNLIKELY(sessionType == SessionType::UNKNOWN))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("protocol stack type to session type map fail stack type:%d"), idx);
                return Status::Failed;
            }
            
            auto iterSessionType = _sessionTypeRefProtocolStack.find(sessionType);
            if(iterSessionType != _sessionTypeRefProtocolStack.end())
            {
                g_Log->Error(LOGFMT_OBJ_TAG("repeate protocol stack to session type map, old stack type:%d, new stack type:%d, sessionType:%d")
                                , iterSessionType->first,  idx, sessionType);
                return Status::Failed;
            }

            _sessionTypeRefProtocolStack.insert(std::make_pair(sessionType, stack));
        }
    }

    return Status::Success;
}

bool MyTestService::_CheckOpcode(Int32 opcode, KERNEL_NS::LibString &errInfo)
{
    if(UNLIKELY(!Opcodes::CheckOpcode(opcode)))
    {
        errInfo.AppendFormat("unknown opcode:%d", opcode);
        return false;
    }

    return true;
}

void MyTestService::_GetOpcodeInfo(Int32 opcode, KERNEL_NS::LibString &opcodeInfo)
{
    auto info = Opcodes::GetOpcodeInfo(opcode);
    if(UNLIKELY(!info))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no opcode info opcode:%d"), opcode);
        return;
    }

    opcodeInfo = info->ToString();
}

bool MyTestService::_CheckOpcodeEnable(Int32 opcode)
{
    return Opcodes::CheckOpcode(opcode);
}

SERVICE_END
