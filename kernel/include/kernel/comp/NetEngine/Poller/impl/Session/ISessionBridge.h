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
// Date: 2026-09-24 22:09:26
// Author: Eric Yonng
// Description:

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_SESSION_BRIDGE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_SESSION_BRIDGE_H__

#pragma once

#include <kernel/comp/CompObject/CompObject.h>
#include <kernel/comp/Delegate/LibDelegate.h>

KERNEL_BEGIN

template<typename ObjType, typename BuildType = _Build::MT>
class LibList;

class LibSession;
class IProtocolStack;
class Poller;
struct PollerEvent;

// session 桥接器, 如果是Listener 那么给Accept产生的SessionBridge, 如果是Connect创建的，由发起Connect方提供
class KERNEL_EXPORT ISessionBridge : public CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, ISessionBridge);

public:
    ISessionBridge(UInt64 objTypeId)
    :CompObject(objTypeId)
        ,_bridgePoller(NULL)
    {
        
    }
    
    ~ISessionBridge() override;

    // 发消息
    virtual void PostMsg(PollerEvent *msg, Int64 packetsCount = 0);
    // 获取协议栈
    virtual IProtocolStack *GetProtocolStack(LibSession *session) = 0;

    // ISessionBridge所在的线程Poller才可调用
    void DoEvents(LibList<PollerEvent *> *&events);

    // 订阅消息处理
    template<typename ObjType>
    void Subscribe(Int32 eventType, ObjType *obj, void (ObjType::*handler)(KERNEL_NS::PollerEvent *&));
    void Subscribe(Int32 eventType, void (*handler)(KERNEL_NS::PollerEvent *&));
    void Subscribe(Int32 eventType, KERNEL_NS::IDelegate<void, KERNEL_NS::PollerEvent *&> *deleg);
    void UnSubscribe(Int32 eventType);

    void SetPoller(Poller *poller);
    
protected:
    Int32 _OnInit() override;
    Int32 _OnStart() override;

    // 派生类重写接口来定制PollerEvent回调, 纯虚函数以便提醒使用者实现该接口
    virtual void _OnSessionCreated(PollerEvent *&ev) = 0;
    virtual void _OnAsynConnectRes(PollerEvent *&ev) = 0;
    virtual void _OnAddListenRes(PollerEvent *&ev) = 0;
    virtual void _OnSessionDestroy(PollerEvent *&ev) = 0;
    virtual void _OnRecvMsg(PollerEvent *&ev) = 0;

private:
    Poller *_bridgePoller;
    std::unordered_map<Int32, KERNEL_NS::IDelegate<void, KERNEL_NS::PollerEvent *&> *> _pollerEventHandler;
};

template<typename ObjType>
ALWAYS_INLINE void ISessionBridge::Subscribe(Int32 eventType, ObjType *obj, void (ObjType::*handler)(KERNEL_NS::PollerEvent *&))
{
    auto delg = DelegateFactory::Create(obj, handler);
    Subscribe(eventType, delg);
}

ALWAYS_INLINE void ISessionBridge::Subscribe(Int32 eventType, void (*handler)(KERNEL_NS::PollerEvent *&))
{
    auto delg = DelegateFactory::Create(handler);
    Subscribe(eventType, delg);
}

ALWAYS_INLINE void ISessionBridge::UnSubscribe(Int32 eventType)
{
    auto iter = _pollerEventHandler.find(eventType);
    if(UNLIKELY(iter == _pollerEventHandler.end()))
        return;

    iter->second->Release();
    _pollerEventHandler.erase(iter);
}

ALWAYS_INLINE void ISessionBridge::SetPoller(Poller *poller)
{
    _bridgePoller = poller;
}


KERNEL_END

#endif
