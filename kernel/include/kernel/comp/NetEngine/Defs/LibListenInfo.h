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
 * Date: 2021-12-06 00:43:51
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_LIB_LISTEN_INFO_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_LIB_LISTEN_INFO_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/NetEngine/Poller/impl/Session/SessionOption.h>

KERNEL_BEGIN

struct KERNEL_EXPORT LibListenInfo
{
    POOL_CREATE_OBJ_DEFAULT(LibListenInfo);
    
    LibListenInfo();

    LibString ToString() const;

    LibString _ip;          // ip
    UInt16 _port;           // 端口 0表示由系统指定
    Int32 _family;          // ipv6/ipv4
    UInt64 _serviceId;      // 主服务id
    UInt64 _stub;           // 业务层生成并会原样返回
    UInt32 _priorityLevel;  // 优先级别
    Int32 _protocolType;    // ProtocolType
    Int32 _sessionCount;    // 监听同一个端口创建多少个会话

    SessionOption _sessionOption;   // 会话选项
};

KERNEL_END

#endif
