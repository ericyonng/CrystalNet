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
 * Date: 2022-05-28 02:00:50
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_SESSION_SESSION_OPTION_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_SESSION_SESSION_OPTION_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

struct KERNEL_EXPORT SessionOption
{
    POOL_CREATE_OBJ_DEFAULT(SessionOption);

    SessionOption();

    LibString ToString() const;

    bool _noDelay;                  // 是否低延迟
    UInt64 _sockSendBufferSize;     // socket发送缓冲区大小
    UInt64 _sockRecvBufferSize;     // socket接收缓冲区大小
    UInt64 _sessionBufferCapicity;  // 会话缓冲区大小
    UInt64 _sessionRecvPacketSpeedLimit;  // 会话每毫秒限速
    UInt64 _sessionRecvPacketSpeedTimeUnitMs;       // 收包限速时间单位毫秒数
    UInt64 _sessionRecvPacketStackLimit;    // 收包堆叠上限
    UInt64 _maxPacketSize;          // 最大包大小
    bool _forbidRecv;               // 禁止接收数据
    Int32 _sessionType;             // 会话的类型
    Int32 _protocolStackType;           // 协议栈类型
    UInt64 _sessionRecvPacketContentLimit;             // 包内容大小限制 0表示无限制
    UInt64 _sessionSendPacketContentLimit;             // 包内容大小限制 0表示无限制
};

KERNEL_END

#endif
