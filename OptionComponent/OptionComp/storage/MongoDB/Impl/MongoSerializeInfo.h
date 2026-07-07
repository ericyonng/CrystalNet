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
// Date: 2026-06-18 11:40:04
// Author: Eric Yonng
// Description:


#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_MONGO_SERIALIZE_INFO_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_MONGO_SERIALIZE_INFO_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <OptionComp/storage/MongoDB/Impl/MongoSerializeInfoType.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/LibStream.h>

KERNEL_BEGIN

// 序列化方法: LibStream中存的是什么数据
struct MongoSerializeInfo
{
    POOL_CREATE_OBJ_DEFAULT(MongoSerializeInfo);

    MongoSerializeInfo(Int32 dataType, LibStream<_Build::TL> *stream)
        :DataType(dataType)
        ,_stream(stream)
    {
        
    }
    // MongoSerializeInfoType LibStream中存的是什么数据, 从db查询回来, 这个类型作为写入stream的数据类型, 作为什么数据类型写入
    Int32 DataType = MongoSerializeInfoType::JSON;

    LibStream<_Build::TL> *_stream = NULL;
};

struct SmartMongoSerializeInfoWrapper
{
    SmartMongoSerializeInfoWrapper()
        :Ptr(new std::map<KERNEL_NS::LibString, KERNEL_NS::MongoSerializeInfo>())
    {
        Ptr.SetClosureDelegate([](void *p)
        {
            auto ptr = KERNEL_NS::KernelCastTo<std::map<KERNEL_NS::LibString, KERNEL_NS::MongoSerializeInfo>>(p);
            ContainerUtil::DelContainer(*ptr, [](const KERNEL_NS::MongoSerializeInfo &info)
            {
                KERNEL_NS::LibStreamTL::DeleteThreadLocal_LibStream(info._stream);
            });
            delete ptr;
        });
    }

    SmartMongoSerializeInfoWrapper(std::map<KERNEL_NS::LibString, KERNEL_NS::MongoSerializeInfo> *dict)
    :Ptr(dict)
    {
        Ptr.SetClosureDelegate([](void *p)
        {
            auto ptr = KERNEL_NS::KernelCastTo<std::map<KERNEL_NS::LibString, KERNEL_NS::MongoSerializeInfo>>(p);
            ContainerUtil::DelContainer(*ptr, [](const KERNEL_NS::MongoSerializeInfo &info)
            {
                KERNEL_NS::LibStreamTL::DeleteThreadLocal_LibStream(info._stream);
            });
            delete ptr;
        });
    }
    ~SmartMongoSerializeInfoWrapper() = default;
    
    KERNEL_NS::SmartPtr<std::map<KERNEL_NS::LibString, KERNEL_NS::MongoSerializeInfo>, KERNEL_NS::AutoDelMethods::CustomDelete> Ptr;
};

KERNEL_END

#endif