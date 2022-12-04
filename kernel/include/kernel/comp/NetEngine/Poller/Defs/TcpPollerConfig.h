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
 * Date: 2022-04-21 16:27:41
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_DEFS_TCP_POLLER_CONFIG_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_DEFS_TCP_POLLER_CONFIG_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

struct TcpPollerConfig;
struct TcpPollerFeatureConfig;

// 每个poller实例配置
struct KERNEL_EXPORT TcpPollerInstConfig
{
    POOL_CREATE_OBJ_DEFAULT(TcpPollerInstConfig);

    TcpPollerInstConfig(const TcpPollerFeatureConfig *owner, UInt32 id);

    LibString ToString() const;

    void Copy(const TcpPollerInstConfig &cfg);

    const UInt32 _id; // TcpPollerFeatureConfig中vector的下标
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

    const TcpPollerFeatureConfig *_owner;   // owner
};

struct KERNEL_EXPORT TcpPollerFeatureConfig
{
    POOL_CREATE_OBJ_DEFAULT(TcpPollerFeatureConfig);

    TcpPollerFeatureConfig(const TcpPollerConfig *owner, Int32 pollerFeature);
    ~TcpPollerFeatureConfig();

    LibString ToString() const;
    void Copy(const TcpPollerFeatureConfig &cfg);

    const Int32 _pollerFeature;
    const TcpPollerConfig *_owner;
    std::vector<TcpPollerInstConfig *> _pollerInstConfigs;
};

// poller 配置
struct KERNEL_EXPORT TcpPollerConfig
{
    POOL_CREATE_OBJ_DEFAULT(TcpPollerConfig);

    TcpPollerConfig();
    ~TcpPollerConfig();

    LibString ToString() const;

    void Copy(const TcpPollerConfig &cfg);

    std::unordered_map<Int32, TcpPollerFeatureConfig *> _pollerFeatureRefConfig;
};

KERNEL_END

#endif
