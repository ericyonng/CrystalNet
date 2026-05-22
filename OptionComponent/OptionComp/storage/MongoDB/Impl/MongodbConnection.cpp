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
// Date: 2026-05-17 20:05:33
// Author: Eric Yonng
// Description:

#include <pch.h>
#include <thread>
#include <mongocxx/exception/exception.hpp>
#include <kernel/comp/Utils/GuidUtil.h>
#include <OptionComp/storage/MongoDB/Impl/MongodbConnection.h>
#include <OptionComp/storage/MongoDB/Impl/ShardingLock.h>
#include <kernel/comp/LibTime.h>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>

#include "testsuit/testinst/TestMongo.h"


KERNEL_BEGIN
    static constexpr Int32 MONGODB_LOCK_EXPIRE_SECONDS = 30;

MongodbConnection::MongodbConnection(mongocxx::pool::entry& entry)
    :_entry(entry)
{
    
}

MongodbConnection::~MongodbConnection()
{
    
}

KERNEL_NS::CoTask<bool> MongodbConnection::EnableDatabaseSharding(KERNEL_NS::LibString dbName)
{
    // 生成唯一锁 ID (使用 UUID)
    auto &&lockOwner = KERNEL_NS::GuidUtil::GenStr();
    KERNEL_NS::LibString &&lockName = "db_sharding_" + dbName;
    
    try
    {
        // 最多尝试获取锁3次
        for(Int32 idx = 0; idx < 3; ++idx)
        {
            // 先检查是否已启用
            if (_CheckDatabaseSharded(dbName))
            {
                CLOG_DEBUG("database %s already sharded, skip", dbName.c_str());
                co_return true;
            }
            
            // 尝试获取分布式锁
            auto lock = TryAcquireLock(lockName, lockOwner);
            if (!lock->Acquired)
            {
                // 等待锁释放后再次检查状态
                CLOG_WARN("waiting for lock %s, will check sharding status after timeout dbName:%s", lockName.c_str(), dbName.c_str());

                // 再次检查分片状态(锁可能已超时自动释放)
                co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(MONGODB_LOCK_EXPIRE_SECONDS + 1));
                continue;
            }
            
            // 获取锁成功，再次检查是否已被其他进程启用
            if (_CheckDatabaseSharded(dbName))
            {
                CLOG_DEBUG("database %s already sharded (checked after lock), skip", dbName.c_str());
                ReleaseLock( lock);
                co_return true;
            }
            
            // 通过 admin 数据库执行 enableSharding 命令
            auto adminDb = _entry["admin"];
            auto cmd = bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp("enableSharding", dbName.GetRaw())
            );
            
            auto result = adminDb.run_command(cmd.view());
            auto resultView = result.view();
            
            // 释放锁
            ReleaseLock(lock);
            
            // 检查命令执行结果
            auto okIter = resultView.find("ok");
            if (okIter != resultView.end() && okIter->get_double() > 0)
            {
                CLOG_INFO("enableSharding %s success", dbName.c_str());
                co_return true;
            }

            CLOG_ERROR("enableSharding %s failed:%s", dbName.c_str(), bsoncxx::to_json(resultView).c_str());
            
            co_return false;
        }

        CLOG_WARN("TryAcquireLock enableSharding fail dbName:%s", dbName.c_str());
    }
    catch (const mongocxx::exception &e)
    {
        CLOG_ERROR("EnableDatabaseSharding failed:%s, dbName:%s", e.code().message().c_str(), dbName.c_str());
        co_return false;
    }
    catch (const std::exception &e)
    {
        CLOG_ERROR("EnableDatabaseSharding std exception failed:%s, dbName:%s", e.what(), dbName.c_str());
        co_return false;
    }
    catch (...)
    {
        CLOG_ERROR("EnableDatabaseSharding unkown dbName:%s", dbName.c_str());
        co_return false;
    }

    co_return false;
}

KERNEL_NS::SmartPtr<ShardingLock, KERNEL_NS::AutoDelMethods::CustomDelete> MongodbConnection::TryAcquireLock(const KERNEL_NS::LibString &lockName, const KERNEL_NS::LibString &ownerId)
{
    KERNEL_NS::SmartPtr<ShardingLock, KERNEL_NS::AutoDelMethods::CustomDelete> lock = ShardingLock::NewThreadLocal_ShardingLock();
    lock->LockId = ownerId;
    lock->LockCollection = lockName;
    lock.SetClosureDelegate([](void *p)
    {
        ShardingLock::DeleteThreadLocal_ShardingLock(KERNEL_NS::KernelCastTo<ShardingLock>(p));
    });
    
    try
    {
        auto locksDb = _entry["config"]["_sharding_locks"];
        auto &&now = LibTime::Now();
        auto &&expireTime = now + KERNEL_NS::TimeSlice::FromSeconds(30);
        
        // 原子性获取锁的 filter:
        // 1. expireAt 字段不存在 (锁不存在)
        // 2. expireAt < now (锁已过期)
        bsoncxx::builder::basic::document filterDoc;
        filterDoc.append(bsoncxx::builder::basic::kvp("_id", lockName.GetRaw()));
        filterDoc.append(bsoncxx::builder::basic::kvp("$or", bsoncxx::builder::basic::make_array(
            bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("expireAt", bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp("$exists", false)
            ))),
            bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("expireAt", bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp("$lt", now.GetMilliTimestamp())  // 已过期
            )))
        )));
        
        // 更新操作: 设置 owner 和新的过期时间
        bsoncxx::builder::basic::document updateDoc;
        updateDoc.append(bsoncxx::builder::basic::kvp("owner", ownerId.GetRaw()));
        updateDoc.append(bsoncxx::builder::basic::kvp("expireAt", expireTime.GetMilliTimestamp()));
        
        mongocxx::options::find_one_and_update options;
        options.upsert(true);
        options.return_document(mongocxx::options::return_document::k_after);
        
        auto result = locksDb.find_one_and_update(filterDoc.view(), updateDoc.view(), options);
        
        if (result)
        {
            auto doc = result->view();
            auto ownerIter = doc.find("owner");
            
            // 如果 owner 是我们设置的，说明获取成功
            if (ownerIter != doc.end())
            {
                std::string setOwner = ownerIter->get_string().value.data();
                if (setOwner == ownerId)
                {
                    lock->Acquired = true;
                    CLOG_DEBUG("lock %s acquired by %s", lockName.c_str(), ownerId.c_str());
                    return lock;
                }
            }
            
            // 能走到这里说明 owner 不是我们设置的（并发情况）
            std::string currentOwner = (ownerIter != doc.end()) ? ownerIter->get_string().value.data() : "unknown";
            CLOG_DEBUG("lock %s already held by %s, waiting...", lockName.c_str(), currentOwner.c_str());
        }
        else
        {
            // result 为空表示没有匹配任何文档（理论上不应该发生，因为有 upsert:true）
            lock->Acquired = true;
            CLOG_WARN("lock %s acquired by %s (upserted)", lockName.c_str(), ownerId.c_str());
        }
    }
    catch (const mongocxx::exception &e)
    {
        CLOG_ERROR("TryAcquireLock failed:%s, lockName:%s, ownerId:%s", e.code().message().c_str(), lockName.c_str(), ownerId.c_str());
    }
    catch (const std::exception &e)
    {
        CLOG_ERROR("TryAcquireLock failed std exception:%s, lockName:%s, ownerId:%s", e.what(), lockName.c_str(), ownerId.c_str());
    }
    catch (...)
    {
        CLOG_ERROR("TryAcquireLock failed: lockName:%s, ownerId:%s", lockName.c_str(), ownerId.c_str());
    }
    
    return lock;
}

void MongodbConnection::ReleaseLock(KERNEL_NS::SmartPtr<ShardingLock, KERNEL_NS::AutoDelMethods::CustomDelete> &lock)
{
    if (!lock->Acquired)
        return;
            
    try
    {
        auto locksDb = _entry["config"]["_sharding_locks"];
        auto filter = bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp("_id", lock->LockCollection.GetRaw()),
            bsoncxx::builder::basic::kvp("owner", lock->LockId)  // 只删除自己拥有的锁
        );
            
        locksDb.delete_one(filter.view());
        CLOG_DEBUG("lock %s released, lockId:%s", lock->LockCollection.c_str(), lock->LockId.c_str());
    }
    catch (const mongocxx::exception &e)
    {
        CLOG_ERROR("ReleaseLock failed:%s, lock collection:%s, lockId:%s", e.code().message().c_str(), lock->LockCollection.c_str(), lock->LockId.c_str());
    }
    catch (const std::exception &e)
    {
        CLOG_ERROR("ReleaseLock failed std exception:%s, lock collection:%s, lockId:%s", e.what(), lock->LockCollection.c_str(), lock->LockId.c_str());
    }
    catch (...)
    {
        CLOG_ERROR("ReleaseLock failed: lock collection:%s, lockId:%s", lock->LockCollection.c_str(), lock->LockId.c_str());
    }
}

KERNEL_NS::CoTask<bool> MongodbConnection::ShardCollection(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collName, std::vector<ShardKeyInfo> shardKeys, Int32 numChunks)
{
    // 生成唯一锁 ID (使用 UUID)
    auto &&lockOwner = KERNEL_NS::GuidUtil::GenStr();
    KERNEL_NS::LibString &&fullNs = dbName + "." + collName;
    KERNEL_NS::LibString &&lockName = "coll_sharding_" + fullNs;
    
    try
    {
        // 先检查是否已设置分片键
        if (_CheckCollectionSharded(dbName, collName))
        {
            CLOG_DEBUG("collection %s.%s already sharded, skip", dbName.c_str(), collName.c_str());
            co_return true;
        }
        
        // 确保数据库已启用分片
        if (!co_await EnableDatabaseSharding(dbName))
        {
            CLOG_ERROR("failed to enable sharding for database %s, collName:%s", dbName.c_str(), collName.c_str());
            co_return false;
        }

        for (Int32 idx = 0; idx < 3; ++idx)
        {
            // 尝试获取分布式锁
            auto lock = TryAcquireLock(lockName, lockOwner);
            if (!lock->Acquired)
            {
                // 等待锁释放后再次检查状态
                g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "waiting for lock %s, will check sharding status after timeout"), lockName.c_str());
                co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(MONGODB_LOCK_EXPIRE_SECONDS + 1));

                if (_CheckCollectionSharded(dbName, lockName))
                    co_return true;

                continue;
            }
        
            // 获取锁成功，再次检查是否已被其他进程设置
            if (_CheckCollectionSharded(dbName, collName))
            {
                CLOG_DEBUG("collection %s.%s already sharded (checked after lock), skip", dbName.c_str(), collName.c_str());
                ReleaseLock(lock);
                co_return true;
            }
        
            // 通过 admin 数据库执行 shardCollection 命令
            // 复合分片键: playerId 使用 hash 分片, CreateTime 使用范围分片
            auto adminDb = _entry["admin"];

            auto builder = bsoncxx::builder::basic::document();
            for (auto &shardKey : shardKeys)
            {
                switch (shardKey.ValueType)
                {
                    case ShardKeyType::HASHED:
                    {
                        builder.append(bsoncxx::builder::basic::kvp(shardKey.KeyName.GetRaw(), "hashed"));
                        break;
                    }
                    case ShardKeyType::ASC:
                    {
                        builder.append(bsoncxx::builder::basic::kvp(shardKey.KeyName.GetRaw(), 1));
                        break;
                    }
                    case ShardKeyType::DESC:
                    {
                        builder.append(bsoncxx::builder::basic::kvp(shardKey.KeyName.GetRaw(), -1));
                        break;
                    }
                }
            }
            
            auto cmd = bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp("shardCollection", fullNs),
                bsoncxx::builder::basic::kvp("key", builder.extract()),
                bsoncxx::builder::basic::kvp("numInitialChunks", numChunks)
            );

            CLOG_INFO("sharding collection %s with key:%s, numInitialChunks:%d", fullNs.c_str(), bsoncxx::to_json(builder.view()).c_str(), numChunks);
        
            auto result = adminDb.run_command(cmd.view());
            auto resultView = result.view();
        
            // 释放锁
            ReleaseLock(lock);
        
            // 检查命令执行结果
            auto okIter = resultView.find("ok");
            if (okIter != resultView.end() && okIter->get_double() > 0)
            {
                CLOG_INFO("shardCollection %s success", fullNs.c_str());
                co_return true;
            }

            CLOG_ERROR("shardCollection %s failed:%s", fullNs.c_str(), bsoncxx::to_json(resultView).c_str());
            co_return false;
        }

        CLOG_ERROR("shardCollection try many times %s failed", fullNs.c_str());
        co_return false;
    }
    catch (const mongocxx::exception &e)
    {
        CLOG_ERROR("ShardCollection failed:%s, dbName:%s, collName:%s, shardKeys:%s"
            , e.code().message().c_str(), dbName.c_str(), collName.c_str(), KERNEL_NS::StringUtil::ToString(shardKeys, ',').c_str());
        co_return false;
    }
    catch (const std::exception &e)
    {
        CLOG_ERROR("ShardCollection std exception failed:%s, dbName:%s, collName:%s, shardKeys:%s", e.what(), dbName.c_str(), collName.c_str(), KERNEL_NS::StringUtil::ToString(shardKeys, ',').c_str());
    }
    catch (...)
    {
        CLOG_ERROR("ShardCollection unknown failed:%s, dbName:%s, collName:%s, shardKeys:%s", dbName.c_str(), collName.c_str(), KERNEL_NS::StringUtil::ToString(shardKeys, ',').c_str());
    }

    co_return false;
}

bool MongodbConnection::_CheckDatabaseSharded(const KERNEL_NS::LibString &dbName)
{
    try
    {
        // 通过 config.databases 检查数据库是否已启用分片
        auto configDb = _entry["config"];
        auto collections = configDb["databases"];
            
        auto filter = bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp("_id", dbName.GetRaw())
        );

        auto result = collections.find_one(filter.view());
        if (!result)
        {
            CLOG_DEBUG("database %s not found in config.databases, need to enable sharding", dbName.c_str());
            return false;
        }
            
        auto doc = result->view();
        auto partitionedIter = doc.find("partitioned");
        if (partitionedIter == doc.end())
        {
            CLOG_DEBUG("database %s found but partitioned field missing, need to enable sharding", dbName.c_str());
            return false;
        }
        
        bool partitioned = partitionedIter->get_bool();
        CLOG_INFO("database %s partitioned:%d", dbName.c_str(), partitioned);
        return partitioned;
    }
    catch (const mongocxx::exception &e)
    {
        CLOG_ERROR("CheckDatabaseSharded failed:%s, db:%s", e.code().message().c_str(), dbName.c_str());
        return false;
    }
    catch (const std::exception &e)
    {
        CLOG_ERROR("CheckDatabaseSharded failed std exception:%s, db:%s", e.what(), dbName.c_str());
        return false;
    }
    catch (...)
    {
        CLOG_ERROR("CheckDatabaseSharded failed: db:%s", dbName.c_str());
        return false;
    }
}



// 检查 collection 是否已设置分片键
bool MongodbConnection::_CheckCollectionSharded(const KERNEL_NS::LibString &dbName, const KERNEL_NS::LibString &collName)
{
    try
    {
        // 通过 config.collections 检查 collection 是否已设置分片键
        auto configDb = _entry["config"];
        auto collections = configDb["collections"];
        
        auto &&fullNs = dbName + "." + collName;
        auto filter = bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp("_id", fullNs.GetRaw())
        );
        
        auto result = collections.find_one(filter.view());
        if (!result)
        {
            CLOG_DEBUG("collection %s not found in config.collections, need to shard", fullNs.c_str());
            return false;
        }
        
        auto doc = result->view();
        auto keyIter = doc.find("key");
        if (keyIter == doc.end())
        {
            CLOG_DEBUG("collection %s found but key field missing, need to shard", fullNs.c_str());
            return false;
        }

        CLOG_DEBUG("collection %s already sharded with key:%s", fullNs.c_str(), bsoncxx::to_json(keyIter->get_document().view()).c_str());
        return true;
    }
    catch (const mongocxx::exception &e)
    {
        CLOG_ERROR("CheckCollectionSharded failed:%s, dbName:%s, collName:%s", e.code().message().c_str(), dbName.c_str(), collName.c_str());
        return false;
    }
    catch (const std::exception &e)
    {
        CLOG_ERROR("CheckCollectionSharded std exception::%s, dbName:%s, collName:%s", e.what(), dbName.c_str(), collName.c_str());
        return false;
    }
    catch (...)
    {
        CLOG_ERROR("CheckCollectionSharded unknown exception, dbName:%s, collName:%s", dbName.c_str(), collName.c_str());
        return false;
    }
}

KERNEL_END
