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
// Date: 2026-02-02 00:01:46
// Author: Eric Yonng
// Description:

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_MONITOR_FILE_MONITOR_CONCEPTS_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_MONITOR_FILE_MONITOR_CONCEPTS_H__

#pragma once

#if CRYSTAL_NET_CPP20

#include <kernel/kernel_export.h>
#include <kernel/common/BaseType.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class YamlMemory;

// @param(ObjType): 反序列化最终的结果, 需要有:CreateNewObj/Release 接口
// @param(FileDeserializerType): 反序列化器, 将文件反序列化成ObjType, 需要有Create/Release/SwapNewData/Register接口, 
template<typename ObjType, typename FileDeserializerType>
concept FileMonitorConcept = requires(ObjType obj, FileDeserializerType *d, KERNEL_NS::LibString path, YamlMemory *fromMemory)
{
    ObjType::CreateNewObj(std::move(obj));
    obj.Release();

    d = FileDeserializerType::Create();
    d->Release();
    d->template SwapNewData<ObjType>();
    d->template Register<ObjType>(path, fromMemory);
};

KERNEL_END

#endif

#endif