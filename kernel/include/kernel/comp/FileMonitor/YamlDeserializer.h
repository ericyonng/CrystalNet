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
// Date: 2026-01-21 22:01:02
// Author: Eric Yonng
// Description:

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_MONITOR_YAML_DESERIALIZER_FACTORY_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_MONITOR_YAML_DESERIALIZER_FACTORY_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class YamlDeserializer
{
    POOL_CREATE_OBJ_DEFAULT(YamlDeserializer);
    
public:
    YamlDeserializer();
    ~YamlDeserializer();

    // tls delete, 建议tls创建factory
    void Release();

    // 会阻塞线程等待返回, 所以应该在用在启服
    template<typename T>
    T *Load();
    
    template<typename T>
    bool Register(const LibString &path, T *monitor);
    void UnRegister();

private:
    void _LoadYaml();

private:
    LibString _path;
    UInt64 _stub;
};

template<typename T>
ALWAYS_INLINE T *YamlDeserializer::Load()
{
    
}

KERNEL_END

#endif

