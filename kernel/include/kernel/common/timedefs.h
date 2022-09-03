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
 * Date: 2021-01-24 23:04:49
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_TIME_DEFS_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_TIME_DEFS_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/common/type.h>

KERNEL_BEGIN

// 解决时间中静态变量在程序startup阶段的初始化依赖问题
class KERNEL_EXPORT TimeDefs
{
public:
    enum ENUMS : Int64
    {
        // 每天
        HOUR_PER_DAY = 24,
        MINUTE_PER_DAY = 1440,
        SECOND_PER_DAY = 86400,
        MILLI_SECOND_PER_DAY = 86400000,
        MICRO_SECOND_PER_DAY = 86400000000,
        NANO_SECOND_PER_DAY  = 86400000000000,

        // 每小时
        MINUTE_PER_HOUR = 60,
        SECOND_PER_HOUR = 3600,
        MILLI_SECOND_PER_HOUR = 3600000,
        MICRO_SECOND_PER_HOUR = 3600000000,
        NANO_SECOND_PER_HOUR  = 3600000000000,

        // 每分钟
        SECOND_PER_MINUTE = 60,
        MILLI_SECOND_PER_MINUTE = 60000,
        MICRO_SECOND_PER_MINUTE = 60000000,
        NANO_SECOND_PER_MINUTE  = 60000000000,

        // 每秒
        MILLI_SECOND_PER_SECOND = 1000,
        MICRO_SECOND_PER_SECOND = 1000000,
        NANO_SECOND_PER_SECOND  = 1000000000,

        // 每毫秒
        MICRO_SECOND_PER_MILLI_SECOND = 1000,
        NANO_SECOND_PER_MILLI_SECOND  = 1000000,

        // 每微秒
        NANO_SECOND_PER_MICRO_SECOND = 1000,

        // 平台下的精度
        // 不同平台时钟精度 std::chrono::system_clock::now().time_since_epoch().count() 精度问题
        #if CRYSTAL_TARGET_PLATFORM_WINDOWS

        // 在windows下系统时间最小分辨率只能到微妙以下10分位
        RESOLUTION_PER_MICROSECOND = 10,
        
        #else
        
        // linux最小分辨率可以到微妙下千分位即可以精确到纳秒
        RESOLUTION_PER_MICROSECOND = 1000,
        
        #endif

    };
};

KERNEL_END


#endif

