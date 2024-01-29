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
 * Date: 2024-01-21 17:31:00
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_SERVICE_SERVICE_STATISTICS_INFO_H__
#define __CRYSTAL_NET_SERVICE_COMMON_SERVICE_SERVICE_STATISTICS_INFO_H__

#pragma once

#include <service_common/common/macro.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerStatisticsInfo.h>
#include <kernel/comp/Poller/PollerCompStatistics.h>

SERVICE_COMMON_BEGIN

struct ServiceStatisticsInfo
{
    POOL_CREATE_OBJ_DEFAULT(ServiceStatisticsInfo);

    UInt64 _serviceId = 0;

    Int64 _recvPackets = 0;
    Int64 _consumePackets = 0;
    UInt64 _sessionAmount = 0;

    // service poller info
    KERNEL_NS::PollerCompStatistics _servicePollerInfo;

    KERNEL_NS::PollerMgrStatisticsInfo _pollerMgrStatisticsInfo;
};


SERVICE_COMMON_END

#endif
