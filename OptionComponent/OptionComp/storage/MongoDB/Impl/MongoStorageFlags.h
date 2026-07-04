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
// Date: 2026-07-01 02:07:06
// Author: Eric Yonng
// Description:

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_MONGO_STORAGE_FLAGS_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_MONGO_STORAGE_FLAGS_H__

#pragma once

#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>

KERNEL_BEGIN

class MongoStorageFlags
{
private:
    enum POS_ENUMS : UInt64
    {
        UNKNOWN = 0,
        AS_FIELD_FLAG_POS,
        KEY_VALUE_SYSTEM_FLAG_POS,
        MULTI_FIELD_SYSTEM_FLAG_POS,
    };

public:

    // flag标志位掩码
    enum FLAGS_MASK : UInt64
    {
        NO_FLAGS = 0,

        // 作为其他系统的一个字段
        AS_FIELD_FLAG = 1LLU << AS_FIELD_FLAG_POS,
        // 多字段系统, 包括单字段, 两字段或者以上
        MULTI_FIELD_SYSTEM_FLAG = 1LLU << MULTI_FIELD_SYSTEM_FLAG_POS,
        // key - value 系统 多字段系统的一个特例
        KEY_VALUE_SYSTEM_FLAG = MULTI_FIELD_SYSTEM_FLAG | (1LLU << KEY_VALUE_SYSTEM_FLAG_POS),
    };

    static bool IsKvSystem(UInt64 flags);
    static bool IsMultiSystem(UInt64 flags);
    static bool IsFieldSystem(UInt64 flags);
    static UInt64 AsKvSystem(UInt64 flags);
    static UInt64 AsMultiSystem(UInt64 flags);
    static UInt64 AsFieldSystem(UInt64 flags);
};

ALWAYS_INLINE bool MongoStorageFlags::IsKvSystem(UInt64 flags)
{
    return (flags & KEY_VALUE_SYSTEM_FLAG) == KEY_VALUE_SYSTEM_FLAG;
}

ALWAYS_INLINE bool MongoStorageFlags::IsMultiSystem(UInt64 flags)
{
    return (flags & MULTI_FIELD_SYSTEM_FLAG) == MULTI_FIELD_SYSTEM_FLAG;
}

ALWAYS_INLINE bool MongoStorageFlags::IsFieldSystem(UInt64 flags)
{
    return (flags & AS_FIELD_FLAG) == AS_FIELD_FLAG;
}

ALWAYS_INLINE UInt64 MongoStorageFlags::AsKvSystem(UInt64 flags)
{
    return flags | KEY_VALUE_SYSTEM_FLAG;
}

ALWAYS_INLINE UInt64 MongoStorageFlags::AsMultiSystem(UInt64 flags)
{
    return flags | MULTI_FIELD_SYSTEM_FLAG;
}

ALWAYS_INLINE UInt64 MongoStorageFlags::AsFieldSystem(UInt64 flags)
{
    return flags | AS_FIELD_FLAG;
}


KERNEL_END

#endif