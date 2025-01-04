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
 * Date: 2025-01-04 00:00:00
 * Author: Eric Yonng
 * Description: 如果当前不是处在协程内, 则使用当前线程的Tls级别的traceId, 如果当前处于协程内, 则使用协程的TraceId, 必须是 main trace id(追踪每一帧) + sub trace id(细化到每一帧的子事件) + LibString 额外的描述 数据结构
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LIB_TRACE_ID_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LIB_TRACE_ID_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>

#include "NetEngine/BriefSockAddr.h"

KERNEL_BEGIN
    struct KERNEL_EXPORT LibTraceId
{
  POOL_CREATE_OBJ_DEFAULT(LibTraceId);

  LibTraceId();
  LibString ToString() const;

    // 更新主trace
    void UpdateMain();
    // 更新子trace
    void UpdateSub();

    static LibTraceId *GetCurrentTrace();
    static std::atomic<UInt64> &GetAtomicMaxId();

  UInt64 _mainTraceId;
  UInt64 _subTraceId;
  LibString _desc;

};

ALWAYS_INLINE LibString LibTraceId::ToString() const
{
    LibString info;
    return  info.AppendFormat("MainTrace:%llu,SubTrace:%llu,Desc:%s", _mainTraceId, _subTraceId, _desc.c_str());
}


struct KERNEL_EXPORT TlsLibTrace
{
    POOL_CREATE_OBJ_DEFAULT(TlsLibTrace);

    TlsLibTrace() {}
    ~TlsLibTrace();

    LibTraceId *_trace = NULL;
};

KERNEL_END

#endif // __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LIB_TRACE_ID_H__
