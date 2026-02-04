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

KERNEL_BEGIN

YamlDeserializer::YamlDeserializer()
    :_handle(NULL)
{
    
}

YamlDeserializer::~YamlDeserializer()
{
    // YamlDeserializer结束后就不监听了
    if(_handle)
    {
        _handle->_notListen.store(true, std::memory_order_release);
    }
}

void YamlDeserializer::Release()
{
    delete this;
}

YamlDeserializer *YamlDeserializer::Create()
{
    return new YamlDeserializer();
}

void *YamlDeserializer::_Register(const LibString &dataName,  IDelegate<void, void *> * releaseObj, IDelegate<void *, YAML::Node *> *deserializeObj, YamlMemory *fromMemory)
{
    auto poller = g_FileChangeManager->GetPoller();
    auto fileChangeManager = g_FileChangeManager;
    void *obj = NULL;
    std::atomic_bool isFinish = {false};
    void *registerKey = this;
    auto path = _path;
    // 不用担心投递到poller线程后由于没执行导致delegate泄露, 因为如果没执行那么程序应该处于关闭状态, 无所谓内存泄露
    poller->Push([this, dataName, registerKey, releaseObj, deserializeObj, &isFinish, &obj, path, fileChangeManager, fromMemory]()
    {
        KERNEL_NS::SmartPtr<IDelegate<void *, YAML::Node *>, KERNEL_NS::AutoDelMethods::Release> deserializePtr(deserializeObj);
        KERNEL_NS::SmartPtr<IDelegate<void, void *>, KERNEL_NS::AutoDelMethods::Release> releaseObjPtr(releaseObj);

        // 注册monitorInfo
        auto &filePathRefFileObj = fileChangeManager->GetFilePathRefFileObj();
        auto iter = filePathRefFileObj.find(path);
        FileMonitorInfo *monitorInfo = NULL;
        YAML::Node* config;

        if(iter == filePathRefFileObj.end())
        {
            auto loadNewObjLamb = [path](void *fromMemoryData) mutable  -> void *
            {
                YAML::Node *config = NULL;
                // fromMemoryData 加载后就得释放, 已经脱离生命周期，这里用智能指针管理
                KERNEL_NS::SmartPtr<YamlMemoryData, KERNEL_NS::AutoDelMethods::Release> fromMemoryCache(KERNEL_NS::KernelCastTo<YamlMemoryData>(fromMemoryData));
                try
                {
                    // 优先使用内存的
                    if (fromMemoryData)
                    {
                        if (fromMemoryCache && (!fromMemoryCache->_data.empty()))
                        {
                            config = new YAML::Node(YAML::Load(fromMemoryCache->_data.GetRaw()));
                        }
                    }

                    // 内存的没有就使用path
                    if (!config)
                        config = new YAML::Node(YAML::LoadFile(path.c_str()));
                }
                catch (std::exception &e)
                {
                    if (g_Log)
                        g_Log->Error(LOGFMT_NON_OBJ_TAG(YamlDeserializer, "path:%s, load yaml fail, exception:%s, fromMemoryData:%p"), path.c_str(), e.what(), fromMemoryData);
                }
                catch (...)
                {                    
                    if (g_Log)
                        g_Log->Error(LOGFMT_NON_OBJ_TAG(YamlDeserializer, "path:%s, load yaml fail, fromMemoryData:%p"), path.c_str(), fromMemoryData);
                }

                return config;
            };

            // 内存yaml(交换出memoryData)
            YamlMemoryData *memoryData = NULL;
            if (fromMemory)
            {
                memoryData = fromMemory->CheckAndChange();
                if (!memoryData)
                {
                    if (g_Log && g_Log->IsEnable(LogLevel::Error))
                    {
                        g_Log->Error(LOGFMT_NON_OBJ_TAG(YamlDeserializer, "use yaml memory data, but have no yaml memory data, from memory:%p, dataName:%s, path:%s")
                            , fromMemory, dataName.c_str(), path.c_str());
                    }
                }
            }

            // memoryData在loadNewObjLamb中释放
            config = KERNEL_NS::KernelCastTo<YAML::Node>(loadNewObjLamb(memoryData));
            if(config)
            {
                monitorInfo = new FileMonitorInfo();
                monitorInfo->_path = path;
                monitorInfo->_sourceObj = config;

                // 来自内存
                monitorInfo->_fromMemory =  fromMemory;

                // 加载新的配置
                monitorInfo->_loadNewObj = KERNEL_CREATE_CLOSURE_DELEGATE(loadNewObjLamb, void *, void *);

                // 处理文件变化
                {
                    // 内存yaml
                    if (monitorInfo->_fromMemory)
                    {
                        // 检查文件是否变化回调
                        auto ckeckChange = [monitorInfo](void *&outFromMemoryData) mutable -> bool
                        {
                            if (!monitorInfo->_fromMemory)
                                return false;
                            
                            auto yamlMemory = KERNEL_NS::KernelCastTo<YamlMemory>(monitorInfo->_fromMemory);
                            outFromMemoryData = yamlMemory->CheckAndChange();

                            return outFromMemoryData != NULL;
                        };
                
                        monitorInfo->_checkChange = KERNEL_CREATE_CLOSURE_DELEGATE(ckeckChange, bool, void *&);
                    }
                    else
                    {
                        KERNEL_NS::SmartPtr<Int64> fileSize(new Int64);
                        KERNEL_NS::SmartPtr<KERNEL_NS::LibTime> modifyTime(new KERNEL_NS::LibTime());

                        *fileSize = 0;
                        if(KERNEL_NS::FileUtil::IsFileExist(path.c_str()))
                        {
                            // 初始化文件大小
                            *fileSize = KERNEL_NS::FileUtil::GetFileSizeEx(path.c_str());

                            // 初始化文件时间
                            *modifyTime = KERNEL_NS::FileUtil::GetFileModifyTime(path.c_str());
                        }

                        // 检查文件是否变化回调
                        auto ckeckChange = [path, fileSize, modifyTime](void *&) mutable -> bool
                        {
                            if(!KERNEL_NS::FileUtil::IsFileExist(path.c_str()))
                                return false;

                            auto curSize = KERNEL_NS::FileUtil::GetFileSizeEx(path.c_str());
                            if(curSize <= 0)
                                return false;
                        
                            auto curModifyTime = KERNEL_NS::FileUtil::GetFileModifyTime(path.c_str());
                            if(!curModifyTime)
                                return false;
                        
                            if(curSize != *fileSize || curModifyTime != *modifyTime)
                            {
                                *fileSize = curSize;
                                *modifyTime = curModifyTime;
                                return true;
                            }

                            return false;
                        };
                
                        monitorInfo->_checkChange = KERNEL_CREATE_CLOSURE_DELEGATE(ckeckChange, bool, void *&);
                    }
                }

                // 释放config
                {
                    auto releaseConfig = [](void *obj)
                    {
                        auto yamlConfig  = KERNEL_NS::KernelCastTo<YAML::Node>(obj);
                        delete yamlConfig;
                    };
                    monitorInfo->_releaseObj = KERNEL_CREATE_CLOSURE_DELEGATE(releaseConfig, void, void *);
                }

                iter = filePathRefFileObj.insert(std::make_pair(path, monitorInfo)).first;
            }
        }
        else
        {
            monitorInfo = iter->second;
            config = KERNEL_NS::KernelCastTo<YAML::Node>(monitorInfo->_sourceObj);
        }

        // 注册handle并反序列化
        if(monitorInfo != NULL)
        {
            auto iterHandle = monitorInfo->_keyRefFileChangeHandle.find(registerKey);
            if(iterHandle == monitorInfo->_keyRefFileChangeHandle.end())
            {
                auto handle = new FileChangeHandle();
                auto deserializeLamb = [deserializePtr](void *config)->void *
                {
                    return deserializePtr->Invoke(KERNEL_NS::KernelCastTo<YAML::Node>(config));
                };
                handle->_deserialize = KERNEL_CREATE_CLOSURE_DELEGATE(deserializeLamb, void *, void *);

                auto releaseLamb = [releaseObjPtr](void *ptr)
                {
                    releaseObjPtr->Invoke(ptr);
                };
                handle->_release = KERNEL_CREATE_CLOSURE_DELEGATE(releaseLamb, void, void *);
                handle->_dataName = dataName;
                
                iterHandle = monitorInfo->_keyRefFileChangeHandle.insert(std::make_pair(registerKey, handle)).first;
            }

            // 注册需要持有handle, 以便及时的更新配置
            _handle = iterHandle->second;

            // 反序列化
            if(config)
            {
                try
                {
                    obj = iterHandle->second->_deserialize->Invoke(config);
                }
                catch (std::exception &e)
                {
                    if (g_Log)
                        g_Log->Error(LOGFMT_NON_OBJ_TAG(YamlDeserializer, "path:%s, deserialize yaml fail, exception:%s, deserializeObj:%s")
                        , path.c_str(), e.what(), KERNEL_NS::RttiUtil::GetByObj(iterHandle->second->_deserialize).c_str());
                }
                catch (...)
                {
                    if (g_Log)
                        g_Log->Error(LOGFMT_NON_OBJ_TAG(YamlDeserializer, "path:%s, deserialize yaml fail, deserializeObj:%s")
                        , path.c_str(), KERNEL_NS::RttiUtil::GetByObj(iterHandle->second->_deserialize).c_str());
                }
            }
          
        }

        isFinish.store(true, std::memory_order_release);
    });

    // 等待完成
    while (!isFinish.load(std::memory_order_acquire))
    {
        KERNEL_NS::SystemUtil::ThreadSleep(2);
    }

    if(!obj)
    {
        if (g_Log)
            g_Log->Error(LOGFMT_OBJ_TAG("register yaml fail path:%s"), _path.c_str());
        return NULL;
    }

    return obj;
}



KERNEL_END