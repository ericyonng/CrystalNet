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
#include <kernel/comp/Poller/PollerEventInternalType.h>

KERNEL_BEGIN

template <typename Rtn, typename... Args>
class IDelegate;

struct AsyncTask;

class Poller;

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

    ActionPollerEvent();
    ~ActionPollerEvent();

    virtual void Release() override;
    virtual LibString ToString() const override;


    IDelegate<void> *_action;
};

struct KERNEL_EXPORT EmptyPollerEvent : public PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, EmptyPollerEvent);

    EmptyPollerEvent();
    ~EmptyPollerEvent()
    {
    }

    virtual void Release() override
    {
        EmptyPollerEvent::Delete_EmptyPollerEvent(this);
    }
};

// 异步任务
struct KERNEL_EXPORT AsyncTaskPollerEvent : public PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, AsyncTaskPollerEvent);

    AsyncTaskPollerEvent();
    ~AsyncTaskPollerEvent();

    virtual void Release() override;

    AsyncTask *_asyncTask;
};

struct KERNEL_EXPORT StubPollerEvent : public PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, StubPollerEvent);

    StubPollerEvent(Int32 type, UInt64 stub, UInt64 objTypeId, Poller *poller);
    ~StubPollerEvent();

    virtual LibString ToString() const override;

    UInt64 _stub;
    bool _isResponse;
    UInt64 _objTypeId;
    Poller *_srcPoller;
};

// obj对象得有Release实现
template<typename ObjType>
requires requires(ObjType obj)
{
    obj.Release();
}
struct KERNEL_EXPORT ObjectPollerEvent : public StubPollerEvent
{
    POOL_CREATE_TEMPLATE_OBJ_DEFAULT_P1(StubPollerEvent, ObjectPollerEvent, ObjType)

    ObjectPollerEvent(UInt64 stub, bool isResponse, Poller *poller)
        :StubPollerEvent(PollerEventInternalType::ObjectPollerEventType, stub, KERNEL_NS::RttiUtil::GetTypeId<ObjType>(), poller)
    ,_obj(NULL)
    {
        _isResponse = isResponse;
    }

    ~ObjectPollerEvent()
    {
        CRYSTAL_RELEASE_SAFE(_obj);
    }

    void Release() override
    {
        ObjectPollerEvent<ObjType>::Delete_ObjectPollerEvent(this);
    }

    LibString ToString() const override
    {
        LibString info;
        info.AppendFormat("ObjType:%s, %s, %s\n", KERNEL_NS::RttiUtil::GetByType<ObjType>().c_str(), (_obj ? _obj->ToString().c_str() : "")
            , PollerEvent::ToString().c_str(), _stub, (_isResponse ? 1:0), _objTypeId);

        return info;
    }

    // 会在ObjectPollerEvent中释放
    ObjType *_obj;
};

template<typename ObjType>
requires requires(ObjType obj)
{
    obj.Release();
}
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_IMPL(ObjectPollerEvent, ObjType);

KERNEL_END

#endif
