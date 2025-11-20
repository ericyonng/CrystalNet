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
 * Date: 2021-06-18 01:14:57
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_DEFS_POLLER_INNER_EVENT_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_DEFS_POLLER_INNER_EVENT_H__

#pragma once

#include <kernel/comp/NetEngine/Poller/Defs/PollerEventType.h>
#include <kernel/comp/Poller/PollerEvent.h>
#include <kernel/common/LibObject.h>
#include <kernel/comp/LibList.h>

#if CRYSTAL_TARGET_PLATFORM_LINUX
 #include <sys/epoll.h>
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
 #include <kernel/comp/NetEngine/Defs/IoEvent.h>
#endif

KERNEL_BEGIN

struct BuildSessionInfo;
class LibPacket;
struct LibConnectInfo;
struct LibListenInfo;
struct IpControlInfo;

struct KERNEL_EXPORT MonitorPollerEvent : public PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, MonitorPollerEvent);
    
    MonitorPollerEvent();
    ~MonitorPollerEvent();
    virtual void Release() override;
    LibString ToString() const override;

    #if CRYSTAL_TARGET_PLATFORM_LINUX
        union
        {
            Byte8 *_bytes;          // 内存池创建epoll_event 的缓存
            epoll_event *_epEvents;
        }_epEvents;                     // 事件数组
        Int32 _count;                   // 事件个数
    #endif

    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        IoEvent _io;
        Int32 _errCode;
    #endif
};

// 新建会话
struct NewSessionEvent : public PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, NewSessionEvent);

    NewSessionEvent();
    virtual void Release() override;
    LibString ToString() const override;

    BuildSessionInfo *_buildInfo;
};

struct AsynSendEvent : public PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, AsynSendEvent);

    AsynSendEvent();
    ~AsynSendEvent();
    virtual void Release() override;
    LibString ToString() const override;

    UInt64 _sessionId; 
    LibList<LibPacket *> *_packets;
};

struct AsynConnectEvent : public PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, AsynConnectEvent);

    AsynConnectEvent();
    virtual void Release() override;
    LibString ToString() const override;

    LibConnectInfo *_connectInfo;
};


// 关闭:对端关闭保留读消息,本端关闭,保留写消息
struct CloseSessionEvent : public PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, CloseSessionEvent);

    CloseSessionEvent();
    virtual void Release() override;
    LibString ToString() const override;

    UInt64 _sessionId;
    UInt64 _fromServiceId;
    Int64 _closeMillisecondTime;  // 指定关闭时间戳 延迟关闭一般是为了保证底层消息的正常到达远端
    UInt64 _stub;
    bool _forbidRead;
    bool _forbidWrite;
};

// 根据serviceId关闭所有session
struct QuitServiceSessionsEvent : public PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, QuitServiceSessionsEvent);

    QuitServiceSessionsEvent();
    virtual void Release() override;
    LibString ToString() const override;

    UInt64 _fromServiceId;
};

struct QuitSessionInfo
{
    POOL_CREATE_OBJ_DEFAULT(QuitSessionInfo);

    UInt64 _sessionId;
};

struct RealDoQuitServiceSessionEvent : public PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, RealDoQuitServiceSessionEvent);
    RealDoQuitServiceSessionEvent();
    ~RealDoQuitServiceSessionEvent();

    virtual void Release() override;
    LibString ToString() const override;

    UInt64 _fromServiceId;
    LibList<QuitSessionInfo *> *_quitSessionInfo;
};

// 监听
struct AddListenEvent : public PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, AddListenEvent);
    AddListenEvent();

    virtual void Release() override;
    LibString ToString() const override;

    std::vector<LibListenInfo *> _addListenInfoList;
};

// ip规则控制
struct IpRuleControlEvent : public PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, IpRuleControlEvent);

    IpRuleControlEvent();
    ~IpRuleControlEvent();

    static IpRuleControlEvent *Create();
    virtual void Release() override;

    LibString ToString() const override;

    std::list<IpControlInfo *> _ipControlList;
};

KERNEL_END

#endif
