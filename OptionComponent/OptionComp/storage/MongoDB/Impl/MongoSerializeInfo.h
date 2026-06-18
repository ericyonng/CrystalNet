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

KERNEL_BEGIN

class MongoSerializeInfoType
{
public:
    enum ENUMS
    {
        JSON = 0,
        BINARY = 1,
    };
};

// 序列化方法: LibStream中存的是什么数据
struct MongoSerializeInfo
{
    POOL_CREATE_OBJ_DEFAULT(MongoSerializeInfo);
    
    // MongoSerializeInfoType LibStream中存的是什么数据, 从db查询回来, 这个类型作为写入stream的数据类型, 作为什么数据类型写入
    Int32 DataType = MongoSerializeInfoType::JSON;

    LibStream<_Build::TL> *_stream = NULL;
};

KERNEL_END

#endif