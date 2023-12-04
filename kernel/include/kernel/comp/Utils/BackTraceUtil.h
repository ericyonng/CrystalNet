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
 * Date: 2021-03-19 00:13:54
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_BACK_TRACE_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_BACK_TRACE_UTIL_H__

#pragma once

#include <kernel/comp/LibString.h>

KERNEL_BEGIN

template <typename Rtn, typename... Args>
class IDelegate;

class ILog;
struct LogData;

class KERNEL_EXPORT BackTraceUtil
{
public:
    // 初始化crashdump信息 isUseSehExceptionHandler是外部手动加了__try __except的seh crashDestroy
    static Int32 InitCrashHandleParams(ILog *log, IDelegate<void> *crashDestroy);

    // 抓取堆栈快照 主动打印堆栈信息
    static LibString CrystalCaptureStackBackTrace(size_t skipFrames = 0, size_t captureFrames = CRYSTAL_INFINITE);

protected:
    static void _OnBeforeCrashLogHook(LogData *logData);
    // 初始化pdb等符号信息 用于打印堆栈信息
    static Int32 _InitSymbol();
};

KERNEL_END

#endif
