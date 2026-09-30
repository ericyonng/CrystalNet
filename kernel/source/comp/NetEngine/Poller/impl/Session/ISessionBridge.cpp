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

#include "kernel/comp/NetEngine/Poller/Defs/PollerEvent.h"
#include <kernel/comp/Log/Log.h>

#include "kernel/comp/Utils/ContainerUtil.h"

KERNEL_BEGIN
    
ISessionBridge::~ISessionBridge()
{
    KERNEL_NS::ContainerUtil::DelContainer2(_pollerEventHandler);
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


Int32 ISessionBridge::_OnInit()
{
    if(UNLIKELY(!_bridgePoller))
    {
        CLOG_INFO("have no _bridgePoller");
        return Status::Failed;
    }

    Subscribe(KERNEL_NS::PollerEventType::SessionCreated, this, &ISessionBridge::_OnSessionCreated);
    Subscribe(KERNEL_NS::PollerEventType::AsynConnectRes, this, &ISessionBridge::_OnAsynConnectRes);
    Subscribe(KERNEL_NS::PollerEventType::AddListenRes, this, &ISessionBridge::_OnAddListenRes);
    Subscribe(KERNEL_NS::PollerEventType::SessionDestroy, this, &ISessionBridge::_OnSessionDestroy);
    Subscribe(KERNEL_NS::PollerEventType::RecvMsg, this, &ISessionBridge::_OnRecvMsg);

    return Status::Success;
}
//
// void ISessionBridge::_OnSessionCreated(PollerEvent *&ev)
// {
//     auto *created = static_cast<SessionCreatedEvent *>(ev);
//     if (created->_isLinker || created->_isFromConnect)
//         return;
//
//     CLOG_DEBUG("session created ev:%s", created->ToString().c_str());
//     
//     _sessionIdRefPollerId[created->_sessionId] = created->_sessionPollerId;
// }
//
// void ISessionBridge::_OnAsynConnectRes(PollerEvent *&ev)
// {
//     
// }
//
// void ISessionBridge::_OnAddListenRes(PollerEvent *&ev)
// {
//     
// }
//
// void ISessionBridge::_OnSessionDestroy(PollerEvent *&ev)
// {
//     
// }
//
// void ISessionBridge::_OnRecvMsg(PollerEvent *&ev);
// {
//     
// }

Int32 ISessionBridge::_OnStart()
{
    CLOG_INFO("session bridge start");
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
