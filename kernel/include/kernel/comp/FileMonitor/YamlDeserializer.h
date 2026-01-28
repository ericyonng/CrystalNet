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
#include <kernel/comp/Log/log.h>
#include <kernel/comp/FileMonitor/FileChangeDefine.h>

KERNEL_BEGIN

struct FileChangeHandle;

class KERNEL_EXPORT YamlDeserializer
{
    POOL_CREATE_OBJ_DEFAULT(YamlDeserializer);
    
public:
    YamlDeserializer();
    ~YamlDeserializer();

    // tls delete, 建议tls创建factory
    void Release();

    template<typename T>
#ifdef CRYSTAL_NET_CPP20
    requires requires(T t, YAML::Node node)
    {
        // 需要有Release接口
        t.Release();

        // 需要有创建NewObj接口
        T::CreateNewObj(T());

        // 需要支持yaml的序列化反序列化: TODO:测试类型是否具有yaml序列化反序列化接口
        YAML::convert<T>::encode(t);
        YAML::convert<T>::decode(node, t);
    }
#endif
    T *Register(const LibString &path)
    {
        _path = path;
        
        // 释放T对象
        auto releaseLamb = [](void *ptr)
        {
            auto p = KERNEL_NS::KernelCastTo<T>(ptr);
            p->Release();
        };
        auto releaseDeleg = KERNEL_CREATE_CLOSURE_DELEGATE(releaseLamb, void, void *);
        // 反序列化
        const auto &dataName = KERNEL_NS::RttiUtil::GetByType<T>();
        auto deserializeLamb = [dataName, path](YAML::Node *config) -> void *
        {
            // TODO:需要测试是不是把命名空间等移除掉
            auto tName = KERNEL_NS::RttiUtil::GetSimpleTypeName(dataName);
            tName.strip();

            T *yamlOption = NULL;
            try
            {
                yamlOption = T::CreateNewObj(config[tName.c_str()].as<T>());
            }
            catch (std::exception &e)
            {
                g_Log->Error(LOGFMT_NON_OBJ_TAG(YamlDeserializer, "yaml deserialize fail, path:%s, exception:%s")
                    , path.c_str(), e.what());
            }
            catch (...)
            {
                g_Log->Error(LOGFMT_NON_OBJ_TAG(YamlDeserializer, "yaml deserialize fail, path:%s"), path.c_str());
            }

            return yamlOption;
        };
        auto deserializeDelg = KERNEL_CREATE_CLOSURE_DELEGATE(deserializeLamb, void *, YAML::Node *);
        
        return _Register(dataName, releaseDeleg, deserializeDelg);
    }

    template<typename T>
    T *SwapNewData()
    {
        auto data = _handle->_data.load(std::memory_order_relaxed);
        while (!_handle->_data.compare_exchange_weak(data, NULL, std::memory_order_acq_rel))
        {
        }
        return KERNEL_NS::KernelCastTo<T>(data);
    }
    
private:
    // return:具体的对象T
    void *_Register(const LibString &dataName, IDelegate<void, void *> * releaseObj, IDelegate<void *, YAML::Node *> *deserializeObj);

private:
    LibString _path;
    // 注册后获取到的_handle,handle的生命周期比YamlDeserializer长, 由FileChangeManager管理
    FileChangeHandle *_handle;
};

class KERNEL_EXPORT YamlDeserializerFactory
{
public:
    static YamlDeserializer *Create()
    {
        return YamlDeserializer::NewThreadLocal_YamlDeserializer();
    }
};

KERNEL_END

#endif

