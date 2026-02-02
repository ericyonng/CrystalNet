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

#include <pch.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerStatisticsInfo.h>

KERNEL_BEGIN


LibString PollerMgrStatisticsInfo::ToString() const
{
    PollerCompStatistics totalPoller;
    for(auto &poller : _netPollerStatistics)
    {
        totalPoller._loadedScore += poller._pollerStatistics._loadedScore;
        totalPoller._pollerGenQps += poller._pollerStatistics._pollerGenQps;
        totalPoller._pollerConsumeQps += poller._pollerStatistics._pollerConsumeQps;
        totalPoller._pollerBacklog += poller._pollerStatistics._pollerBacklog;
    }
    
    return LibString().AppendFormat("total session - [online:%llu, speed:%llu, history:%llu]\n", _onlineSessionTotalCount, _sessionSpeedTotalCount, _historySessionTotalCount)
            .AppendFormat("accepted session - [online:%llu, speed:%llu, history:%llu]\n", _acceptedOnlineSessionTotalCount, _acceptedSpeedTotalCount, _acceptedHistoryTotalCount)
            .AppendFormat("connect to session - [online:%llu, speed:%llu, history:%llu]\n", _connectOnlineSessionTotalCount, _connectSpeedTotalCount, _connectHistoryTotalCount)
            .AppendFormat("listener session - [online:%llu]\n", _listenerSessionTotalCount)
            .AppendFormat("total net poller info - [poller count:%llu:[data transfer:%llu, linker:%llu], loaded:%llu, gen:%llu, consume:%llu, backlog:%llu]\n"
            , _totalPollerTotalCount, _dataTransferPollerCount, _linkerPollerTotalCount, totalPoller._loadedScore, totalPoller._pollerGenQps, totalPoller._pollerConsumeQps, totalPoller._pollerBacklog)
            .AppendFormat("packet - [recv qps:%llu, bytes speed:%llu | send qps:%llu, bytes speed:%llu | history recv count:%llu-bytes:%llu, history send count:%llu-bytes:%llu]\n"
            , _packetRecvQps, _packetRecvBytesSpeed, _packetSendQps, _packetSendBytesSpeed, _totalHistoryRecvPacket, _totalHistoryRecvBytes, _totalHistorySendPacket, _totalHistorySendBytes);
}

KERNEL_END