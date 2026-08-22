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
// Description:Yaml整个文件解析, 转成YAML::Node

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_MONITOR_YAML_FILE_DESERIALIZE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_MONITOR_YAML_FILE_DESERIALIZE_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/FileMonitor/SourceWrap.h>
#include <kernel/comp/SmartPtr.h>

KERNEL_BEGIN

class FileChangeImpl;
class YamlMemoryData;

class KERNEL_EXPORT YamlFileDeserializer
{
private:
    YamlFileDeserializer(const SourceWrap &source);
    ~YamlFileDeserializer();
    
public:
    static YamlFileDeserializer *Create(const SourceWrap &path);
    void Release();

    UInt64 GetVersion() const;
    
private:
    void _Run();
    void _FirstRun();
    
    void _ReplaceConfig(void *data);
    bool _CheckFileChange();
    
private:
    alignas(SYSTEM_ALIGN_SIZE) const SourceWrap _source;
    alignas(SYSTEM_ALIGN_SIZE) FileChangeImpl *_impl;
    
    // yaml::node
    alignas(SYSTEM_ALIGN_SIZE) std::atomic<void *> _data;
    alignas(SYSTEM_ALIGN_SIZE) std::atomic<UInt64> _version;
    
    alignas(SYSTEM_ALIGN_SIZE) Int64 _fileSize;
    alignas(SYSTEM_ALIGN_SIZE) Int64 _modifyTIme;
};

ALWAYS_INLINE UInt64 YamlFileDeserializer::GetVersion() const
{
    return _version.load(std::memory_order_acquire);
}


KERNEL_END


#endif
