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
 * Date: 2021-02-17 15:17:38
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOG_LOG_CONFIG_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOG_LOG_CONFIG_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/comp/LibString.h>
#include <vector>

KERNEL_BEGIN

struct KERNEL_EXPORT LogLevelCfg
{
    Int32 _level = 0;                   // 日志等级
    LibString _levelName;               // 日志等级名
    bool _enable = false;               // 是否允许写日志
    Int32 _fileId = 0;                  // 绑定日志文件id
    bool _enableConsole = false;            // 是否允许控制台显示
    Int32 _fgColor = 0;                 // 前景色 默认白色
    Int32 _bgColor = 0;                 // 背景色 默认黑色
    bool _enableRealTime = false;       // 是否开启实时着盘
    bool _printStackTraceBack = false;  // 开启打印堆栈信息
    bool _needWriteFile = true;         // 是否需要写入文件
};

struct KERNEL_EXPORT LogConfig
{
    Int32 _logFileId = 0;                               // 与文件名捆绑 0表示程序进程名做文件名
    std::vector<LogLevelCfg *>    _levelIdxRefCfg;      // level当下表索引  需要额外判空,有可能为空
    LibString _logFileName;                             // 文件名
    LibString _extName;                                 // 文件后缀名
    Int32 _intervalMs = 0;                              // 日志着盘间隔
    Int64 _maxFileSize = 0;                             // 文件最大大小 
    Int32 _threadRelationId;                            // 线程相关性id
    bool _isEnable = false;                             // 只要有一个级别的日志开启则为true
    bool _needWriteFile = false;                        // 只要有一个级别的日志需要写文件那么就需要着盘写文件
};

KERNEL_END

// 文件名配置=0,|1,文件名|2,文件名|3,文件名
// 最大缓存限制=maxMB ;用于限制日志缓存大小,避免内存暴涨
// 日志着盘时间间隔:
// 日志文件最大大小
// ini 配置：[日志等级] = [enable],[fileid],[enableConsole],[fgcolor]:white,red,black,[bgcolor]:...
#endif

