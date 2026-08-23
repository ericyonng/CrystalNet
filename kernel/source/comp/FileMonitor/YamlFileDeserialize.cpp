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

#include <pch.h>
#include <kernel/comp/FileMonitor/YamlFileDeserialize.h>
#include <kernel/comp/FileMonitor/FileChangeImpl.h>
#include <kernel/comp/FileMonitor/YamlMemory.h>
#include <kernel/comp/Log/log.h>

#include "kernel/comp/Delegate/LibDelegate.h"
#include "kernel/comp/FileMonitor/FileChangeManager.h"
#include <kernel/comp/Utils/FileUtil.h>

KERNEL_BEGIN
    
YamlFileDeserializer::YamlFileDeserializer(const SourceWrap &source)
    :_source(source)
    , _impl(NULL)
    ,_data{NULL}
    ,_version{0}
    ,_fileSize(0)
{
    _impl = FileChangeImpl::Create(DelegateFactory::Create(this, &YamlFileDeserializer::_FirstRun)
        , DelegateFactory::Create(this, &YamlFileDeserializer::_Run));
}

YamlFileDeserializer::~YamlFileDeserializer()
{
    if (_impl)
        g_FileChangeManager->UnRegister(_impl->GetHandle());
    
    CRYSTAL_RELEASE_SAFE(_impl);

    if (auto data = _data.exchange(NULL, std::memory_order_acq_rel))
        delete data;
}

YamlFileDeserializer *YamlFileDeserializer::Create(const SourceWrap &source)
{
    KERNEL_NS::SmartPtr<YamlFileDeserializer, KERNEL_NS::AutoDelMethods::Release> obj = new YamlFileDeserializer(source);
    if(!g_FileChangeManager->Register(obj->_impl))
    {
        if(g_Log)
            CLOG_ERROR_GLOBAL(YamlFileDeserializer, "file change manager register fail source:%s", source.ToString().c_str());
        
        return NULL;
    }

    if(g_Log)
        CRYSTAL_TRACE("yaml file desiriallize source:%s, impl:%s", source.ToString().c_str(), obj->_impl->ToString().c_str());
    
    return obj.pop();
}

void YamlFileDeserializer::Release()
{
    delete this;
}

KERNEL_NS::SmartPtr<YAML::Node> YamlFileDeserializer::SwapNewData(UInt64 &currentVersion)
{
    KERNEL_NS::SmartPtr<YAML::Node> node;
    auto oldVersion = _version.load(std::memory_order_acquire);
    if(oldVersion == currentVersion)
        return node;

    node = _data.exchange(NULL, std::memory_order_acq_rel);
    if(node)
        currentVersion = oldVersion;

    return node;
}


void YamlFileDeserializer::_Run()
{
    // 在FileChangeManager线程执行
    
    // 1. 检查是否变更
    YAML::Node* config = NULL;
    if (_source.FromMemory)
    {
        auto yamlMemory = KERNEL_NS::KernelCastTo<YamlMemory>(_source.FromMemory);
        
        // 没有yamlData说明没变化
        KERNEL_NS::SmartPtr<YamlMemoryData, KERNEL_NS::AutoDelMethods::Release> yamlData = yamlMemory->CheckAndChange();
        if (yamlData && (!yamlData->_data.empty()))
        {
            try
            {
                config = new YAML::Node(YAML::Load(yamlData->_data.GetRaw()));
            }
            catch (std::exception &e)
            {
                if (g_Log)
                    CLOG_ERROR("path:%s, load yaml fail, exception:%s, yamlData:%p"
                        , _source.ToString().c_str(), e.what(), yamlData.AsSelf());
            }
            catch (...)
            {
                if (g_Log)
                    CLOG_ERROR("source:%s, unkonwn exception, load yaml fail, yamlData:%p"
                        , _source.ToString().c_str(), yamlData.AsSelf());
            }
            
            if (config)
            {
                _ReplaceConfig(config);
                
                if (g_Log)
                    CLOG_INFO("source:%s, load yaml success, yamlData:%p"
                        , _source.ToString().c_str(), yamlData.AsSelf());
                return;
            }
        }
    }
    
    // 如果是按照路径的, 那么根据路径来加载
    if (!config && (!_source.Path.empty()))
    {
        do
        {
            try
            {
                // 先检查是否变更x
                if (!_CheckFileChange())
                    break;
                
                config = new YAML::Node(YAML::LoadFile(_source.Path.c_str()));
            }
            catch (std::exception &e)
            {
                if (g_Log)
                    CLOG_ERROR("run load yaml fail source:%s, exception:%s"
                        , _source.ToString().c_str(), e.what());
            }
            catch (...)
            {
                if (g_Log)
                    CLOG_ERROR("run load yaml fail source:%s, unkonwn exception"
                        , _source.ToString().c_str());
            }
        
            // 日志加载配置成功
            if (config)
            {
                _ReplaceConfig(config);
                if (g_Log)
                    CLOG_INFO("source:%s, load yaml success"
                        , _source.ToString().c_str());
                return;
            }
        }
        while (false);
    }
}

void YamlFileDeserializer::_FirstRun()
{
    // 在FileChangeManager线程执行
    // 1. 根据sourceWrap加载数据
    
    // 配置来自内存
    KERNEL_NS::SmartPtr<YamlMemoryData, KERNEL_NS::AutoDelMethods::Release> memoryData;
    if (_source.FromMemory)
    {
        auto fromMemory = KERNEL_NS::KernelCastTo<YamlMemory>(_source.FromMemory);
        memoryData = fromMemory->CheckAndChange();
        // 内存数据不存在或者没变化,不可不存在, 因为是首次加载
        if (!memoryData)
        {
            if (g_Log)
            {
                CLOG_ERROR("use yaml memory data, but have no yaml memory data, source:%s"
                    ,_source.ToString().c_str());
            }
        }
    }
    
    // 初始化文件大小和时间
    if (!_source.Path.empty())
    {
        if(KERNEL_NS::FileUtil::IsFileExist(_source.Path.c_str()))
        {
            // 初始化文件大小
            _fileSize = KERNEL_NS::FileUtil::GetFileSizeEx(_source.Path.c_str());

            // 初始化文件时间
            _modifyTIme = KERNEL_NS::FileUtil::GetFileModifyTime(_source.Path.c_str());
        }
    }
    
    YAML::Node *config = NULL;
    try
    {
        // 内存有数据
        if (memoryData && !memoryData->_data.empty())
        {
            config = new YAML::Node(YAML::Load(memoryData->_data));
        }
    
        // 从路径加载
        else
        {
            // 内存的没有就使用path
            config = new YAML::Node(YAML::LoadFile(_source.Path.c_str()));
        }
    }
    catch (std::exception &e)
    {
        if (g_Log)
            CLOG_ERROR("path:%s, load yaml fail, exception:%s, FromMemory:%p, memoryData:%p"
                , _source.Path.c_str(), e.what(), _source.FromMemory, memoryData.AsSelf());
    }
    catch (...)
    {
        if (g_Log)
            CLOG_ERROR("unknown exception path:%s, load yaml fail, FromMemory:%p, memoryData:%p"
                , _source.Path.c_str(), _source.FromMemory, memoryData.AsSelf());
    }
    
    if (UNLIKELY(!config))
    {
        if (g_Log)
            CLOG_ERROR("config is null, load yaml fail, source:%s", _source.ToString().c_str());
    }
    
    // 更新新数据, 移除旧数据
    if(config)
        _ReplaceConfig(config);
}

void YamlFileDeserializer::_ReplaceConfig(YAML::Node *data)
{
    // 更新新数据, 移除旧数据
    if (auto oldData = _data.exchange(data, std::memory_order_acq_rel))
    {
        delete oldData;
    }
    
    _version.fetch_add(1, std::memory_order_release);
}

bool YamlFileDeserializer::_CheckFileChange()
{
    auto &path = _source.Path;
    if(!KERNEL_NS::FileUtil::IsFileExist(path.c_str()))
        return false;

    auto curSize = KERNEL_NS::FileUtil::GetFileSizeEx(path.c_str());
    if(curSize <= 0)
        return false;
                        
    auto curModifyTime = KERNEL_NS::FileUtil::GetFileModifyTime(path.c_str());
    if(!curModifyTime)
        return false;
                        
    if(curSize != _fileSize || curModifyTime != _modifyTIme)
    {
        _fileSize = curSize;
        _modifyTIme = curModifyTime;
        return true;
    }

    return false;
}




KERNEL_END
