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
 * Date: 2021-12-12 02:33:31
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_APP_BASE_APP_OPTION_H__
#define  __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_APP_BASE_APP_OPTION_H__


#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

struct KERNEL_EXPORT BaseAppOption
{
    LibString ToString() const;

    LibString _logIniName;                          // 日志的ini文件名
    LibString _iniPath;                             // ini配置所在目录
    UInt64 _appMonitorFrameMilliSecInterval = 0;    // app监控线程扫描的间隔毫秒数
    UInt32 _blackWhiteListFlag = 0;                 // 黑白名单模式
    UInt32 _instanceId = 0;                         // 机器id
};

inline LibString BaseAppOption::ToString() const
{
    LibString info;

    info.AppendFormat("_logIniName:%s, ", _logIniName.c_str())
        .AppendFormat("_iniPath:%s, ", _iniPath.c_str())
        .AppendFormat("_appMonitorFrameMilliSecInterval:%llu, ", _appMonitorFrameMilliSecInterval)
        .AppendFormat("_blackWhiteListFlag:%u, ", _blackWhiteListFlag)
        .AppendFormat("_instanceId:%u, ", _instanceId)
        ;
        
    return info;
}


KERNEL_END

#endif
