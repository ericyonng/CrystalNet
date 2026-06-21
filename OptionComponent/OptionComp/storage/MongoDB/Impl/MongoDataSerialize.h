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
// Date: 2026-06-21 16:06:20
// Author: Eric Yonng
// Description:


#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_MONGO_DATA_SERIALIZE_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_MONGO_DATA_SERIALIZE_H__

#pragma once

#include <kernel/common/BaseMacro.h>
#include <bsoncxx/builder/basic/document.hpp>
#include <kernel/comp/LibString.h>
#include <bsoncxx/v_noabi/bsoncxx/types/bson_value/value.hpp>

KERNEL_BEGIN

struct MongoSerializeInfo;

class MongoDataSerialize
{
public:
    static bool AppendSerialize(bsoncxx::builder::basic::document &doc, const KERNEL_NS::LibString &keyName, const MongoSerializeInfo &data);
    static bool Deserialize(const bsoncxx::types::bson_value::view &bsonValue, MongoSerializeInfo &data);
    static Int32 GetSuitableSerializeType(const bsoncxx::v_noabi::type &bsonType);
};

KERNEL_END


#endif