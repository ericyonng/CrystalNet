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
#include <yaml-cpp/yaml.h>
#include <kernel/comp/Log/ILog.h>
#include <kernel/comp/FileMonitor/SourceWrap.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Delegate/IDelegate.h>

KERNEL_BEGIN

class YamlMemory;
class FileChangeImpl;

class YamlDeserializer;


class KERNEL_EXPORT YamlDeserializer
{
    YamlDeserializer(const SourceWrap &source, const KERNEL_NS::LibString &dataName, IDelegate<void *, YAML::Node &> *parser, IDelegate<void, void *> *releaseData, const KERNEL_NS::LibString &key);
    ~YamlDeserializer();

public:
    // tls delete, 建议tls创建factory
    void Release();

    // key:可以是多级的, 比如:a.b.c
    static YamlDeserializer *Create(const SourceWrap &source, const LibString &dataName, IDelegate<void *, YAML::Node &> *parser, IDelegate<void, void *> *releaseData, const KERNEL_NS::LibString &key = "");

    // key:可以是多级的, 比如:a.b.c
    template<typename T>
    requires requires(T t, YAML::Node node)
    {
        // 需要有Release接口
        t.Release();

        // 需要有创建NewObj接口
        T::CreateNewObj(std::move(t));

        // 需要支持yaml的序列化反序列化: TODO:测试类型是否具有yaml序列化反序列化接口
        YAML::convert<T>::encode(t);
        YAML::convert<T>::decode(node, t);
    }
    static YamlDeserializer *Create(const SourceWrap &source, const KERNEL_NS::LibString &key = "")
    {
        // 释放T对象
        auto releaseLamb = [](void *ptr)
        {
            if(!ptr)
                return;
            
            auto p = KERNEL_NS::KernelCastTo<T>(ptr);
            p->Release();
        };
        auto parser = [](YAML::Node &node) -> void *
        {
            return T::CreateNewObj(node.template as<T>());
        };
        auto releaseDeleg = KERNEL_CREATE_CLOSURE_DELEGATE(releaseLamb, void, void *);
        auto parserDeleg = KERNEL_CREATE_CLOSURE_DELEGATE(parser, void *, YAML::Node &);
        const auto &dataName = KERNEL_NS::RttiUtil::GetByType<T>();
        return YamlDeserializer::Create(source, dataName, parserDeleg, releaseDeleg, key);
    }

    // 返回新数据
    template<typename T>
    T *SwapNewData()
    {
        return KERNEL_NS::KernelCastTo<T>(_data.exchange(NULL, std::memory_order_acq_rel));
    }

private:
    // 多级YamlNode索引, keys多级key集合
    static YAML::Node IndexNode(YAML::Node &config, const std::vector<KERNEL_NS::LibString> &keys, Int32 curIndex, Int32 maxIndex)
    {
        // 最后一级config
        if(curIndex == maxIndex)
        {
            return config[keys[curIndex].c_str()];
        }

        // 下一级config
        auto node = config[keys[curIndex].c_str()];
        return IndexNode(node, keys, curIndex + 1, maxIndex);
    }

private:
    void _Run();
    void _FirstRun();
    void _ReplaceConfig(YAML::Node *data);

    KERNEL_NS::SmartPtr<YAML::Node> GetTlsSourceYaml(bool &isChange) const;
    
private:
    alignas(SYSTEM_ALIGN_SIZE) const SourceWrap _source;
    alignas(SYSTEM_ALIGN_SIZE) const LibString _key;
    alignas(SYSTEM_ALIGN_SIZE) const LibString _dataName;
    alignas(SYSTEM_ALIGN_SIZE) FileChangeImpl *_impl;
    
    alignas(SYSTEM_ALIGN_SIZE) std::atomic<void *> _data;
    alignas(SYSTEM_ALIGN_SIZE) mutable std::atomic<UInt64> _version;

    alignas(SYSTEM_ALIGN_SIZE) IDelegate<void *, YAML::Node &> *_parser;
    alignas(SYSTEM_ALIGN_SIZE) IDelegate<void, void *> *_releaseData;
};


KERNEL_END

#endif

