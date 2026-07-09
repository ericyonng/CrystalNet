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
// Date: 2026-07-09 23:07:34
// Author: Eric Yonng
// Description:

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_MONGODB_HELPER_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_MONGODB_HELPER_H__

#pragma once

#include <kernel/common/BaseMacro.h>

#include "mongocxx/write_concern.hpp"

KERNEL_BEGIN

class MongoHelper
{
public:
    // 写大多数节点完成
    template<typename  WriteConcernType>
    static WriteConcernType MakeMongoMajorityWriteConcern();

    // rc级别读
    template<typename ReadPreferenceType, typename ReadConcernType, typename CollectionType>
    static void MakeRCReadOption(CollectionType &collection);
};

template<typename  WriteConcernType>
ALWAYS_INLINE WriteConcernType MongoHelper::MakeMongoMajorityWriteConcern()
{
    WriteConcernType concern;
    // 大多数节点成功后成功
    concern.acknowledge_level(WriteConcernType::level::k_majority);
    // 写操作落盘后成功
    concern.journal(true);
    return concern;
}

template<typename ReadPreferenceType, typename ReadConcernType, typename CollectionType>
ALWAYS_INLINE void MongoHelper::MakeRCReadOption(CollectionType &collection)
{
    // 优先从节点读
    ReadPreferenceType rp;
    rp.mode(ReadPreferenceType::read_mode::k_secondary_preferred);
    collection.read_preference(rp);
            
    // 设置majority, 常用的隔离级别,相当于读已提交级别(rc级别) 解决事务的隔离性问题
    ReadConcernType rc;
    rc.acknowledge_level(ReadConcernType::level::k_majority);
    collection.read_concern(rc);
}

KERNEL_END

#endif

