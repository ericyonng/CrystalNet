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
 * Date: 2024-01-15 14:13:57
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_DEFS_POLLER_STATISTICS_INFO_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_DEFS_POLLER_STATISTICS_INFO_H__

#pragma once


#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Poller/PollerCompStatistics.h>

KERNEL_BEGIN

struct KERNEL_EXPORT NetPollerCompStatistics
{
    POOL_CREATE_OBJ_DEFAULT(NetPollerCompStatistics);

    UInt64 _pollerId = 0;
    UInt64 _sessionCount = 0;
    PollerCompStatistics _pollerStatistics;
};

struct KERNEL_EXPORT PollerMgrStatisticsInfo
{
    POOL_CREATE_OBJ_DEFAULT(PollerMgrStatisticsInfo);

    LibString ToString() const;

    UInt64 _onlineSessionTotalCount = 0;
    UInt64 _historySessionTotalCount = 0;
    UInt64 _sessionSpeedTotalCount = 0;

    UInt64 _acceptedOnlineSessionTotalCount = 0;
    UInt64 _acceptedHistoryTotalCount = 0;
    UInt64 _acceptedSpeedTotalCount = 0;

    UInt64 _connectOnlineSessionTotalCount = 0;
    UInt64 _connectHistoryTotalCount = 0;
    UInt64 _connectSpeedTotalCount = 0;

    UInt64 _listenerSessionTotalCount = 0;

    std::vector<NetPollerCompStatistics> _netPollerStatistics;

    UInt64 _packetRecvQps = 0;
    UInt64 _packetRecvBytesSpeed = 0;
    UInt64 _packetSendQps = 0;
    UInt64 _packetSendBytesSpeed = 0;
    UInt64 _totalHistoryRecvPacket = 0;
    UInt64 _totalHistoryRecvBytes = 0;
    UInt64 _totalHistorySendPacket = 0;
    UInt64 _totalHistorySendBytes = 0;

    UInt64 _dataTransferPollerCount = 0;
    UInt64 _linkerPollerTotalCount = 0;
    UInt64 _totalPollerTotalCount = 0;
};

KERNEL_END

#endif