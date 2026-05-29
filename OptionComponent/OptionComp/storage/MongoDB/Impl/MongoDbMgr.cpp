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
 * Date: 2025-10-14 14:15:00
 * Author: Eric Yonng
 * Description: mongodb数据库管理
*/
#include <pch.h>
#include <OptionComp/storage/MongoDB/Impl/MongoDbMgr.h>
#include <OptionComp/storage/MongoDB/Impl/MongoDbMgrFactory.h>
#include <mongocxx/exception/exception.hpp>
#include <kernel/comp/Utils/StringUtil.h>
#include <OptionComp/storage/MongoDB/Impl/MongodbConnection.h>
#include <OptionComp/storage/MongoDB/Impl/MongoAsyncRes.h>

KERNEL_BEGIN

mongocxx::instance MongoDbMgr::_instance;

class MongodbThreadStartup : public KERNEL_NS::IThreadStartUp
{
public:
    MongodbThreadStartup(MongoDbMgr *mongodbMgr)
        :_mongodbMgr(mongodbMgr)
    {
        
    }

    virtual void Run() override
    {
        // 启用分片, 并设置分片键, 失败则返回错误给MongoDBMgr
        KERNEL_NS::PostCaller([this]()->KERNEL_NS::CoTask<>
        {
            // 等待mongodbmgr will started
            while (true)
            {
                if(_mongodbMgr->_willStarted.load(std::memory_order_acquire))
                    break;
                
                co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(1));
                CLOG_INFO("waiting mongodb mgr will started");
            }

            // 设置分片键
            CLOG_INFO("ShardCollection...");
            auto dbShardKeyInfos = _mongodbMgr->_dbRefcollectionRefShardKeyInfos;
            for(auto iter : dbShardKeyInfos)
            {
                auto &dbName = iter.first;
                // 数据库设置分片
                CLOG_INFO("enable %s sharding...", dbName.c_str());
                {
                    auto entry = _mongodbMgr->_connectionPool->acquire();
                    auto connection = MongodbConnection::Create(entry);
                    auto isSuc = co_await connection->EnableDatabaseSharding(dbName);
                    if (!isSuc)
                    {
                        CLOG_ERROR("mongodb EnableDatabaseSharding fail");
                        _mongodbMgr->SetDbFailErr(Status::Failed);
                        co_return;
                    }
                }
                CLOG_INFO("enable db:%s sharding success.", dbName.c_str());
                
                auto &collectionShardKeys = iter.second;
                for (auto iterCollection : collectionShardKeys)
                {
                    auto &collectionName = iterCollection.first;
                    auto &shardKeys = iterCollection.second;
                    auto entry = _mongodbMgr->_connectionPool->acquire();
                    auto connection = MongodbConnection::Create(entry);
                    auto isSuc = co_await connection->ShardCollection(dbName, collectionName, shardKeys);
                    if(!isSuc)
                    {
                        CLOG_ERROR("mongodb ShardCollection fail dbname:%s collectionName:%s, shard keys:%s", dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(shardKeys, ',').c_str());
                        _mongodbMgr->SetDbFailErr(Status::Failed);
                        co_return;
                    }
                }
            }
            CLOG_INFO("ShardCollection success.");

            auto dbRefCollectionRefIndexInfos = _mongodbMgr->_dbRefCollectionRefMongoIndexInfos;
            CLOG_INFO("CreateIndex collection count:%d...", static_cast<Int32>(dbRefCollectionRefIndexInfos.size()));

            for(auto iter : dbRefCollectionRefIndexInfos)
            {
                auto &dbName = iter.first;
                auto &collectionRefIndexNameRefInfos = iter.second;
                for(auto iterInfo : collectionRefIndexNameRefInfos)
                {
                    auto &collectionName = iterInfo.first;
                    auto &indexNameRefIndexInfo = iterInfo.second;

                    for (auto iterIndex : indexNameRefIndexInfo)
                    {
                        auto &indexName = iterIndex.first;
                        auto &indexInfo = iterIndex.second;
                        
                        auto entry = _mongodbMgr->_connectionPool->acquire();
                        auto connection = MongodbConnection::Create(entry);

                        auto isSuc = co_await connection->CreateIndex(dbName, collectionName, indexName, indexInfo.Fields, indexInfo.Unique);
                        if(!isSuc)
                        {
                            KERNEL_NS::LibString fieldStr;
                            for(auto &field : indexInfo.Fields)
                            {
                                fieldStr.AppendFormat("%s:%d, ", field.first.c_str(), field.second);
                            }
                            CLOG_ERROR("mongodb CreateIndex fail db name:%s, collection:%s, indexName:%s, fields:%s, unique:%d"
                                , dbName.c_str(), collectionName.c_str(), indexName.c_str(), fieldStr.c_str(), indexInfo.Unique);
                            _mongodbMgr->SetDbFailErr(Status::Failed);
                            co_return;
                        }
                    }

                }
            }
            CLOG_INFO("CreateIndex success.");

            CLOG_INFO("mongodb ready...");
            _mongodbMgr->DbReady(true);

            // 等待任务过来(增删改查) TODO:
        });
    }
    virtual void Release() override
    {
        delete this;
    }
    
private:
    MongoDbMgr *_mongodbMgr;
};

static ALWAYS_INLINE mongocxx::write_concern _MakeMongoMajorityWriteConcern()
{
    mongocxx::write_concern concern;
    // 大多数节点成功后成功
    concern.acknowledge_level(mongocxx::write_concern::level::k_majority);
    // 写操作落盘后成功
    concern.journal(true);
    return concern;
}

MongoDbMgr::MongoDbMgr()
 :IMongoDbMgr(KERNEL_NS::RttiUtil::GetTypeId<MongoDbMgr>())
,_connectionPool(NULL)
,_eventLoopThread(NULL)
,_isDbReady{false}
,_dbFailErrCode{0}
,_willStarted{false}
{
 
}

MongoDbMgr::~MongoDbMgr()
{
    _Clear();
}

void MongoDbMgr::Release()
{
    MongoDbMgr::DeleteByAdapter_MongoDbMgr(MongoDbMgrFactory::_buildType.V, this);
}

void MongoDbMgr::OnRegisterComps()
{

}

void MongoDbMgr::SetUri(const KERNEL_NS::LibString &uri)
{
    _uri = uri;
}

void MongoDbMgr::SetAccountPwd(const KERNEL_NS::LibString &account, const KERNEL_NS::LibString &pwd)
{
    _account = account;

    // 转义
    _pwd = pwd;
}

void MongoDbMgr::SetSrvHostName(const KERNEL_NS::LibString &hostName)
{
    _srvHostName = hostName;
}

void MongoDbMgr::SetShardKeyInfo(const KERNEL_NS::LibString &dbName, const KERNEL_NS::LibString &collectionName, const std::vector<ShardKeyInfo> &shardKeyInfos)
{
    auto iterDb = _dbRefcollectionRefShardKeyInfos.find(dbName);
    if (iterDb == _dbRefcollectionRefShardKeyInfos.end())
        iterDb = _dbRefcollectionRefShardKeyInfos.insert(std::make_pair(dbName, std::unordered_map<KERNEL_NS::LibString, std::vector<ShardKeyInfo>>())).first;

    auto &collectionRefShardKeyInfos = iterDb->second;
    auto iterCollection = collectionRefShardKeyInfos.find(collectionName);
    if (iterCollection == collectionRefShardKeyInfos.end())
        iterCollection = collectionRefShardKeyInfos.insert(std::make_pair(collectionName, std::vector<ShardKeyInfo>())).first;
    iterCollection->second = shardKeyInfos;

    CLOG_INFO("SetShardKeyInfo db:%s collection:%s, shard key:%s", dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(shardKeyInfos, ',').c_str());
}

void MongoDbMgr::CreateIndex(const KERNEL_NS::LibString &dbName, const KERNEL_NS::LibString &collectionName, const KERNEL_NS::LibString &indexName, const std::vector<std::pair<KERNEL_NS::LibString, Int32>> &fields, bool unique)
{
    auto iterDb = _dbRefCollectionRefMongoIndexInfos.find(dbName);
    if (iterDb == _dbRefCollectionRefMongoIndexInfos.end())
        iterDb = _dbRefCollectionRefMongoIndexInfos.insert(std::make_pair(dbName, std::unordered_map<KERNEL_NS::LibString, std::unordered_map<KERNEL_NS::LibString, MongoIndexInfo>>())).first;

    auto &collectionRefMongoIndexInfos = iterDb->second;
    auto iter = collectionRefMongoIndexInfos.find(collectionName);
    if(iter == collectionRefMongoIndexInfos.end())
    {
        iter = collectionRefMongoIndexInfos.insert(std::make_pair(collectionName, std::unordered_map<KERNEL_NS::LibString, MongoIndexInfo>())).first;
    }

    auto &indexNameRefInfo = iter->second;
    auto iterInfo = indexNameRefInfo.find(indexName);
    if(iterInfo == indexNameRefInfo.end())
    {
        iterInfo = indexNameRefInfo.insert(std::make_pair(indexName, MongoIndexInfo())).first;
    }

    auto &mongoIndexInfo = iterInfo->second;
    mongoIndexInfo.IndexName = indexName;
    mongoIndexInfo.Fields = fields;
    mongoIndexInfo.Unique = unique;
    
    KERNEL_NS::LibString fieldsStr;
    for(auto &field:fields)
    {
        fieldsStr.AppendFormat("%s:%d, ", field.first.c_str(), field.second);
    }
    CLOG_INFO("create index dbName:%s, collectionName:%s, indexName:%s fields:%s", dbName.c_str(), collectionName.c_str(), indexName.c_str(), fieldsStr.c_str());
}


#ifdef CRYSTAL_NET_CPP20

KERNEL_NS::CoTask<bool> MongoDbMgr::Query(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collection, KERNEL_NS::LibString keyName, UInt64 keyValue)
{
    co_return false;
}

KERNEL_NS::CoTask<bool> MongoDbMgr::Query(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collection, KERNEL_NS::LibString keyName, KERNEL_NS::LibString keyValue)
{
    co_return false;
}

KERNEL_NS::CoTask<bool> MongoDbMgr::AddData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, KERNEL_NS::LibString keyName, Int64 keyValue)
{
    auto isSuc = co_await _eventLoopThread->SendAsync<MongoAsyncRes>([this, dbName, collectionName, keyName, keyValue]()->MongoAsyncRes
    {
        MongoAsyncRes res;
        try
        {
            auto client = _connectionPool->acquire();
            auto db = client[dbName.GetRaw()];
            auto collection = db[collectionName.GetRaw()];
                    
            // 设置表的大多数写成功, 且journal写完成功才算成功
            auto concern = _MakeMongoMajorityWriteConcern();
            collection.write_concern(concern);

            auto ret = collection.insert_one(bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp(keyName.GetRaw(), keyValue)));
            if(!ret)
            {
                CLOG_ERROR("Failed to insert data into collection, dbName:%s, collectionName:%s, keyName:%s, keyValue:%lld", dbName.GetRaw().c_str(), collectionName.GetRaw().c_str()
                    , keyName.GetRaw().c_str(), keyValue);
                return res;
            }

            auto &&id = ret->inserted_id().get_oid().value.to_string();
            CLOG_DEBUG("insert one success, dbName:%s, collectionName:%s, keyName:%s, keyValue:%lld, mongodb _id:%s", dbName.GetRaw().c_str(), collectionName.GetRaw().c_str()
                    , keyName.GetRaw().c_str(), keyValue, id.c_str());
            res.IsSuccess = true;
        }
        catch (const mongocxx::exception &e)
        {
            CLOG_ERROR("AddData mongodb mongocxx exception: %s, dbName:%s collectionName:%s, keyName:%s, keyValue:%lld"
                , e.what(), dbName.c_str(), collectionName.c_str(), keyName.c_str(), keyValue);
            return res;
        }
        catch (const std::exception &e)
        {
            CLOG_ERROR("AddData mongodb std exception: %s, dbName:%s collectionName:%s, keyName:%s, keyValue:%lld"
                , e.what(), dbName.c_str(), collectionName.c_str(), keyName.c_str(), keyValue);
            return res;
        }
        catch (...)
        {
            CLOG_ERROR("AddData mongodb unknown exception: %s, dbName:%s collectionName:%s, keyName:%s, keyValue:%lld"
                , dbName.c_str(), collectionName.c_str(), keyName.c_str(), keyValue);
            
            return res;
        }

        return res;
    });

    if(!isSuc->IsSuccess)
    {
        CLOG_ERROR("AddData fail, db:%s, collection:%s, key:%s, value:%lld", dbName.c_str(), collectionName.c_str(), keyName.c_str(), keyValue);
        co_return false;
    }

    CLOG_DEBUG("AddData success, db:%s, collection:%s, key:%s, value:%lld", dbName.c_str(), collectionName.c_str(), keyName.c_str(), keyValue);
    co_return true;
}

KERNEL_NS::CoTask<bool> MongoDbMgr::AddData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, KERNEL_NS::LibString keyName, KERNEL_NS::LibString keyValue)
{
    auto isSuc = co_await _eventLoopThread->SendAsync<MongoAsyncRes>([this, dbName, collectionName, keyName, keyValue]()->MongoAsyncRes
    {
        MongoAsyncRes res;
        try
        {
            auto client = _connectionPool->acquire();
            auto db = client[dbName];
            auto collection = db[collectionName];
                    
            // 设置表的大多数写成功, 且journal写完成功才算成功
            auto &&concern = _MakeMongoMajorityWriteConcern();
            collection.write_concern(concern);

            auto ret = collection.insert_one(bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp(keyName.GetRaw(), keyValue.GetRaw())));
            if(!ret)
            {
                CLOG_ERROR("Failed to insert data into collection, dbName:%s, collectionName:%s, keyName:%s, keyValue:%s", dbName.c_str(), collectionName.c_str()
                    , keyName.c_str(), keyValue.c_str());
                return res;
            }

            auto &&id = ret->inserted_id().get_oid().value.to_string();
            CLOG_DEBUG("insert one success, dbName:%s, collectionName:%s, keyName:%s, keyValue:%s, mongodb _id:%s", dbName.c_str(), collectionName.c_str()
                    , keyName.c_str(), keyValue.c_str(), id.c_str());
            res.IsSuccess = true;
        }
        catch (const mongocxx::exception &e)
        {
            CLOG_ERROR("AddData mongodb mongocxx exception: %s, dbName:%s collectionName:%s, keyName:%s, keyValue:%s"
                , e.what(), dbName.c_str(), collectionName.c_str(), keyName.c_str(), keyValue.c_str());
            return res;
        }
        catch (const std::exception &e)
        {
            CLOG_ERROR("AddData mongodb std exception: %s, dbName:%s collectionName:%s, keyName:%s, keyValue:%s"
                , e.what(), dbName.c_str(), collectionName.c_str(), keyName.c_str(), keyValue.c_str());
            return res;
        }
        catch (...)
        {
            CLOG_ERROR("AddData mongodb unknown exception: %s, dbName:%s collectionName:%s, keyName:%s, keyValue:%s"
                , dbName.c_str(), collectionName.c_str(), keyName.c_str(), keyValue.c_str());
            
            return res;
        }

        return res;
    });

    if(!isSuc->IsSuccess)
    {
        CLOG_ERROR("AddData fail, db:%s, collection:%s, key:%s, value:%lld", dbName.c_str(), collectionName.c_str(), keyName.c_str(), keyValue);
        co_return false;
    }

    CLOG_DEBUG("AddData success, db:%s, collection:%s, key:%s, value:%lld", dbName.c_str(), collectionName.c_str(), keyName.c_str(), keyValue);
    co_return true;
}

#endif

void MongoDbMgr::DbReady(bool isReady)
{
    _isDbReady.store(isReady, std::memory_order_release);
}

void MongoDbMgr::SetDbFailErr(Int32 err)
{
    _dbFailErrCode.store(err, std::memory_order_release);
}

Int32 MongoDbMgr::_OnHostInit()
{
    try
    {
        // 外部没有指定uri则使用srv的设置, 如果还是没有则报错
        if(_uri.empty())
        {
            if(_account.empty() || _pwd.empty() || _srvHostName.empty())
            {
                CLOG_ERROR("have no mongodb+srv uri config, please check, account:%s, pwd:%s, srvHostName:%s", _account.c_str(), _pwd.c_str(), _srvHostName.c_str());
                return Status::ConfigError;
            }

            _uri.AppendFormat("mongodb+srv://%s:%s@%s/?authSource=admin&w=majority&journal=true&readConcernLevel=majority&maxPoolSize=100&connectTimeoutMS=180000&socketTimeoutMS=30000&retryWrites=true&retryReads=true&tls=false", _account.c_str(), _pwd.c_str(), _srvHostName.c_str());
        }

        auto uri = mongocxx::uri(_uri.GetRaw());
        _connectionPool = new mongocxx::pool(uri);

        _eventLoopThread = new KERNEL_NS::LibEventLoopThread("MongoDbMgr", new MongodbThreadStartup(this));
        _eventLoopThread->Start();
    }
    catch (const mongocxx::exception &e)
    {
        CLOG_ERROR("init mongodb mongocxx exception: %s, uri:%s account:%s, pwd:%s, srvHostName:%s", e.what(), _uri.c_str(), _account.c_str(), _pwd.c_str(), _srvHostName.c_str());
        return Status::ConfigError;
    }
    catch (const std::exception &e)
    {
        CLOG_ERROR("init mongodb std exception: %s, uri:%s account:%s, pwd:%s, srvHostName:%s", e.what(), _uri.c_str(), _account.c_str(), _pwd.c_str(), _srvHostName.c_str());
        return Status::ConfigError;
    }
    catch (...)
    {
        CLOG_ERROR("unknown exception: uri:%s account:%s, pwd:%s, srvHostName:%s", _uri.c_str(), _account.c_str(), _pwd.c_str(), _srvHostName.c_str());
        return Status::Failed;
    }
    
    return Status::Success;
}

Int32 MongoDbMgr::_OnCompsCreated()
{
    return Status::Success;
}

Int32 MongoDbMgr::_OnHostWillStart()
{
    _willStarted.store(true, std::memory_order_release);

    // 等待mongodb准备完成
    while (true)
    {
        // 是否准备就绪
        if(_isDbReady.load(std::memory_order_acquire))
        {
            CLOG_INFO("mongodb is ready.");
            break;
        }

        // db是否准备失败
        auto err = _dbFailErrCode.load(std::memory_order_acquire);
        if(err != Status::Success)
        {
            CLOG_ERROR("db fail err:%d", err);
            return err;
        }

        KERNEL_NS::SystemUtil::ThreadSleep(1000);
        CLOG_INFO("waiting mongodb, ready...");
    }
    return Status::Success;
}

Int32 MongoDbMgr::_OnHostStart()
{
    return Status::Success;
}

void MongoDbMgr::_OnHostBeforeCompsWillClose()
{
    
}

void MongoDbMgr::_OnHostClose()
{
    _eventLoopThread->Close();
    _Clear();
}

void MongoDbMgr::_Clear()
{
    CRYSTAL_DELETE_SAFE(_connectionPool);
    CRYSTAL_RELEASE_SAFE(_eventLoopThread);
}

KERNEL_END

