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
 * Author: Eric Yonng
 * Date: 2021-03-22 15:20:39
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_LIB_CONNECT_INFO_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_LIB_CONNECT_INFO_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>

#include <kernel/comp/LibString.h>
#include <kernel/comp/NetEngine/BriefSockAddr.h>
#include <kernel/comp/NetEngine/Defs/NetDefs.h>
#include <kernel/comp/NetEngine/Poller/impl/Session/SessionOption.h>

KERNEL_BEGIN

class IMsgDispatcher;
class LibTimer;
class IProtocolStack;

// TODO:初始化列表未完善
struct KERNEL_EXPORT LibConnectInfo
{
    POOL_CREATE_OBJ_DEFAULT(LibConnectInfo);

    LibConnectInfo();

    LibString ToString() const;

    LibString _localIp;     // 本地ip: 指定本地ip 内部数据传输尽量和客户端网卡区分,以便于内部大数据快速流转,可以不指定
    UInt16 _localPort;      // 本地端口: 指定本地端口号 可以不指定,不知道则自动分配端口,建议不指定
    LibString _targetIp;    // 目标ip
    UInt16 _targetPort;     // 目标端口
    UInt16 _family;         // 协议族 AF_INET/AF_INET6
    Int32 _protocolType;    // 协议类型（udp/tcp/quic等）ProtocolType
    UInt32 _priorityLevel;  // 优先级级别 
    UInt64 _pollerId;       // pollerid，若指定了pollerid则使用指定的pollerid投递(此时priorityLevel, _protocolType, 可以不填)

    Int32 _retryTimes;      // 超时重试次数
    Int64 _periodMs;        // 定时重连时间 ms
    UInt64 _stub;           // 业务层指定stub,用于透传参数

    UInt64 _fromServiceId;  // 发起连接的serviceid
    IProtocolStack *_stack; // 连接时候必须指定协议栈,否则取的是 _serviceType 指定service默认的协议栈或者根据配置取协议栈

    SessionOption _sessionOption;   // 会话选项
};

struct KERNEL_EXPORT LibConnectPendingInfo
{
    POOL_CREATE_OBJ_DEFAULT(LibConnectPendingInfo);

    LibConnectPendingInfo();

    LibString ToString() const;

    LibConnectInfo *_connectInfo;

    // 由网络层填充 业务层请勿填充
    LibTimer *_reconnectTimer;  // 网络引擎定时器,业务层请勿填充
    Int32 _leftRetryTimes;      // 网络引擎填充 剩余超时重连次数
    SOCKET _newSock;            // 网络引擎填充
    UInt64 _sessionId;          // 网络引擎填充
    BriefSockAddr _localAddr;   // 本地地址
    BriefSockAddr _remoteAddr;  // 远程地址
    LibString _finalLocalIp;    // 最终本地分配的ip
    UInt16 _finalLocalPort;     // 最终本地分配的端口
};

KERNEL_END

#endif
