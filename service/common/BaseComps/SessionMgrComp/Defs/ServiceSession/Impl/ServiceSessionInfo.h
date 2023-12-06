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
 * Date: 2022-09-18 20:25:18
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service/common/macro.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/NetEngine/BriefSockAddr.h>

KERNEL_BEGIN

class IProtocolStack;

KERNEL_END


SERVICE_BEGIN

struct ServiceSessionInfo
{
    POOL_CREATE_OBJ_DEFAULT(ServiceSessionInfo);

    ServiceSessionInfo()
    :_serviceId(0)
    ,_sessionId(0)
    ,_pollerId(0)
    ,_priorityLevel(0)
    ,_sessionType(0)
    ,_sessionSendBytesLimit(0)
    ,_localAddr(true)
    ,_remoteAddr(true)
    ,_protocolStack(NULL)
    {

    }

    ~ServiceSessionInfo()
    {

    }

    KERNEL_NS::LibString ToString() const
    {
        KERNEL_NS::LibString info;
        info.AppendFormat("service id:%llu, sessionId:%llu, pollerId:%llu, priorityLevel:%u, sessionType:%d, localAddr:%s, remoteAddr:%s"
                        , _serviceId, _sessionId, _pollerId, _priorityLevel, _sessionType, _localAddr.ToString().c_str(), _remoteAddr.ToString().c_str());

        return info;
    }

    UInt64 _serviceId;      // 服务id
    UInt64 _sessionId;      // 会话id
    UInt64 _pollerId;       // poller
    UInt32 _priorityLevel;  // 优先级
    Int32 _sessionType;     // 会话类型 SessionType
    UInt64 _sessionSendBytesLimit;  // 发送消息长度限制 0表示无限制
    UInt64 _sessionRecvBytesLimit;  // 接收消息长度限制 0表示无限制

    KERNEL_NS::BriefSockAddr _localAddr;    // 本地地址
    KERNEL_NS::BriefSockAddr _remoteAddr;   // 远程地址

    KERNEL_NS::IProtocolStack *_protocolStack;
};

SERVICE_END
