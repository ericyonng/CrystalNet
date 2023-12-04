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
 * Date: 2022-03-23 11:59:46
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_POLLER_POLLER_EVENT_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_POLLER_POLLER_EVENT_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

template <typename Rtn, typename... Args>
class IDelegate;

struct KERNEL_EXPORT PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT(PollerEvent);
    PollerEvent(Int32 type);
    virtual ~PollerEvent();

    virtual void Release() = 0;
    template<typename TargetType>
    TargetType *CastTo()
    {
        return reinterpret_cast<TargetType *>(this);
    }
    template<typename TargetType>
    const TargetType *CastTo() const
    {
        return reinterpret_cast<const TargetType *>(this);
    }

    virtual LibString ToString() const;

    Int32 _type;
};

// TODO:添加poller闭包的支持，可以使得外部在poller所在线程执行一些事情而不必另外添加事件支持

struct KERNEL_EXPORT ActionPollerEvent : public PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, ActionPollerEvent);

    ActionPollerEvent(Int32 type)
    :PollerEvent(type)
    ,_action(NULL)
    {

    }

    ~ActionPollerEvent();

    virtual void Release() override;
    virtual LibString ToString() const override;


    IDelegate<void> *_action;
};

struct KERNEL_EXPORT EmptyPollerEvent : public PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, EmptyPollerEvent);

    EmptyPollerEvent(Int32 type)
    :PollerEvent(type)
    {

    }

    ~EmptyPollerEvent()
    {
    }

    virtual void Release() override
    {
        EmptyPollerEvent::Delete_EmptyPollerEvent(this);
    }
};

KERNEL_END

#endif
