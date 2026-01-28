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
// Date: 2026-01-29 02:01:13
// Author: Eric Yonng
// Description:


#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_MONITOR_FILE_CHANGE_DEFINE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_MONITOR_FILE_CHANGE_DEFINE_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <unordered_map>

KERNEL_BEGIN


// 由FileManager释放, 其他只需要attach它的地址, 并在生命周期结束的时候, 设置原子变量NotListen即可
struct KERNEL_EXPORT FileChangeHandle
{
    POOL_CREATE_OBJ_DEFAULT(FileChangeHandle);

    FileChangeHandle();
    ~FileChangeHandle();
    void Release();

    // 是否监听
    alignas(SYSTEM_ALIGN_SIZE) std::atomic_bool _notListen;
    // 新数据
    alignas(SYSTEM_ALIGN_SIZE) std::atomic<void *> _data;
    // 反序列化: rtn:result, param:source obj
    alignas(SYSTEM_ALIGN_SIZE) IDelegate<void *, void *> *_deserialize;
    // 释放资源
    alignas(SYSTEM_ALIGN_SIZE) IDelegate<void, void *> *_release;
    // objname
    LibString _dataName;
};

struct KERNEL_EXPORT FileMonitorInfo
{
    POOL_CREATE_OBJ_DEFAULT(FileMonitorInfo);

    FileMonitorInfo();
    ~FileMonitorInfo();
    void Release();

    KERNEL_NS::LibString _path;
    IDelegate<bool> *_checkChange;
    IDelegate<void, void *> *_releaseObj;
    IDelegate<void *> *_loadNewObj;

    std::unordered_map<void *, FileChangeHandle *> _keyRefFileChangeHandle;
};

KERNEL_END

#endif