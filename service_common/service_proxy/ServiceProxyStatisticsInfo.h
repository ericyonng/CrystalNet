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

#ifndef __CRYSTAL_NET_SERVICE_COMMON_SERVICE_PROXY_SERVICE_PROXY_STATISTICS_INFO_H__
#define __CRYSTAL_NET_SERVICE_COMMON_SERVICE_PROXY_SERVICE_PROXY_STATISTICS_INFO_H__

#pragma once

#include <service_common/common/macro.h>
#include <service_common/service/ServiceStatisticsInfo.h>
#include <kernel/comp/LibString.h>

SERVICE_COMMON_BEGIN

struct ServiceProxyStatisticsInfo
{
    POOL_CREATE_OBJ_DEFAULT(ServiceProxyStatisticsInfo);

    KERNEL_NS::LibString ToString() const;
    KERNEL_NS::LibString ToSummaryInfo() const;

    std::vector<ServiceStatisticsInfo> _serviceStatistatics;
};

SERVICE_COMMON_END

#endif