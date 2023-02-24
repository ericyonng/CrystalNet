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
 * Date: 2022-11-23 21:12:07
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/PerformanceRecord.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/LibTime.h>

KERNEL_BEGIN

#ifdef CRYSTAL_NET_PORFORMANCE_RECORD

POOL_CREATE_OBJ_DEFAULT_IMPL(PerformanceRecord);

PerformanceRecord::~PerformanceRecord()
{
    const auto ms = LibCpuCounter().Update().ElapseMilliseconds(_start);
    if(LIKELY(_outputLogMsLine > ms))
        return;

    g_Log->Warn(LOGFMT_OBJ_TAG("[PERFORMANCE RECORD]:%s, cost:%llu (ms).")
                ,_getContent().c_str(), ms);
}

#endif

KERNEL_END
