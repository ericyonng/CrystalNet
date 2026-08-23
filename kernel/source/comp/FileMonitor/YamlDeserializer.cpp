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
// Date: 2026-01-21 23:01:08
// Author: Eric Yonng
// Description:


#include <pch.h>
#include <kernel/comp/FileMonitor/YamlDeserializer.h>
#include <kernel/comp/FileMonitor/FileChangeManager.h>

#include "kernel/comp/Poller/Poller.h"
#include "kernel/comp/Utils/FileUtil.h"
#include <kernel/comp/FileMonitor/YamlMemory.h>
#include <kernel/comp/FileMonitor/FileChangeImpl.h>
#include <kernel/comp/FileMonitor/YamlFileDeserialize.h>

namespace
{
    struct YamlFileInfoWrapper
    {
        KERNEL_NS::SmartPtr<KERNEL_NS::YamlFileDeserializer, KERNEL_NS::AutoDelMethods::Release> _fileDeserializer;
        UInt64 _curVersion = 0;
        KERNEL_NS::SmartPtr<YAML::Node> _curNode;
    };
}

KERNEL_BEGIN

YamlDeserializer::YamlDeserializer(const SourceWrap &source, const LibString &dataName, IDelegate<void *, YAML::Node &> *parser, IDelegate<void, void *> *releaseData, const KERNEL_NS::LibString &key)
    :_source(source)
    ,_key(key.strip())
    ,_dataName(dataName.strip())
    ,_impl(NULL)
    , _data{NULL}
    ,_version{0}
    ,_parser(parser)
    ,_releaseData(releaseData)
{
    _impl = FileChangeImpl::Create(DelegateFactory::Create(this, &YamlDeserializer::_FirstRun)
     , DelegateFactory::Create(this, &YamlDeserializer::_Run)); 
}

YamlDeserializer::~YamlDeserializer()
{
    if (_impl)
        g_FileChangeManager->UnRegister(_impl->GetHandle());

    CRYSTAL_RELEASE_SAFE(_impl);

    if (auto data = _data.exchange(NULL, std::memory_order_acq_rel))
        _releaseData->Invoke(data);

    CRYSTAL_RELEASE_SAFE(_parser);
    CRYSTAL_RELEASE_SAFE(_releaseData);
}

void YamlDeserializer::Release()
{
    delete this;
}

YamlDeserializer *YamlDeserializer::Create(const SourceWrap &source, const LibString &dataName, IDelegate<void *, YAML::Node &> *parser, IDelegate<void, void *> *releaseData, const KERNEL_NS::LibString &key)
{
    KERNEL_NS::SmartPtr<YamlDeserializer, KERNEL_NS::AutoDelMethods::Release> obj = new YamlDeserializer(source, dataName, parser, releaseData, key);

    if(!g_FileChangeManager->Register(obj->_impl))
    {
        if(g_Log)
            CLOG_ERROR_GLOBAL(YamlDeserializer, "file change manager register fail source:%s, dataName:%s, key:%s", source.ToString().c_str(), dataName.c_str(), key.c_str());
        
        return NULL;
    }

    if(g_Log)
        CRYSTAL_TRACE("yaml desiriallize source:%s, impl:%s", source.ToString().c_str(), obj->_impl->ToString().c_str());
    
    return obj.pop();
}

void YamlDeserializer::_Run()
{
    bool isChange = false;
    auto yamlFileNode = GetTlsSourceYaml(isChange);
    if(!isChange)
        return;
    
    try
    {
        if(_key.empty())
        {
            auto tName = KERNEL_NS::RttiUtil::GetSimpleTypeName(_dataName);
            tName.strip();
            auto value = (*yamlFileNode)[tName.c_str()];
            auto data = _parser->Invoke(value);
            auto oldData = _data.exchange(data, std::memory_order_acq_rel);
            if(oldData)
            {
                _releaseData->Invoke(oldData);
            }
        }
        else
        {
            auto &&parts = _key.Split(".");
            if(UNLIKELY(parts.empty()))
            {
                if (g_Log)
                {
                    CLOG_ERROR("yaml deserialize fail tName invalid, source:%s, key:%s", _source.ToString().c_str(), _key.c_str());
                }

                return;
            }

            const Int32 sz = static_cast<Int32>(parts.size());
            auto node = IndexNode(*yamlFileNode, parts, 0, sz - 1);
            auto data = _parser->Invoke(node);
            auto oldData = _data.exchange(data, std::memory_order_acq_rel);
            if(oldData)
            {
                _releaseData->Invoke(oldData);
            }
        }
    }
    catch (std::exception &e)
    {
        if (g_Log)
        {
            CLOG_ERROR("yaml deserialize fail exception:%s source:%s, key:%s"
                ,e.what(), _source.ToString().c_str(), _key.c_str());
        }
    }
    catch (...)
    {
        if (g_Log)
        {
            CLOG_ERROR("yaml deserialize fail unknown exception source:%s, key:%s"
                , _source.ToString().c_str(), _key.c_str());
        }
    }
}

void YamlDeserializer::_FirstRun()
{
    bool isChange = false;
    auto yamlFileNode = GetTlsSourceYaml(isChange);
    if(!yamlFileNode)
    {
        if(g_Log)
            CLOG_ERROR("GetTlsSourceYaml fail source:%s", _source.ToString().c_str());
        return;
    }
    try
    {
        if(_key.empty())
        {
            auto tName = KERNEL_NS::RttiUtil::GetSimpleTypeName(_dataName);
            tName.strip();
            auto value = (*yamlFileNode)[tName.c_str()];
            auto data = _parser->Invoke(value);
            auto oldData = _data.exchange(data, std::memory_order_acq_rel);
            if(oldData)
            {
                _releaseData->Invoke(oldData);
            }
        }
        else
        {
            auto &&parts = _key.Split(".");
            if(UNLIKELY(parts.empty()))
            {
                if (g_Log)
                {
                    CLOG_ERROR("yaml deserialize fail tName invalid, source:%s, key:%s", _source.ToString().c_str(), _key.c_str());
                }

                return;
            }

            const Int32 sz = static_cast<Int32>(parts.size());
            auto node = IndexNode(*yamlFileNode, parts, 0, sz - 1);
            auto data = _parser->Invoke(node);
            auto oldData = _data.exchange(data, std::memory_order_acq_rel);
            if(oldData)
            {
                _releaseData->Invoke(oldData);
            }
        }
    }
    catch (std::exception &e)
    {
        if (g_Log)
        {
            CLOG_ERROR("yaml deserialize fail exception:%s source:%s, key:%s"
                ,e.what(), _source.ToString().c_str(), _key.c_str());
        }
    }
    catch (...)
    {
        if (g_Log)
        {
            CLOG_ERROR("yaml deserialize fail unknown exception source:%s, key:%s"
                , _source.ToString().c_str(), _key.c_str());
        }
    }
}

KERNEL_NS::SmartPtr<YAML::Node> YamlDeserializer::GetTlsSourceYaml(bool &isChange) const
{
    // key:SourceWrap::MakeKey, pair:first:yaml文件解析, second:文件解析获取的数据的版本号
    DEF_STATIC_THREAD_LOCAL_DECLEAR std::unordered_map<LibString, YamlFileInfoWrapper> *s_SourceRefYamlFile = NULL;
    if(UNLIKELY(!s_SourceRefYamlFile))
    {
        s_SourceRefYamlFile = new std::unordered_map<LibString, YamlFileInfoWrapper>();
        auto ptr = s_SourceRefYamlFile;
        auto lamb = [ptr]()
        {
            delete ptr;
        };
        KERNEL_REGISTER_GLOBAL_LIFE(lamb);
    }

    auto &key = _source.MakeKey();
    auto iter = s_SourceRefYamlFile->find(key);
    if(UNLIKELY(iter == s_SourceRefYamlFile->end()))
    {
        KERNEL_NS::SmartPtr<YamlFileDeserializer, KERNEL_NS::AutoDelMethods::Release> fileDeserializer = YamlFileDeserializer::Create(_source);
        if(UNLIKELY(!fileDeserializer))
        {
            if(g_Log)
                CLOG_ERROR("YamlFileDeserializer create fail source:%s", _source.ToString().c_str());
            return NULL;
        }
        
        iter = s_SourceRefYamlFile->insert(std::make_pair(key,YamlFileInfoWrapper{fileDeserializer, 0, NULL})).first;
    }

    // yaml文件解析器
    auto &yamlFileWrapper = iter->second;
    auto yamlFileDeserializer = yamlFileWrapper._fileDeserializer;
    // 获取新的数据
    auto current = yamlFileDeserializer->SwapNewData(yamlFileWrapper._curVersion);
    if(current)
    {
        yamlFileWrapper._curNode = current;
        isChange = true;
    }

    // 本地 YamlDeserializer 的版本号与yamlFileWrapper的不同说明热更了
    auto oldVersion = _version.exchange(yamlFileWrapper._curVersion, std::memory_order_acq_rel);
    if(oldVersion != yamlFileWrapper._curVersion)
        isChange = true;
    
    return yamlFileWrapper._curNode;
}


KERNEL_END