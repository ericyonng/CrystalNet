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
 * 设计需求
 * 网络io消息的竟可能及时性,需要在poller层有每个session的事件数据缓存
 * 事件缓存包括:
 *            1. 连入事件
 *            2. 连接远程成功事件
 *            3. 接收数据事件
 *            4. 发送数据事件
 *            5. 远程断开事件
 *            6. 主动断开事件
 *            7. 
 *          端口配置:端口,主服务id,
 *          服务配置:服务id, 默认协议名
 *          协议配置:名称,类型id
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_DEFS_POLLER_EVENT_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_DEFS_POLLER_EVENT_H__

#pragma once

#include <kernel/comp/LibList.h>
#include <kernel/comp/Poller/PollerEvent.h>
#include <kernel/comp/NetEngine/BriefSockAddr.h>

#include <kernel/comp/NetEngine/Poller/Defs/PollerEventType.h>

#include <vector>

KERNEL_BEGIN

struct LibConnectInfo;
template<typename T>
class LibStream;
class LibPacket;

// poller网络事件
// class KERNEL_EXPORT PollerNetEventType
// {
// public:
//     enum Type
//     {
//         EvNone    = 0,   // 无效
//         EvConnSuc = 1,   // 连接建立成功(包括连入与连接远程).
//         EvClosed  = 2,   // 连接断开(包括远程断开，与本地主动断开,一般指的是远程断开，因为本地的话是不需要处理的).
//         EvRecved  = 3,   // 接收到数据
//         EvSend    = 4,   // 发送完成
//         EvMax,           // 事件最大枚举
//     };
// };


// 监听
struct KERNEL_EXPORT SessionCreatedEvent : public PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, SessionCreatedEvent);
    SessionCreatedEvent();

    virtual void Release() override;
    LibString ToString() const override;

    UInt64 _sessionId;          // 会话id
    BriefSockAddr _localAddr;   // 本地ip地址信息
    BriefSockAddr _targetAddr;  // 远程ip地址信息
    UInt16 _family;             // 协议族 AF_INET/AF_INET6
    Int32 _protocolType;        // 协议类型（udp/tcp/quic等）ProtocolType
    UInt32 _priorityLevel;      // 会话优先级级别 
    Int32 _sessionType;         // 会话类型 
    UInt64 _sessionPollerId;    // 会话最终所在poller
    UInt64 _belongServiceId;    // 会话所属的服务,连接时会指定服务id
    UInt64 _stub;               // 存根 给需要回执的 一般是连接远程时会有stub透传 addListen也有
    bool _isFromConnect;        // 是否主动连接远程而创建的会话
    bool _isLinker;             // 是否监听者
    Int32 _protocolStackType;   // 协议栈类型
};

// 连接回包
struct KERNEL_EXPORT AsynConnectResEvent : public PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, AsynConnectResEvent);

    AsynConnectResEvent();
    virtual void Release() override;

    LibString ToString() const override;

    Int32 _errCode;
    BriefSockAddr _localAddr;
    BriefSockAddr _targetAddr;
    UInt16 _family;             // 协议族 AF_INET/AF_INET6
    Int32 _protocolType;        // 协议类型（udp/tcp/quic等）ProtocolType
    UInt32 _priorityLevel;      // 会话优先级级别 
    UInt64 _sessionPollerId;    // 会话最终所在poller
    UInt64 _fromServiceId;      // 发起连接的serviceid
    UInt64 _stub;               // 回传的存根
    UInt64 _sessionId;          // 会话id
};

// 监听回包
struct KERNEL_EXPORT AddListenResEvent : public PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, AddListenResEvent);

    AddListenResEvent();
    virtual void Release() override;

    LibString ToString() const override;

    Int32 _errCode;
    BriefSockAddr _localAddr;
    UInt16 _family;             // 协议族 AF_INET/AF_INET6
    UInt64 _serviceId;          // 服务id
    UInt64 _stub;               // 存根透传
    UInt32 _priorityLevel;      // 消息优先级 会决定连入的session 的消息优先级
    Int32 _protocolType;       // 协议类型流还是报文等
    UInt64 _sessionId;          // 会话id
};

// 会话销毁
struct KERNEL_EXPORT SessionDestroyEvent : public PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, SessionDestroyEvent);

    SessionDestroyEvent();
    virtual void Release() override;

    LibString ToString() const override;

    Int32 _closeReason;
    UInt64 _sessionId;
    UInt64 _serviceId;
    UInt32 _priorityLevel;
    UInt64 _stub;
};

// 收到网络消息
struct KERNEL_EXPORT RecvMsgEvent : public PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, RecvMsgEvent);

    RecvMsgEvent();
    virtual ~RecvMsgEvent();
    virtual void Release() override;

    LibString ToString() const override;

    UInt64 _sessionId;
    UInt64 _serviceId;
    UInt32 _priorityLevel;
    LibList<LibPacket *> *_packets;
};

// ip控制信息
struct KERNEL_EXPORT IpControlInfo
{
    POOL_CREATE_OBJ_DEFAULT(IpControlInfo);

    static IpControlInfo *Create();
    void Release() override;

    enum CONTROL_TYPE
    {
        ADD_WHITE = 0,  // 设置白名单
        ADD_BLACK = 1,  // 设置黑名单
        ERASE_WHITE = 2,    // 从白名单中移除
        ERASE_BLACK = 3,    // 从黑名单中移除
    };

    LibString ToString() const;

    std::vector<Int32> _controlFlow;    // 控制流
    LibString _ip;   // ip
};

// 退出服务
struct KERNEL_EXPORT QuitServiceEvent : public PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, QuitServiceEvent);
    QuitServiceEvent();
    ~QuitServiceEvent();

    virtual void Release() override;

    LibString ToString() const override;
};

// 退出app
struct KERNEL_EXPORT QuitApplicationEvent : public PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, QuitApplicationEvent);
    QuitApplicationEvent();
    ~QuitApplicationEvent();

    virtual void Release() override;

    LibString ToString() const override;
};

KERNEL_END

#endif
