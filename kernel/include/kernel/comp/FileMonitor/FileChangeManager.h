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
// Date: 2026-01-09 00:01:24
// Author: Eric Yonng
// Description: 如果多个文件监视器监听同一个文件，为了避免多次读取文件, 这些文件应该被统一管理,
// 变化的时候通过反序列化反序列化出来, 然后返回结果到FileMonitor即可, 不需要FileMonitor各自单独的去监听, 只需要注册监听到这里即可
// 不需要监听时, 取消注册即可

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_MONITOR_FILE_CHANGE_MANAGER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_MONITOR_FILE_CHANGE_MANAGER_H__

#pragma once

#include <unordered_set>
#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>

#include "kernel/comp/CompObject/CompObject.h"

KERNEL_BEGIN

class KERNEL_EXPORT FileChangeManager : public CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, FileChangeManager);
    
public:
    FileChangeManager();
    ~FileChangeManager() override;

public:
    

private:
    std::unordered_set<KERNEL_NS::LibString> _files;
};

KERNEL_END

#endif



