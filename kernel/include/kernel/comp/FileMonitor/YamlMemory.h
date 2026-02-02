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
// Date: 2026-02-02 12:30:02
// Author: Eric Yonng
// 一个内存区域, 用来监控内存变化
// Description:

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_MONITOR_YAML_MEMORY_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_MONITOR_YAML_MEMORY_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

// 生产者必须生成Version, 以便校验版本是否更新
struct KERNEL_EXPORT YamlMemoryData
{
    POOL_CREATE_OBJ_DEFAULT(YamlMemoryData);

    void Release();

    LibString _data;
    // data的sha256
    LibString _version;
};

// yaml来自内存数据
class KERNEL_EXPORT YamlMemory
{
    POOL_CREATE_OBJ_DEFAULT(YamlMemory)

public:
    YamlMemory();
    ~YamlMemory();
    
    void Release();

    // 检查变更, 并获取最新的MemoryData,更新当前版本号
    YamlMemoryData *CheckAndChange();

    // 更新yaml内存(来自内存数据,或者网络更新到内存)
    void SetNewData(YamlMemoryData *data);

    // 有变化的话_source部位null, 且_source不等_data
private:
    // 每次更新yaml都需要
    LibString _currentVersion;
    std::atomic<YamlMemoryData *> _source;
};

KERNEL_END

#endif
