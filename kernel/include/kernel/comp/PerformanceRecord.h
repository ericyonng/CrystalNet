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
 * Date: 2022-11-23 21:11:41
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_PERFORMANCE_RECORD_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_PERFORMANCE_RECORD_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Cpu/LibCpuCounter.h>
#include <kernel/comp/Log/log.h>

KERNEL_BEGIN

#ifndef CRYSTAL_NET_PORFORMANCE_RECORD
 #define CRYSTAL_NET_PORFORMANCE_RECORD
#endif

#undef PERFORMANCE_RECORD_DEF

#undef PR_FMT
#define PR_FMT(CONTENT_FMT) "%s:%d %s " CONTENT_FMT, __FILE__,__LINE__,__FUNCTION__

#ifdef CRYSTAL_NET_PORFORMANCE_RECORD

// 耗时超过OUTPUT_LOG_MS_LINE 会输出日志
#define PERFORMANCE_RECORD_DEF(PR, GET_CONTENT_LAMBDA, OUTPUT_LOG_MS_LINE) KERNEL_NS::PerformanceRecord<decltype(GET_CONTENT_LAMBDA)> PR(GET_CONTENT_LAMBDA, OUTPUT_LOG_MS_LINE)

template<typename GetContentLambda>
class PerformanceRecord
{
    POOL_CREATE_TEMPLATE_OBJ_DEFAULT(PerformanceRecord, GetContentLambda);

public:
    PerformanceRecord(GetContentLambda &getContentFun, UInt64 outputLogMsLine);
    ~PerformanceRecord();

private:
    GetContentLambda &_getContent;
    LibCpuCounter _start;
    UInt64 _outputLogMsLine;
};

template<typename GetContentLambda>
ALWAYS_INLINE PerformanceRecord<GetContentLambda>::PerformanceRecord(GetContentLambda &getContentFun, UInt64 outputLogMsLine)
:_getContent(getContentFun)
{
    _outputLogMsLine = outputLogMsLine;
    _start.Update();
}

template<typename GetContentLambda>
ALWAYS_INLINE PerformanceRecord<GetContentLambda>::~PerformanceRecord()
{
    const auto ms = LibCpuCounter().Update().ElapseMilliseconds(_start);
    if(LIKELY(_outputLogMsLine > ms))
        return;

    g_Log->Warn(LOGFMT_OBJ_TAG("[PERFORMANCE RECORD]:%s, cost:%llu (ms).")
                ,_getContent().c_str(), ms);
}


template<typename GetContentLambda>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_IMPL(PerformanceRecord, GetContentLambda);

template<typename GetContentLambda>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_TL_IMPL(PerformanceRecord, GetContentLambda);

#else

#define PERFORMANCE_RECORD_DEF(PR, GET_CONTENT, OUTPUT_LOG_MS_LINE) 

#endif

KERNEL_END

#endif
