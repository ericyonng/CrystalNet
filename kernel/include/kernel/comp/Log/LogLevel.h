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
 * Date: 2021-02-17 15:12:51
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOG_LOG_LEVEL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOG_LOG_LEVEL_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

// 这里仅仅保留系统默认提供的日志等级,其他扩展请在业务层面继承并在配置中配置相应的等级id即可,且系统
// 且系统默认提供的等级id配置中不允许更改
class KERNEL_EXPORT LogLevel
{
public:
    enum ENUMS
    {
        None = -1,
        Begin = 0,
        Debug = 1,
        Info = 2,
        Warn = 3,
        Error = 4,

        Crash = 5,
        Net = 6,
        NetDebug = 7,
        NetInfo = 8,
        NetWarn = 9,
        NetError = 10,
        NetTrace = 11,
        Custom = 12,

        Sys = 13,            // 系统初始化情况等,系统信息打印
        MemMonitor = 14,     // 内存监视器日志

        Trace = 15,         // 追踪

        Monitor = 16,       // 监控日志
        End,
    };
};

// 由配置生成
struct KERNEL_EXPORT LogLevelInfo
{
    Int32 _levelId;                 // 配置表
    LibString _levelName;           // "Warn, Info"等
};

// 等级配置各个值枚举
class KERNEL_EXPORT LogLevelValue
{
public:
    enum
    {
        ENALBLE = 0,        // 是否开启该等级日志
        FILE_ID = 1,        // 该等级日志对应的日志文件id（配置中定义）
        ENABLE_CONSOLE = 2, // 是否允许控制台显示
        FG_BG_COLOR = 3,    // 配置控制台前景色|背景色
        LEVEL_ID = 4,       // 配置日志等级的等级id
        ENABLE_REAL_TIME = 5,   // 开启实时着盘
        PRINT_STACK_TRACE_BACK = 6,   // 打印堆栈
    };
};

KERNEL_END

#endif