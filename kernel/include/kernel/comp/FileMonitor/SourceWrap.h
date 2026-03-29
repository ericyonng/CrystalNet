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
// Date: 2026-03-28 18:03:32
// Author: Eric Yonng
// Description:


#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_MONITOR_SOURCE_WRAP_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_MONITOR_SOURCE_WRAP_H__

#pragma once


#include <kernel/kernel_export.h>
#include <kernel/common/BaseType.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

struct KERNEL_EXPORT SourceWrap
{
    KERNEL_NS::LibString Path = "";

    // 共享的一块内存配置, FileMonitor不能操作, 由FileChangeManager检查是否内容变化,fromMemory由外部管生命周期, 这里不管生命周期, 但是不好处理fromMemory, 战略性泄露fromMemory
    void *FromMemory = NULL;
};

KERNEL_END

#endif
