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

KERNEL_BEGIN

#ifndef CRYSTAL_NET_PORFORMANCE_RECORD
 #define CRYSTAL_NET_PORFORMANCE_RECORD
#endif

#undef PERFORMANCE_RECORD_DEF

#ifdef CRYSTAL_NET_PORFORMANCE_RECORD

#define PERFORMANCE_RECORD_DEF(PR, CONTENT) KERNEL_NS::PerformanceRecord PR = KERNEL_NS::PerformanceRecord(CONTENT, __FUNCTION__, __FILE__, __LINE__)

class KERNEL_EXPORT PerformanceRecord
{
    POOL_CREATE_OBJ_DEFAULT(PerformanceRecord);

private:
    explicit PerformanceRecord() {}

public:
    PerformanceRecord(const LibString &content, const Byte8 *func, const Byte8 *fileName, Int32 line);

    ~PerformanceRecord();

private:
    LibString _content;
    LibCpuCounter _start;
};

ALWAYS_INLINE PerformanceRecord::PerformanceRecord(const LibString &content, const Byte8 *func, const Byte8 *fileName, Int32 line)
{
    _content.AppendFormat("%s[%d]-%s, %s", fileName, line, func, content.c_str());
    _start.Update();
}

#else

#define PERFORMANCE_RECORD_DEF(PR, CONTENT) 

#endif

KERNEL_END

#endif
