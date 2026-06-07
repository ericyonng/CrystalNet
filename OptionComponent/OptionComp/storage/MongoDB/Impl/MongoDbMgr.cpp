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

#include "bsoncxx/json.hpp"

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

            // 先拷贝必要的数据
            _mongodbMgr->_cantAddSchemaInfo.store(true, std::memory_order_release);
            _mongodbMgr->_schemaLock.Lock();
            auto dbShardKeyInfos = _mongodbMgr->_dbRefcollectionRefShardKeyInfos;
            auto dbRefCollectionRefIndexInfos = _mongodbMgr->_dbRefCollectionRefMongoIndexInfos;
            auto allForcosDbs = _mongodbMgr->_focusDbs;

            // 拉取所有库, 所有表的索引, 提取唯一索引
            CLOG_INFO("load all db unique index...");

            for(auto &dbName : allForcosDbs)
            {
                CLOG_INFO("load %s collections...", dbName.c_str());

                auto entry = _mongodbMgr->_connectionPool->acquire();
                auto db = entry[dbName];
                auto &&collectionNames = db.list_collection_names();
                for(auto &colName :collectionNames)
                {
                    auto collection = db[colName];
                    auto listIndex = collection.list_indexes();
                    for(auto &index : listIndex)
                    {
                        bool isUnique = false;
                        
                        auto indexName = index["name"];
                        KERNEL_NS::LibString indexFinalName;
                        if(indexName && (indexName.type() == bsoncxx::type::k_string))
                        {
                            // 获取indexName
                            indexFinalName = indexName.get_string().value.data();

                            // _id_默认是唯一的
                            if(indexFinalName == "_id_")
                            {
                                isUnique = true;
                                // _id默认是唯一的
                                auto indexKey = index["key"];
                                if(indexKey && indexKey.type() == bsoncxx::type::k_document)
                                {
                                    auto indexKeyDoc = indexKey.get_document();
                                    auto iterView = indexKeyDoc.view().find("_id");
                                    if(iterView != indexKeyDoc.view().end() && iterView->type() == bsoncxx::type::k_int32)
                                    {
                                        auto sortValue = iterView->get_int32().value;
                                        CLOG_INFO("load %s:%s index:%s isUnique:%d...", dbName.c_str(), colName.c_str(), indexFinalName.c_str(), isUnique);

                                        _mongodbMgr->_DoAddIndex(dbName, colName, "_id_", {std::make_pair("_id", sortValue)}, true);
                                        continue;
                                    }
                                }
                            }
                        }
                        
                        // _id默认是唯一的
                        auto indexKey = index["key"];
                        std::vector<std::pair<KERNEL_NS::LibString, Int32>> fields;
                        if(indexKey && indexKey.type() == bsoncxx::type::k_document)
                        {
                            auto indexKeyDoc = indexKey.get_document();
                            auto viewIndexKey = indexKeyDoc.view();
                            for(auto iter : viewIndexKey)
                            {
                                auto viewIterKey = iter.key();
                                auto keyValueIter = iter.get_value();
                                if((keyValueIter.type() == bsoncxx::type::k_string) && (keyValueIter.get_string().value == "hashed"))
                                {
                                    fields.push_back(std::make_pair(KERNEL_NS::LibString(viewIterKey.data()), -2));
                                }
                                else if(keyValueIter.type() == bsoncxx::type::k_int32)
                                {
                                    fields.push_back(std::make_pair(KERNEL_NS::LibString(viewIterKey.data()), keyValueIter.get_int32().value));
                                }
                            }
                            
                            auto iterIdView = indexKeyDoc.view().find("_id");
                            if(iterIdView != indexKeyDoc.view().end() && iterIdView->type() == bsoncxx::type::k_int32)
                            {
                                auto sortValue = iterIdView->get_int32().value;
                                isUnique = true;

                                if(!indexFinalName.empty())
                                {
                                    CLOG_INFO("load %s:%s indexFinalName:%s isUnique:%d...", dbName.c_str(), colName.c_str(), indexFinalName.c_str(), isUnique);

                                    _mongodbMgr->_DoAddIndex(dbName, colName, indexFinalName, {std::make_pair("_id", sortValue)}, true);
                                    continue;
                                }
                            }
                        }
                        
                        auto checkUnique = index["unique"];
                        if(checkUnique && checkUnique.type() == bsoncxx::type::k_bool)
                        {
                            isUnique = checkUnique.get_bool().value;
                            if(isUnique)
                            {
                                _mongodbMgr->_DoAddIndex(dbName, colName, indexFinalName, fields, true);
                            }
                        }
                        
                        CLOG_INFO("load %s:%s index:%s isUnique:%d...", dbName.c_str(), colName.c_str(), indexFinalName.c_str(), isUnique);
                    }
                }
            }
            CLOG_INFO("load all db unique index finish.");
            _mongodbMgr->_schemaLock.Unlock();
            
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
                    auto &shardKeyGroup = iterCollection.second;
                    auto entry = _mongodbMgr->_connectionPool->acquire();
                    auto connection = MongodbConnection::Create(entry);
                    auto isSuc = co_await connection->ShardCollection(dbName, collectionName, shardKeyGroup);
                    if(!isSuc)
                    {
                        CLOG_ERROR("mongodb ShardCollection fail dbname:%s collectionName:%s, shard keys:%s", dbName.c_str(), collectionName.c_str(), shardKeyGroup.ToString().c_str());
                        _mongodbMgr->SetDbFailErr(Status::Failed);
                        co_return;
                    }
                }
            }
            CLOG_INFO("ShardCollection success.");

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
,_cantAddSchemaInfo{false}
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

bool MongoDbMgr::SetShardKeyInfo(const KERNEL_NS::LibString &dbName, const KERNEL_NS::LibString &collectionName, const std::vector<ShardKeyInfo> &shardKeyInfos, bool isUnique)
{
    if(_cantAddSchemaInfo.load(std::memory_order_acquire))
    {
        CLOG_ERROR("cant add shard key info db name:%s, collection:%s, shardKeyInfos:%s", dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(shardKeyInfos, ',').c_str());
        return false;
    }

    _schemaLock.Lock();
    auto iterDb = _dbRefcollectionRefShardKeyInfos.find(dbName);
    if (iterDb == _dbRefcollectionRefShardKeyInfos.end())
        iterDb = _dbRefcollectionRefShardKeyInfos.insert(std::make_pair(dbName, std::unordered_map<KERNEL_NS::LibString, ShardKeyInfoGroup>())).first;

    auto &collectionRefShardKeyInfos = iterDb->second;
    auto iterCollection = collectionRefShardKeyInfos.find(collectionName);
    if (iterCollection == collectionRefShardKeyInfos.end())
        iterCollection = collectionRefShardKeyInfos.insert(std::make_pair(collectionName, ShardKeyInfoGroup())).first;
    iterCollection->second.ShardKeyInfos = shardKeyInfos;
    iterCollection->second.IsUnique = isUnique;
    auto &shardKeyGroup = iterCollection->second;
    _schemaLock.Unlock();
    
    CLOG_INFO("SetShardKeyInfo db:%s collection:%s, shard key:%s", dbName.c_str(), collectionName.c_str(), shardKeyGroup.ToString().c_str());

    // 自动关注
    FocusDb(dbName);
    return true;
}

bool MongoDbMgr::CreateIndex(const KERNEL_NS::LibString &dbName, const KERNEL_NS::LibString &collectionName, const KERNEL_NS::LibString &indexName, const std::vector<std::pair<KERNEL_NS::LibString, Int32>> &fields, bool unique)
{
    if(_cantAddSchemaInfo.load(std::memory_order_acquire))
    {
        KERNEL_NS::LibString fieldStr;
        for(auto &field : fields)
        {
            fieldStr.AppendFormat("%s:%d, ", field.first.c_str(), field.second);
        }
        CLOG_ERROR("cant add index db name:%s, collection:%s, indexName:%s, fields:%s, unique:%d"
            , dbName.c_str(), collectionName.c_str(), indexName.c_str(), fieldStr.c_str(), unique);
        
        return false;
    }

    _schemaLock.Lock();
    _DoAddIndex(dbName, collectionName, indexName, fields, unique);
    _schemaLock.Unlock();

    {
        KERNEL_NS::LibString fieldsStr;
        for(auto &field:fields)
        {
            fieldsStr.AppendFormat("%s:%d, ", field.first.c_str(), field.second);
        }
        CLOG_INFO("create index dbName:%s, collectionName:%s, indexName:%s fields:%s", dbName.c_str(), collectionName.c_str(), indexName.c_str(), fieldsStr.c_str());
    }

    // 自动关注
    FocusDb(dbName);
    return true;
}

void MongoDbMgr::_DoAddIndex(const KERNEL_NS::LibString &dbName, const KERNEL_NS::LibString &collectionName, const KERNEL_NS::LibString &indexName, const std::vector<std::pair<KERNEL_NS::LibString, Int32>> &fields, bool unique)
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
    CLOG_INFO("add index info db:%s, collection:%s, indexName:%s, fields:%s, unique:%d", dbName.c_str(), collectionName.c_str(), indexName.c_str(), KERNEL_NS::StringUtil::ToString(mongoIndexInfo.Fields, ',', [](const std::pair<KERNEL_NS::LibString, Int32> &elem) ->KERNEL_NS::LibString
    {
        return KERNEL_NS::LibString().AppendFormat("%s:%d", elem.first.c_str(), elem.second);
    }).c_str(), unique);
}


bool MongoDbMgr::FocusDb(const KERNEL_NS::LibString &dbName)
{
    if(_cantAddSchemaInfo.load(std::memory_order_acquire))
    {
        CLOG_ERROR("cant ForcusDb db name:%s", dbName.c_str());
        
        return false;
    }
    
    _schemaLock.Lock();
    _focusDbs.insert(dbName);
    _schemaLock.Unlock();

    CLOG_INFO("FocusDb :%s", dbName.c_str());

    return true;
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

KERNEL_NS::CoTask<bool> MongoDbMgr::AddData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, KERNEL_NS::LibString *jsonString)
{
    if(_focusDbs.find(dbName) == _focusDbs.end())
    {
        CLOG_ERROR("AddData fail, db:%s, not focus before will started, collection:%s, jsonString:%p", dbName.c_str(), collectionName.c_str(), jsonString);
        jsonString->Release();
        co_return false;
    }
    
    auto isSuc = co_await _eventLoopThread->SendAsync<MongoAsyncRes>([this, dbName, collectionName, jsonString]()->MongoAsyncRes
    {
        MongoAsyncRes res;
        SmartPtr<LibString, AutoDelMethods::Release> jsonStringPtr(jsonString);

        try
        {
            auto client = _connectionPool->acquire();
            auto db = client[dbName];
            auto collection = db[collectionName];
                    
            // 设置表的大多数写成功, 且journal写完成功才算成功
            auto &&concern = _MakeMongoMajorityWriteConcern();
            collection.write_concern(concern);

            auto &&bsonDoc = bsoncxx::from_json(*jsonStringPtr);

            // 唯一键检查
            if(!_CheckHasUniqueKey(dbName, collectionName, [&bsonDoc](const KERNEL_NS::LibString &idxName, const KERNEL_NS::LibString &field) -> bool
            {
                auto iterKey = bsonDoc.find(field);
                return iterKey != bsonDoc.end();
            }))
            {
                CLOG_ERROR("Failed to insert data into collection of:lack unique key please check, dbName:%s, collectionName:%s, jsonStringPtr:%s"
                    , dbName.c_str(), collectionName.c_str(), jsonStringPtr->c_str());
                                                                            
                return res;
            }
            
            auto ret = collection.insert_one(bsonDoc.view());
            if(!ret)
            {
                CLOG_ERROR("Failed to insert data into collection, dbName:%s, collectionName:%s, jsonStringPtr:%s", dbName.c_str(), collectionName.c_str()
                    , jsonStringPtr->c_str());
                return res;
            }

            auto &&id = ret->inserted_id().get_oid().value.to_string();
            CLOG_DEBUG("insert one success, dbName:%s, collectionName:%s, keyValue:%s, mongodb _id:%s", dbName.c_str(), collectionName.c_str()
                    , jsonStringPtr->c_str(), id.c_str());
            res.IsSuccess = true;
        }
        catch (const mongocxx::exception &e)
        {
            CLOG_ERROR("AddData mongodb mongocxx exception: %s, dbName:%s collectionName:%s, jsonStringPtr:%s"
                , e.what(), dbName.c_str(), collectionName.c_str(), jsonStringPtr->c_str());
            return res;
        }
        catch (const std::exception &e)
        {
            CLOG_ERROR("AddData mongodb std exception: %s, dbName:%s collectionName:%s, jsonStringPtr:%s"
                , e.what(), dbName.c_str(), collectionName.c_str(),  jsonStringPtr->c_str());
            return res;
        }
        catch (...)
        {
            CLOG_ERROR("AddData mongodb unknown exception: dbName:%s collectionName:%s, jsonStringPtr:%s"
                , dbName.c_str(), collectionName.c_str(), jsonStringPtr->c_str());
            
            return res;
        }

        return res;
    });

    if(!isSuc->IsSuccess)
    {
        CLOG_ERROR("AddData fail, db:%s, collection:%s", dbName.c_str(), collectionName.c_str());
        co_return false;
    }

    CLOG_DEBUG("AddData success, db:%s, collection:%s", dbName.c_str(), collectionName.c_str());
    co_return true;
}

KERNEL_NS::CoTask<bool> MongoDbMgr::AddData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::vector<std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant>> uniqueKv, KERNEL_NS::LibString binaryKeyName, KERNEL_NS::LibStreamTL *binaryData)
{
    if(_focusDbs.find(dbName) == _focusDbs.end())
    {
        CLOG_ERROR("AddData fail, db:%s, not focus before will started, collection:%s, kv:%s, binaryName:%s, binaryData:%p, len:%lld", dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',', [](const std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant> &elem)->KERNEL_NS::LibString
        {
            return KERNEL_NS::LibString().AppendFormat("%s:%s", elem.first.c_str(), elem.second.ToString().c_str());
        }).c_str(), binaryKeyName.c_str(), binaryData, binaryData->GetReadableSize());
        KERNEL_NS::LibStreamTL::DeleteThreadLocal_LibStream(binaryData);
        co_return false;
    }
    
    auto isSuc = co_await _eventLoopThread->SendAsync<MongoAsyncRes>([this, dbName, collectionName, uniqueKv, binaryKeyName, binaryData]()->MongoAsyncRes
    {
        MongoAsyncRes res;
        SmartPtr<KERNEL_NS::LibStreamTL, AutoDelMethods::CustomDelete> steam(binaryData);
        steam.SetClosureDelegate([](void *arg)
        {
            KERNEL_NS::LibStreamTL::DeleteThreadLocal_LibStream(KERNEL_NS::KernelCastTo<KERNEL_NS::LibStreamTL>(arg));
        });

        try
        {
            auto client = _connectionPool->acquire();
            auto db = client[dbName];
            auto collection = db[collectionName];
                    
            // 设置表的大多数写成功, 且journal写完成功才算成功
            auto &&concern = _MakeMongoMajorityWriteConcern();
            collection.write_concern(concern);

            // 唯一键检查
             if(!_CheckHasUniqueKey(dbName, collectionName, [&uniqueKv](const KERNEL_NS::LibString &idxName, const KERNEL_NS::LibString &field) -> bool
             {
                 for(auto &kv : uniqueKv)
                 {
                     if(kv.first == field)
                         return true;
                 }
                 return false;
             }))
             {
                 CLOG_ERROR("Failed to insert data into collection of:lack unique key please check, kv:%s dbName:%s, collectionName:%s, binaryKeyName:%s, binary size:%lld, binaryData:%p"
                     ,  KERNEL_NS::StringUtil::ToString(uniqueKv, ',', [](const std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant> &elem) -> KERNEL_NS::LibString
                     {
                         return KERNEL_NS::LibString().AppendFormat("%s:%s", elem.first.c_str(), elem.second.ToString().c_str());
                     }).c_str(), dbName.c_str(), collectionName.c_str(), binaryKeyName.c_str(), binaryData->GetReadableSize(), binaryData);
                 return res;
             }
            
            bsoncxx::builder::basic::document fullDoc;
            for(auto &kv : uniqueKv)
            {
                if(kv.second.IsBriefData())
                {
                    fullDoc.append(bsoncxx::builder::basic::kvp(kv.first.GetRaw(), kv.second.AsInt64()));
                }
                else if(kv.second.IsStr())
                {
                    fullDoc.append(bsoncxx::builder::basic::kvp(kv.first.GetRaw(), kv.second.AsStr().GetRaw()));
                }
            }

            // 存储二进制数据
            auto binData = bsoncxx::types::b_binary();
            binData.sub_type = bsoncxx::binary_sub_type::k_binary;
            binData.size = static_cast<uint32_t>(binaryData->GetReadableSize());
            binData.bytes = (uint8_t *) binaryData->GetReadBegin();
            fullDoc.append(bsoncxx::builder::basic::kvp(binaryKeyName.GetRaw(), binData));

            // extract让数据脱离构建器, 建议使用, 而不是使用view(),view是借用指针, 序列化的时候会拷走数据, 此处为了少一些拷贝用view()
            auto ret = collection.insert_one(fullDoc.view());
            if(!ret)
            {
                auto &&kvStr = KERNEL_NS::StringUtil::ToString(uniqueKv, ',', [](const std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant> &elem)-> KERNEL_NS::LibString
                {
                    return KERNEL_NS::LibString().AppendFormat("%s:%s", elem.first.c_str(), elem.second.ToString().c_str());
                });
                CLOG_ERROR("Failed to insert data into collection, dbName:%s, collectionName:%s, kvStr:%s, binarySize:%lld, binary:%p"
                    , dbName.c_str(), collectionName.c_str(), kvStr.c_str(), steam->GetReadableSize(), steam.AsSelf());
                return res;
            }

            auto &&id = ret->inserted_id().get_oid().value.to_string();
            CLOG_DEBUG("insert one success, dbName:%s, collectionName:%s, kv:%s, binarySize:%lld, binary:%p, mongodb _id:%s", dbName.c_str(), collectionName.c_str()
                , KERNEL_NS::StringUtil::ToString(uniqueKv, ',', [](const std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant> &elem)-> KERNEL_NS::LibString
                    {
                        return KERNEL_NS::LibString().AppendFormat("%s:%s", elem.first.c_str(), elem.second.ToString().c_str());
                    }).c_str(), steam->GetReadableSize(), steam.AsSelf(), id.c_str());
            
            res.IsSuccess = true;
        }
        catch (const mongocxx::exception &e)
        {
            auto &&kvStr = KERNEL_NS::StringUtil::ToString(uniqueKv, ',', [](const std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant> &elem)-> KERNEL_NS::LibString
            {
                return KERNEL_NS::LibString().AppendFormat("%s:%s", elem.first.c_str(), elem.second.ToString().c_str());
            });
            CLOG_ERROR("AddData mongodb mongocxx exception: %s, dbName:%s collectionName:%s, kvStr:%s, binarySize:%lld, binary:%p"
                , e.what(), dbName.c_str(), collectionName.c_str(), kvStr.c_str(), steam->GetReadableSize(), steam.AsSelf());
            return res;
        }
        catch (const std::exception &e)
        {
            auto &&kvStr = KERNEL_NS::StringUtil::ToString(uniqueKv, ',', [](const std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant> &elem)-> KERNEL_NS::LibString
            {
                return KERNEL_NS::LibString().AppendFormat("%s:%s", elem.first.c_str(), elem.second.ToString().c_str());
            });
            CLOG_ERROR("AddData mongodb std exception: %s, dbName:%s collectionName:%s, kvStr:%s, binarySize:%lld, binary:%p"
                , e.what(), dbName.c_str(), collectionName.c_str(), kvStr.c_str(), steam->GetReadableSize(), steam.AsSelf());
            return res;
        }
        catch (...)
        {
            auto &&kvStr = KERNEL_NS::StringUtil::ToString(uniqueKv, ',', [](const std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant> &elem)-> KERNEL_NS::LibString
            {
                return KERNEL_NS::LibString().AppendFormat("%s:%s", elem.first.c_str(), elem.second.ToString().c_str());
            });
            CLOG_ERROR("AddData mongodb unknown exception: dbName:%s collectionName:%s, kvStr:%s, binarySize:%lld, binary:%p"
                ,dbName.c_str(), collectionName.c_str(), kvStr.c_str(), steam->GetReadableSize(), steam.AsSelf());
            
            return res;
        }

        return res;
    });

    if(!isSuc->IsSuccess)
    {
        auto &&kvStr = KERNEL_NS::StringUtil::ToString(uniqueKv, ',', [](const std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant> &elem)-> KERNEL_NS::LibString
        {
            return KERNEL_NS::LibString().AppendFormat("%s:%s", elem.first.c_str(), elem.second.ToString().c_str());
        });
        
        CLOG_ERROR("AddData fail, db:%s, collection:%s, unique kv:%s", dbName.c_str(), collectionName.c_str(), kvStr.c_str());
        co_return false;
    }

    CLOG_DEBUG("AddData success, db:%s, collection:%s, uniqueKv:%s", dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',', [](const std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant> &elem)-> KERNEL_NS::LibString
    {
        return KERNEL_NS::LibString().AppendFormat("%s:%s", elem.first.c_str(), elem.second.ToString().c_str());
    }).c_str());
    co_return true;
}

KERNEL_NS::CoTask<bool> MongoDbMgr::DelData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::vector<std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant>> uniqueKv)
{
    if(_focusDbs.find(dbName) == _focusDbs.end())
    {
        CLOG_ERROR("DelData fail, db:%s, not focus before will started, collection:%s, uniqueKv:%s"
            , dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',', [](const std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant> &elem) -> LibString
            {
                return LibString().AppendFormat("%s:%s", elem.first.c_str(), elem.second.ToString().c_str());
            }).c_str());
        
        co_return false;
    }
    
    auto isSuc = co_await _eventLoopThread->SendAsync<MongoAsyncRes>([this, dbName, collectionName, uniqueKv]()->MongoAsyncRes
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
            
            bsoncxx::builder::basic::document fullKv;
            for(auto &kv : uniqueKv)
            {
                if(kv.second.IsBriefData())
                {
                    fullKv.append(bsoncxx::builder::basic::kvp(kv.first.GetRaw(), kv.second.AsInt64()));
                }
                else if(kv.second.IsStr())
                {
                    fullKv.append(bsoncxx::builder::basic::kvp(kv.first.GetRaw(), kv.second.AsStr().GetRaw()));
                }
            }
            
            auto result = collection.delete_one(fullKv.view());
            Int32 deletedCount = 0;
            if(result)
            {
                deletedCount = result->deleted_count();
            }

            CLOG_DEBUG("DelData success deletedCount:%d, keyJson:%s, db:%s, collection:%s, uniqueKv:%s"
                ,deletedCount,  bsoncxx::to_json(fullKv).c_str(), dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',', [](const std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant> &elem)
                {
                    return KERNEL_NS::LibString().AppendFormat("%s:%s", elem.first.c_str(), elem.second.ToString().c_str());
                }).c_str());

            res.IsSuccess = true;

            return res;
        }
        catch (const mongocxx::exception &e)
        {
            CLOG_ERROR("DelData fail exception:%s, db:%s, collection:%s, uniqueKv:%s"
            ,e.what(), dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',', [](const std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant> &elem)
            {
                return KERNEL_NS::LibString().AppendFormat("%s:%s", elem.first.c_str(), elem.second.ToString().c_str());
            }).c_str());
        }
        catch (const std::exception &e)
        {
            CLOG_ERROR("DelData fail std exception:%s, db:%s, collection:%s, uniqueKv:%s"
            ,e.what(), dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',', [](const std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant> &elem)
            {
                return KERNEL_NS::LibString().AppendFormat("%s:%s", elem.first.c_str(), elem.second.ToString().c_str());
            }).c_str());
        }
        catch (...)
        {
            CLOG_ERROR("DelData fail unknown exception, db:%s, collection:%s, uniqueKv:%s"
            , dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',', [](const std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant> &elem)
            {
                return KERNEL_NS::LibString().AppendFormat("%s:%s", elem.first.c_str(), elem.second.ToString().c_str());
            }).c_str());
        }

        return res;
    });

    if(!isSuc->IsSuccess)
    {
        CLOG_ERROR("DelData fail, db:%s, collection:%s, uniqueKv:%s", dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',', [](const std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant> &elem)
            {
                return KERNEL_NS::LibString().AppendFormat("%s:%s", elem.first.c_str(), elem.second.ToString().c_str());
            }).c_str());
        co_return false;
    }

    CLOG_DEBUG("DelData success, db:%s, collection:%s, uniqueKv:%s", dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',', [](const std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant> &elem)
            {
                return KERNEL_NS::LibString().AppendFormat("%s:%s", elem.first.c_str(), elem.second.ToString().c_str());
            }).c_str());
    
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

