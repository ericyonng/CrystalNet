// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2026-10-19 00:10:01
// Author: Eric Yonng
// Description:

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_DEFS_ASYNC_RESULT_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_DEFS_ASYNC_RESULT_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <vector>

KERNEL_BEGIN

struct KERNEL_EXPORT AsyncResult
{
    POOL_CREATE_OBJ_DEFAULT(AsyncResult);

    // DeleteThreadLocal_AsyncResult
    void Release();

    Int32 _errCode = 0;
    UInt64 _sessionId;
};

struct KERNEL_EXPORT AsyncMultiResult
{
    POOL_CREATE_OBJ_DEFAULT(AsyncMultiResult);

    // DeleteThreadLocal_AsyncMultiResult
    void Release();

    Int32 _errCode = 0;

    // 最终返回的session集合
    std::vector<UInt64> _sessionIds;
};

KERNEL_END


#endif

