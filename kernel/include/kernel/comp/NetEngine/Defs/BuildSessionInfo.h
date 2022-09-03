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
 * Date: 2021-03-24 18:12:54
 * Description: 
 *              1.从监听器->io传输器->dispatcher->logic都要attach其引用,便于收发消息等
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_BUILD_SESSION_INFO_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_BUILD_SESSION_INFO_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/NetEngine/BriefSockAddr.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/NetEngine/Poller/impl/Session/SessionOption.h>

KERNEL_BEGIN

class IProtocolStack;

struct KERNEL_EXPORT BuildSessionInfo
{
    POOL_CREATE_OBJ_DEFAULT(BuildSessionInfo);

    BuildSessionInfo(bool isIpv4);

    LibString ToString() const;

    Int32 _protocolType;        // udp/tcp/quic等 ProtocolType
    UInt16 _family;             // 协议族
    UInt64 _sessionId;          // 会话id
    BriefSockAddr _localAddr;   // 本地地址
    BriefSockAddr _remoteAddr;  // 远程地址
    SOCKET _sock;               // 套接字
    UInt64 _serviceId;          // 监听者可以不必绑定serviceid,连入时只需要广播即可,connect 可能需要,因为连接成功后可能会需要回调 为0表示需要广播
    UInt64 _stub;               // 对于连接远程来说可能需要stub
    UInt32 _priorityLevel;      // 优先级
    bool _isFromConnect;        // connect需要connectres
    IProtocolStack *_protocolStack; // 协议栈
    bool _isLinker;                 // 是否监听者
    bool _isWinSockNonBlock;        // windows下指定是否非阻塞

    SessionOption _sessionOption;
};

KERNEL_END

#endif

