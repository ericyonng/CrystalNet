// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2026-09-24 22:09:54
// Author: Eric Yonng
// Description:

#include <pch.h>
#include <kernel/comp/NetEngine/Poller/impl/Session/ISessionBridge.h>
#include <kernel/comp/Poller/Poller.h>

#include "kernel/comp/Lock/Impl/CoLocker.h"
#include "kernel/comp/NetEngine/Poller/Defs/PollerEvent.h"
#include "kernel/comp/NetEngine/Poller/impl/Session/SessionOption.h"
#include "kernel/comp/Utils/SocketUtil.h"
#include <kernel/comp/Config/Config.h>

#include "kernel/comp/NetEngine/Defs/LibListenInfo.h"
#include "kernel/comp/NetEngine/Defs/ProtocolType.h"
#include "kernel/comp/NetEngine/Poller/impl/Tcp/TcpPollerMgr.h"
#include "kernel/comp/Utils/ContainerUtil.h"
#include "kernel/comp/Utils/StringUtil.h"

KERNEL_BEGIN
    ISessionBridge::ISessionBridge(UInt64 objTypeId)
:CompObject(objTypeId)
    ,_bridgePoller(NULL)
    ,_netConfig(NULL)
{
        
}

ISessionBridge::~ISessionBridge()
{
    if(_netConfig)
    {
        NetConfig::DeleteThreadLocal_NetConfig(_netConfig);
    }
    ContainerUtil::DelContainer2(_pollerEventHandler);
}

void ISessionBridge::PostMsg(PollerEvent *msg, Int64 packetsCount)
{
    _bridgePoller->Push(msg);
}

void ISessionBridge::DoEvents(LibList<PollerEvent *> *&events)
{
    if(UNLIKELY(!events))
        return;

    for(auto iter = events->Begin(); iter; )
    {
        auto data = iter->_data;
        
        auto iterHandler = _pollerEventHandler.find(data->_type);
        if(LIKELY(iterHandler != _pollerEventHandler.end()))
        {
            auto handler = iterHandler->second;
            handler->Invoke(data);
        }
        else
        {
            CLOG_ERROR("event not Subscribe event:%s", data->ToString().c_str());
        }

        if(LIKELY(data))
            data->Release();
        
        iter = events->Erase(iter);
    }
}

void ISessionBridge::SetNetConfig(const KERNEL_NS::NetConfig &config)
{
    if(UNLIKELY(_netConfig))
    {
        CLOG_ERROR("net config releated");
        return;
    }

    _netConfig = NetConfig::NewThreadLocal_NetConfig(config);
}


CoTask<KERNEL_NS::AsyncMultiResult> ISessionBridge::AddTcpListen(TcpPollerMgr *tcpMgr, Int32 protocolStackType, UInt16 port, const LibString addr
    , Int32 sessionCount, const KERNEL_NS::PacketOptions packetOptions)
{
    KERNEL_NS::AsyncMultiResult finalRes;

    // 1.校验ip
    if(!addr.empty())
    {
        if(!KERNEL_NS::SocketUtil::IsIp(addr))
        {
            CLOG_ERROR("illegal ip:%s", addr.c_str());
            finalRes._errCode = Status::ParamError;
            co_return finalRes;
        }
    }

    if(UNLIKELY(!tcpMgr))
    {
        CLOG_ERROR("TCP poller mgr null");
        finalRes._errCode = Status::ParamError;

        co_return finalRes;
    }
    
    KERNEL_NS::SessionOption option;

    // 发送和接收缓冲区使用默认值
    option._noDelay = true;
    option._sockSendBufferSize = 0;
    option._sockRecvBufferSize = 0;
    option._sessionBufferCapicity = _netConfig->SessionBufferCapacity;

    if(packetOptions.PacketSpeedLimitSwitch)
    {
        option._sessionRecvPacketSpeedLimit =       _netConfig->SessionRecvPacketSpeedLimit;
        option._sessionRecvPacketSpeedTimeUnitMs =  _netConfig->SessionRecvPacketSpeedTimeUnitMs;
    }
    else
    {
        option._sessionRecvPacketSpeedLimit = 0;
        option._sessionRecvPacketSpeedTimeUnitMs = 0;
    }

    option._sessionRecvPacketStackLimit = _netConfig->SessionRecvPacketStackLimit;

    option._forbidRecv = true;
    option._protocolStackType = protocolStackType;

    // 消息接收限制
    if(packetOptions.PacketRecvBytesLimitSwitch)
    {
        option._sessionRecvPacketContentLimit = _netConfig->SessionRecvPacketContentLimit;
    }
    else
    {
        option._sessionRecvPacketContentLimit = 0;
    }

    // 消息发送限制
    if(packetOptions.PacketSendBytesLimitSwitch)
    {
        option._sessionSendPacketContentLimit = _netConfig->SessionSendPacketContentLimit;
    }
    else
    {
        option._sessionSendPacketContentLimit = 0;
    }

    KERNEL_NS::SmartPtr<KERNEL_NS::AsyncResult, KERNEL_NS::AutoDelMethods::Release> res = KERNEL_NS::AsyncResult::NewThreadLocal_AsyncResult();
    auto listenInfo = KERNEL_NS::LibListenInfo::New_LibListenInfo();
    auto family = KERNEL_NS::SocketUtil::IsIpv4(addr) ? AF_INET : AF_INET6;
    listenInfo->_ip = addr;
    listenInfo->_port = port;
    listenInfo->_family = family;
    listenInfo->_protocolType = KERNEL_NS::ProtocolType::TCP;

    // windows下同一个端口只能一个session
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    if(listenInfo->_port != 0)
        sessionCount = 1;
#endif
    
    listenInfo->_sessionCount = sessionCount;
    listenInfo->_sessionOption = option;
    listenInfo->_bridge = this;

    KERNEL_NS::SmartPtr<CoLocker, KERNEL_NS::AutoDelMethods::Release> lck = KERNEL_NS::CoLocker::NewThreadLocal_CoLocker();
    listenInfo->_lck = lck.AsSelf();
    listenInfo->_asyncResult = res.AsSelf();

    CLOG_INFO("will add listen info:%s", listenInfo->ToString().c_str());

    if(!tcpMgr->PostAddlisten(listenInfo))
    {
        CLOG_ERROR("tcp mgr post add listen fail, [%s:%hu]", addr.c_str(), port);
        finalRes._errCode = Status::ParamError;

        co_return finalRes;
    }

    // 所有session count 监听完成才返回
    KERNEL_NS::SmartPtr<Int32, KERNEL_NS::AutoDelMethods::CustomDelete> countRes = KERNEL_NS::KernelCastTo<Int32>(KERNEL_ALLOC_MEMORY_TL(sizeof(Int32)));
    countRes.SetClosureDelegate([](void *arg)
    {
        KERNEL_FREE_MEMORY_TL(arg);
    });

    do
    {
        // 在ListenInfo回调时唤醒
        co_await lck->Wait();

        if(res->_errCode != Status::Success)
        {
            finalRes._errCode = res->_errCode;
        }
        else
        {
            finalRes._sessionIds.push_back(res->_sessionId);
        }

        ++(*countRes);
        if(*countRes >= sessionCount)
        {
            CLOG_INFO("add listen all session count [%s:%hu], session count:%d", addr.c_str(), port, sessionCount);
            break;
        }

        CLOG_INFO("add listen count:%d, final count:%d [%s:%hu]", *countRes, sessionCount, addr.c_str(), port);
    }
    while (true);

    // listenInfo 已销毁
    CLOG_INFO("add listen all success, res:[%s:%hu], family:%d, ErrCode:%d, sessionIds:%s"
        , addr.c_str(), port, family, finalRes._errCode, KERNEL_NS::StringUtil::ToString(finalRes._sessionIds, ',').c_str());
    
    co_return finalRes;
}

Int32 ISessionBridge::_OnInit()
{
    if(UNLIKELY(!_bridgePoller))
    {
        CLOG_ERROR("have no _bridgePoller");
        return Status::Failed;
    }

    if(UNLIKELY(!_netConfig))
    {
        CLOG_ERROR("net config not set");
        return Status::ConfigError;
    }

    Subscribe(KERNEL_NS::PollerEventType::SessionCreated, this, &ISessionBridge::_OnSessionCreated);
    Subscribe(KERNEL_NS::PollerEventType::AsynConnectRes, this, &ISessionBridge::_OnAsynConnectRes);
    Subscribe(KERNEL_NS::PollerEventType::AddListenRes, this, &ISessionBridge::_OnAddListenRes);
    Subscribe(KERNEL_NS::PollerEventType::SessionDestroy, this, &ISessionBridge::_OnSessionDestroy);
    Subscribe(KERNEL_NS::PollerEventType::RecvMsg, this, &ISessionBridge::_OnRecvMsg);

    return Status::Success;
}

void ISessionBridge::_OnSessionCreated(PollerEvent *&ev)
{
    auto *created = static_cast<SessionCreatedEvent *>(ev);
    if (created->_isLinker || created->_isFromConnect)
        return;

    CLOG_DEBUG("session created ev:%s", created->ToString().c_str());
    
    _sessionIdRefPollerId[created->_sessionId] = created->_sessionPollerId;
}

void ISessionBridge::_OnAsynConnectRes(PollerEvent *&ev)
{
    CLOG_DEBUG("session connected ev:%s", ev->ToString().c_str());
}

void ISessionBridge::_OnAddListenRes(PollerEvent *&ev)
{
    KERNEL_NS::AddListenResEvent *addListenEv = ev->CastTo<KERNEL_NS::AddListenResEvent>();

    // 异步返回包设置
    addListenEv->_asyncResult->_sessionId = addListenEv->_sessionId;
    addListenEv->_asyncResult->_errCode = addListenEv->_errCode;
    auto lck = addListenEv->_lck;

    _OnBeforeAddListen(ev);

    // 唤醒处理异步回调
    lck->Broadcast();
    
    CLOG_DEBUG("session add listen res ev:%s", ev->ToString().c_str());
}

void ISessionBridge::_OnSessionDestroy(PollerEvent *&ev)
{
    CLOG_DEBUG("session destroy ev:%s", ev->ToString().c_str());
}

void ISessionBridge::_OnRecvMsg(PollerEvent *&ev)
{
    CLOG_DEBUG("session recv msg:%s", ev->ToString().c_str());
}

Int32 ISessionBridge::_OnStart()
{
    return Status::Success;
}

void ISessionBridge::Subscribe(Int32 eventType, KERNEL_NS::IDelegate<void, KERNEL_NS::PollerEvent *&> *deleg)
{
    auto iter = _pollerEventHandler.find(eventType);
    if(iter != _pollerEventHandler.end())
    {
        auto cb = iter->second;
        if (g_Log && g_Log->IsEnable(LogLevel::Warn))
            g_Log->Warn(LOGFMT_OBJ_TAG("repeat eventType:%d callback, old callback owner:%s, callback:%s , and will replace with new one: owner:%s, callback:%s")
        ,eventType, cb->GetOwnerRtti().c_str(), cb->GetCallbackRtti().c_str(), deleg->GetOwnerRtti().c_str(), deleg->GetCallbackRtti().c_str());
        cb->Release();

        iter->second = deleg;
        return;
    }

    _pollerEventHandler.insert(std::make_pair(eventType, deleg));
}



KERNEL_END
