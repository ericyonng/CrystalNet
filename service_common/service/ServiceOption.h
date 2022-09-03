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
 * Date: 2021-12-11 23:03:50
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_SERVICE_SERVICE_OPTION_H__
#define __CRYSTAL_NET_SERVICE_COMMON_SERVICE_SERVICE_OPTION_H__

#pragma once

#include <service_common/common/common.h>
#include <kernel/kernel.h>

SERVICE_COMMON_BEGIN

struct ServiceOption
{
    Int32 _serviceType = 0;                     // 服务类型 Login, DB, Logic, 某个服务等
    UInt64 _serviceId = 0;                      // 服务id proxy生成
    KERNEL_NS::LibString _serviceTypeString;    // 服务类型  Login, DB, Logic, 某个服务等

    UInt64 _maxPieceTimeInMicroseconds = 0;     // 最大时间片
    UInt64 _maxSleepMilliseconds;               // poller扫描时间间隔
    Int32 _maxPriorityLevel = 0;                // 优先级从0开始最大等级
};

SERVICE_COMMON_END

#endif
