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
 * Date: 2024-01-21 18:20:17
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <service_common/service_proxy/ServiceProxyStatisticsInfo.h>
#include <kernel/comp/Utils/SocketUtil.h>

SERVICE_COMMON_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(ServiceProxyStatisticsInfo);

KERNEL_NS::LibString ServiceProxyStatisticsInfo::ToString() const
{
    KERNEL_NS::LibString info;
    for(auto &serviceInfo : _serviceStatistatics)
    {
        if(!serviceInfo._servicePollerInfo._isEnable)
            continue;

        auto &pollerMgrInfo = serviceInfo._pollerMgrStatisticsInfo;

        info.AppendFormat("[SERVICE MONITOR id:%llu BEGIN]\n", serviceInfo._serviceId);
        info.AppendFormat("service session:[amount:%llu, recv packets:%llu, consume packets:%llu]\n"
        , serviceInfo._sessionAmount, serviceInfo._recvPackets, serviceInfo._consumePackets);
        info.AppendFormat("service poller info:[loaded:%llu, gen:%llu, consume:%llu, backlog:%llu]\n"
        , serviceInfo._servicePollerInfo._loadedScore, serviceInfo._servicePollerInfo._pollerGenQps, serviceInfo._servicePollerInfo._pollerConsumeQps
        , serviceInfo._servicePollerInfo._pollerBacklog);

        info.AppendFormat("poller mgr info session:[online:%llu, speed:%llu, history:%llu]\n"
        , pollerMgrInfo._onlineSessionTotalCount, pollerMgrInfo._sessionSpeedTotalCount, pollerMgrInfo._historySessionTotalCount);
        info.AppendFormat("poller mgr accepted session:[online:%llu, speed:%llu, history:%llu]\n"
        , pollerMgrInfo._acceptedOnlineSessionTotalCount, pollerMgrInfo._acceptedSpeedTotalCount, pollerMgrInfo._acceptedHistoryTotalCount);
        info.AppendFormat("poller mgr connected session:[online:%llu, speed:%llu, history:%llu]\n"
        , pollerMgrInfo._connectOnlineSessionTotalCount, pollerMgrInfo._connectSpeedTotalCount, pollerMgrInfo._connectHistoryTotalCount);
        info.AppendFormat("poller mgr listener session:[online:%llu]\n"
        , pollerMgrInfo._listenerSessionTotalCount);

        info.AppendFormat("tcp poller info:[data transfer:%llu, linker:%llu total:%llu]\n"
        ,  pollerMgrInfo._dataTransferPollerCount, pollerMgrInfo._linkerPollerTotalCount, pollerMgrInfo._totalPollerTotalCount);

        for(auto &item : pollerMgrInfo._netPollerStatistics)
        {
            if(!item._pollerStatistics._isEnable)
                continue;

            info.AppendFormat("poller id:%llu info:[loaded:%llu, gen:%llu, consume:%llu, backlog:%llu]\n"
            , item._pollerId,  item._pollerStatistics._loadedScore, item._pollerStatistics._pollerGenQps, item._pollerStatistics._pollerConsumeQps
            , item._pollerStatistics._pollerBacklog);
        }

        info.AppendFormat("packets info:[recv -qps:%llu -bytes:%s -history count:%llu -bytes:%llu]\n"
        , pollerMgrInfo._packetRecvQps, KERNEL_NS::SocketUtil::ToFmtSpeedPerSec(pollerMgrInfo._packetRecvBytesSpeed).c_str()
        , pollerMgrInfo._totalHistoryRecvPacket, pollerMgrInfo._totalHistoryRecvBytes);

        info.AppendFormat("packets info:[send -qps:%llu -bytes:%s -history count:%llu -bytes:%llu]\n"
        , pollerMgrInfo._packetSendQps, KERNEL_NS::SocketUtil::ToFmtSpeedPerSec(pollerMgrInfo._packetSendBytesSpeed).c_str()
        , pollerMgrInfo._totalHistorySendPacket, pollerMgrInfo._totalHistorySendBytes);

        info.AppendFormat("[SERVICE MONITOR id:%llu END]\n", serviceInfo._serviceId);
    }

    return info;
}

KERNEL_NS::LibString ServiceProxyStatisticsInfo::ToSummaryInfo() const
{
    KERNEL_NS::LibString info;

    UInt64 sessionNum = 0;
    Int32 serviceCount = 0;
    Int64 recvPackets = 0;
    Int64 consumePackets = 0;
    Int32 enablePollerNum = 0;
    UInt64 loaded = 0;
    UInt64 gen = 0;
    UInt64 consume = 0;
    UInt64 backlog = 0;
    UInt64 onlineSessionTotalCount = 0;
    UInt64 sessionSpeedTotalCount = 0;
    UInt64 historySessionTotalCount = 0;

    UInt64 acceptedOnlineSessionTotalCount = 0;
    UInt64 acceptedHistoryTotalCount = 0;
    UInt64 acceptedSpeedTotalCount = 0;
    UInt64 connectOnlineSessionTotalCount = 0;
    UInt64 connectHistoryTotalCount = 0;
    UInt64 connectSpeedTotalCount = 0;

    UInt64 listenerSessionTotalCount = 0;

    UInt64 packetRecvQps = 0;
    UInt64 packetRecvBytesSpeed = 0;
    UInt64 packetSendQps = 0;
    UInt64 packetSendBytesSpeed = 0;

    UInt64 totalHistoryRecvPacket = 0;
    UInt64 totalHistoryRecvBytes = 0;
    UInt64 totalHistorySendPacket = 0;
    UInt64 totalHistorySendBytes = 0;

    UInt64 dataTransferPollerCount = 0;
    UInt64 linkerPollerTotalCount = 0;
    UInt64 totalPollerTotalCount = 0;
    KERNEL_NS::NetPollerCompStatistics netPollerStatistics;
    for(auto &serviceInfo : _serviceStatistatics)
    {
        ++serviceCount;
        sessionNum += serviceInfo._sessionAmount; 
        recvPackets += serviceInfo._recvPackets; 
        consumePackets += serviceInfo._consumePackets; 
        if(serviceInfo._servicePollerInfo._isEnable)
            enablePollerNum += 1;

        loaded += serviceInfo._servicePollerInfo._loadedScore;
        gen += serviceInfo._servicePollerInfo._pollerGenQps;
        consume += serviceInfo._servicePollerInfo._pollerConsumeQps;
        backlog += serviceInfo._servicePollerInfo._pollerBacklog;

        auto &pollerMgrInfo = serviceInfo._pollerMgrStatisticsInfo;
        onlineSessionTotalCount += pollerMgrInfo._onlineSessionTotalCount;
        sessionSpeedTotalCount += pollerMgrInfo._sessionSpeedTotalCount;
        historySessionTotalCount += pollerMgrInfo._historySessionTotalCount;

        acceptedOnlineSessionTotalCount += pollerMgrInfo._acceptedOnlineSessionTotalCount;
        acceptedHistoryTotalCount += pollerMgrInfo._acceptedHistoryTotalCount;
        acceptedSpeedTotalCount += pollerMgrInfo._acceptedSpeedTotalCount;

        connectOnlineSessionTotalCount += pollerMgrInfo._connectOnlineSessionTotalCount;
        connectHistoryTotalCount += pollerMgrInfo._connectHistoryTotalCount;
        connectSpeedTotalCount += pollerMgrInfo._connectSpeedTotalCount;

        listenerSessionTotalCount += pollerMgrInfo._listenerSessionTotalCount;

        for(auto &item : pollerMgrInfo._netPollerStatistics)
        {
            if(!item._pollerStatistics._isEnable)
                continue;

            netPollerStatistics._pollerStatistics._loadedScore += item._pollerStatistics._loadedScore;
            netPollerStatistics._pollerStatistics._pollerGenQps += item._pollerStatistics._pollerGenQps;
            netPollerStatistics._pollerStatistics._pollerConsumeQps += item._pollerStatistics._pollerConsumeQps;
            netPollerStatistics._pollerStatistics._pollerBacklog += item._pollerStatistics._pollerBacklog;
        }

        packetRecvQps += pollerMgrInfo._packetRecvQps;
        packetRecvBytesSpeed += pollerMgrInfo._packetRecvBytesSpeed;
        packetSendQps += pollerMgrInfo._packetSendQps;
        packetSendBytesSpeed += pollerMgrInfo._packetSendBytesSpeed;
        totalHistoryRecvPacket += pollerMgrInfo._totalHistoryRecvPacket;
        totalHistoryRecvBytes += pollerMgrInfo._totalHistoryRecvBytes;
        totalHistorySendPacket += pollerMgrInfo._totalHistorySendPacket;
        totalHistorySendBytes += pollerMgrInfo._totalHistorySendBytes;

        dataTransferPollerCount += pollerMgrInfo._dataTransferPollerCount;
        linkerPollerTotalCount += pollerMgrInfo._linkerPollerTotalCount;
        totalPollerTotalCount += pollerMgrInfo._totalPollerTotalCount;
    }

    info.AppendFormat("service:[num:%d, session:%llu, packets:[recv:%lld, consume:%lld], poller:[enable:%d, loaded:%llu, gen:%llu, consume:%llu, backlog:%llu]\n"
    , serviceCount, sessionNum, recvPackets, consumePackets, enablePollerNum, loaded, gen, consume, backlog);

    info.AppendFormat("session:[online:%llu, created speed:%llu, history:%llu]\n"
    , onlineSessionTotalCount, sessionSpeedTotalCount, historySessionTotalCount);

    info.AppendFormat("accepted session:[online:%llu, speed:%llu, history:%llu]\n"
    , acceptedOnlineSessionTotalCount, acceptedSpeedTotalCount, acceptedHistoryTotalCount);

    info.AppendFormat("connected session:[online:%llu, speed:%llu, history:%llu]\n"
    , connectOnlineSessionTotalCount, connectSpeedTotalCount, connectHistoryTotalCount);

    info.AppendFormat("listener session:%llu\n"
    , listenerSessionTotalCount);

    info.AppendFormat("tcp manager:[data transfer:%llu, linker:%llu, total:%llu]\n"
    , dataTransferPollerCount, linkerPollerTotalCount, totalPollerTotalCount);
    info.AppendFormat("tcp manager packets:[recv -qps:%llu -bytes:%s -history count:%llu -bytes:%llu, send -qps:%llu -bytes:%s -history count:%llu -bytes:%llu]\n"
    , packetRecvQps, KERNEL_NS::SocketUtil::ToFmtSpeedPerSec(packetRecvBytesSpeed).c_str(), totalHistoryRecvPacket, totalHistoryRecvBytes, packetSendQps
    , KERNEL_NS::SocketUtil::ToFmtSpeedPerSec(packetSendBytesSpeed).c_str(), totalHistorySendPacket, totalHistorySendBytes);

    info.AppendFormat("tcp manager poller:[loaded:%llu gen:%llu consume:%llu backlog:%llu]"
    , netPollerStatistics._pollerStatistics._loadedScore, netPollerStatistics._pollerStatistics._pollerGenQps
    , netPollerStatistics._pollerStatistics._pollerConsumeQps, netPollerStatistics._pollerStatistics._pollerBacklog);

    return info;
}


SERVICE_COMMON_END