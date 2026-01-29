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

KERNEL_BEGIN

// @param(ObjType): 反序列化最终的结果, 需要有:CreateNewObj/Release 接口
// @param(FileDeserializer): 反序列化器, 将文件反序列化成ObjType, 需要有SwapNewData/Register接口, 
// @param(FileDeserializerFactoryType): 反序列化器工厂, 需要有Create/Release接口, 
template<typename ObjType, typename FileDeserializerType>
#ifdef CRYSTAL_NET_CPP20
requires requires(ObjType obj, FileDeserializerType *d, KERNEL_NS::LibString path)
{
    ObjType::CreateNewObj(std::move(obj));
    obj.Release();

    d = FileDeserializerType::Create();
    d->Release();
    d->template SwapNewData<ObjType>();
    d->template Register<ObjType>(path);
    
}
#endif
class FileMonitor
{
    POOL_CREATE_TEMPLATE_OBJ_DEFAULT(FileMonitor, ObjType, FileDeserializerType);

public:
    FileMonitor()
        :_deserialize(NULL)
    {
        
    }
    
    ~FileMonitor()
    {
        CRYSTAL_RELEASE_SAFE(_deserialize);
    }

public:
    // 获取当前配置
    SmartPtr<ObjType, AutoDelMethods::Release> Current() const;
    // 初始化
    void Init(const KERNEL_NS::LibString &path);

    const LibString &GetPath() const;

private:
    mutable alignas(SYSTEM_ALIGN_SIZE) SmartPtr<ObjType, AutoDelMethods::Release> _currentObject;
    alignas(SYSTEM_ALIGN_SIZE) FileDeserializerType *_deserialize;

    alignas(SYSTEM_ALIGN_SIZE) LibString _filePath;
};

template<typename ObjType, typename FileDeserializerType>
#ifdef CRYSTAL_NET_CPP20
requires requires(ObjType obj, FileDeserializerType *d, KERNEL_NS::LibString path)
{
    ObjType::CreateNewObj(std::move(obj));
    obj.Release();

    d = FileDeserializerType::Create();
    d->Release();
    d->template SwapNewData<ObjType>();
    d->template Register<ObjType>(path);
    
}
#endif
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_IMPL(FileMonitor, ObjType, FileDeserializerType);

template<typename ObjType, typename FileDeserializerType>
#ifdef CRYSTAL_NET_CPP20
requires requires(ObjType obj, FileDeserializerType *d, KERNEL_NS::LibString path)
{
    ObjType::CreateNewObj(std::move(obj));
    obj.Release();

    d = FileDeserializerType::Create();
    d->Release();
    d->template SwapNewData<ObjType>();
    d->template Register<ObjType>(path);
    
}
#endif
ALWAYS_INLINE SmartPtr<ObjType, AutoDelMethods::Release> FileMonitor<ObjType, FileDeserializerType>::Current() const
{
    // 如果配置有变更, 则更新配置
    ObjType *newObj = _deserialize->template SwapNewData<ObjType>();
    if(UNLIKELY((newObj != NULL) && (newObj != _currentObject.AsSelf())))
    {
        _currentObject = newObj;
    }

    return _currentObject;
}

template<typename ObjType, typename FileDeserializerType>
#ifdef CRYSTAL_NET_CPP20
requires requires(ObjType obj, FileDeserializerType *d, KERNEL_NS::LibString path)
{
    ObjType::CreateNewObj(std::move(obj));
    obj.Release();

    d = FileDeserializerType::Create();
    d->Release();
    d->template SwapNewData<ObjType>();
    d->template Register<ObjType>(path);
    
}
#endif
ALWAYS_INLINE void FileMonitor<ObjType, FileDeserializerType>::Init(const KERNEL_NS::LibString &path)
{
    _filePath = path;
    _deserialize = FileDeserializerType::Create();
    _currentObject = _deserialize->template Register<ObjType>(_filePath);
}

KERNEL_END

#endif

