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
#include <service/TestService/SessionType.h>
#include <protocols/MyTestServiceProtocols/protocols.h>

#include <service/ServiceFactory.h>
#include <service/TestService/MyTestService.h>

SERVICE_BEGIN

// 配置项

static const UInt64 s_maxPieceTimeInMicrosecondsDefault = 8*1000;   // 默认的poller事务超时时间片
static const UInt64 s_maxPollerSleepInMilliSecondsDefault = 20;      // 默认的poller扫描时间间隔(20ms)
static const UInt64 s_defaultFrameUpdateTimeMs = 50;      // 默认帧更新时间间隔(50ms)

MyTestService::MyTestService()
:_timerMgr(NULL)
,_updateTimer(NULL)
,_frameUpdateTimeMs(0)
{

}

MyTestService::~MyTestService()
{
    _OnServiceClear();
}

void MyTestService::Release()
{
    delete this;
    // MyTestService::DeleteByAdapter_MyTestService(ServiceFactory::_buildType.V, this);
}

KERNEL_NS::IProtocolStack *MyTestService::GetProtocolStack(KERNEL_NS::LibSession *session)
{
    auto iter = this->_sessionTypeRefProtocolStack.find(session->GetSessionType());
    return iter == this->_sessionTypeRefProtocolStack.end() ? NULL : iter->second;
}

void MyTestService::_OnServiceClear()
{
    g_Log->Info(LOGFMT_OBJ_TAG("service %s service clear "), GetObjName().c_str());
    _Clear();
}

void MyTestService::_OnServiceRegisterComps()
{
    // TODO:注册组件
    RegisterComp<MyServiceCompFactory>();
}

Int32 MyTestService::_OnServiceInit()
{
    // 1.opcode 初始化
    auto err = Opcodes::Init();
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("opcodes init fail err:%d"), err);
        return err;
    }
    
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
            Int64 cache = 0;
            if(!ini->CheckReadNumber(GetServiceName().c_str(), "MaxPriorityLevel", cache))
            {
                cache = SERVICE_COMMON_NS::MessagePriorityLevel::End;
                if(!ini->WriteNumber(GetServiceName().c_str(), "MaxPriorityLevel", cache))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("write nubmer fail ini:%s, service name:%s, MaxPriorityLevel"), ini->GetPath().c_str(), GetServiceName().c_str());
                    ini->Unlock();
                    return Status::ConfigError;
                }
            }

            _maxPriorityLevel = static_cast<Int32>(cache);
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

    g_Log->Info(LOGFMT_OBJ_TAG("service %s startup with  "), GetObjName().c_str());
    return Status::Success;
}

void MyTestService::_OnServiceWillClose() 
{
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

    g_Log->Info(LOGFMT_OBJ_TAG("service %s closed "), GetObjName().c_str());
}

void MyTestService::_OnSessionCreated(KERNEL_NS::PollerEvent *msg)
{
    // TODO:事件派发
}

void MyTestService::_OnSessionDestroy(KERNEL_NS::PollerEvent *msg)
{

}

void MyTestService::_OnAsynConnectRes(KERNEL_NS::PollerEvent *msg)
{

}

void MyTestService::_OnAddListenRes(KERNEL_NS::PollerEvent *msg)
{

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
    if(_updateTimer)
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(_updateTimer);
    _updateTimer = NULL;

    KERNEL_NS::ContainerUtil::DelContainer<Int32, KERNEL_NS::IProtocolStack *, KERNEL_NS::AutoDelMethods::Release>(_stackTypeRefProtocolStack);
    KERNEL_NS::ContainerUtil::DelContainer<Int32, KERNEL_NS::IDelegate<void, KERNEL_NS::LibPacket *> *, KERNEL_NS::AutoDelMethods::Release>(_opcodeRefHandler);
    
    _sessionTypeRefProtocolStack.clear();
    Opcodes::Destroy();
}

void MyTestService::_OnFrameTimer(KERNEL_NS::LibTimer *timer)
{
    g_Log->Debug(LOGFMT_OBJ_TAG("my test service frame timer."));
    OnUpdate();
}

Int32 MyTestService::_InitProtocolStack()
{
    for(Int32 idx = SERVICE_COMMON_NS::CrystalProtocolStackType::BEGIN; idx < SERVICE_COMMON_NS::CrystalProtocolStackType::END; ++idx)
    {
        auto stack = SERVICE_COMMON_NS::CrystalProtocolStackFactory::Create(idx);
        if(stack)
        {
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
    auto info = Opcodes::GetOpcodeInfo(opcode);
    if(UNLIKELY(!info))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no opcode info opcode:%d"), opcode);
        return false;
    }

    return info->_enable;
}

SERVICE_END
