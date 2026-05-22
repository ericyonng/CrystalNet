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
// Date: 2026-05-17 16:05:31
// Author: Eric Yonng
// Description:


#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_MONGODB_CONNECTION_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_MONGODB_CONNECTION_H__

#pragma once

#include <kernel/common/macro.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <mongocxx/pool.hpp>
#include <kernel/comp/LibString.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Coroutines/CoTask.h>
#include <OptionComp/storage/MongoDB/Impl/ShardKeyInfo.h>

KERNEL_BEGIN

struct ShardingLock;

class MongodbConnection
{
    POOL_CREATE_OBJ_DEFAULT(MongodbConnection);

public:
    MongodbConnection(mongocxx::pool::entry& entry);
    ~MongodbConnection();

    // 数据库分片
    KERNEL_NS::CoTask<bool> EnableDatabaseSharding(KERNEL_NS::LibString dbName);

    // 尝试获取分布式锁 (使用 findOneAndUpdate + upsert 原子操作)
    // 逻辑: 如果锁不存在则创建; 如果锁存在但已过期则更新; 如果锁存在且未过期则失败
    KERNEL_NS::SmartPtr<ShardingLock, KERNEL_NS::AutoDelMethods::CustomDelete> TryAcquireLock(const KERNEL_NS::LibString &lockName, const KERNEL_NS::LibString &ownerId);
    
    // 释放分布式锁
    void ReleaseLock(KERNEL_NS::SmartPtr<ShardingLock, KERNEL_NS::AutoDelMethods::CustomDelete> &lock);

    // 设置分片键
    KERNEL_NS::CoTask<bool> ShardCollection(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collName, std::vector<ShardKeyInfo> shardKeys, Int32 numChunks = 1024);
    
private:
    bool _CheckDatabaseSharded(const KERNEL_NS::LibString &dbName);
    bool _CheckCollectionSharded(const KERNEL_NS::LibString &dbName, const KERNEL_NS::LibString &collName);

private:
    mongocxx::pool::entry& _entry;
};

KERNEL_END

#endif