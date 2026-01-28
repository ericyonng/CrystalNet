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

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(YamlDeserializer);

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
    YamlDeserializer::DeleteThreadLocal_YamlDeserializer(this);
}


void *YamlDeserializer::_Register(const LibString &dataName,  IDelegate<void, void *> * releaseObj, IDelegate<void *, YAML::Node *> *deserializeObj)
{
    auto poller = g_FileChangeManager->GetPoller();
    auto fileChangeManager = g_FileChangeManager;
    void *obj = NULL;
    std::atomic_bool isFinish = {false};
    void *registerKey = this;
    auto path = _path;
    // 不用担心投递到poller线程后由于没执行导致delegate泄露, 因为如果没执行那么程序应该处于关闭状态, 无所谓内存泄露
    poller->Push([this, dataName, registerKey, releaseObj, deserializeObj, &isFinish, &obj, path, fileChangeManager]()
    {
        KERNEL_NS::SmartPtr<IDelegate<void *, YAML::Node *>, KERNEL_NS::AutoDelMethods::Release> deserializePtr(deserializeObj);
        KERNEL_NS::SmartPtr<IDelegate<void, void *>, KERNEL_NS::AutoDelMethods::Release> releaseObjPtr(releaseObj);

        // 注册monitorInfo
        auto &filePathRefFileObj = fileChangeManager->GetFilePathRefFileObj();
        auto iter = filePathRefFileObj.find(path);
        FileMonitorInfo *monitorInfo = NULL;
        KERNEL_NS::SmartPtr<YAML::Node> config;

        if(iter == filePathRefFileObj.end())
        {
            auto loadNewObjLamb = [path]()-> void *
            {
                YAML::Node *config = NULL;
                try
                {
                    config = new YAML::Node(YAML::LoadFile(path.c_str()));
                }
                catch (std::exception &e)
                {
                    g_Log->Error(LOGFMT_NON_OBJ_TAG(YamlDeserializer, "path:%s, load yaml fail, exception:%s"), path.c_str(), e.what());
                }
                catch (...)
                {
                    g_Log->Error(LOGFMT_NON_OBJ_TAG(YamlDeserializer, "path:%s, load yaml fail"), path.c_str());
                }

                return config;
            };

            config = KERNEL_NS::KernelCastTo<YAML::Node>(loadNewObjLamb());
            if(config)
            {
                monitorInfo = FileMonitorInfo::New_FileMonitorInfo();
                monitorInfo->_path = path;

                // 加载新的配置
                monitorInfo->_loadNewObj = KERNEL_CREATE_CLOSURE_DELEGATE(loadNewObjLamb, void *);

                // 处理文件变化
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
                    auto ckeckChange = [path, fileSize, modifyTime]() mutable -> bool
                    {
                        if(!KERNEL_NS::FileUtil::IsFileExist(path.c_str()))
                            return false;

                        auto curSize = KERNEL_NS::FileUtil::GetFileSizeEx(path.c_str());
                        auto curModifyTime = KERNEL_NS::FileUtil::GetFileModifyTime(path.c_str());
                        if(curSize != *fileSize || curModifyTime != *modifyTime)
                        {
                            *fileSize = curSize;
                            *modifyTime = curModifyTime;
                            return true;
                        }

                        return false;
                    };
                
                    monitorInfo->_checkChange = KERNEL_CREATE_CLOSURE_DELEGATE(ckeckChange, bool);
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
        }

        // 注册handle并反序列化
        if(monitorInfo != NULL)
        {
            auto iterHandle = monitorInfo->_keyRefFileChangeHandle.find(registerKey);
            if(iterHandle == monitorInfo->_keyRefFileChangeHandle.end())
            {
                auto handle = FileChangeHandle::New_FileChangeHandle();
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
                    obj = iterHandle->second->_deserialize->Invoke(config.AsSelf());
                }
                catch (std::exception &e)
                {
                    g_Log->Error(LOGFMT_NON_OBJ_TAG(YamlDeserializer, "path:%s, deserialize yaml fail, exception:%s, deserializeObj:%s")
                        , path.c_str(), e.what(), KERNEL_NS::RttiUtil::GetByObj(iterHandle->second->_deserialize).c_str());
                }
                catch (...)
                {
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
        KERNEL_NS::SystemUtil::ThreadSleep(5);
    }

    if(!obj)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("register yaml fail path:%s"), _path.c_str());
        return NULL;
    }

    return obj;
}



KERNEL_END