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
// Date: 2026-05-18 11:10:18
// Author: Eric Yonng
// Description:


#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_SHARD_KEY_INFO_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_SHARD_KEY_INFO_H__

#pragma once

#include <kernel/comp/LibString.h>
#include <kernel/comp/memory/ObjPoolMacro.h>

KERNEL_BEGIN

// shard key 分片键的类型(比如hashed, 字符串类型, 1（升序）数值类型等)
class ShardKeyType
{
public:
    enum ENUMS
    {
        HASHED = 0,
        ASC,
        DESC,
    };
};

struct ShardKeyInfo
{
    KERNEL_NS::LibString KeyName;

    ShardKeyType::ENUMS ValueType;

    KERNEL_NS::LibString ToString() const
    {
        return KERNEL_NS::LibString().AppendFormat("%s, %d", KeyName.c_str(), ValueType);
    }
};

KERNEL_END

#endif