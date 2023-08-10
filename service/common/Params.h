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
 * Date: 2022-09-18 18:16:10
 * Author: Eric Yonng
 * Description: 
 * 注意：往后追加参数 不指定枚举值，使用自增，防止枚举值冲突，导致逻辑很难追踪
*/

#pragma once

#include <kernel/kernel.h>
#include <service/common/macro.h>

SERVICE_BEGIN

namespace Params
{
    enum ENUMS : Int32
    {
        UNKNOWN = 0,        // 未知
        SESSION_ID,     // 会话id UInt64
        LOCAL_ADDR,     // 本地地址 BriefSockAddr ptr
        REMOTE_ADDR,     // 远程地址 BriefSockAddr ptr
        PROTOCOL_TYPE,   // 协议类型（udp/tcp/quic等）ProtocolType Int32
        PRIORITY_LEVEL,  // 会话优先级级别 UInt32
        SESSION_POLLER_ID,  // 会话最终所在poller UInt64
        SERVICE_ID,         // 所属服务id UInt64
        STUB,               // 存根 UInt64
        IS_FROM_CONNECT,    // 是否来自connect操作 bool
        IS_FROM_LINKER,    // 是否来自listen监听 bool
        SESSION_CLOSE_REASON,    // 关闭原因, Int32
        SESSION_TYPE,    // 会话类型 SessionType Int32
        PROTOCOL_STACK, // 协议栈类型
        LAST_TIME,           // 最后一次时间
        ERROR_CODE,           // 错误码 Int32
        FAMILY,             // AF UInt16

        VAR_HANDLER,        // 处理函数
        VAR_COUNTER,        // 计数
        VAR_LOGIC,          // logic参数
        VAR_ERROR_INFO,     // VarErrInfoList
        VAR_MSG_QUEUE,      // 放一个消息队列

        USER_OBJ,           // 用户对象 IUser *
        USER_ID,            // 用户id
        PACKET,             // 数据包
        OPCODE,             // 协议号
    };
}

SERVICE_END
