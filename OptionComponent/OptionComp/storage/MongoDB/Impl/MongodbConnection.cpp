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

KERNEL_NS::SmartPtr<MongodbConnection, KERNEL_NS::AutoDelMethods::CustomDelete> MongodbConnection::Create(mongocxx::pool::entry& entry)
{
    KERNEL_NS::SmartPtr<MongodbConnection, KERNEL_NS::AutoDelMethods::CustomDelete> ptr = MongodbConnection::NewThreadLocal_MongodbConnection(entry);
    ptr.SetClosureDelegate([](void *ptr)
    {
        MongodbConnection::DeleteThreadLocal_MongodbConnection(KernelCastTo<MongodbConnection>(ptr));
    });

    return ptr;
}

KERNEL_NS::CoTask<bool> MongodbConnection::EnableDatabaseSharding(KERNEL_NS::LibString dbName)
{
    // 生成唯一锁 ID (使用 UUID)
    auto &&lockOwner = KERNEL_NS::GuidUtil::GenStr();
    KERNEL_NS::LibString &&lockName = "db_sharding_" + dbName;

    KERNEL_NS::SmartPtr<ShardingLock, KERNEL_NS::AutoDelMethods::CustomDelete> lock;
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
            lock = TryAcquireLock(lockName, lockOwner);
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
        CLOG_ERROR("EnableDatabaseSharding failed:%s, dbName:%s", e.what(), dbName.c_str());
    }
    catch (const std::exception &e)
    {
        CLOG_ERROR("EnableDatabaseSharding std exception failed:%s, dbName:%s", e.what(), dbName.c_str());
    }
    catch (...)
    {
        CLOG_ERROR("EnableDatabaseSharding unkown dbName:%s", dbName.c_str());
    }

    if(lock)
    {
        ReleaseLock(lock);
    }

    co_return false;
}

KERNEL_NS::SmartPtr<ShardingLock, KERNEL_NS::AutoDelMethods::CustomDelete> MongodbConnection::TryAcquireLock(const KERNEL_NS::LibString &lockName, const KERNEL_NS::LibString &ownerId, const KERNEL_NS::TimeSlice &lockSlice) noexcept
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
        auto &&expireTime = now + lockSlice;
        
        // 原子性获取锁的 filter:
        // 1. owner 是自己 → 续期
        // 2. expireAt 不存在 → 锁记录异常, 抢占
        // 3. expireAt 已过期 → 抢占过期锁
        bsoncxx::builder::basic::document filterDoc;
        filterDoc.append(bsoncxx::builder::basic::kvp("_id", lockName.GetRaw()));
        filterDoc.append(bsoncxx::builder::basic::kvp("$or", bsoncxx::builder::basic::make_array(
            bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("owner", ownerId.GetRaw())),
            bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("expireAt", bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp("$exists", false)
            ))),
            bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("expireAt", bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp("$lt", static_cast<std::int64_t>(now.GetMilliTimestamp()))
            )))
        )));
        
        // 更新操作: 设置 owner 和新的过期时间
        bsoncxx::builder::basic::document updateDoc;
        updateDoc.append(bsoncxx::builder::basic::kvp("_id", lockName.GetRaw()));
        updateDoc.append(bsoncxx::builder::basic::kvp("owner", ownerId.GetRaw()));
        updateDoc.append(bsoncxx::builder::basic::kvp("expireAt", static_cast<std::int64_t>(expireTime.GetMilliTimestamp())));
        
        mongocxx::options::find_one_and_update options;
        options.upsert(true);
        options.return_document(mongocxx::options::return_document::k_after);
        
        auto result = locksDb.find_one_and_update(filterDoc.view(), updateDoc.view(), options);
        // return_document: k_after + upsert: true → result 永远不会为空
        // 匹配到: owner匹配则续期 / 锁已过期则抢占; 未匹配到: upsert 创建新文档(首次获取)
        lock->Acquired = true;
        CLOG_DEBUG("lock %s acquired by %s", lockName.c_str(), ownerId.c_str());
    }
    catch (const mongocxx::exception &e)
    {
        // 锁已被其他进程持有
        if (e.code().value() == 11000)
        {
            lock->Acquired = false;
            CLOG_DEBUG("TryAcquireLock failed lock already held by another:%s, lockName:%s, ownerId:%s", e.what(), lockName.c_str(), ownerId.c_str());
        }
        else
        {
            CLOG_ERROR("TryAcquireLock failed:%s, lockName:%s, ownerId:%s", e.what(), lockName.c_str(), ownerId.c_str());
        }
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

void MongodbConnection::ReleaseLock(KERNEL_NS::SmartPtr<ShardingLock, KERNEL_NS::AutoDelMethods::CustomDelete> &lock) noexcept
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
        CLOG_ERROR("ReleaseLock failed:%s, lock collection:%s, lockId:%s", e.what(), lock->LockCollection.c_str(), lock->LockId.c_str());
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

KERNEL_NS::CoTask<bool> MongodbConnection::ShardCollection(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collName, ShardKeyInfoGroup shardKeyGroup, Int32 numChunks)
{
    // 生成唯一锁 ID (使用 UUID)
    auto &&lockOwner = KERNEL_NS::GuidUtil::GenStr();
    KERNEL_NS::LibString &&fullNs = dbName + "." + collName;
    KERNEL_NS::LibString &&lockName = "coll_sharding_" + fullNs;

    KERNEL_NS::SmartPtr<ShardingLock, KERNEL_NS::AutoDelMethods::CustomDelete> lock;
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
            lock = TryAcquireLock(lockName, lockOwner);
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
            bool isUnique = shardKeyGroup.IsUnique;
            for (auto &shardKey : shardKeyGroup.ShardKeyInfos)
            {
                switch (shardKey.ValueType)
                {
                    case ShardKeyType::HASHED:
                    {
                        // hashed 字段不能做唯一索引
                        isUnique = false;    
                        builder.append(bsoncxx::builder::basic::kvp(shardKey.KeyName.GetRaw(), "hashed"));
                        CLOG_WARN("db:%s, collection:%s, ShardKeyGroup: %s, field:%s hashed, cant use unique index"
                            , dbName.c_str(), collName.c_str(), shardKeyGroup.ToString().c_str(), shardKey.KeyName.c_str());    
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

            if (isUnique)
            {
                auto cmd = bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp("shardCollection", fullNs),
                bsoncxx::builder::basic::kvp("key", builder.extract()),
                bsoncxx::builder::basic::kvp("unique", true),
                bsoncxx::builder::basic::kvp("numInitialChunks", numChunks)
                );

                CLOG_INFO("sharding collection %s with key:%s, numInitialChunks:%d unique:true", fullNs.c_str(), bsoncxx::to_json(builder.view()).c_str(), numChunks);
        
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
            }
            else
            {
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
            }
           
            co_return false;
        }

        CLOG_ERROR("shardCollection try many times %s failed", fullNs.c_str());
        co_return false;
    }
    catch (const mongocxx::exception &e)
    {
        CLOG_ERROR("ShardCollection failed:%s, dbName:%s, collName:%s, shardKeys:%s"
            , e.what(), dbName.c_str(), collName.c_str(), shardKeyGroup.ToString().c_str());
    }
    catch (const std::exception &e)
    {
        CLOG_ERROR("ShardCollection std exception failed:%s, dbName:%s, collName:%s, shardKeys:%s", e.what(), dbName.c_str(), collName.c_str(), shardKeyGroup.ToString().c_str());
    }
    catch (...)
    {
        CLOG_ERROR("ShardCollection unknown failed, dbName:%s, collName:%s, shardKeys:%s", dbName.c_str(), collName.c_str(), shardKeyGroup.ToString().c_str());
    }

    if(lock)
        ReleaseLock(lock);
    co_return false;
}

KERNEL_NS::CoTask<bool> MongodbConnection::CreateIndex(const KERNEL_NS::LibString &dbName, const KERNEL_NS::LibString &collName, const KERNEL_NS::LibString &indexName, 
                       const std::vector<std::pair<KERNEL_NS::LibString, Int32>> &fields, bool unique)
{
    KERNEL_NS::SmartPtr<ShardingLock, KERNEL_NS::AutoDelMethods::CustomDelete> lock;
    try
    {
        auto collection = (*_entry)[dbName.GetRaw()][collName.GetRaw()];
        if(_CheckIndexExists(collection, indexName))
        {
            CLOG_DEBUG("index already exists %s, db:%s, collection:%s", indexName.c_str(), dbName.c_str(), collName.c_str());
            co_return true;
        }
        
        // 生成唯一锁 ID (使用 UUID)
        auto &&lockOwner = KERNEL_NS::GuidUtil::GenStr();
        KERNEL_NS::LibString &&fullNs = dbName + "." + collName;
        KERNEL_NS::LibString &&lockName = "coll_sharding_" + fullNs;
        
        for (Int32 idx = 0; idx < 3; ++idx)
        {
            // 尝试获取分布式锁
            lock = TryAcquireLock(lockName, lockOwner);
            if (!lock->Acquired)
            {
                // 等待锁释放后再次检查状态
                CLOG_INFO("waiting for lock %s, will check sharding status after timeout", lockName.c_str());
                co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(MONGODB_LOCK_EXPIRE_SECONDS + 1));

                if (_CheckIndexExists(collection, indexName))
                    co_return true;

                continue;
            }
        
            if(_CheckIndexExists(collection, indexName))
            {
                CLOG_DEBUG("index already exists %s, db:%s, collection:%s", indexName.c_str(), dbName.c_str(), collName.c_str());
                ReleaseLock(lock);
                co_return true;
            }

            // 构建索引键文档
            bsoncxx::builder::basic::document keyDoc;
            for (const auto &field : fields)
            {
                // -2是哈希
                if(field.second == -2)
                {
                    keyDoc.append(bsoncxx::builder::basic::kvp(field.first.GetRaw(), "hashed"));
                }
                else
                {
                    keyDoc.append(bsoncxx::builder::basic::kvp(field.first.GetRaw(), field.second));
                }
            }
                
            // 使用 mongocxx::options::index_options 构建索引选项
            mongocxx::options::index options;
            options.unique(unique);
            options.name(indexName.GetRaw());

            CLOG_INFO("CreateIndex db:%s, collection:%s, creating index %s on fields:%s, unique:%d..."
                , dbName.c_str(), collName.c_str(), indexName.c_str(), bsoncxx::to_json(keyDoc.view()).c_str(), unique);
            
            collection.create_index(keyDoc.view(), options);

            CLOG_INFO("CreateIndex db:%s, collection:%s, index %s created successfully."
                , dbName.c_str(), collName.c_str(), indexName.c_str());

            ReleaseLock(lock);
            co_return true;
        }
    }
    catch (const mongocxx::exception &e)
    {
        KERNEL_NS::LibString fieldStr;
        for(auto &field : fields)
        {
            fieldStr.AppendFormat("%s:%d, ", field.first.c_str(), field.second);
        }
        CLOG_ERROR("CreateIndex fail exception:%s, db:%s, collection:%s, indexName:%s, fields:%s, unique:%d"
            , e.what(), dbName.c_str(), collName.c_str(), indexName.c_str(), fieldStr.c_str(), unique);
    }
    catch (const std::exception &e)
    {
        KERNEL_NS::LibString fieldStr;
        for(auto &field : fields)
        {
            fieldStr.AppendFormat("%s:%d, ", field.first.c_str(), field.second);
        }
        CLOG_ERROR("CreateIndex fail std exception:%s, db:%s, collection:%s, indexName:%s, fields:%s, unique:%d"
            , e.what(), dbName.c_str(), collName.c_str(), indexName.c_str(), fieldStr.c_str(), unique);
    }
    catch (...)
    {
        KERNEL_NS::LibString fieldStr;
        for(auto &field : fields)
        {
            fieldStr.AppendFormat("%s:%d, ", field.first.c_str(), field.second);
        }
        CLOG_ERROR("CreateIndex fail unknown exception, db:%s, collection:%s, indexName:%s, fields:%s, unique:%d"
            , dbName.c_str(), collName.c_str(), indexName.c_str(), fieldStr.c_str(), unique);
    }

    if(lock)
    {
        ReleaseLock(lock);
    }
    co_return false;
}

// 检查索引是否存在
bool MongodbConnection::_CheckIndexExists(mongocxx::collection &coll, const KERNEL_NS::LibString &indexName)
{
    try
    {
        auto listIndex = coll.list_indexes();
        for (auto &index : listIndex)
        {
            auto nameIter = index.find("name");
            if (nameIter != index.end())
            {
                std::string name = nameIter->get_string().value.data();
                if (indexName == name)
                {
                    CLOG_DEBUG("index %s already exists", indexName.c_str());
                    return true;
                }
            }
        }

        CLOG_DEBUG("index %s not found", indexName.c_str());
        return false;
    }
    catch (const mongocxx::exception &e)
    {
        CLOG_ERROR("CheckIndexExists failed:%s, indexName:%s", e.what(), indexName.c_str());
    }
    catch (const std::exception &e)
    {
        CLOG_ERROR("CheckIndexExists std exception failed:%s, indexName:%s", e.what(), indexName.c_str());
    }
    catch (...)
    {
        CLOG_ERROR("CheckIndexExists unknown, indexName:%s", indexName.c_str());
    }

    return false;
}

bool MongodbConnection::_CheckDatabaseSharded(const KERNEL_NS::LibString &dbName)
{
    try
    {
        // 通过 config.databases 检查数据库是否已启用分片
        auto configDb = _entry["config"];
        auto collections = configDb["databases"];
            
        auto filter = bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp("_id", dbName)
        );

        auto result = collections.find_one(filter.view());
        if (!result)
        {
            CLOG_DEBUG("database %s not found in config.databases, need to enable sharding", dbName.c_str());
            return false;
        }

        // 8.0 partitioned 字段被废弃
        // auto doc = result->view();
        // auto partitionedIter = doc.find("partitioned");
        // if (partitionedIter == doc.end())
        // {
        //     CLOG_DEBUG("database %s found but partitioned field missing, need to enable sharding", dbName.c_str());
        //     return false;
        // }
        //
        // bool partitioned = partitionedIter->get_bool();
        CLOG_DEBUG("database %s partitioned", dbName.c_str());
        return true;
    }
    catch (const mongocxx::exception &e)
    {
        CLOG_ERROR("CheckDatabaseSharded failed:%s, db:%s", e.what(), dbName.c_str());
    }
    catch (const std::exception &e)
    {
        CLOG_ERROR("CheckDatabaseSharded failed std exception:%s, db:%s", e.what(), dbName.c_str());
    }
    catch (...)
    {
        CLOG_ERROR("CheckDatabaseSharded failed: db:%s", dbName.c_str());
    }

    return false;
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
        CLOG_ERROR("CheckCollectionSharded failed:%s, dbName:%s, collName:%s", e.what(), dbName.c_str(), collName.c_str());
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
