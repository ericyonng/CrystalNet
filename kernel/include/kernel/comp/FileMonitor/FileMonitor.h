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
// Date: 2026-01-09 00:01:46
// Author: Eric Yonng
// Description:

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_MONITOR_FILE_MONITOR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_MONITOR_FILE_MONITOR_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/SmartPtr.h>
#include <atomic>
#include <kernel/comp/FileMonitor/FileMonitorConcepts.h>
#include <unordered_map>

KERNEL_BEGIN

class YamlMemory;

// @param(ObjType): 反序列化最终的结果, 需要有:CreateNewObj/Release 接口
// @param(FileDeserializer): 反序列化器, 将文件反序列化成ObjType, 需要有SwapNewData/Register接口, 
// @param(FileDeserializerFactoryType): 反序列化器工厂, 需要有Create/Release接口, 
template<typename ObjType, typename FileDeserializerType>
#ifdef CRYSTAL_NET_CPP20
requires FileMonitorConcept<ObjType, FileDeserializerType>
#endif
class FileMonitor
{
    POOL_CREATE_TEMPLATE_OBJ_DEFAULT(FileMonitor, ObjType, FileDeserializerType)

public:
    FileMonitor()
    {
        
    }
    
    ~FileMonitor()
    {
    }

public:
    // 获取当前配置,注意如果是多线程环境Current则不安全 TODO:
    SmartPtr<ObjType, AutoDelMethods::Release> Current() const;
    // 初始化
    bool Init(const KERNEL_NS::LibString &path, YamlMemory *fromMemory);

    const LibString &GetPath() const;

    // 对于多线程访问, 应该对_currentObject/_deserialize放在thread_load级别, 这样避免了多线程竞争问题
private:
    // 多线程访问
    DEF_STATIC_THREAD_LOCAL_DECLEAR alignas(SYSTEM_ALIGN_SIZE) std::unordered_map<void *, std::pair<SmartPtr<ObjType, AutoDelMethods::Release>, FileDeserializerType *>> * s_FileMonitorInstRefCurrentObjAndDeserialize = NULL;

    alignas(SYSTEM_ALIGN_SIZE) LibString _filePath;

    // 共享的一块内存配置, FileMonitor不能操作, 由FileChangeManager检查是否内容变化
    alignas(SYSTEM_ALIGN_SIZE) YamlMemory *_fromMemory;
};

template<typename ObjType, typename FileDeserializerType>
#ifdef CRYSTAL_NET_CPP20
requires FileMonitorConcept<ObjType, FileDeserializerType>
#endif
ALWAYS_INLINE SmartPtr<ObjType, AutoDelMethods::Release> FileMonitor<ObjType, FileDeserializerType>::Current() const
{
    if(UNLIKELY(s_FileMonitorInstRefCurrentObjAndDeserialize == NULL))
    {
        s_FileMonitorInstRefCurrentObjAndDeserialize = new std::unordered_map<void *, std::pair<SmartPtr<ObjType, AutoDelMethods::Release>, FileDeserializerType *>>();
    }

    auto iter = s_FileMonitorInstRefCurrentObjAndDeserialize->find(this);
    if(UNLIKELY(iter == s_FileMonitorInstRefCurrentObjAndDeserialize->end()))
    {
        auto deserializeObj = FileDeserializerType::Create();
        iter = s_FileMonitorInstRefCurrentObjAndDeserialize->insert(std::make_pair(this, std::pair<SmartPtr<ObjType, AutoDelMethods::Release>, FileDeserializerType *>( deserializeObj,
            deserializeObj->template Register<ObjType>(_filePath, _fromMemory)
        ))).first;
    }

    auto &currentAndDeserialize = iter->second;
    
    // 如果配置有变更, 则更新配置
    ObjType *newObj = currentAndDeserialize.second->template SwapNewData<ObjType>();
    if(UNLIKELY((newObj != NULL) && (newObj != currentAndDeserialize.first.AsSelf())))
    {
        currentAndDeserialize.first = newObj;
    }

    return currentAndDeserialize.first;
}

template<typename ObjType, typename FileDeserializerType>
#ifdef CRYSTAL_NET_CPP20
requires FileMonitorConcept<ObjType, FileDeserializerType>
#endif
ALWAYS_INLINE bool FileMonitor<ObjType, FileDeserializerType>::Init(const KERNEL_NS::LibString &path, YamlMemory *fromMemory)
{
    _filePath = path;
    _fromMemory = fromMemory;

    return Current();
}

KERNEL_END

#endif

