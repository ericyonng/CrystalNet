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
 * Date: 2022-06-24 02:28:11
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_APPLICATION_KERNEL_CONFIG_H__
#define __CRYSTAL_NET_SERVICE_COMMON_APPLICATION_KERNEL_CONFIG_H__

#pragma once

#include <kernel/kernel.h>
#include <service_common/common/common.h>

SERVICE_COMMON_BEGIN

struct KernelConfig
{
    UInt32 _blackWhiteListMode = 0;                 // 黑白名单模式
    UInt64 _maxSessionQuantity = 0;                 // 最大会话数量
    Int32 _linkInOutPollerAmount = 0;               // 连入连出poller数量
    Int32 _dataTransferPollerAmount = 0;            // 数据传输的poller数量
    UInt64 _maxRecvBytesPerFrame = 0;               // 单帧最大接收数据量
    UInt64 _maxSendBytesPerFrame = 0;               // 单帧最大发送数据量
    UInt64 _maxAcceptCountPerFrame = 0;             // 单帧最大处理连接数
    UInt64 _maxPieceTimeInMicroSecPerFrame = 0;     // 最大帧时间片
    UInt64 _maxPollerScanMilliseconds = 0;          // 最大poller扫描时间间隔
    Int32 _maxPollerMsgPriorityLevel = 0;           // 最大消息优先级等级
    Int32 _pollerMonitorEventPriorityLevel = 0;     // 指定poller monitor事件的消息优先级等级
    UInt64 _sessionBufferCapicity = 0;               // session缓冲大小设置
    UInt64 _sessionRecvPacketSpeedLimit = 0;            // 收包限速
    UInt64 _sessionRecvPacketSpeedTimeUnitMs = 0;       // 收包限速时间单位毫秒数
    UInt64 _sessionRecvPacketStackLimit = 0;       // 收包堆叠上限
    UInt64 _sessionRecvPacketContentLimit = 0;             // 收包单包大小限制 默认4MB 0表示不限制
    UInt64 _sessionSendPacketContentLimit = 0;             // 发包单包大小限制 0表示不限制
};

SERVICE_COMMON_END

#endif
