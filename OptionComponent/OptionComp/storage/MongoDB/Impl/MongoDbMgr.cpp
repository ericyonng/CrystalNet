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


namespace 
{
    static ALWAYS_INLINE KERNEL_NS::LibString DictContainerToString(std::map<KERNEL_NS::LibString, KERNEL_NS::LibStreamTL *> *d)
    {
        return KERNEL_NS::StringUtil::ToStringBy(*d, ",", [](const std::pair<KERNEL_NS::LibString, KERNEL_NS::LibStreamTL *> &elem)->KERNEL_NS::LibString
            {
                auto stream = elem.second;
                auto readSize = static_cast<UInt64>(elem.second->GetReadableSize());
                
                return KERNEL_NS::LibString().AppendFormat("%s:%s(len:%llu)", elem.first.c_str()
                    , KERNEL_NS::LibBase64::Encode(stream->GetReadBegin(), readSize).c_str(), readSize);
            });
    }

    static ALWAYS_INLINE KERNEL_NS::LibString DictContainerToString(const std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> &kv)
    {
        return KERNEL_NS::StringUtil::ToStringBy(kv, ",", [](const std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant> &iter)->KERNEL_NS::LibString
        {
            return KERNEL_NS::LibString().AppendFormat("%s:%s", iter.first.c_str(), iter.second.ToString().c_str());
        });
    }

    static ALWAYS_INLINE KERNEL_NS::LibString DictContainerToString(const std::map<KERNEL_NS::Variant, KERNEL_NS::Variant> &kv)
    {
        return KERNEL_NS::StringUtil::ToStringBy(kv, ",", [](const std::pair<KERNEL_NS::Variant, KERNEL_NS::Variant> &iter)->KERNEL_NS::LibString
        {
            return KERNEL_NS::LibString().AppendFormat("%s:%s", iter.first.ToString().c_str(), iter.second.ToString().c_str());
        });
    }

    static ALWAYS_INLINE void DelStreamContainer(std::map<KERNEL_NS::LibString, KERNEL_NS::LibStreamTL *> *d)
    {
        KERNEL_NS::ContainerUtil::DelContainer(*d, [](KERNEL_NS::LibStreamTL *p)
        {
            KERNEL_NS::LibStreamTL::DeleteThreadLocal_LibStream(p);
        });
        CRYSTAL_DELETE_SAFE(d);
    }

    static ALWAYS_INLINE void DelVarDict(std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> *d)
    {
        KERNEL_NS::ContainerUtil::DelContainer(*d, [](const KERNEL_NS::Variant &var)
        {
            KERNEL_NS::VariantHelper::Del(var);
        });
        CRYSTAL_DELETE_SAFE(d);
    }

}

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

            try
            {
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

                auto entry = _mongodbMgr->_connectionPool->acquire();
                auto &&dataNames = entry->list_database_names();
                for (auto &dbName : dataNames)
                {
                    if (!allForcosDbs.contains(dbName))
                        continue;
                    
                    CLOG_INFO("load %s collections...", dbName.c_str());
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
            }
            catch (const mongocxx::exception &e)
            {
                CLOG_ERROR("init mongodb info fail, exception:%s", e.what());
                _mongodbMgr->SetDbFailErr(Status::Failed);
            }
            catch (const std::exception &e)
            {
                CLOG_ERROR("init mongodb info fail, std exception:%s", e.what());
                _mongodbMgr->SetDbFailErr(Status::Failed);
            }
            catch (...)
            {
                CLOG_ERROR("init mongodb info fail, unknown exception");
                _mongodbMgr->SetDbFailErr(Status::Failed);
            }
            
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
,_configMonitor(KERNEL_NS::FileMonitor<MongodbConfig, KERNEL_NS::YamlDeserializer>::New_FileMonitor())
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

void MongoDbMgr::SetConfigSource(const KERNEL_NS::SourceWrap &source)
{
    _sourceWrap = source;
}

void MongoDbMgr::SetConfigKeyName(const KERNEL_NS::LibString &keyName)
{
    _mongoConfigKeyName = keyName;
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

KERNEL_NS::CoTask<bool> MongoDbMgr::Query(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> *fieldNameRefVariant, bool ignoreOid)
{
    if(UNLIKELY(!fieldNameRefVariant))
    {
        CLOG_ERROR("query data fieldNameRefJsonString is null when query, db:%s, collection:%s, kv:%s", dbName.c_str(), collectionName.c_str(), DictContainerToString(kv).c_str());
        co_return false;
    }

    if(_focusDbs.find(dbName) == _focusDbs.end())
    {
        CLOG_ERROR("query data fail, db:%s, not focus before will started, collection:%s, kv:%s", dbName.c_str(), collectionName.c_str(), DictContainerToString(kv).c_str());
        co_return false;
    }
    
    auto isSuc = co_await _eventLoopThread->SendAsync<MongoAsyncRes>([this, ignoreOid, dbName, collectionName, kv, fieldNameRefVariant]()->MongoAsyncRes
    {
        MongoAsyncRes res;

        try
        {
            auto client = _connectionPool->acquire();
            auto db = client[dbName];
            auto collection = db[collectionName];
            // 设置majority, 常用的隔离级别,相当于读已提交级别(rc级别) 解决事务的隔离性问题
            mongocxx::read_concern rc;
            rc.acknowledge_level(mongocxx::read_concern::level::k_majority);
            collection.read_concern(rc);
                    
            bsoncxx::builder::basic::document fullKv;
            if(!_TurnDoc(kv, fullKv))
            {
                CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("query data build full kv fail, query data into collection, kv:%s dbName:%s, collectionName:%s"
                    ,  DictContainerToString(kv).c_str(), dbName.c_str(), collectionName.c_str()));

                return res;
            }

            
            mongocxx::options::find option;
            bsoncxx::builder::basic::document projectionDoc;
            bool hasProjection = false;
            if(!fieldNameRefVariant->empty())
            {
                for(auto iter : *fieldNameRefVariant)
                {
                    auto &key = iter.first;
                    projectionDoc.append(bsoncxx::builder::basic::kvp(key.GetRaw(), 1));
                    hasProjection = true;
                }

            }
            if(ignoreOid)
            {
                projectionDoc.append(bsoncxx::builder::basic::kvp("_id", 0));
                hasProjection = true;
            }
            if(hasProjection)
                option.projection(projectionDoc.view());

            auto ret = collection.find_one(fullKv.view(), option);
            if(!ret)
            {
                CLOG_WARN_ARGS(KERNEL_NS::LibString().AppendFormat("query data Failed to query data into collection, dbName:%s, collectionName:%s, kv:%s", dbName.c_str(), collectionName.c_str(), DictContainerToString(kv).c_str()));
                return res;
            }

            {
                auto &&value = ret.value();
                for(auto iter : value)
                {
                    auto key = KERNEL_NS::LibString(iter.key().data());
                    auto iterVar = fieldNameRefVariant->find(key);
                    if(iterVar == fieldNameRefVariant->end())
                        iterVar = fieldNameRefVariant->emplace(key, KERNEL_NS::Variant()).first;
                    
                    auto v = iter.get_value();
                    if(!_TurnVariant(v, iterVar->second))
                    {
                        CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("query data one key:%s, data _TurnVariant fail, dbName:%s, collectionName:%s, kv:%s data:",
                        key.c_str(), dbName.c_str(), collectionName.c_str(), DictContainerToString(kv).c_str()), KERNEL_NS::LibString(bsoncxx::to_json(ret.value())));

                        KERNEL_NS::VariantHelper::Del(iterVar->second);
                        fieldNameRefVariant->erase(key);
                    }
                }
            }

            CLOG_DEBUG_ARGS(KERNEL_NS::LibString().AppendFormat("query data one success, dbName:%s, collectionName:%s, kv:%s data:",
                dbName.c_str(), collectionName.c_str(), DictContainerToString(kv).c_str()), KERNEL_NS::LibString(bsoncxx::to_json(ret.value())));

            res.IsSuccess = true;
        }
        catch (const mongocxx::exception &e)
        {
            CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("query data mongodb mongocxx exception: %s, dbName:%s collectionName:%s, kv:"
                , e.what(), dbName.c_str(), collectionName.c_str()), DictContainerToString(kv));
            return res;
        }
        catch (const std::exception &e)
        {
            CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("query data mongodb std exception: %s, dbName:%s collectionName:%s, kv:"
                , e.what(), dbName.c_str(), collectionName.c_str()),  DictContainerToString(kv));
            return res;
        }
        catch (...)
        {
            CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("query data mongodb unknown exception: dbName:%s collectionName:%s, kv:"
                , dbName.c_str(), collectionName.c_str()), DictContainerToString(kv));
            
            return res;
        }

        return res;
    });

    if(!isSuc->IsSuccess)
    {
        CLOG_WARN("query data fail, db:%s, collection:%s, kv:%s", dbName.c_str(), collectionName.c_str(), DictContainerToString(kv).c_str());
        co_return false;
    }

    CLOG_DEBUG("query data success, db:%s, collection:%s, kv:%s", dbName.c_str(), collectionName.c_str(), DictContainerToString(kv).c_str());
    co_return true;
}


KERNEL_NS::CoTask<bool> MongoDbMgr::AddData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, KERNEL_NS::LibString *jsonString)
{
    if (UNLIKELY(!jsonString))
    {
        CLOG_ERROR("AddData fail, db:%s, not focus before will started, collection:%s, jsonString is null", dbName.c_str(), collectionName.c_str());

        co_return false;
    }
    
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
                CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("Failed to insert data into collection of:lack unique key please check, dbName:%s, collectionName:%s, jsonStringPtr:"
                    , dbName.c_str(), collectionName.c_str()), *jsonStringPtr);
                                                                            
                return res;
            }
            
            auto ret = collection.insert_one(bsonDoc.view());
            if(!ret)
            {
                CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("Failed to insert data into collection, dbName:%s, collectionName:%s, jsonStringPtr:", dbName.c_str(), collectionName.c_str()
                    ), *jsonStringPtr);
                return res;
            }

            auto &&id = ret->inserted_id().get_oid().value.to_string();
            CLOG_DEBUG_ARGS(KERNEL_NS::LibString().AppendFormat("insert one success, dbName:%s, collectionName:%s, mongodb _id:%s, jsonString:", dbName.c_str(), collectionName.c_str()
                    , id.c_str()), *jsonStringPtr);
            res.IsSuccess = true;
        }
        catch (const mongocxx::exception &e)
        {
            CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("AddData mongodb mongocxx exception: %s, dbName:%s collectionName:%s, jsonStringPtr:"
                , e.what(), dbName.c_str(), collectionName.c_str()), *jsonStringPtr);
            return res;
        }
        catch (const std::exception &e)
        {
            CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("AddData mongodb std exception: %s, dbName:%s collectionName:%s, jsonStringPtr:"
                , e.what(), dbName.c_str(), collectionName.c_str()),  *jsonStringPtr);
            return res;
        }
        catch (...)
        {
            CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("AddData mongodb unknown exception: dbName:%s collectionName:%s, jsonStringPtr:"
                , dbName.c_str(), collectionName.c_str()), *jsonStringPtr);
            
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


KERNEL_NS::CoTask<bool> MongoDbMgr::AddData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> uniqueKv, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStreamTL *> *binaryKeyNameRefData)
{
    if (UNLIKELY(!binaryKeyNameRefData))
    {
        CLOG_ERROR("AddData fail, db:%s, not focus before will started, collection:%s, kv:%s, binary data dict is null"
        , dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str());
        
        co_return false;
    }
   
    if(_focusDbs.find(dbName) == _focusDbs.end())
    {
        CLOG_ERROR("AddData fail, db:%s, not focus before will started, collection:%s, kv:%s, binary data dict:%s"
            , dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str()
            , DictContainerToString(binaryKeyNameRefData).c_str());

        DelStreamContainer(binaryKeyNameRefData);
        
        co_return false;
    }
    
    auto isSuc = co_await _eventLoopThread->SendAsync<MongoAsyncRes>([this, dbName, collectionName, uniqueKv, binaryKeyNameRefData]()->MongoAsyncRes
    {
        MongoAsyncRes res;
        SmartPtr<std::map<KERNEL_NS::LibString, KERNEL_NS::LibStreamTL *>, AutoDelMethods::CustomDelete> steamDict(binaryKeyNameRefData);
        steamDict.SetClosureDelegate([](void *arg)
        {
            auto dict = KERNEL_NS::KernelCastTo<std::map<KERNEL_NS::LibString, KERNEL_NS::LibStreamTL *>>(arg);
            DelStreamContainer(dict);
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
                 CLOG_ERROR("Failed to insert data into collection of:lack unique key please check, kv:%s dbName:%s, collectionName:%s, binary data:%s"
                     ,  KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str(), dbName.c_str(), collectionName.c_str(),  DictContainerToString(binaryKeyNameRefData).c_str());
                 return res;
             }
            
            bsoncxx::builder::basic::document fullDoc;
            if (!_TurnDoc(uniqueKv, fullDoc))
            {
                CLOG_ERROR("Failed to insert data into collection of:_TurnDoc fail, kv:%s dbName:%s, collectionName:%s, binary data:%s"
                    ,  KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str(), dbName.c_str(), collectionName.c_str(),  DictContainerToString(binaryKeyNameRefData).c_str());
                return res;
            }

            // 存储二进制数据
            for (auto &iter : *binaryKeyNameRefData)
            {
                auto data = iter.second;
                auto &keyName = iter.first;
                auto binData = bsoncxx::types::b_binary();
                binData.sub_type = bsoncxx::binary_sub_type::k_binary;
                binData.size = static_cast<uint32_t>(data->GetReadableSize());
                binData.bytes = (uint8_t *) data->GetReadBegin();
                fullDoc.append(bsoncxx::builder::basic::kvp(keyName.GetRaw(), binData));
            }

            // extract让数据脱离构建器, 建议使用, 而不是使用view(),view是借用指针, 序列化的时候会拷走数据, 此处为了少一些拷贝用view()
            auto ret = collection.insert_one(fullDoc.view());
            if(!ret)
            {
                auto &&kvStr = KERNEL_NS::StringUtil::ToString(uniqueKv, ',');
                CLOG_ERROR("Failed to insert data into collection, dbName:%s, collectionName:%s, kvStr:%s, binary data:%s"
                    , dbName.c_str(), collectionName.c_str(), kvStr.c_str(), DictContainerToString(binaryKeyNameRefData).c_str());
                return res;
            }

            auto &&id = ret->inserted_id().get_oid().value.to_string();
            CLOG_DEBUG("insert one success, dbName:%s, collectionName:%s, kv:%s, binary data:%s mongodb _id:%s", dbName.c_str(), collectionName.c_str()
                , KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str(), DictContainerToString(binaryKeyNameRefData).c_str(), id.c_str());
            
            res.IsSuccess = true;
        }
        catch (const mongocxx::exception &e)
        {
            auto &&kvStr = KERNEL_NS::StringUtil::ToString(uniqueKv, ',');
            CLOG_ERROR("AddData mongodb mongocxx exception: %s, dbName:%s collectionName:%s, kvStr:%s, binary data:%s"
                , e.what(), dbName.c_str(), collectionName.c_str(), kvStr.c_str(), DictContainerToString(binaryKeyNameRefData).c_str());
            return res;
        }
        catch (const std::exception &e)
        {
            auto &&kvStr = KERNEL_NS::StringUtil::ToString(uniqueKv, ',');
            CLOG_ERROR("AddData mongodb std exception: %s, dbName:%s collectionName:%s, kvStr:%s, binary data:%s"
                , e.what(), dbName.c_str(), collectionName.c_str(), kvStr.c_str(), DictContainerToString(binaryKeyNameRefData).c_str());
            return res;
        }
        catch (...)
        {
            auto &&kvStr = KERNEL_NS::StringUtil::ToString(uniqueKv, ',');
            CLOG_ERROR("AddData mongodb unknown exception: dbName:%s collectionName:%s, kvStr:%s, binary data:%s"
                ,dbName.c_str(), collectionName.c_str(), kvStr.c_str(), DictContainerToString(binaryKeyNameRefData).c_str());
            
            return res;
        }

        return res;
    });

    if(!isSuc->IsSuccess)
    {
        auto &&kvStr = KERNEL_NS::StringUtil::ToString(uniqueKv, ',');
        
        CLOG_ERROR("AddData fail, db:%s, collection:%s, unique kv:%s", dbName.c_str(), collectionName.c_str(), kvStr.c_str());
        co_return false;
    }

    CLOG_DEBUG("AddData success, db:%s, collection:%s, uniqueKv:%s", dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str());
    
    co_return true;
}

KERNEL_NS::CoTask<bool> MongoDbMgr::AddData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> uniqueKv, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> *keyRefVariant)
{
    if (UNLIKELY(!keyRefVariant))
    {
        CLOG_ERROR("AddData fail, db:%s, not focus before will started, collection:%s, kv:%s, binary data dict is null"
        , dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str());
        
        co_return false;
    }
   
    if(_focusDbs.find(dbName) == _focusDbs.end())
    {
        CLOG_ERROR("AddData fail, db:%s, not focus before will started, collection:%s, kv:%s, binary data dict:%s"
            , dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str()
            , DictContainerToString(*keyRefVariant).c_str());

        DelVarDict(keyRefVariant);
        
        co_return false;
    }
    
    auto isSuc = co_await _eventLoopThread->SendAsync<MongoAsyncRes>([this, dbName, collectionName, uniqueKv, keyRefVariant]()->MongoAsyncRes
    {
        MongoAsyncRes res;
        SmartPtr<std::map<KERNEL_NS::LibString, KERNEL_NS::Variant>, AutoDelMethods::CustomDelete> steamDict(keyRefVariant);
        steamDict.SetClosureDelegate([](void *arg)
        {
            auto dict = KERNEL_NS::KernelCastTo<std::map<KERNEL_NS::LibString, KERNEL_NS::Variant>>(arg);
            DelVarDict(dict);
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
                 CLOG_ERROR("Failed to insert data into collection of:lack unique key please check, kv:%s dbName:%s, collectionName:%s, binary data:%s"
                     ,  KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str(), dbName.c_str(), collectionName.c_str(),  DictContainerToString(*keyRefVariant).c_str());
                 return res;
             }
            
            bsoncxx::builder::basic::document fullDoc;
            if (!_TurnDoc(uniqueKv, fullDoc))
            {
                CLOG_ERROR("Failed to insert data into collection of:_TurnDoc fail, kv:%s dbName:%s, collectionName:%s, binary data:%s"
                    ,  KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str(), dbName.c_str(), collectionName.c_str(),  DictContainerToString(*keyRefVariant).c_str());
                return res;
            }

            // 转doc
            if(!_TurnDoc(*keyRefVariant, fullDoc))
            {
                auto &&kvStr = KERNEL_NS::StringUtil::ToString(uniqueKv, ',');
                CLOG_ERROR("Failed to insert data into collection _TurnDoc fail, dbName:%s, collectionName:%s, kvStr:%s, binary data:%s"
                    , dbName.c_str(), collectionName.c_str(), kvStr.c_str(), DictContainerToString(*keyRefVariant).c_str());
                return res;
            }

            // extract让数据脱离构建器, 建议使用, 而不是使用view(),view是借用指针, 序列化的时候会拷走数据, 此处为了少一些拷贝用view()
            auto ret = collection.insert_one(fullDoc.view());
            if(!ret)
            {
                auto &&kvStr = KERNEL_NS::StringUtil::ToString(uniqueKv, ',');
                CLOG_ERROR("Failed to insert data into collection, dbName:%s, collectionName:%s, kvStr:%s, binary data:%s"
                    , dbName.c_str(), collectionName.c_str(), kvStr.c_str(), DictContainerToString(*keyRefVariant).c_str());
                return res;
            }

            auto &&id = ret->inserted_id().get_oid().value.to_string();
            CLOG_DEBUG("insert one success, dbName:%s, collectionName:%s, kv:%s, binary data:%s mongodb _id:%s", dbName.c_str(), collectionName.c_str()
                , KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str(), DictContainerToString(*keyRefVariant).c_str(), id.c_str());
            
            res.IsSuccess = true;
        }
        catch (const mongocxx::exception &e)
        {
            auto &&kvStr = KERNEL_NS::StringUtil::ToString(uniqueKv, ',');
            CLOG_ERROR("AddData mongodb mongocxx exception: %s, dbName:%s collectionName:%s, kvStr:%s, keyRefVariant:%s"
                , e.what(), dbName.c_str(), collectionName.c_str(), kvStr.c_str(), DictContainerToString(*keyRefVariant).c_str());
            return res;
        }
        catch (const std::exception &e)
        {
            auto &&kvStr = KERNEL_NS::StringUtil::ToString(uniqueKv, ',');
            CLOG_ERROR("AddData mongodb std exception: %s, dbName:%s collectionName:%s, kvStr:%s, keyRefVariant:%s"
                , e.what(), dbName.c_str(), collectionName.c_str(), kvStr.c_str(), DictContainerToString(*keyRefVariant).c_str());
            return res;
        }
        catch (...)
        {
            auto &&kvStr = KERNEL_NS::StringUtil::ToString(uniqueKv, ',');
            CLOG_ERROR("AddData mongodb unknown exception: dbName:%s collectionName:%s, kvStr:%s, keyRefVariant:%s"
                ,dbName.c_str(), collectionName.c_str(), kvStr.c_str(), DictContainerToString(*keyRefVariant).c_str());
            
            return res;
        }

        return res;
    });

    if(!isSuc->IsSuccess)
    {
        auto &&kvStr = KERNEL_NS::StringUtil::ToString(uniqueKv, ',');
        
        CLOG_ERROR("AddData fail, db:%s, collection:%s, unique kv:%s", dbName.c_str(), collectionName.c_str(), kvStr.c_str());
        co_return false;
    }

    CLOG_DEBUG("AddData success, db:%s, collection:%s, uniqueKv:%s", dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str());
    
    co_return true;
}


KERNEL_NS::CoTask<bool> MongoDbMgr::DelData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> uniqueKv)
{
    if(_focusDbs.find(dbName) == _focusDbs.end())
    {
        CLOG_ERROR("DelData fail, db:%s, not focus before will started, collection:%s, uniqueKv:%s"
            , dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str());
        
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
            if (!_TurnDoc(uniqueKv, fullKv))
            {
                CLOG_ERROR("DelData fail uniqueKv turn doc fail, db:%s, collection:%s, kv:%s"
                    ,dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str());
                
                return res;
            }
            
            auto result = collection.delete_one(fullKv.view());
            Int32 deletedCount = 0;
            if(result)
            {
                deletedCount = result->deleted_count();
            }

            CLOG_DEBUG("DelData success deletedCount:%d, keyJson:%s, db:%s, collection:%s, uniqueKv:%s"
                ,deletedCount,  bsoncxx::to_json(fullKv).c_str(), dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str());

            res.IsSuccess = true;

            return res;
        }
        catch (const mongocxx::exception &e)
        {
            CLOG_ERROR("DelData fail exception:%s, db:%s, collection:%s, uniqueKv:%s"
            ,e.what(), dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str());
        }
        catch (const std::exception &e)
        {
            CLOG_ERROR("DelData fail std exception:%s, db:%s, collection:%s, uniqueKv:%s"
            ,e.what(), dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str());
        }
        catch (...)
        {
            CLOG_ERROR("DelData fail unknown exception, db:%s, collection:%s, uniqueKv:%s"
            , dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str());
        }

        return res;
    });

    if(!isSuc->IsSuccess)
    {
        CLOG_ERROR("DelData fail, db:%s, collection:%s, uniqueKv:%s", dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str());
        co_return false;
    }

    CLOG_DEBUG("DelData success, db:%s, collection:%s, uniqueKv:%s", dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str());
    
    co_return true;
}


KERNEL_NS::CoTask<bool> MongoDbMgr::ReplaceData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::vector<KERNEL_NS::LibString> keyNames, KERNEL_NS::LibString *jsonString)
{
    if (UNLIKELY(!jsonString))
    {
        CLOG_ERROR("ReplaceData fail json string is null, db:%s, not focus before will started, collection:%s, keyNames:%s"
            , dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str());
        co_return false;
    }
    
    if(!_focusDbs.contains(dbName))
    {
        CLOG_ERROR("ReplaceData fail, db:%s, not focus before will started, collection:%s, keyNames:%s"
            , dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str());
        CRYSTAL_DELETE_SAFE(jsonString);

        co_return false;
    }

    auto isSuc = co_await _eventLoopThread->SendAsync<MongoAsyncRes>([this, dbName, collectionName, keyNames, jsonString]()->MongoAsyncRes
    {
        MongoAsyncRes res;
        SmartPtr<LibString, AutoDelMethods::Release> jsonStringPtr(jsonString);

        try
        {
            auto client = _connectionPool->acquire();
            auto db = client[dbName];
            auto collection = db[collectionName];
            
            auto &&bsonDoc = bsoncxx::from_json(*jsonStringPtr);

            // bsonDoc 有没有 uniqueKv
            for (auto &k :keyNames)
            {
                if (bsonDoc.find(k) == bsonDoc.end())
                {
                    CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("Failed to replace data into collection of:jsonString lack unique key please check, dbName:%s, collectionName:%s, keyNames:%s, jsonStringPtr:"
                        , dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str()), *jsonStringPtr);
                    
                    return  res;   
                }
            }

            // 唯一键检查
            if(!_CheckHasUniqueKey(dbName, collectionName, [&bsonDoc](const KERNEL_NS::LibString &idxName, const KERNEL_NS::LibString &field) -> bool
            {
                auto iterKey = bsonDoc.find(field);
                return iterKey != bsonDoc.end();
            }))
            {
                CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("Failed to replace data into collection of:lack unique key please check, dbName:%s, collectionName:%s, jsonStringPtr:"
                    , dbName.c_str(), collectionName.c_str()), *jsonStringPtr);
                                                                            
                return res;
            }

            // 唯一键检查
             if(!_CheckHasUniqueKey(dbName, collectionName, [&keyNames](const KERNEL_NS::LibString &idxName, const KERNEL_NS::LibString &field) -> bool
             {
                 for(auto &k : keyNames)
                 {
                     if(k == field)
                         return true;
                 }
                 return false;
             }))
             {
                 CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("Failed to replace data into collection of:lack unique key please check, keyNames:%s dbName:%s, collectionName:%s,  jsonStringPtr:"
                     ,  KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str(), dbName.c_str(), collectionName.c_str()), *jsonStringPtr);
                 return res;
             }
            
            // 设置表的大多数写成功, 且journal写完成功才算成功
            auto &&concern = _MakeMongoMajorityWriteConcern();
            collection.write_concern(concern);
            
            bsoncxx::builder::basic::document fullKv;
            for (auto &k : keyNames)
            {
                auto iter = bsonDoc.find(k);
                if (iter != bsonDoc.end())
                {
                    fullKv.append(bsoncxx::builder::basic::kvp(k.GetRaw(), iter->get_owning_value()));
                }
                else
                {
                    CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("build full kv fail, replace data into collection, keyNames:%s dbName:%s, collectionName:%s,  jsonStringPtr:"
                        ,  KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str(), dbName.c_str(), collectionName.c_str()), *jsonStringPtr);
                        return res;
                }
            }

            // 找不到则创建
            mongocxx::options::replace opts{};
            opts.upsert(true);  // 无匹配时插入新文档
            auto result = collection.replace_one(fullKv.view(), bsonDoc.view(), opts);
            bool isSuccess = false;
            if (result)
            {
                // 没找到uniqueKv对应的文档因为设置了upsert, 所以会插入文档, 返回upserted_id modified_count为0
                if (result->upserted_id())
                {
                    CLOG_INFO("replace data success, db:%s, collection:%s and not found unique key:%s, doc, add new doc to mongodb upserted_id:%s, modified_count:%d"
                        ,dbName.c_str(), collectionName.c_str()
                        , KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str(), result->upserted_id()->get_oid().value.to_string().c_str(), result->modified_count());

                    isSuccess = true;
                }
                else if (result->matched_count() > 0)
                {
                    // matched说明找到了文档, modified_count 为1表示新旧文档不同, 发生更新, 0表示新旧文档相同不更新
                    CLOG_INFO_ARGS(KERNEL_NS::LibString().AppendFormat("replace data success, db:%s, collection:%s, and found unique key:%s, doc, modified_count:%d, jsonStringPtr:", dbName.c_str(), collectionName.c_str()
                        , KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str(), result->modified_count()), *jsonStringPtr);

                    isSuccess = true;
                }
                else
                {
                    // 不太可能出现
                    CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("replace data unknown err from db, replace fail, and doc not update to db, db:%s, collection:%s, unique key:%s modified_count:%d, jsonStringPtr:", dbName.c_str(), collectionName.c_str()
                        , KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str(), result->modified_count()), *jsonStringPtr);

                }
            }
            else
            {
                // 不太可能出现
                CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("replace data unknown err from db, replace fail, and doc not update to db, db:%s, collection:%s, unique key:%s modified_count:%d, jsonStringPtr:", dbName.c_str(), collectionName.c_str()
                    , KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str(), result->modified_count()), *jsonStringPtr);
            }

            res.IsSuccess = isSuccess;

            return res;
        }
        catch (const mongocxx::exception &e)
        {
            CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("replace data fail exception:%s, db:%s, collection:%s, uniqueKv:%s jsonStringPtr:"
            ,e.what(), dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str()), *jsonStringPtr);
        }
        catch (const std::exception &e)
        {
            CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("replace data fail std exception:%s, db:%s, collection:%s, uniqueKv:%s, jsonStringPtr:"
            ,e.what(), dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str()), *jsonStringPtr);
        }
        catch (...)
        {
            CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("replace data fail unknown exception, db:%s, collection:%s, uniqueKv:%s, jsonStringPtr:"
            , dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str()), *jsonStringPtr);
        }

        return res;
    });

    // 此时jsonStringPtr已经释放
    if(!isSuc->IsSuccess)
    {
        CLOG_ERROR("replace data fail, db:%s, collection:%s, uniqueKv:%s", dbName.c_str(), collectionName.c_str()
            , KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str());
        co_return false;
    }

    CLOG_DEBUG("replace data success, db:%s, collection:%s, uniqueKv:%s", dbName.c_str(), collectionName.c_str()
        , KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str());
    
    co_return true;
}

KERNEL_NS::CoTask<bool> MongoDbMgr::ReplaceData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::vector<KERNEL_NS::LibString> keyNames, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> *replaceFields)
{
    if (UNLIKELY(!replaceFields))
    {
        CLOG_ERROR("ReplaceData fail replaceFields is null, db:%s, not focus before will started, collection:%s, uniqueKv:%s"
            , dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str());
        co_return false;
    }
    
    if(_focusDbs.find(dbName) == _focusDbs.end())
    {
        CLOG_ERROR("ReplaceData fail, db:%s, not focus before will started, collection:%s, uniqueKv:%s"
            , dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str());
        CRYSTAL_DELETE_SAFE(replaceFields);
        co_return false;
    }

    auto isSuc = co_await _eventLoopThread->SendAsync<MongoAsyncRes>([this, dbName, collectionName, keyNames, replaceFields]()->MongoAsyncRes
    {
        MongoAsyncRes res;
        SmartPtr<std::map<KERNEL_NS::LibString, KERNEL_NS::Variant>> replaceFieldsPtr(replaceFields);

        try
        {
            auto client = _connectionPool->acquire();
            auto db = client[dbName];
            auto collection = db[collectionName];
            
            bsoncxx::builder::basic::document bsonDoc;
            if(!_TurnDoc(*replaceFieldsPtr, bsonDoc))
            {
                CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnDoc replaceFieldsPtr fail to replace data into collection, kv:%s dbName:%s, collectionName:%s,  jsonStringPtr:"
                ,  KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str(), dbName.c_str(), collectionName.c_str())
                , KERNEL_NS::StringUtil::ToString(*replaceFieldsPtr, ','));
                
                return res;
            }
            
            // bsonDoc 有没有 uniqueKv
            auto &&bsonView = bsonDoc.view();
            for (auto &k :keyNames)
            {
                if (bsonView.find(k) == bsonView.end())
                {
                    CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("Failed to replace data into collection of:replaceFields lack unique key please check, dbName:%s, collectionName:%s, uniqueKv:%s, replaceFields:"
                        , dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str()), KERNEL_NS::StringUtil::ToString(*replaceFieldsPtr, ','));
                    
                    return  res;   
                }
            }

            // 唯一键检查
            if(!_CheckHasUniqueKey(dbName, collectionName, [&bsonView](const KERNEL_NS::LibString &idxName, const KERNEL_NS::LibString &field) -> bool
            {
                auto iterKey = bsonView.find(field);
                return iterKey != bsonView.end();
            }))
            {
                CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("Failed to replace data into collection of:lack unique key please check, dbName:%s, collectionName:%s, jsonStringPtr:"
                , dbName.c_str(), collectionName.c_str())
                , KERNEL_NS::StringUtil::ToString(*replaceFieldsPtr, ','));
                                                                            
                return res;
            }

            // 唯一键检查
             if(!_CheckHasUniqueKey(dbName, collectionName, [&keyNames](const KERNEL_NS::LibString &idxName, const KERNEL_NS::LibString &field) -> bool
             {
                 for(auto &k : keyNames)
                 {
                     if(k == field)
                         return true;
                 }
                 return false;
             }))
             {
                 CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("Failed to replace data into collection of:lack unique key please check, kv:%s dbName:%s, collectionName:%s,  jsonStringPtr:"
                     ,  KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str(), dbName.c_str(), collectionName.c_str())
                     , KERNEL_NS::StringUtil::ToString(*replaceFieldsPtr, ','));
                 return res;
             }
            
            // 设置表的大多数写成功, 且journal写完成功才算成功
            auto &&concern = _MakeMongoMajorityWriteConcern();
            collection.write_concern(concern);
            
            bsoncxx::builder::basic::document fullKv;
            for (auto &k : keyNames)
            {
                auto iter = bsonView.find(k);
                if (iter != bsonView.end())
                {
                    fullKv.append(bsoncxx::builder::basic::kvp(k.GetRaw(), iter->get_owning_value()));
                }
                else
                {
                    CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("build full kv fail, replace data into collection, keyNames:%s dbName:%s, collectionName:%s,  replaceFieldsPtr:"
                        ,  KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str(), dbName.c_str(), collectionName.c_str()), KERNEL_NS::StringUtil::ToString(*replaceFieldsPtr, ','));
                        return res;
                }
            }

            // 找不到则创建
            mongocxx::options::replace opts{};
            opts.upsert(true);  // 无匹配时插入新文档
            auto result = collection.replace_one(fullKv.view(), bsonDoc.view(), opts);
            bool isSuccess = false;
            if (result)
            {
                // 没找到uniqueKv对应的文档因为设置了upsert, 所以会插入文档, 返回upserted_id modified_count为0
                if (result->upserted_id())
                {
                    CLOG_INFO("replace data success, db:%s, collection:%s and not found unique key:%s, doc, add new doc to mongodb upserted_id:%s, modified_count:%d"
                        ,dbName.c_str(), collectionName.c_str()
                        , bsoncxx::to_json(fullKv).c_str(), result->upserted_id()->get_oid().value.to_string().c_str(), result->modified_count());

                    isSuccess = true;
                }
                else if (result->matched_count() > 0)
                {
                    // matched说明找到了文档, modified_count 为1表示新旧文档不同, 发生更新, 0表示新旧文档相同不更新
                    CLOG_INFO_ARGS(KERNEL_NS::LibString().AppendFormat("replace data success, db:%s, collection:%s, and found unique key:%s, doc, modified_count:%d", dbName.c_str(), collectionName.c_str()
                        , bsoncxx::to_json(fullKv).c_str(), result->modified_count()));

                    isSuccess = true;
                }
                else
                {
                    // 不太可能出现
                    CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("replace data unknown err from db, replace fail, and doc not update to db, db:%s, collection:%s, unique key:%s modified_count:%d, replaceFields:", dbName.c_str(), collectionName.c_str()
                        , bsoncxx::to_json(fullKv).c_str(), result->modified_count())
                            , KERNEL_NS::StringUtil::ToString(*replaceFieldsPtr, ','));

                }
            }
            else
            {
                // 不太可能出现
                CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("replace data unknown err from db, replace fail, and doc not update to db, db:%s, collection:%s, unique key:%s modified_count:%d, replaceFields:", dbName.c_str(), collectionName.c_str()
                    , bsoncxx::to_json(fullKv).c_str(), result->modified_count())
                        , KERNEL_NS::StringUtil::ToString(*replaceFieldsPtr, ','));
            }

            res.IsSuccess = isSuccess;

            return res;
        }
        catch (const mongocxx::exception &e)
        {
            CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("replace data fail exception:%s, db:%s, collection:%s, uniqueKv:%s replaceFields:"
            ,e.what(), dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str())
            , KERNEL_NS::StringUtil::ToString(*replaceFieldsPtr, ','));
        }
        catch (const std::exception &e)
        {
            CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("replace data fail std exception:%s, db:%s, collection:%s, uniqueKv:%s, replaceFields:"
            ,e.what(), dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str())
            , KERNEL_NS::StringUtil::ToString(*replaceFieldsPtr, ','));
        }
        catch (...)
        {
            CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("replace data fail unknown exception, db:%s, collection:%s, uniqueKv:%s, replaceFields:"
            , dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(keyNames, ',').c_str())
            , KERNEL_NS::StringUtil::ToString(*replaceFieldsPtr, ','));
        }

        return res;
    });

    // 此时jsonStringPtr已经释放
    if(!isSuc->IsSuccess)
    {
        CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("replace data fail, db:%s, collection:%s, uniqueKv:", dbName.c_str(), collectionName.c_str())
            , KERNEL_NS::StringUtil::ToString(keyNames, ','));
        co_return false;
    }

    CLOG_DEBUG_ARGS(KERNEL_NS::LibString().AppendFormat("replace data success, db:%s, collection:%s, uniqueKv:", dbName.c_str(), collectionName.c_str())
        , KERNEL_NS::StringUtil::ToString(keyNames, ','));
    
    co_return true;
}

KERNEL_NS::CoTask<bool> MongoDbMgr::ReplaceData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> uniqueKv, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStreamTL *> *binaryKeyNameRefData)
{
    if (UNLIKELY(!binaryKeyNameRefData))
    {
        CLOG_ERROR("ReplaceData fail binaryKeyNameRefData is null, db:%s, not focus before will started, collection:%s, kv:%s, binary data:%s", dbName.c_str(), collectionName.c_str()
            , KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str(), DictContainerToString(binaryKeyNameRefData).c_str());
        co_return false;
    }
    
    if(_focusDbs.find(dbName) == _focusDbs.end())
    {
        CLOG_ERROR("ReplaceData fail, db:%s, not focus before will started, collection:%s, kv:%s, binary data:%s", dbName.c_str(), collectionName.c_str()
            , KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str(), DictContainerToString(binaryKeyNameRefData).c_str());

        DelStreamContainer(binaryKeyNameRefData);
        
        co_return false;
    }
    
    auto isSuc = co_await _eventLoopThread->SendAsync<MongoAsyncRes>([this, dbName, collectionName, uniqueKv, binaryKeyNameRefData]()->MongoAsyncRes
    {
        MongoAsyncRes res;
        SmartPtr<std::map<KERNEL_NS::LibString, KERNEL_NS::LibStreamTL *>, AutoDelMethods::CustomDelete> steamDict(binaryKeyNameRefData);
        steamDict.SetClosureDelegate([](void *arg)
        {
            DelStreamContainer(KERNEL_NS::KernelCastTo<std::map<KERNEL_NS::LibString, KERNEL_NS::LibStreamTL *>>(arg));
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
                 CLOG_ERROR("Failed to ReplaceData into collection of:lack unique key please check, kv:%s dbName:%s, collectionName:%s, binary data:%s"
                     ,  KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str(), dbName.c_str(), collectionName.c_str(),DictContainerToString(binaryKeyNameRefData).c_str());
                 return res;
             }
            
            bsoncxx::builder::basic::document fullDoc;
            bsoncxx::builder::basic::document kvDoc;
            for(auto &kv : uniqueKv)
            {
                auto &firstStr = kv.first;
                auto &v = kv.second;
                if(v.IsBriefData() || v.IsStr())
                {
                    auto middleValue = bsoncxx::types::bson_value::value(bsoncxx::types::b_null{});
                    if(!_TurnSimpleToDoc(v, middleValue))
                    {
                        CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnSimpleToDoc fail value key:%s, value:", firstStr.c_str()), v.ToString());
                        return res;
                    }
                    if(middleValue.view().type() == bsoncxx::v_noabi::type::k_null)
                    {
                        CLOG_ERROR("Failed to replace data into collection of:lack unique key please check, kv:%s dbName:%s, collectionName:%s, binary data:%s"
                            ,  KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str(), dbName.c_str(), collectionName.c_str(), DictContainerToString(binaryKeyNameRefData).c_str());
                        
                        return res;
                    }
                    
                    fullDoc.append(bsoncxx::builder::basic::kvp(firstStr.GetRaw(), middleValue));
                    kvDoc.append(bsoncxx::builder::basic::kvp(firstStr.GetRaw(), middleValue));
                }
                else if(v.IsSeq())
                {
                    bsoncxx::builder::basic::array subArray;
                    if(!_TurnDoc(v.AsSequence(), subArray))
                    {
                        CLOG_ERROR("_TurnDoc fail value type to replace data into collection, kv:%s dbName:%s, collectionName:%s, binary data:%s, k:%s, vale:%s"
                        ,  KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str(), dbName.c_str(), collectionName.c_str(), DictContainerToString(binaryKeyNameRefData).c_str(), firstStr.c_str(), v.ToString().c_str());
                        
                        return res;
                    }
                    
                    fullDoc.append(bsoncxx::builder::basic::kvp(firstStr.GetRaw(), subArray.extract()));
                    kvDoc.append(bsoncxx::builder::basic::kvp(firstStr.GetRaw(), subArray.extract()));
                }
                else if(v.IsDict())
                {
                    bsoncxx::builder::basic::document subDoc;
                    if(!_TurnDoc(v.AsDict(), subDoc))
                    {
                        CLOG_ERROR("_TurnDoc dict fail value type to replace data into collection, kv:%s dbName:%s, collectionName:%s, binary data:%s, k:%s, vale:%s"
                        ,  KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str(), dbName.c_str(), collectionName.c_str(), DictContainerToString(binaryKeyNameRefData).c_str(), firstStr.c_str(), v.ToString().c_str());
                        
                        return res;
                    }
                    fullDoc.append(bsoncxx::builder::basic::kvp(firstStr.GetRaw(), subDoc.extract()));
                    kvDoc.append(bsoncxx::builder::basic::kvp(firstStr.GetRaw(), subDoc.extract()));
                }
                else
                {
                    CLOG_ERROR("unsurpport value type to replace data into collection of:lack unique key please check, kv:%s dbName:%s, collectionName:%s, binary data:%s, k:%s, vale:%s"
                        ,  KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str(), dbName.c_str(), collectionName.c_str(), DictContainerToString(binaryKeyNameRefData).c_str(), firstStr.c_str(), v.ToString().c_str());
                    return res;
                }
            }

            // 存储二进制数据
            for (auto &iter : *binaryKeyNameRefData)
            {
                auto &keyName = iter.first;
                auto data = iter.second;
                auto binData = bsoncxx::types::b_binary();
                binData.sub_type = bsoncxx::binary_sub_type::k_binary;
                binData.size = static_cast<uint32_t>(data->GetReadableSize());
                binData.bytes = (uint8_t *) data->GetReadBegin();
                fullDoc.append(bsoncxx::builder::basic::kvp(keyName.GetRaw(), binData));
            }

            // 找不到则创建
            mongocxx::options::replace opts{};
            opts.upsert(true);  // 无匹配时插入新文档
            auto result = collection.replace_one(kvDoc.view(), fullDoc.view(), opts);
            bool isSuccess = false;
            if (result)
            {
                // 没找到uniqueKv对应的文档因为设置了upsert, 所以会插入文档, 返回upserted_id modified_count为0
                if (result->upserted_id())
                {
                    CLOG_INFO("replace data success, db:%s, collection:%s and not found unique key:%s, doc, add new doc to mongodb upserted_id:%s, modified_count:%d"
                        ,dbName.c_str(), collectionName.c_str()
                        , KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str(), result->upserted_id()->get_oid().value.to_string().c_str(), result->modified_count());

                    isSuccess = true;
                }
                else if (result->matched_count() > 0)
                {
                    // matched说明找到了文档, modified_count 为1表示新旧文档不同, 发生更新, 0表示新旧文档相同不更新
                    CLOG_INFO_ARGS(KERNEL_NS::LibString().AppendFormat("replace data success, db:%s, collection:%s, and found unique key:%s, doc, modified_count:%d", dbName.c_str(), collectionName.c_str()
                        , KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str(), result->modified_count()));

                    isSuccess = true;
                }
                else
                {
                    // 不太可能出现
                    CLOG_ERROR("replace data unknown err from db, replace fail, and doc not update to db, db:%s, collection:%s, unique key:%s modified_count:%d, binary data:%s"
                        , dbName.c_str(), collectionName.c_str()
                        , KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str(), result->modified_count()
                            ,DictContainerToString(binaryKeyNameRefData).c_str());
                }
            }
            else
            {
                // 不太可能出现
                CLOG_ERROR("replace data unknown err from db, replace fail, and doc not update to db, db:%s, collection:%s, unique key:%s modified_count:%d,binary data:%s"
                    , dbName.c_str(), collectionName.c_str()
                    , KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str(), result->modified_count(), DictContainerToString(binaryKeyNameRefData).c_str());
            }

            res.IsSuccess = isSuccess;
        }
        catch (const mongocxx::exception &e)
        {
            auto &&kvStr = KERNEL_NS::StringUtil::ToString(uniqueKv, ',');
            CLOG_ERROR("replace data mongodb mongocxx exception: %s, dbName:%s collectionName:%s, kvStr:%s,binary data:%s"
                , e.what(), dbName.c_str(), collectionName.c_str(), kvStr.c_str(), DictContainerToString(binaryKeyNameRefData).c_str());
            return res;
        }
        catch (const std::exception &e)
        {
            auto &&kvStr = KERNEL_NS::StringUtil::ToString(uniqueKv, ',');
            CLOG_ERROR("replace data mongodb std exception: %s, dbName:%s collectionName:%s, kvStr:%s,binary data:%s"
                , e.what(), dbName.c_str(), collectionName.c_str(), kvStr.c_str(), DictContainerToString(binaryKeyNameRefData).c_str());
            return res;
        }
        catch (...)
        {
            auto &&kvStr = KERNEL_NS::StringUtil::ToString(uniqueKv, ',');
            CLOG_ERROR("replace data mongodb unknown exception: dbName:%s collectionName:%s, kvStr:%s, binary data:%s"
                ,dbName.c_str(), collectionName.c_str(), kvStr.c_str(), DictContainerToString(binaryKeyNameRefData).c_str());
            
            return res;
        }

        return res;
    });

    if(!isSuc->IsSuccess)
    {
        auto &&kvStr = KERNEL_NS::StringUtil::ToString(uniqueKv, ',');
        
        CLOG_ERROR("replace data fail, db:%s, collection:%s, unique kv:%s", dbName.c_str(), collectionName.c_str(), kvStr.c_str());
        co_return false;
    }

    CLOG_DEBUG("replace data success, db:%s, collection:%s, uniqueKv:%s", dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(uniqueKv, ',').c_str());
    
    co_return true;
}

KERNEL_NS::CoTask<bool> MongoDbMgr::UpdateData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv, KERNEL_NS::LibString *jsonFields, bool createIfNotExists)
{
    if (UNLIKELY(!jsonFields))
    {
        CLOG_ERROR("UpdateData fail jsonFields is null, db:%s, not focus before will started, collection:%s, keyNames:%s"
            , dbName.c_str(), collectionName.c_str(), DictContainerToString(kv).c_str());
        co_return false;
    }
    
    if(!_focusDbs.contains(dbName))
    {
        CLOG_ERROR("UpdateData fail, db:%s, not focus before will started, collection:%s, kv:%s"
            , dbName.c_str(), collectionName.c_str(), DictContainerToString(kv).c_str());
        CRYSTAL_DELETE_SAFE(jsonFields);

        co_return false;
    }

    auto isSuc = co_await _eventLoopThread->SendAsync<MongoAsyncRes>([this, createIfNotExists, dbName, collectionName, kv, jsonFields]()->MongoAsyncRes
    {
        MongoAsyncRes res;
        SmartPtr<LibString, AutoDelMethods::Release> jsonStringPtr(jsonFields);

        try
        {
            auto client = _connectionPool->acquire();
            auto db = client[dbName];
            auto collection = db[collectionName];

            // 唯一键检查如果需要在不存在的情况下创建, kv必须包含唯一键索引
            if(createIfNotExists)
            {
                 if(!_CheckHasUniqueKey(dbName, collectionName, [&kv](const KERNEL_NS::LibString &idxName, const KERNEL_NS::LibString &field) -> bool
                 {
                     for(auto &it : kv)
                     {
                         if(it.first == field)
                             return true;
                     }
                     return false;
                 }))
                 {
                    CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("Failed to update data into collection of:lack unique key please check, dbName:%s, collectionName:%s, kv:%s jsonStringPtr:"
                        , dbName.c_str(), collectionName.c_str(), DictContainerToString(kv).c_str()), *jsonStringPtr);
                                                                                                
                    return res;
                 }
            }
            
            auto &&bsonDoc = bsoncxx::from_json(*jsonStringPtr);
            
            // 设置表的大多数写成功, 且journal写完成功才算成功
            auto &&concern = _MakeMongoMajorityWriteConcern();
            collection.write_concern(concern);
            
            bsoncxx::builder::basic::document fullKv;
            if(!_TurnDoc(kv, fullKv))
            {
                CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("build full kv fail, update data into collection, kv:%s dbName:%s, collectionName:%s,  jsonStringPtr:"
                    ,  DictContainerToString(kv).c_str(), dbName.c_str(), collectionName.c_str()), *jsonStringPtr);

                return res;
            }

            // 找不到则创建
            mongocxx::options::update opts{};
            if(createIfNotExists)
                opts.upsert(true);

            auto result = collection.update_one(fullKv.view(), bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("$set", bsonDoc.view())), opts);
            if (!result)
            {
                // 极少操作失败
                CLOG_INFO("update data op fail, db:%s, collection:%s unique kv:%s"
                    ,dbName.c_str(), collectionName.c_str()
                    , DictContainerToString(kv).c_str());

                return res;
            }

            if(result->upserted_id())
            {
                CLOG_DEBUG("update data success not found data, and and new data,  db:%s, collection:%s unique kv:%s, modified count:%d, upsert_id:%s", dbName.c_str(), collectionName.c_str()
                    , DictContainerToString(kv).c_str(), result->modified_count(), result->upserted_id()->get_oid().value.to_string().c_str());

                res.IsSuccess = true;
                return res;
            }

            // 找不到数据
            if(result->matched_count() == 0)
            {
                CLOG_WARN("update data fail: not found data, db:%s, collection:%s unique kv:%s"
                    ,dbName.c_str(), collectionName.c_str()
                    , DictContainerToString(kv).c_str());
                
                return res;
            }

            // modified_count为0表示前后无变化, > 0 表示前后有变化, 变化的数量
            CLOG_DEBUG("update data success  db:%s, collection:%s unique kv:%s, modified count:%d", dbName.c_str(), collectionName.c_str()
                    , DictContainerToString(kv).c_str(), result->modified_count());
            
            res.IsSuccess = true;

            return res;
        }
        catch (const mongocxx::exception &e)
        {
            CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("update data fail exception:%s, db:%s, collection:%s, uniqueKv:%s jsonStringPtr:"
            ,e.what(), dbName.c_str(), collectionName.c_str(), DictContainerToString(kv).c_str()), *jsonStringPtr);
        }
        catch (const std::exception &e)
        {
            CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("update data fail std exception:%s, db:%s, collection:%s, uniqueKv:%s, jsonStringPtr:"
            ,e.what(), dbName.c_str(), collectionName.c_str(), DictContainerToString(kv).c_str()), *jsonStringPtr);
        }
        catch (...)
        {
            CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("update data fail unknown exception, db:%s, collection:%s, uniqueKv:%s, jsonStringPtr:"
            , dbName.c_str(), collectionName.c_str(), DictContainerToString(kv).c_str()), *jsonStringPtr);
        }

        return res;
    });

    // 此时jsonStringPtr已经释放
    if(!isSuc->IsSuccess)
    {
        CLOG_ERROR("update data fail, db:%s, collection:%s, uniqueKv:%s", dbName.c_str(), collectionName.c_str()
            , DictContainerToString(kv).c_str());
        co_return false;
    }

    CLOG_DEBUG("update data success, db:%s, collection:%s, uniqueKv:%s", dbName.c_str(), collectionName.c_str()
        , DictContainerToString(kv).c_str());
    
    co_return true;
}

KERNEL_NS::CoTask<bool> MongoDbMgr::UpdateData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> *updateFields, bool createIfNotExists)
{
    if (UNLIKELY(!updateFields))
    {
        CLOG_ERROR("UpdateData fail updateFields is null, db:%s, not focus before will started, collection:%s, kv:%s"
            , dbName.c_str(), collectionName.c_str(), DictContainerToString(kv).c_str());
        co_return false;
    }
    
    if(_focusDbs.find(dbName) == _focusDbs.end())
    {
        CLOG_ERROR("UpdateData fail, db:%s, not focus before will started, collection:%s, kv:%s"
            , dbName.c_str(), collectionName.c_str(), DictContainerToString(kv).c_str());
        CRYSTAL_DELETE_SAFE(updateFields);
        co_return false;
    }

    auto isSuc = co_await _eventLoopThread->SendAsync<MongoAsyncRes>([this, createIfNotExists, dbName, collectionName, kv, updateFields]()->MongoAsyncRes
    {
        MongoAsyncRes res;
        SmartPtr<std::map<KERNEL_NS::LibString, KERNEL_NS::Variant>> updateFieldsPtr(updateFields);

        try
        {
            auto client = _connectionPool->acquire();
            auto db = client[dbName];
            auto collection = db[collectionName];
            
            // 唯一键检查如果需要在不存在的情况下创建, kv必须包含唯一键索引
            if(createIfNotExists)
            {
                 if(!_CheckHasUniqueKey(dbName, collectionName, [&kv](const KERNEL_NS::LibString &idxName, const KERNEL_NS::LibString &field) -> bool
                 {
                     for(auto &it : kv)
                     {
                         if(it.first == field)
                             return true;
                     }
                     return false;
                 }))
                 {
                    CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("Failed to update data into collection of:lack unique key please check, dbName:%s, collectionName:%s, kv:%s updateFields:"
                        , dbName.c_str(), collectionName.c_str(), DictContainerToString(kv).c_str()), DictContainerToString(*updateFields));
                                                                                                
                    return res;
                 }
            }

            
            bsoncxx::builder::basic::document updateFieldsDoc;
            if(!_TurnDoc(*updateFields, updateFieldsDoc))
            {
                CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnDoc updateFieldsPtr fail to update data into collection, kv:%s dbName:%s, collectionName:%s,  updateFields:"
                ,  DictContainerToString(kv).c_str(), dbName.c_str(), collectionName.c_str())
                , KERNEL_NS::StringUtil::ToString(*updateFields, ','));
                
                return res;
            }
            
            // 设置表的大多数写成功, 且journal写完成功才算成功
            auto &&concern = _MakeMongoMajorityWriteConcern();
            collection.write_concern(concern);
            
            // 构建 $set: { ... } 操作符文档
            bsoncxx::builder::basic::document setDoc;
            setDoc.append(bsoncxx::builder::basic::kvp("$set", updateFieldsDoc.view()));
            
            bsoncxx::builder::basic::document fullKv;
            if(!_TurnDoc(kv, fullKv))
            {
                CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnDoc updateFieldsPtr fail to update data into collection, kv:%s dbName:%s, collectionName:%s,  updateFields:"
                ,  DictContainerToString(kv).c_str(), dbName.c_str(), collectionName.c_str())
                , KERNEL_NS::StringUtil::ToString(*updateFields, ','));

                return res;
            }

            // 找不到则创建
            mongocxx::options::update opts{};
            if(createIfNotExists)
                opts.upsert(true);  // 无匹配时插入新文档
            
            auto result = collection.update_one(fullKv.view(), setDoc.view(), opts);
            if (!result)
            {
                // 极少操作失败
                CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("update data op fail, db:%s, collection:%s unique kv:%s"
                    ,dbName.c_str(), collectionName.c_str()
                    , DictContainerToString(kv).c_str()), KERNEL_NS::StringUtil::ToString(*updateFields, ','));

                return res;
            }

            if(result->upserted_id())
            {
                CLOG_DEBUG("update data success not found data, and and new data,  db:%s, collection:%s unique kv:%s, modified count:%d, upsert_id:%s", dbName.c_str(), collectionName.c_str()
                    , DictContainerToString(kv).c_str(), result->modified_count(), result->upserted_id()->get_oid().value.to_string().c_str());

                res.IsSuccess = true;
                return res;
            }

            // 找不到数据
            if(result->matched_count() == 0)
            {
                CLOG_WARN_ARGS(KERNEL_NS::LibString().AppendFormat("update data fail: not found data, db:%s, collection:%s unique kv:%s"
                ,dbName.c_str(), collectionName.c_str()
                    , DictContainerToString(kv).c_str()), KERNEL_NS::StringUtil::ToString(*updateFields, ','));
                
                return res;
            }

            // modified_count为0表示前后无变化, > 0 表示前后有变化, 变化的数量
            CLOG_DEBUG("update data success  db:%s, collection:%s unique kv:%s, modified count:%d", dbName.c_str(), collectionName.c_str()
                    , DictContainerToString(kv).c_str(), result->modified_count());

            res.IsSuccess = true;

            return res;
        }
        catch (const mongocxx::exception &e)
        {
            CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("update data fail exception:%s, db:%s, collection:%s, kv:%s updateFields:"
            ,e.what(), dbName.c_str(), collectionName.c_str(), DictContainerToString(kv).c_str())
            , KERNEL_NS::StringUtil::ToString(*updateFields, ','));
        }
        catch (const std::exception &e)
        {
            CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("update data fail std exception:%s, db:%s, collection:%s, kv:%s, updateFields:"
            ,e.what(), dbName.c_str(), collectionName.c_str(), DictContainerToString(kv).c_str())
            , KERNEL_NS::StringUtil::ToString(*updateFields, ','));
        }
        catch (...)
        {
            CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("update data fail unknown exception, db:%s, collection:%s, kv:%s, updateFields:"
            , dbName.c_str(), collectionName.c_str(), DictContainerToString(kv).c_str())
            , KERNEL_NS::StringUtil::ToString(*updateFields, ','));
        }

        return res;
    });

    // 此时jsonStringPtr已经释放
    if(!isSuc->IsSuccess)
    {
        CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("update data fail, db:%s, collection:%s, kv:", dbName.c_str(), collectionName.c_str())
            , DictContainerToString(kv));
        co_return false;
    }

    CLOG_DEBUG_ARGS(KERNEL_NS::LibString().AppendFormat("update data success, db:%s, collection:%s, kv:", dbName.c_str(), collectionName.c_str())
        , DictContainerToString(kv));
    
    co_return true;
}

KERNEL_NS::CoTask<bool> MongoDbMgr::UpdateData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStreamTL *> *binaryKeyNameRefData, bool createIfNotExists)
{
    if (UNLIKELY(!binaryKeyNameRefData))
    {
        CLOG_ERROR("UpdateData fail binaryKeyNameRefData is null, db:%s, not focus before will started, collection:%s, kv:%s, binary data:%s", dbName.c_str(), collectionName.c_str()
            , KERNEL_NS::StringUtil::ToString(kv, ',').c_str(), DictContainerToString(binaryKeyNameRefData).c_str());
        co_return false;
    }
    
    if(_focusDbs.find(dbName) == _focusDbs.end())
    {
        CLOG_ERROR("UpdateData fail, db:%s, not focus before will started, collection:%s, kv:%s, binary data:%s", dbName.c_str(), collectionName.c_str()
            , KERNEL_NS::StringUtil::ToString(kv, ',').c_str(), DictContainerToString(binaryKeyNameRefData).c_str());

        DelStreamContainer(binaryKeyNameRefData);
        
        co_return false;
    }
    
    auto isSuc = co_await _eventLoopThread->SendAsync<MongoAsyncRes>([this, dbName, collectionName, kv, binaryKeyNameRefData, createIfNotExists]()->MongoAsyncRes
    {
        MongoAsyncRes res;
        SmartPtr<std::map<KERNEL_NS::LibString, KERNEL_NS::LibStreamTL *>, AutoDelMethods::CustomDelete> steamDict(binaryKeyNameRefData);
        steamDict.SetClosureDelegate([](void *arg)
        {
            DelStreamContainer(KERNEL_NS::KernelCastTo<std::map<KERNEL_NS::LibString, KERNEL_NS::LibStreamTL *>>(arg));
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
            if(createIfNotExists)
            {
                if(!_CheckHasUniqueKey(dbName, collectionName, [&kv](const KERNEL_NS::LibString &idxName, const KERNEL_NS::LibString &field) -> bool
                {
                    for(auto &iter : kv)
                    {
                        if(iter.first == field)
                            return true;
                    }
                    return false;
                }))
                {
                    CLOG_ERROR("Failed to UpdateData into collection of:lack unique key please check, kv:%s dbName:%s, collectionName:%s, binary data:%s"
                        ,  KERNEL_NS::StringUtil::ToString(kv, ',').c_str(), dbName.c_str(), collectionName.c_str(),DictContainerToString(binaryKeyNameRefData).c_str());
                    return res;
                }
            }
            
            bsoncxx::builder::basic::document kvDoc;
            for(auto &it : kv)
            {
                auto &k = it.first;
                auto &v = it.second;
                if(v.IsBriefData() || v.IsStr())
                {
                    auto middleValue = bsoncxx::types::bson_value::value(bsoncxx::types::b_null{});
                    if(!_TurnSimpleToDoc(v, middleValue))
                    {
                        CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnSimpleToDoc fail value key:%s, value:", k.c_str()), v.ToString());
                        return res;
                    }
                    if(middleValue.view().type() == bsoncxx::v_noabi::type::k_null)
                    {
                        CLOG_ERROR("Failed to update data into collection of:lack unique key please check, kv:%s dbName:%s, collectionName:%s, binary data:%s"
                            ,  DictContainerToString(kv).c_str(), dbName.c_str(), collectionName.c_str(), DictContainerToString(binaryKeyNameRefData).c_str());
                        
                        return res;
                    }
                    
                    kvDoc.append(bsoncxx::builder::basic::kvp(k.GetRaw(), middleValue));
                }
                else if(v.IsSeq())
                {
                    bsoncxx::builder::basic::array subArray;
                    if(!_TurnDoc(v.AsSequence(), subArray))
                    {
                        CLOG_ERROR("_TurnDoc fail value type to update data into collection, kv:%s dbName:%s, collectionName:%s, binary data:%s, k:%s, vale:%s"
                        ,  KERNEL_NS::StringUtil::ToString(kv, ',').c_str(), dbName.c_str(), collectionName.c_str(), DictContainerToString(binaryKeyNameRefData).c_str(), k.c_str(), v.ToString().c_str());
                        
                        return res;
                    }
                    
                    kvDoc.append(bsoncxx::builder::basic::kvp(k.GetRaw(), subArray.extract()));
                }
                else if(v.IsDict())
                {
                    bsoncxx::builder::basic::document subDoc;
                    if(!_TurnDoc(v.AsDict(), subDoc))
                    {
                        CLOG_ERROR("_TurnDoc dict fail value type to update data into collection, kv:%s dbName:%s, collectionName:%s, binary data:%s, k:%s, vale:%s"
                        ,  KERNEL_NS::StringUtil::ToString(kv, ',').c_str(), dbName.c_str(), collectionName.c_str(), DictContainerToString(binaryKeyNameRefData).c_str(), k.c_str(), v.ToString().c_str());
                        
                        return res;
                    }
                    kvDoc.append(bsoncxx::builder::basic::kvp(k.GetRaw(), subDoc.extract()));
                }
                else
                {
                    CLOG_ERROR("unsurpport value type to update data into collection of:lack unique key please check, kv:%s dbName:%s, collectionName:%s, binary data:%s, k:%s, vale:%s"
                        ,  KERNEL_NS::StringUtil::ToString(kv, ',').c_str(), dbName.c_str(), collectionName.c_str(), DictContainerToString(binaryKeyNameRefData).c_str(), k.c_str(), v.ToString().c_str());
                    return res;
                }
            }

            // 存储二进制数据
            bsoncxx::builder::basic::document fullDoc;
            for (auto &iter : *binaryKeyNameRefData)
            {
                auto &keyName = iter.first;
                auto data = iter.second;
                auto binData = bsoncxx::types::b_binary();
                binData.sub_type = bsoncxx::binary_sub_type::k_binary;
                binData.size = static_cast<uint32_t>(data->GetReadableSize());
                binData.bytes = (uint8_t *) data->GetReadBegin();
                fullDoc.append(bsoncxx::builder::basic::kvp(keyName.GetRaw(), binData));
            }

            // 构建 $set: { ... } 操作符文档
            bsoncxx::builder::basic::document setDoc;
            setDoc.append(bsoncxx::builder::basic::kvp("$set", fullDoc.view()));
            
            // 找不到则创建
            mongocxx::options::update opts{};
            if(createIfNotExists)
                opts.upsert(true);  // 无匹配时插入新文档
            auto result = collection.update_one(kvDoc.view(), setDoc.view(), opts);
            if (!result)
            {
                // 极少操作失败
                CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("update data op fail, db:%s, collection:%s unique kv:%s binary data:"
                    ,dbName.c_str(), collectionName.c_str()
                    , DictContainerToString(kv).c_str()), DictContainerToString(binaryKeyNameRefData));

                return res;
            }

            if(result->upserted_id())
            {
                CLOG_DEBUG("update data success not found data, and and new data,  db:%s, collection:%s unique kv:%s, modified count:%d, upsert_id:%s", dbName.c_str(), collectionName.c_str()
                    , DictContainerToString(kv).c_str(), result->modified_count(), result->upserted_id()->get_oid().value.to_string().c_str());

                res.IsSuccess = true;
                return res;
            }

            // 找不到数据
            if(result->matched_count() == 0)
            {
                CLOG_WARN_ARGS(KERNEL_NS::LibString().AppendFormat("update data fail: not found data, db:%s, collection:%s unique kv:%s binary data:"
                ,dbName.c_str(), collectionName.c_str()
                    , DictContainerToString(kv).c_str()), DictContainerToString(binaryKeyNameRefData));
                
                return res;
            }

            // modified_count为0表示前后无变化, > 0 表示前后有变化, 变化的数量
            CLOG_DEBUG("update data success  db:%s, collection:%s unique kv:%s, modified count:%d", dbName.c_str(), collectionName.c_str()
                    , DictContainerToString(kv).c_str(), result->modified_count());

            res.IsSuccess = true;
        }
        catch (const mongocxx::exception &e)
        {
            auto &&kvStr = KERNEL_NS::StringUtil::ToString(kv, ',');
            CLOG_ERROR("update data mongodb mongocxx exception: %s, dbName:%s collectionName:%s, kvStr:%s,binary data:%s"
                , e.what(), dbName.c_str(), collectionName.c_str(), kvStr.c_str(), DictContainerToString(binaryKeyNameRefData).c_str());
            return res;
        }
        catch (const std::exception &e)
        {
            auto &&kvStr = KERNEL_NS::StringUtil::ToString(kv, ',');
            CLOG_ERROR("update data mongodb std exception: %s, dbName:%s collectionName:%s, kvStr:%s,binary data:%s"
                , e.what(), dbName.c_str(), collectionName.c_str(), kvStr.c_str(), DictContainerToString(binaryKeyNameRefData).c_str());
            return res;
        }
        catch (...)
        {
            auto &&kvStr = KERNEL_NS::StringUtil::ToString(kv, ',');
            CLOG_ERROR("update data mongodb unknown exception: dbName:%s collectionName:%s, kvStr:%s, binary data:%s"
                ,dbName.c_str(), collectionName.c_str(), kvStr.c_str(), DictContainerToString(binaryKeyNameRefData).c_str());
            
            return res;
        }

        return res;
    });

    if(!isSuc->IsSuccess)
    {
        auto &&kvStr = KERNEL_NS::StringUtil::ToString(kv, ',');
        
        CLOG_ERROR("update data fail, db:%s, collection:%s, unique kv:%s", dbName.c_str(), collectionName.c_str(), kvStr.c_str());
        co_return false;
    }

    CLOG_DEBUG("update data success, db:%s, collection:%s, uniqueKv:%s", dbName.c_str(), collectionName.c_str(), KERNEL_NS::StringUtil::ToString(kv, ',').c_str());
    
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
        if ((!_sourceWrap.Path.empty()) || _sourceWrap.FromMemory)
        {
            if (!_configMonitor->Init(&_sourceWrap, _mongoConfigKeyName))
            {
                CLOG_ERROR("mongodb init config fail source wrap:%s,%p, _mongoConfigKeyName:%s"
                    , _sourceWrap.Path.c_str(), _sourceWrap.FromMemory, _mongoConfigKeyName.c_str());
                return Status::ConfigError;
            }

            auto current = _configMonitor->Current();
            _uri.AppendFormat("mongodb+srv://%s:%s@%s/?authSource=admin&w=majority&journal=true&readConcernLevel=majority&maxPoolSize=100&connectTimeoutMS=180000&socketTimeoutMS=30000&retryWrites=true&retryReads=true&tls=false"
                , current->Account.c_str(), current->Pwd.c_str(), current->SrvHostName.c_str());
        }
        else
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
    if (_configMonitor)
    {
        KERNEL_NS::FileMonitor<MongodbConfig, KERNEL_NS::YamlDeserializer>::Delete_FileMonitor(_configMonitor);
        _configMonitor = NULL;
    }
}


bool MongoDbMgr::_TurnDoc(const std::vector<KERNEL_NS::Variant> &fields, bsoncxx::builder::basic::array &doc)
{
    const Int32 count = static_cast<Int32>(fields.size());
    for(Int32 idx = 0; idx < count; ++idx)
    {
        auto &elem = fields[idx];
        if(elem.IsBriefData() || elem.IsStr())
        {
            auto middleValue = bsoncxx::types::bson_value::value(bsoncxx::types::b_null{});
            if(!_TurnSimpleToDoc(elem, middleValue))
            {
                CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnSimpleToDoc fail middle elem :"), elem.ToString());
                return false;
            }
            if(middleValue.view().type() == bsoncxx::v_noabi::type::k_null)
            {
                CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnDoc middle elem is null value:"), elem.ToString());
                return false;
            }
            doc.append(middleValue);
        }
        else if(elem.IsStr())
        {
            auto &&str = elem.AsStr();
            doc.append(str.GetRaw());
        }
        else if(elem.IsSeq())
        {
            bsoncxx::builder::basic::array subArray;
            if(!_TurnDoc(elem.AsSequence(), subArray))
            {
                CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnDoc sub array fail value:"), elem.ToString());
                return false;
            }
            doc.append(subArray.extract());
        }
        else if(elem.IsDict())
        {
            bsoncxx::builder::basic::document subDoc;
            if(!_TurnDoc(elem.AsDict(), subDoc))
            {
                CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnDoc sub doc fail value:"), elem.ToString());
                return false;
            }
            doc.append(subDoc.extract());
        }
        else
        {
            CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnDoc value type value:"), elem.ToString());
            return false;
        }
    }

    return true;
}

bool MongoDbMgr::_TurnDoc(const KERNEL_NS::Variant::Dict &fields, bsoncxx::builder::basic::document &doc)
{
    for(auto iter : fields)
    {
        auto &key = iter.first;
        auto &value = iter.second;

       if(key.IsStr() || key.IsBriefData())
        {
            if(value.IsBriefData() || value.IsStr())
            {
                auto middleValue = bsoncxx::types::bson_value::value(bsoncxx::types::b_null{});
                if(!_TurnSimpleToDoc(value, middleValue))
                {
                    CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnSimpleToDoc value fail key:%s value:", key.ToString().c_str()), value.ToString());
                    return false;
                }
                if(middleValue.view().type() == bsoncxx::v_noabi::type::k_null)
                {
                    CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnDoc middle value is null key:%s value:", key.ToString().c_str()), value.ToString());
                    return false;
                }
                auto &&keyStr = key.AsStr();
                doc.append(bsoncxx::builder::basic::kvp(keyStr.GetRaw(), middleValue));
            }
            else if(value.IsSeq())
            {
                bsoncxx::builder::basic::array arrDoc;
                if(!_TurnDoc(value.AsSequence(), arrDoc))
                {
                    CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnDoc arrDoc fail is key:%s value:", key.ToString().c_str()), value.ToString());
                    return false;
                }
                auto &&keyStr = key.AsStr();
                doc.append(bsoncxx::builder::basic::kvp(keyStr.GetRaw(), arrDoc.extract()));
            }
            else if(value.IsDict())
            {
                bsoncxx::builder::basic::document subDoc;
                if(!_TurnDoc(value.AsDict(), subDoc))
                {
                    CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnDoc subDoc fail is key:%s value:", key.ToString().c_str()), value.ToString());
                    return false;
                }
                auto &&keyStr = key.AsStr();
                doc.append(bsoncxx::builder::basic::kvp(keyStr.GetRaw(), subDoc.extract()));
            }
            else
            {
                CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnDoc unsupported value type key%s, value:", key.ToString().c_str()), value.ToString());
                return false;
            }
        }
        else
        {
            CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("TurnDoc unsupported key:%s, value:", key.ToString().c_str()), value.ToString());
            return false;
        }
    }

    return true;
}

bool MongoDbMgr::_TurnSimpleToDoc(const KERNEL_NS::Variant &elem, bsoncxx::types::bson_value::value &bsonValue)
{
    if(elem.IsBriefData())
    {
        if(elem.IsDouble())
        {
            bsonValue = static_cast<std::double_t>(elem.AsDouble());
        }
        else if(elem.IsFloat())
        {
            bsonValue = static_cast<std::float_t>(elem.AsFloat());
        }
        else if(elem.IsBool())
        {
            bsonValue = static_cast<bool>(elem.AsBool());
        }
        // 二进制数据
        else if(elem.IsStreamTL() || elem.IsStreamMT())
        {
            if(elem.IsStreamTL())
            {
                auto stream = elem.AsStreamTL();
                auto binData = bsoncxx::types::b_binary();
                binData.sub_type = bsoncxx::binary_sub_type::k_binary;
                binData.size = static_cast<uint32_t>(stream->GetReadableSize());
                binData.bytes = (uint8_t *) stream->GetReadBegin();
                bsonValue = binData;
            }
            else
            {
                auto stream = elem.AsStreamMT();
                auto binData = bsoncxx::types::b_binary();
                binData.sub_type = bsoncxx::binary_sub_type::k_binary;
                binData.size = static_cast<uint32_t>(stream->GetReadableSize());
                binData.bytes = (uint8_t *) stream->GetReadBegin();
                bsonValue = binData;
            }
        }
        else
        {
            bsonValue = static_cast<std::int64_t>(elem.AsInt64());
        }
        return true;
    }
    else if(elem.IsStr())
    {
        bsonValue = elem.AsStr();
        return true;
    }
    else
    {
        CLOG_ERROR_ARGS(KERNEL_NS::LibString("_TurnSimpleToDoc unsupported variant:"), elem.ToString());
    }

    return false;
}

bool MongoDbMgr::_TurnDoc(const std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> &fields, bsoncxx::builder::basic::document &doc)
{
    for(auto iter : fields)
    {
        auto &key = iter.first;
        auto &value = iter.second;
        if(value.IsBriefData() || value.IsStr())
        {
            auto middleValue = bsoncxx::types::bson_value::value(bsoncxx::types::b_null{});
            if(!_TurnSimpleToDoc(value, middleValue))
            {
                CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnSimpleToDoc value fail key:%s value:", key.c_str()), value.ToString());
                return false;
            }
            if(middleValue.view().type() == bsoncxx::v_noabi::type::k_null)
            {
                CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnDoc middle value is null key:%s value:", key.c_str()), value.ToString());
                return false;
            }
            doc.append(bsoncxx::builder::basic::kvp(key.GetRaw(), middleValue));
        }
        else if(value.IsSeq())
        {
            bsoncxx::builder::basic::array arrDoc;
            if(!_TurnDoc(value.AsSequence(), arrDoc))
            {
                CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnDoc arrDoc fail is key:%s value:", key.c_str()), value.ToString());
                return false;
            }
            doc.append(bsoncxx::builder::basic::kvp(key.GetRaw(), arrDoc.extract()));
        }
        else if(value.IsDict())
        {
            bsoncxx::builder::basic::document subDoc;
            if(!_TurnDoc(value.AsDict(), subDoc))
            {
                CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnDoc subDoc fail is key:%s value:", key.c_str()), value.ToString());
                return false;
            }
            doc.append(bsoncxx::builder::basic::kvp(key.GetRaw(), subDoc.extract()));
        }
        else
        {
            CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnDoc unsupported value type key%s, value:", key.c_str()), value.ToString());
            return false;
        }
    }

    return true;
}

bool MongoDbMgr::_TurnVariant(const bsoncxx::types::bson_value::view &bsonValue, KERNEL_NS::Variant &var)
{
    switch (bsonValue.type())
    {
        case bsoncxx::v_noabi::type::k_double:
        {
           var = bsonValue.get_double().value;
           break;     
        }
        case bsoncxx::v_noabi::type::k_string:
        {
            var = LibString(bsonValue.get_string().value.data(), static_cast<UInt64>(bsonValue.get_string().value.size()));
            break;    
        }
        // doc直接转json, 业务层自行从json解析
        case bsoncxx::v_noabi::type::k_document:
        {
            auto doc = bsonValue.get_document().value;
            var.BecomeDict();    
            for(auto iter = doc.begin(); iter != doc.end(); ++iter)
            {
                auto k = KERNEL_NS::LibString(iter->key().data());
                KERNEL_NS::Variant subVar;
                if(!_TurnVariant(iter->get_value(), subVar))
                {
                    CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnVariant fail when k_document, k:%s, doc:", k.c_str()), KERNEL_NS::LibString(bsoncxx::to_json(doc)));
                    return false;
                }
                var.InsertDict(k, subVar);
            }
            break;    
        }
        case bsoncxx::v_noabi::type::k_array:
        {
            var.BecomeSeq();
            auto arr = bsonValue.get_array().value;
            Int32 idx = 0;    
            for(auto iterArr = arr.begin(); iterArr != arr.end(); ++iterArr)
            {
                auto elem = iterArr->get_value();
                KERNEL_NS::Variant subValue;
                if(!_TurnVariant(elem, subValue))
                {
                    CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnVariant fail when k_array, idx:%d  doc:", idx));
                    return false;
                }
                ++idx;
                var.SeqPushBack(subValue);
            }
            break;    
        }
        // 二进制则通过LibStreamTL
        case bsoncxx::v_noabi::type::k_binary:
        {
            auto stream = KERNEL_NS::LibStreamTL::NewThreadLocal_LibStream();
            auto binaryData = bsonValue.get_binary();
            stream->Write(binaryData.bytes, static_cast<Int64>(binaryData.size));
            var = stream;
            break;
        }
        // oid转string
        case bsoncxx::v_noabi::type::k_oid:
        {
            var.BecomeStr();
            var = bsonValue.get_oid().value.to_string();    
            break;
        }
        case bsoncxx::v_noabi::type::k_bool:
        {
            var.BecomeBool();
            var = bsonValue.get_bool().value;    
            break;
        }
        // date转时间戳
        case bsoncxx::v_noabi::type::k_date:
        {
            var = bsonValue.get_date().value.count();    
            break;
        }
        case bsoncxx::v_noabi::type::k_int32:
        {
            var = bsonValue.get_int32().value;
            break;
        }
        // 时间戳（秒）
        case bsoncxx::v_noabi::type::k_timestamp:
        {
            var = bsonValue.get_timestamp().timestamp;
            break;
        }
        case bsoncxx::v_noabi::type::k_int64:
        {
            var = bsonValue.get_int64().value;
            break;
        }
        // 128位定点数, 输出字符串
        case bsoncxx::v_noabi::type::k_decimal128:
        {
            var = bsonValue.get_decimal128().value.to_string();
            break;
        }
        default:
        {
            CLOG_ERROR_ARGS(KERNEL_NS::LibString().AppendFormat("_TurnVariant fail unknown type:%d, doc:", bsonValue.type()));
            return false;
        }
    }

    return true;
}

KERNEL_END

