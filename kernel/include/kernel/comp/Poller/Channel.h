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
// Date: 2025-11-19 00:11:28
// Author: Eric Yonng
// Description:

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_POLLER_CHANNEL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_POLLER_CHANNEL_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>

#include "kernel/comp/LibList.h"
#include "kernel/comp/ConcurrentPriorityQueue/SPSCQueue.h"
#include "kernel/comp/Poller//PollerEvent.h"
#include "kernel/comp/Lock/Impl/ConditionLocker.h"
#include "kernel/comp/Utils/SystemUtil.h"
#include <kernel/comp/Poller/PollerEvent.h>
#include <kernel/comp/Delegate/LibDelegate.h>

KERNEL_BEGIN

struct PollerEvent;
class ConditionLocker;
class Poller;

// Poller释放前要通知Channel移除, 所以要关注创建和移除Channel
class KERNEL_EXPORT Channel
{
    POOL_CREATE_OBJ_DEFAULT(Channel);

public:
    Channel(UInt64 channelId, Poller *target, SPSCQueue<PollerEvent *> *queue);
    ~Channel(){}

    static UInt64 GenChannelId();
    UInt64 GetChannelId() const;

    void Send(PollerEvent *ev);
    void Send(LibList<PollerEvent *> *evs);

    // 执行行为
#ifdef CRYSTAL_NET_CPP20
    template<typename LambdaType>
    requires requires(LambdaType lamb)
    {
        {lamb()} -> std::convertible_to<void>;
    }
    void Send(LambdaType &&lamb)
    {
        auto deleg = KERNEL_CREATE_CLOSURE_DELEGATE(lamb, void);
        auto ev = ActionPollerEvent::New_ActionPollerEvent();
        ev->_action = deleg;
        Send(ev);
    }
#endif

    Poller *GetTarget();
    const Poller *GetTarget() const;

    template<typename ObjType>
    #ifdef CRYSTAL_NET_CPP20
    requires requires(ObjType obj)
    {
        obj.Release();
        obj.ToString();
    }
    #endif
    void Send(Poller *srcPoller, ObjType *o);

    template<typename ResType>
    #ifdef CRYSTAL_NET_CPP20
    requires requires(ResType obj)
    {
        obj.Release();
        obj.ToString();
    }
    #endif
    void SendResponse(UInt64 stub, Poller *srcPoller, ResType *res);
    
private:
    // 以下都是attach模式
    UInt64 _channelId;
    SPSCQueue<PollerEvent *> *_events;
    ConditionLocker *_wakeupTarget;
    Poller *_target;
};

ALWAYS_INLINE UInt64 Channel::GetChannelId() const
{
    return _channelId;
}

ALWAYS_INLINE Poller *Channel::GetTarget()
{
    return _target;
}

ALWAYS_INLINE const Poller *Channel::GetTarget() const
{
    return _target;
}

template<typename ResType>
#ifdef CRYSTAL_NET_CPP20
requires requires(ResType obj)
{
    obj.Release();
    obj.ToString();
}
#endif
ALWAYS_INLINE void Channel::SendResponse(UInt64 stub, Poller *srcPoller, ResType *res)
{
    auto objectEvent = KERNEL_NS::ObjectPollerEvent<ResType>::New_ObjectPollerEvent(stub, true, srcPoller, this);
    objectEvent->_obj = res;
    Send(objectEvent);
}

template<typename ObjType>
#ifdef CRYSTAL_NET_CPP20
requires requires(ObjType obj)
{
    obj.Release();
    obj.ToString();
}
#endif
ALWAYS_INLINE void Channel::Send(Poller *srcPoller, ObjType *o)
{
    // 不需要返回包
    KERNEL_NS::ObjectPollerEvent<ObjType> *objectEvent = KERNEL_NS::ObjectPollerEvent<ObjType>::New_ObjectPollerEvent(0, false, srcPoller, this);
    objectEvent->_obj = o;
    Send(objectEvent);
}

KERNEL_END

#endif

