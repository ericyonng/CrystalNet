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
 * Date: 2022-12-26 14:00:00
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_DEFS_UDP_POLLER_CONFIG_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_DEFS_UDP_POLLER_CONFIG_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

struct UdpPollerConfig;

// 每个poller实例配置
struct KERNEL_EXPORT UdpPollerInstConfig
{
    POOL_CREATE_OBJ_DEFAULT(UdpPollerInstConfig);

    UdpPollerInstConfig(const UdpPollerConfig *owner, UInt32 id);

    LibString ToString() const;

    void Copy(const UdpPollerInstConfig &cfg);

    const UInt32 _id; // UdpPollerConfig 中 vector 的下标
    UInt64 _handleRecvBytesPerFrameLimit;   // 单帧最大接收数据量
    UInt64 _handleSendBytesPerFrameLimit;   // 单帧最大发送数据量
    UInt64 _handleAcceptPerFrameLimit;      // 单帧最大处理连接数
    UInt64 _maxPieceTimeInMicroseconds;     // 最大时间片
    UInt64 _maxSleepMilliseconds;           // poller扫描时间间隔
    Int32 _maxPriorityLevel;                // 优先级从0开始最大等级
    Int32 _pollerInstMonitorPriorityLevel;  // 指定PollerMonotor事件的poller优先级，-1表示会取_maxPriorityLevel
    UInt64 _bufferCapacity;                 // session缓冲大小
    UInt64 _sessionRecvPacketSpeedLimit;  // session每毫秒限速
    UInt64 _sessionRecvPacketSpeedTimeUnitMs;       // 收包限速时间单位毫秒数
    UInt64 _sessionRecvPacketStackLimit;    // 收包堆叠上限

    const UdpPollerConfig *_owner;   // owner
};

// poller 配置
struct KERNEL_EXPORT UdpPollerConfig
{
    POOL_CREATE_OBJ_DEFAULT(UdpPollerConfig);

    UdpPollerConfig();
    ~UdpPollerConfig();

    LibString ToString() const;

    void Copy(const UdpPollerConfig &cfg);

    std::vector<UdpPollerInstConfig *> _pollerInstConfigs;
};

KERNEL_END

#endif
