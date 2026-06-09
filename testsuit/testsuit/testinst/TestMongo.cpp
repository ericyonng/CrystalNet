/*!
 *  MIT License
 *  
 *  Copyright (c) 2020 ericyonng<120453674@qq.com>
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 * 
 * Date: 2025-05-12 23:29:10
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestMongo.h>

#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/pool.hpp>
#include <iostream>

#include <protocols/protocols.h>

#include "OptionComp/storage/MongoDB/Impl/MongoDbMgrFactory.h"
#include "OptionComp/storage/MongoDB/Interface/IMongoDbMgr.h"


// ==================== 分片集群管理实现 ====================
//
// namespace
// {
//     // 分片集群管理函数 (使用 pool 连接池)
//     static bool CheckDatabaseSharded(mongocxx::pool &pool, const std::string &dbName);
//     static bool EnableDatabaseSharding(mongocxx::pool &pool, const std::string &dbName);
//     static bool CheckCollectionSharded(mongocxx::pool &pool, const std::string &dbName, const std::string &collName);
//     static bool ShardCollection(mongocxx::pool &pool, const std::string &dbName, const std::string &collName, Int32 numChunks = 1024);
//     // static bool CheckIndexExists(mongocxx::collection &coll, const std::string &indexName);
//     // static bool CreateIndex(mongocxx::collection &coll, const std::string &indexName, const std::vector<std::pair<std::string, Int32>> &fields, bool unique = false);
//
//     // ==================== 分布式锁实现 ====================
//     // 使用 MongoDB findAndModify 实现分布式锁，解决多进程并发初始化问题
//     
//     struct ShardingLock
//     {
//         std::string lockId;           // 锁 ID
//         std::string lockCollection;   // 锁目标 (dbName.collName 或 dbName)
//         bool acquired = false;        // 是否成功获取锁
//     };
//     
//     // 锁过期时间 (秒)，防止进程崩溃后锁不释放
//     static constexpr Int32 LOCK_EXPIRE_SECONDS = 30;
//     
//     // 尝试获取分布式锁 (使用 findOneAndUpdate + upsert 原子操作)
//     // 逻辑: 如果锁不存在则创建; 如果锁存在但已过期则更新; 如果锁存在且未过期则失败
//     static ShardingLock TryAcquireLock(mongocxx::client &client, const std::string &lockName, const std::string &ownerId)
//     {
//         ShardingLock lock;
//         lock.lockCollection = lockName;
//         lock.lockId = ownerId;
//         
//         try
//         {
//             auto locksDb = client["config"]["_sharding_locks"];
//             auto now = std::chrono::system_clock::now();
//             auto expireTime = bsoncxx::types::b_date(now + std::chrono::seconds(LOCK_EXPIRE_SECONDS));
//             
//             // 原子性获取锁的 filter:
//             // 1. expireAt 字段不存在 (锁不存在)
//             // 2. expireAt < now (锁已过期)
//             bsoncxx::builder::basic::document filterDoc;
//             filterDoc.append(bsoncxx::builder::basic::kvp("_id", lockName));
//             filterDoc.append(bsoncxx::builder::basic::kvp("$or", bsoncxx::builder::basic::make_array(
//                 bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("expireAt", bsoncxx::builder::basic::make_document(
//                     bsoncxx::builder::basic::kvp("$exists", false)
//                 ))),
//                 bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("expireAt", bsoncxx::builder::basic::make_document(
//                     bsoncxx::builder::basic::kvp("$lt", bsoncxx::types::b_date(now))  // 已过期
//                 )))
//             )));
//             
//             // 更新操作: 设置 owner 和新的过期时间
//             bsoncxx::builder::basic::document updateDoc;
//             updateDoc.append(bsoncxx::builder::basic::kvp("owner", ownerId));
//             updateDoc.append(bsoncxx::builder::basic::kvp("expireAt", expireTime));
//             
//             mongocxx::options::find_one_and_update options;
//             options.upsert(true);
//             options.return_document(mongocxx::options::return_document::k_after);
//             
//             auto result = locksDb.find_one_and_update(filterDoc.view(), updateDoc.view(), options);
//             
//             if (result)
//             {
//                 auto doc = result->view();
//                 auto ownerIter = doc.find("owner");
//                 
//                 // 如果 owner 是我们设置的，说明获取成功
//                 if (ownerIter != doc.end())
//                 {
//                     std::string setOwner = ownerIter->get_string().value.data();
//                     if (setOwner == ownerId)
//                     {
//                         lock.acquired = true;
//                         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "lock %s acquired by %s"), lockName.c_str(), ownerId.c_str());
//                         return lock;
//                     }
//                 }
//                 
//                 // 能走到这里说明 owner 不是我们设置的（并发情况）
//                 std::string currentOwner = (ownerIter != doc.end()) ? ownerIter->get_string().value.data() : "unknown";
//                 g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "lock %s already held by %s, waiting..."), 
//                             lockName.c_str(), currentOwner.c_str());
//             }
//             else
//             {
//                 // result 为空表示没有匹配任何文档（理论上不应该发生，因为有 upsert:true）
//                 lock.acquired = true;
//                 g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "lock %s acquired by %s (upserted)"), lockName.c_str(), ownerId.c_str());
//             }
//         }
//         catch (const mongocxx::exception &e)
//         {
//             g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "TryAcquireLock failed:%s"), e.what());
//         }
//         
//         return lock;
//     }
//     
//     // 释放分布式锁
//     static void ReleaseLock(mongocxx::client &client, ShardingLock &lock)
//     {
//         if (!lock.acquired)
//             return;
//             
//         try
//         {
//             auto locksDb = client["config"]["_sharding_locks"];
//             auto filter = bsoncxx::builder::basic::make_document(
//                 bsoncxx::builder::basic::kvp("_id", lock.lockCollection),
//                 bsoncxx::builder::basic::kvp("owner", lock.lockId)  // 只删除自己拥有的锁
//             );
//             
//             locksDb.delete_one(filter.view());
//             g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "lock %s released"), lock.lockCollection.c_str());
//         }
//         catch (const mongocxx::exception &e)
//         {
//             g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "ReleaseLock failed:%s"), e.what());
//         }
//     }
//
//     // ==================== 分片集群管理实现 ====================
//
//     // 检查数据库是否已启用分片
//     static bool CheckDatabaseSharded(mongocxx::pool &pool, const std::string &dbName)
//     {
//         try
//         {
//             // 从连接池获取连接
//             auto client = pool.acquire();
//             
//             // 通过 config.databases 检查数据库是否已启用分片
//             auto configDb = (*client)["config"];
//             auto collections = configDb["databases"];
//             
//             auto filter = bsoncxx::builder::basic::make_document(
//                 bsoncxx::builder::basic::kvp("_id", dbName)
//             );
//
//             auto result = collections.find_one(filter.view());
//             if (!result)
//             {
//                 g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "database %s not found in config.databases, need to enable sharding"), dbName.c_str());
//                 return false;
//             }
//             
//
//             return true;
//
//             // partitioned 8.0已废弃
//             // auto doc = result->view();
//             // auto partitionedIter = doc.find("partitioned");
//             // if (partitionedIter == doc.end())
//             // {
//             // g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "database %s found but partitioned field missing, need to enable sharding"), dbName.c_str());
//             // return false;
//             // }
//         
//         // bool partitioned = partitionedIter->get_bool();
//         // g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "database %s partitioned:%d"), dbName.c_str(), partitioned);
//         // return partitioned;
//     }
//     catch (const mongocxx::exception &e)
//     {
//         g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "CheckDatabaseSharded failed:%s"), e.what());
//         return false;
//     }
// }
//
//     // 启用数据库分片 (线程安全，支持多进程并发)
//     static bool EnableDatabaseSharding(mongocxx::pool &pool, const std::string &dbName)
//     {
//         // 生成唯一锁 ID (使用 UUID)
//         std::string lockOwner = KERNEL_NS::GuidUtil::GenStr().GetRaw();
//         std::string lockName = "db_sharding_" + dbName;
//         
//         try
//         {
//             // 先检查是否已启用
//             if (CheckDatabaseSharded(pool, dbName))
//             {
//                 g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "database %s already sharded, skip"), dbName.c_str());
//                 return true;
//             }
//             
//             // 从连接池获取连接用于获取锁
//             auto client = pool.acquire();
//             
//             // 尝试获取分布式锁
//             ShardingLock lock = TryAcquireLock(*client, lockName, lockOwner);
//             if (!lock.acquired)
//             {
//                 // 等待锁释放后再次检查状态
//                 g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "waiting for lock %s, will check sharding status after timeout"), lockName.c_str());
//                 // 再次检查分片状态(锁可能已超时自动释放)
//                 KERNEL_NS::SystemUtil::ThreadSleep((LOCK_EXPIRE_SECONDS + 1) * 1000);
//                 return CheckDatabaseSharded(pool, dbName);
//             }
//             
//             // 获取锁成功，再次检查是否已被其他进程启用
//             if (CheckDatabaseSharded(pool, dbName))
//             {
//                 g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "database %s already sharded (checked after lock), skip"), dbName.c_str());
//                 ReleaseLock(*client, lock);
//                 return true;
//             }
//             
//             // 通过 admin 数据库执行 enableSharding 命令
//             auto adminDb = (*client)["admin"];
//             auto cmd = bsoncxx::builder::basic::make_document(
//                 bsoncxx::builder::basic::kvp("enableSharding", dbName)
//             );
//             
//             auto result = adminDb.run_command(cmd.view());
//             auto resultView = result.view();
//             
//             // 释放锁
//             ReleaseLock(*client, lock);
//             
//             // 检查命令执行结果
//             auto okIter = resultView.find("ok");
//             if (okIter != resultView.end() && okIter->get_double() > 0)
//             {
//                 g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "enableSharding %s success"), dbName.c_str());
//                 return true;
//             }
//             
//             g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "enableSharding %s failed:%s"), dbName.c_str(), bsoncxx::to_json(resultView).c_str());
//             return false;
//         }
//         catch (const mongocxx::exception &e)
//         {
//             g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "EnableDatabaseSharding failed:%s"), e.what());
//         return false;
//     }
// }
//
//     // 检查 collection 是否已设置分片键
//     static bool CheckCollectionSharded(mongocxx::pool &pool, const std::string &dbName, const std::string &collName)
//     {
//     try
//     {
//         // 从连接池获取连接
//         auto client = pool.acquire();
//         
//         // 通过 config.collections 检查 collection 是否已设置分片键
//         auto configDb = (*client)["config"];
//         auto collections = configDb["collections"];
//         
//         std::string fullNs = dbName + "." + collName;
//         auto filter = bsoncxx::builder::basic::make_document(
//             bsoncxx::builder::basic::kvp("_id", fullNs)
//         );
//         
//         auto result = collections.find_one(filter.view());
//         if (!result)
//         {
//             g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "collection %s not found in config.collections, need to shard"), fullNs.c_str());
//             return false;
//         }
//         
//         auto doc = result->view();
//         auto keyIter = doc.find("key");
//         if (keyIter == doc.end())
//         {
//             g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "collection %s found but key field missing, need to shard"), fullNs.c_str());
//             return false;
//         }
//         
//         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "collection %s already sharded with key:%s"), 
//                     fullNs.c_str(), bsoncxx::to_json(keyIter->get_document().view()).c_str());
//         return true;
//     }
//     catch (const mongocxx::exception &e)
//     {
//         g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "CheckCollectionSharded failed:%s"), e.what());
//         return false;
//     }
// }
//
// // 创建分片键 (复合分片键, playerId hash分片 + CreateTime 范围分片, 线程安全)
//     static bool ShardCollection(mongocxx::pool &pool, const std::string &dbName, const std::string &collName, Int32 numChunks)
//     {
//         // 生成唯一锁 ID (使用 UUID)
//         std::string lockOwner = KERNEL_NS::GuidUtil::GenStr().GetRaw();
//         std::string fullNs = dbName + "." + collName;
//         std::string lockName = "coll_sharding_" + fullNs;
//         
//         try
//         {
//             // 先检查是否已设置分片键
//             if (CheckCollectionSharded(pool, dbName, collName))
//             {
//                 g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "collection %s.%s already sharded, skip"), dbName.c_str(), collName.c_str());
//                 return true;
//             }
//             
//             // 确保数据库已启用分片
//             if (!EnableDatabaseSharding(pool, dbName))
//             {
//                 g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "failed to enable sharding for database %s"), dbName.c_str());
//                 return false;
//             }
//             
//             // 从连接池获取连接用于获取锁
//             auto client = pool.acquire();
//             
//             // 尝试获取分布式锁
//             ShardingLock lock = TryAcquireLock(*client, lockName, lockOwner);
//             if (!lock.acquired)
//             {
//                 // 等待锁释放后再次检查状态
//                 g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "waiting for lock %s, will check sharding status after timeout"), lockName.c_str());
//                 KERNEL_NS::SystemUtil::ThreadSleep((LOCK_EXPIRE_SECONDS + 1)*1000);
//                 return CheckCollectionSharded(pool, dbName, collName);
//             }
//             
//             // 获取锁成功，再次检查是否已被其他进程设置
//             if (CheckCollectionSharded(pool, dbName, collName))
//             {
//                 g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "collection %s.%s already sharded (checked after lock), skip"), dbName.c_str(), collName.c_str());
//                 ReleaseLock(*client, lock);
//                 return true;
//             }
//             
//             // 通过 admin 数据库执行 shardCollection 命令
//             // 复合分片键: playerId 使用 hash 分片, CreateTime 使用范围分片
//             auto adminDb = (*client)["admin"];
//             
//             auto cmd = bsoncxx::builder::basic::make_document(
//                 bsoncxx::builder::basic::kvp("shardCollection", fullNs),
//                 bsoncxx::builder::basic::kvp("key", bsoncxx::builder::basic::make_document(
//                     bsoncxx::builder::basic::kvp("playerId", "hashed")
//                 )),
//                 bsoncxx::builder::basic::kvp("numInitialChunks", numChunks)
//             );
//             
//             g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "sharding collection %s with key:{playerId:'hashed', createTime:1}, numInitialChunks:%d"), 
//                         fullNs.c_str(), numChunks);
//             
//             auto result = adminDb.run_command(cmd.view());
//             auto resultView = result.view();
//             
//             // 释放锁
//             ReleaseLock(*client, lock);
//             
//             // 检查命令执行结果
//             auto okIter = resultView.find("ok");
//             if (okIter != resultView.end() && okIter->get_double() > 0)
//             {
//                 g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "shardCollection %s success"), fullNs.c_str());
//                 return true;
//             }
//             
//             g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "shardCollection %s failed:%s"), fullNs.c_str(), bsoncxx::to_json(resultView).c_str());
//             return false;
//         }
//     catch (const mongocxx::exception &e)
//     {
//         g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "ShardCollection failed:%s"), e.what());
//         return false;
//     }
// }
//
//     // 检查索引是否存在
//     // static bool CheckIndexExists(mongocxx::collection &coll, const std::string &indexName)
//     // {
//     //     try
//     //     {
//     //         auto listIndex = coll.list_indexes();
//     //         for (auto &index : listIndex)
//     //         {
//     //             auto nameIter = index.find("name");
//     //             if (nameIter != index.end())
//     //             {
//     //                 std::string name = nameIter->get_string().value.data();
//     //                 if (name == indexName)
//     //                 {
//     //                     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "index %s already exists"), indexName.c_str());
//     //                     return true;
//     //                 }
//     //             }
//     //         }
//     //         
//     //         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "index %s not found"), indexName.c_str());
//     //         return false;
//     //     }
//     //     catch (const mongocxx::exception &e)
//     //     {
//     //         g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "CheckIndexExists failed:%s"), e.what());
//     //         return false;
//     //     }
//     // }
//
//     // 创建索引 (支持复合索引)
//     // static bool CreateIndex(mongocxx::collection &coll, const std::string &indexName, 
//     //                        const std::vector<std::pair<std::string, Int32>> &fields, bool unique)
//     // {
//     //     try
//     //     {
//     //         // 先检查索引是否已存在
//     //         if (CheckIndexExists(coll, indexName))
//     //         {
//     //             g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "index %s already exists, skip"), indexName.c_str());
//     //             return true;
//     //         }
//     //         
//     //         // 构建索引键文档
//     //         bsoncxx::builder::basic::document keyDoc;
//     //         for (const auto &field : fields)
//     //         {
//     //             keyDoc.append(bsoncxx::builder::basic::kvp(field.first, field.second));
//     //         }
//     //         
//     //         // 使用 mongocxx::options::index_options 构建索引选项
//     //         mongocxx::options::index options;
//     //         options.unique(unique);
//     //         options.name(indexName);
//     //         
//     //         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "creating index %s on fields:%s, unique:%d"), 
//     //                     indexName.c_str(), bsoncxx::to_json(keyDoc.view()).c_str(), unique);
//     //         
//     //         coll.create_index(keyDoc.view(), options);
//     //         
//     //         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "index %s created successfully"), indexName.c_str());
//     //         return true;
//     //     }
//     //     catch (const mongocxx::exception &e)
//     //     {
//     //         g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "CreateIndex %s failed:%s"), indexName.c_str(), e.what());
//     //         return false;
//     //     }
//     // }
//
//     // 分片集群管理测试 (使用 pool 连接池)
// //     static void TestShardClusterManagement()
// //     {
// //     // mongocxx::instance instance;
// //     
// //     try
// //     {
// //         // 连接分片集群 (mongos 节点地址列表)
// //         // 格式: mongodb://用户名:密码@host1:port1,host2:port2,.../?authSource=admin
// //         // mongodb+srv://xxx:xxx@mongoscluster.ericyonng.com/
// //         mongocxx::uri shardUri;
// //         // std::string uriStr = "mongodb://testmongo:abc%5E159@127.0.0.1:28017,127.0.0.1:28018,127.0.0.1:28019/?authSource=admin&replicaSet=rs0&w=majority&journal=true&readConcernLevel=majority&maxPoolSize=100&connectTimeoutMS=10000&socketTimeoutMS=30000";
// //         // mongocxx::uri uri(uriStr);
// //         mongocxx::pool pool(shardUri);
// //         
// //         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "connected to shard cluster with pool"));
// //         
// //         // 测试数据库和集合名称
// //         std::string dbName = "test_shard_db";
// //         std::string collName = "test_shard_coll2";
// //         
// //         // 1. 启用数据库分片
// //         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "===== Step 1: Enable database sharding ====="));
// //         if (!EnableDatabaseSharding(pool, dbName))
// //         {
// //             g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "failed to enable database sharding"));
// //             return;
// //         }
// //         
// //         // 2. 设置 collection 分片键
// //         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "===== Step 2: Shard collection ====="));
// //         if (!ShardCollection(pool, dbName, collName, 1024))
// //         {
// //             g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "failed to shard collection"));
// //             return;
// //         }
// //         
// //         // 3. 创建索引
// //         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "===== Step 3: Create indexes ====="));
// //         // 从连接池获取连接用于创建索引
// //         auto client = pool.acquire();
// //         auto coll = (*client)[dbName][collName];
// //         
// //         // 创建索引: playerId (分片集群中不能对非分片键字段创建唯一索引，改为普通索引)
// //         // std::vector<std::pair<std::string, Int32>> playerIdIndexFields = {{"playerId", 1}};
// //         // if (!CreateIndex(coll, "idx_playerId", playerIdIndexFields, false))
// //         // {
// //         //     g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "failed to create playerId index"));
// //         //     return;
// //         // }
// //         //
// //         // // 创建普通索引: createTime
// //         // std::vector<std::pair<std::string, Int32>> createTimeIndexFields = {{"createTime", 1}};
// //         // if (!CreateIndex(coll, "idx_createTime", createTimeIndexFields, false))
// //         // {
// //         //     g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "failed to create createTime index"));
// //         //     return;
// //         // }
// //         //
// //         // // 创建复合索引: playerId + createTime
// //         // std::vector<std::pair<std::string, Int32>> compoundIndexFields = {{"playerId", 1}, {"createTime", 1}};
// //         // if (!CreateIndex(coll, "idx_playerId_createTime", compoundIndexFields, false))
// //         // {
// //         //     g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "failed to create compound index"));
// //         //     return;
// //         // }
// //         
// //         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "===== All shard cluster management operations completed successfully ====="));
// //         
// //         // 列出所有索引验证
// //         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "===== Listing all indexes on collection %s.%s ====="), dbName.c_str(), collName.c_str());
// //         auto listIndex = coll.list_indexes();
// //         for (auto &index : listIndex)
// //         {
// //             g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "index:%s"), bsoncxx::to_json(index).c_str());
// //         }
// //     }
// //     catch (const mongocxx::exception &e)
// //     {
// //         g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "TestShardClusterManagement failed:%s"), e.what());
// //     }
// // }
// }  // end anonymous namespace


class TestMongoHost : public KERNEL_NS::CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, TestMongoHost);

public:
    TestMongoHost()
        :CompHostObject(KERNEL_NS::RttiUtil::GetTypeId<TestMongoHost>())
    {
        
    }
    virtual void Release() override
    {
        TestMongoHost::Delete_TestMongoHost(this); 
    }
    

private:
    virtual void OnRegisterComps() override
    {
        RegisterComp<KERNEL_NS::MongoDbMgrFactory>();
    }

    virtual Int32 _OnCompsCreated() override
    {
        auto mongoDbMgr = GetComp<KERNEL_NS::IMongoDbMgr>();
        mongoDbMgr->SetSrvHostName("xxx");
        mongoDbMgr->SetAccountPwd("eric", "xxx");

        mongoDbMgr->FocusDb("testsuit1");
        mongoDbMgr->FocusDb("testsuit2");
        mongoDbMgr->FocusDb("testsuit3");
        mongoDbMgr->FocusDb("testsuit4");
        mongoDbMgr->FocusDb("testsuit5");
        mongoDbMgr->FocusDb("testsuit6");
        mongoDbMgr->FocusDb("testsuit7");
        mongoDbMgr->FocusDb("testsuit8");
        mongoDbMgr->FocusDb("testsuit9");
        mongoDbMgr->FocusDb("testsuit10");

        std::vector<KERNEL_NS::ShardKeyInfo> shardKeys;
        KERNEL_NS::ShardKeyInfo info;
        info.KeyName = "player_id";
        info.ValueType = KERNEL_NS::ShardKeyType::HASHED;
        // hashed不能做唯一索引否则会失败
        // info.IsUnique = true;
        shardKeys.push_back(info);
        mongoDbMgr->SetShardKeyInfo("testsuit2", "player", shardKeys);
        shardKeys.clear();
        info.KeyName = "player_id";
        info.ValueType = KERNEL_NS::ShardKeyType::HASHED;
        // hashed不能做唯一索引否则会失败
        // info.IsUnique = true;
        shardKeys.push_back(info);
        mongoDbMgr->SetShardKeyInfo("testsuit8", "player", shardKeys);
        mongoDbMgr->SetShardKeyInfo("testsuit9", "player", shardKeys);
        mongoDbMgr->SetShardKeyInfo("testsuit10", "player", shardKeys);

        mongoDbMgr->CreateIndex("testsuit8", "player", "idx_player_id", {{"player_id", 1}}, true);
        mongoDbMgr->CreateIndex("testsuit9", "player", "idx_player_id_role_id", {{"player_id", 1}, {"role_id", 1}}, true);
        mongoDbMgr->CreateIndex("testsuit10", "player", "idx_player_id_name", {{"player_id", 1}, {"name", 1}}, true);

        return Status::Success;
    }

    virtual Int32 _OnHostStart() override
    {
        // 添加数据
        KERNEL_NS::RunRightNow([this]()->KERNEL_NS::CoTask<>
        {
            auto mongodbMgr = GetComp<KERNEL_NS::IMongoDbMgr>();

            auto ret = co_await mongodbMgr->DelData("testsuit6", "player", {std::make_pair("player_id", KERNEL_NS::Variant(88888LL))});
            CLOG_INFO("DelData:%d", ret);

            nlohmann::json json1;
            json1["player_id"] = 88888;
            KERNEL_NS::LibString *str1 = new KERNEL_NS::LibString(json1.dump());
            
            ret = co_await mongodbMgr->AddData("testsuit6", "player", str1);
            CLOG_INFO("add data:%d", ret);

            // 移除数据
            ret = co_await mongodbMgr->DelData("testsuit8", "player", {std::make_pair("player_id", KERNEL_NS::Variant("DAsssS"))});
            CLOG_INFO("DelData:%d", ret);
            
            nlohmann::json json;
            json["player_id"] = "DAsssS";
            json["age"] = 500;
            json["bignum"] = 5000000000000000LLU;
            KERNEL_NS::LibString *str = new KERNEL_NS::LibString(json.dump());
            ret = co_await mongodbMgr->AddData("testsuit8", "player", str);
            CLOG_INFO("add data:%d", ret);

            // 移除数据
            ret = co_await mongodbMgr->DelData("testsuit6", "player", {std::make_pair("player_id", KERNEL_NS::Variant(5566654545646LL))});
            CLOG_INFO("DelData:%d", ret);
            
            // 添加一条二进制数据
            CRYSTAL_NET::service::LoginReq loginReq;
            auto userInfo = loginReq.mutable_loginuserinfo();
            userInfo->set_loginmode(1);
            userInfo->set_accountname("xiaoming");
            userInfo->set_pwd("xiaoming");
            userInfo->set_logintoken("xxxxxxx4554");
            userInfo->set_port(5555);
            auto tl = KERNEL_NS::LibStreamTL::NewThreadLocal_LibStream();
            loginReq.Encode(*tl);
            std::vector<std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant>> uniqueKv;
            uniqueKv.push_back(std::make_pair("player_id", KERNEL_NS::Variant(5566654545646)));
            ret = co_await mongodbMgr->AddData("testsuit6", "player", uniqueKv, "LoginReq", tl);
            CLOG_INFO("add data:%d, tl:%p, uniqueKv:%s", ret, tl, KERNEL_NS::StringUtil::ToString(uniqueKv, ',', [](const std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant> &elem)->KERNEL_NS::LibString
            {
                return KERNEL_NS::LibString().AppendFormat("%s:%s", elem.first.c_str(), elem.second.ToString().c_str());
            }).c_str());

            // 移除数据
            std::vector<std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant>> multikv = {std::make_pair("player_id", KERNEL_NS::Variant(5566654545646LL)), std::make_pair("name", KERNEL_NS::Variant("xiaoming"))};
            ret = co_await mongodbMgr->DelData("testsuit10", "player", multikv);
            CLOG_INFO("DelData:%d, multiindex:%s", ret, KERNEL_NS::StringUtil::ToString(multikv, ',', [](const std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant> &elem)
            {
                return KERNEL_NS::LibString().AppendFormat("%s:%s", elem.first.c_str(), elem.second.ToString().c_str());
            }).c_str());

            // 添加数据
            tl = KERNEL_NS::LibStreamTL::NewThreadLocal_LibStream();
            loginReq.Encode(*tl);
            ret = co_await mongodbMgr->AddData("testsuit10", "player", multikv, "LoginReq2", tl);
            CLOG_INFO("add data:%d", ret);
        });

        return Status::Success;
    };

};

void TestMongo::Run()
{
    // mongocxx::instance instance;

    try
    {
        auto poller = KERNEL_NS::TlsUtil::GetPoller();
        poller->PrepareLoop();
        
        auto testObj = TestMongoHost::New_TestMongoHost();
        testObj->Init();
        testObj->Start();

        poller->EventLoop();

        testObj->WillClose();
        testObj->Close();
        
        // 分片集群管理功能测试
        //TestShardClusterManagement();
    }
    catch (const mongocxx::exception &e)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "An exception occurred:%s"), e.what());
    }
}


//
// static void Run2()
// {
//     mongocxx::instance instance;
//
//     try
//     {
//         // Start example code here 密码特殊符号需要使用url编码, 需要把所有的节点域名或者ip列出来避免某个节点不可用
//         // 写关注：w=majority, journal=true
//         // 读关注：&readConcernLevel=majority
//         mongocxx::uri uri("mongodb://testmongo:abc%5E159%40@127.0.0.1:28017,127.0.0.1:28018,127.0.0.1:28019/?authSource=admin&replicaSet=rs0&w=majority&journal=true&readConcernLevel=majority");
//         mongocxx::client client(uri);
//         // End example code here
//
//         // 分片集群管理功能测试
//         TestShardClusterManagement();
//
//         auto test2 = client["test2"];
//         auto fruit = test2["fruit"];
//         auto key = bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("name", "testmongo"));
//         auto result = fruit.find_one(key.view());
//         if(result)
//         {
//             g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "find data:%s"), bsoncxx::to_json(*result).c_str());
//
//         }
//         else
//         {
//             g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "find fail key:%s"), key.view().data());
//         }
//
//         // 连接池
//         mongocxx::uri shardUri;
//         mongocxx::pool pool(shardUri);
//
//         KERNEL_NS::LibThreadPool threadPool;
//         threadPool.Init(0, 10);
//         threadPool.Start();
//
//         threadPool.AddTask2([&pool](KERNEL_NS::LibThreadPool *threadPool, KERNEL_NS::Variant *param)
//         {
//             try
//             {
//                     // 取一个连接
//                 auto client = pool.acquire();
//
//                 // 访问test2 数据库(不存在会在插入)
//                 auto test2 = client["test2"];
//                 auto collection = test2["new_collection"];
//
//                 // 设置表的大多数写成功, 且journal写完成功才算成功
//                 mongocxx::write_concern concern;
//                 // 大多数节点成功后成功
//                 concern.acknowledge_level(mongocxx::write_concern::level::k_majority);
//                 // 写操作落盘后成功
//                 concern.journal(true);
//                 collection.write_concern(concern);
//                 
//                 auto ret = collection.insert_one(bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("name", "testmongo")
//                     , bsoncxx::builder::basic::kvp("sex", 1)));
//
//
//                 auto newDb = client->database("new_mongodb");
//                 auto player = newDb.create_collection("player4");
//
//                 // 创建唯一索引
//                 auto list_index = player.list_indexes();
//                 auto has_index = [&list_index](const std::string &fieldName)->bool
//                 {
//                     for(auto &index : list_index)
//                     {
//                         auto key = index["key"];
//                         auto bsonStr = bsoncxx::to_json(index);
//                         auto keyDoc = key.get_document();
//                         auto docStr = bsoncxx::to_json(keyDoc);
//
//                         if(keyDoc.view().find(fieldName) != key.get_document().view().end())
//                             return true;
//                     }
//
//                     return false;
//                 };
//                 
//                 if(!has_index("PlayerId"))
//                 {
//                     auto key_index = bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("PlayerId", 1));
//                     auto options = bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("unique", true), bsoncxx::builder::basic::kvp("name", "PlayerIdIndex"));
//                     player.create_index(key_index.view(), options.view());
//                 }
//
//                 // 显示的创建表
//                 auto member = newDb.create_collection("member");
//
//                 auto generator = KERNEL_NS::TlsUtil::GetIdGenerator();
//
//                 auto playerId = generator->NewId();
//                 auto key = bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("PlayerId", static_cast<std::int64_t>(playerId)));
//
//                 // 读数据配置
//                 mongocxx::read_preference rp;
//                 // // 从主节点上读数据
//                 // rp.mode(mongocxx::read_preference::read_mode::k_primary);
//                 // // 只从从节点读
//                 // rp.mode(mongocxx::read_preference::read_mode::k_secondary);
//                 // // 优先从从节点读，不可用时到主节点
//                 // rp.mode(mongocxx::read_preference::read_mode::k_secondary_preferred);
//                 // // 优先从主节点读，不可用时到从节点
//                 // rp.mode(mongocxx::read_preference::read_mode::k_primary_preferred);
//                 // // 从延迟最低的节点读取(就近路由)
//                 // rp.mode(mongocxx::read_preference::read_mode::k_nearest);
//                 // // 结合标签过滤节点(region标签为east的节点， tags可以在mongodb的mongod.conf中的replication:tags:region:east, 配置)
//                 // rp.tags(bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("region", "east")));
//                 // // 设置读数据的节点
//                 // player.read_preference(rp);
//
//                 // 设置majority, 常用的隔离级别,相当于读已提交级别 解决事务的隔离性问题
//                 mongocxx::read_concern rc;
//                 rc.acknowledge_level(mongocxx::read_concern::level::k_majority);
//                 player.read_concern(rc);
//
//                 // 设置查询的最大等待时间, 防止游标因网络问题长时间阻塞：
//                 mongocxx::options::find opts{};
//                 opts.max_await_time(std::chrono::seconds(10)); // 等待新批次最多 10 秒
//
//                 auto findOne = player.find_one(key.view(), opts);
//                 if(findOne)
//                 {
//                     // 更新
//                     player.update_one(key.view(), bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("$set", bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("name", "bba")))));
//
//                     // 替换
//                     player.replace_one(key.view(), bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("PlayerId", static_cast<std::int64_t>(playerId))));
//
//                     // 删除
//                     // player.delete_one(key.view());
//
//                 }
//                 else
//                 {
//                     player.insert_one(bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("PlayerId", static_cast<std::int64_t>(playerId))
//                     , bsoncxx::builder::basic::kvp("name", "xiaoming")));
//                 }
//
//                 // GridFS存储大文件
//                 ::CRYSTAL_NET::service::LoginReq *req = new ::CRYSTAL_NET::service::LoginReq;
//                 auto userInfo = req->mutable_loginuserinfo();
//                 userInfo->set_loginmode(1);
//                 userInfo->set_accountname("xiaoming");
//                 userInfo->set_pwd("xiaoming");
//                 userInfo->set_logintoken("xxxxxxx4554");
//                 userInfo->set_port(5555);
//
//
//                 KERNEL_NS::LibString info;
//                 std::string data;
//                 req->SerializeToString(&data);
//                 auto dataJson = req->ToJsonString();
//
//                 // 存储二进制数据
//                 auto binData = bsoncxx::types::b_binary();
//                 binData.sub_type = bsoncxx::binary_sub_type::k_binary;
//                 binData.size = data.size();
//                 binData.bytes = (uint8_t *) data.data();
//                 auto binDoc = bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("BinData", binData));
//                 player.update_one(key.view(), bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("$set", bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("BinData", binData)))));
//                 
//                 // 创建GridFs存储桶
//                 auto bucket = newDb.gridfs_bucket();
//                 auto uploader = bucket.open_upload_stream(KERNEL_NS::LibString().AppendFormat("Player_%llu", playerId).c_str());
//                 uint8_t const *ptr = (uint8_t const *)data.data();
//                 uploader.write(ptr, data.size());
//                 auto result = uploader.close();
//
//                 // 更新gridfs 引用id
//                 auto resultId = result.id();
//                 player.update_one(key.view(), bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("$set", bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("GridFS_id", resultId)))));
//                 auto newPlayerData = player.find_one(key.view());
//                 if(newPlayerData)
//                 {
//                     auto iter = newPlayerData.value().find("BinData");
//                     if(iter != newPlayerData.value().end())
//                     {
//                         auto resultBin = iter->get_binary();
//                         KERNEL_NS::LibString newBinBuffer;
//                         newBinBuffer.AppendData((Byte8 *)resultBin.bytes, resultBin.size);
//                         ::CRYSTAL_NET::service::LoginReq reqResult;
//                         reqResult.ParseFromString(newBinBuffer.GetRaw());
//                         auto resultJson = reqResult.ToJsonString();
//                     }
//                 }
//
//                 auto downloader = bucket.open_download_stream(resultId);
//                 // auto chunkSize = 16 * 1024 *1024;
//                 auto buffer = KERNEL_NS::KernelAllocMemory<KERNEL_NS::_Build::TL>(1024);
//                 KERNEL_NS::LibString parseData;
//                 while(auto chunk = downloader.read((std::uint8_t*)buffer, 1024))
//                 {
//                     parseData.AppendData((const Byte8 *)buffer, chunk);
//                 }
//
//                 ::CRYSTAL_NET::service::LoginReq *req2 = new ::CRYSTAL_NET::service::LoginReq;
//                 req2->ParseFromString(parseData.GetRaw());
//                 auto parseJson = req2->ToJsonString();
//
//                 // 遍历数据库表
//                 auto collections = newDb.list_collections();
//                 for(auto &doc : collections)
//                     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "doc:%s"), bsoncxx::to_json(doc).c_str());
//
//                 // 删除集合
//                 auto test_drop = newDb.create_collection("test_drop_collection");
//                 test_drop.drop();
//             }
//             catch (const mongocxx::exception &e)
//             {
//                 g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "mongodb operation err:%s"), e.what());
//                 throw e;
//             }
//             catch (...)
//             {
//                 g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "mongodb operation err :unknown"));
//                 throw;
//             }
//         });
//
//         threadPool.Close();
//     }
//     catch (const mongocxx::exception &e)
//     {
//         g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "An exception occurred:%s"), e.what());
//     }
// }