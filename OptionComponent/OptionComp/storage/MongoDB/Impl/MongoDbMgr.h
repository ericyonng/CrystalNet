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

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_MONGODB_MGR_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_MONGODB_MGR_H__

#pragma once

#include <OptionComp/storage/MongoDB/Interface/IMongoDbMgr.h>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#include <OptionComp/storage/MongoDB/Impl/MongoIndexInfo.h>
#include "bsoncxx/json.hpp"
#include <kernel/comp/FileMonitor/SourceWrap.h>
#include <kernel/comp/FileMonitor/FileMonitor.h>
#include <OptionComp/storage/MongoDB/Impl/MongodbConfig.h>
#include <kernel/comp/FileMonitor/YamlDeserializer.h>

KERNEL_BEGIN

class MongoDbMgr : public IMongoDbMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(IMongoDbMgr, MongoDbMgr);

public:
    MongoDbMgr();
    ~MongoDbMgr() override;

    // 释放
    virtual void Release() override;
    // 需要手动设置ready
    virtual void DefaultMaskReady(bool isReady) override {}
    virtual void OnRegisterComps() override;

    void SetUri(const KERNEL_NS::LibString &uri) override;

    // 默认如果不传入uri就是srv格式连接mongos(自动发现, 动态解析域名，支持断开重连新ip)
    // 账号密码
    virtual void SetAccountPwd(const KERNEL_NS::LibString &account, const KERNEL_NS::LibString &pwd) override;
    // 设置srv连接的域名
    virtual void SetSrvHostName(const KERNEL_NS::LibString &hostName) override;
    // 设置配置来源
    virtual void SetConfigSource(const KERNEL_NS::SourceWrap &source) override;
    virtual void SetConfigKeyName(const KERNEL_NS::LibString &keyName) override;

    // 给表设置分片键
    virtual bool SetShardKeyInfo(const KERNEL_NS::LibString &dbName, const KERNEL_NS::LibString &collectionName, const std::vector<ShardKeyInfo> &shardKeyInfos, bool isUnique = false) override;
    virtual bool CreateIndex(const KERNEL_NS::LibString &dbName, const KERNEL_NS::LibString &collectionName, const KERNEL_NS::LibString &indexName, const std::vector<std::pair<KERNEL_NS::LibString, Int32>> &fields, bool unique = false) override;
    virtual bool FocusDb(const KERNEL_NS::LibString &dbName) override;

    #ifdef CRYSTAL_NET_CPP20
    // 尝试获取分布式锁 (使用 findOneAndUpdate + upsert 原子操作)
    // 逻辑: 如果锁不存在则创建; 如果锁存在但已过期则更新; 如果锁存在且未过期则失败 lockName:目标, ownerId:锁id
    virtual CoTask<bool> TryAcquireLock(KERNEL_NS::LibString lockTargetId, KERNEL_NS::LibString lockId, KERNEL_NS::TimeSlice slice = KERNEL_NS::TimeSlice::FromSeconds(30)) override;
    
    // 释放分布式锁
    virtual CoTask<> ReleaseLock(const KERNEL_NS::LibString &lockTargetId, const KERNEL_NS::LibString &lockId) override;
    
    // 查任意数据, Variant支持json/二进制数据(LibStreamTL/LibStreamMT等)
    virtual KERNEL_NS::CoTask<bool> Query(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> *fieldNameRefVariant, bool ignoreOid = false) override;

    virtual KERNEL_NS::CoTask<bool> AddData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collection, KERNEL_NS::LibString *jsonString) override;
    virtual KERNEL_NS::CoTask<bool> AddData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> uniqueKv, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStreamTL *> *binaryKeyNameRefData) override;
    virtual KERNEL_NS::CoTask<bool> AddData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> uniqueKv, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> *keyRefVariant) override;

    virtual KERNEL_NS::CoTask<bool> DelData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> uniqueKv) override;

    // uniqueKv:唯一索引
    virtual KERNEL_NS::CoTask<bool> ReplaceData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::vector<KERNEL_NS::LibString> keyNames, KERNEL_NS::LibString *jsonString) override;
    virtual KERNEL_NS::CoTask<bool> ReplaceData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::vector<KERNEL_NS::LibString> keyNames, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> *replaceFields) override;
    virtual KERNEL_NS::CoTask<bool> ReplaceData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> uniqueKv, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStreamTL *> *binaryKeyNameRefData) override;

    virtual KERNEL_NS::CoTask<bool> UpdateData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv, KERNEL_NS::LibString *jsonFields, bool createIfNotExists = false) override;
    virtual KERNEL_NS::CoTask<bool> UpdateData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> *updateFields, bool createIfNotExists = false) override;
    virtual KERNEL_NS::CoTask<bool> UpdateData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStreamTL *> *binaryKeyNameRefData, bool createIfNotExists = false) override;

    #endif

    void DbReady(bool isReady);
    void SetDbFailErr(Int32 err);

protected:
    virtual Int32 _OnHostInit() override;
    virtual Int32 _OnCompsCreated() override;
    virtual Int32 _OnHostWillStart() override;

    virtual Int32 _OnHostStart() override;
    virtual void _OnHostBeforeCompsWillClose() override;
    virtual void _OnHostClose() override;
    void _Clear();
    bool _TurnDoc(const std::vector<KERNEL_NS::Variant> &fields, bsoncxx::builder::basic::array &doc);
    bool _TurnDoc(const KERNEL_NS::Variant::Dict &fields, bsoncxx::builder::basic::document &doc);
    bool _TurnSimpleToDoc(const KERNEL_NS::Variant &elem, bsoncxx::types::bson_value::value &bsonValue);
    bool _TurnDoc(const std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> &fields, bsoncxx::builder::basic::document &doc);
    // k_oid:转成string
    // k_date:Int64
    // k_timestamp:UInt32
    // k_decimal128:转string（定点数）
    bool _TurnVariant(const bsoncxx::types::bson_value::view &bsonValue, KERNEL_NS::Variant &var);

    template<typename LambdaType>
    bool _CheckHasUniqueKey(const KERNEL_NS::LibString &dbName, const KERNEL_NS::LibString &collectionName, LambdaType &&checkHasField) const
    {
        auto iterDb = _dbRefCollectionRefMongoIndexInfos.find(dbName);
        if(iterDb == _dbRefCollectionRefMongoIndexInfos.end())
            return false;

        auto &collectionRefIndexInfos = iterDb->second;
        auto iterCollection = collectionRefIndexInfos.find(collectionName);
        if(iterCollection == collectionRefIndexInfos.end())
            return false;

        auto &indexNameRefIndexInfo = iterCollection->second;
        for(auto iterIndex : indexNameRefIndexInfo)
        {
            auto &indexInfo = iterIndex.second;
            if(indexInfo.Fields.empty())
                continue;
                    
            if(!indexInfo.Unique)
                continue;

            Int32 hasKeyCount = 0;
            for(auto &field : indexInfo.Fields)
            {
                // 没有字段就跳出
                if(!checkHasField(indexInfo.IndexName, field.first))
                {
                    break;
                }

                ++hasKeyCount;
            }

            // 有没有唯一索引的所有key
            if(hasKeyCount == static_cast<Int32>(indexInfo.Fields.size()))
            {
                return true;
            }
        }

        return false;
    }

    void _DoAddIndex(const KERNEL_NS::LibString &dbName, const KERNEL_NS::LibString &collectionName, const KERNEL_NS::LibString &indexName, const std::vector<std::pair<KERNEL_NS::LibString, Int32>> &fields, bool unique = false);

    friend class MongodbThreadStartup;
    // 连接mongo
    KERNEL_NS::LibString _uri;

    // srv必要信息
    KERNEL_NS::LibString _account;
    KERNEL_NS::LibString _pwd;
    // mongodb + srv 连接域名
    KERNEL_NS::LibString _srvHostName;
    // db => 表 => 分片键
    std::unordered_map<KERNEL_NS::LibString, std::unordered_map<KERNEL_NS::LibString, ShardKeyInfoGroup>> _dbRefcollectionRefShardKeyInfos;
    // db => 表 => 索引信息(indexname => 索引信息)
    std::unordered_map<KERNEL_NS::LibString, std::unordered_map<KERNEL_NS::LibString, std::unordered_map<KERNEL_NS::LibString, MongoIndexInfo>>> _dbRefCollectionRefMongoIndexInfos;
    // 不可添加分片键, 索引信息
    std::atomic_bool _cantAddSchemaInfo;
    SpinLock _schemaLock;
    // 是否关注的数据库(不是关注的数据库不会提供服务)
    std::unordered_set<KERNEL_NS::LibString> _focusDbs;

    // 初始化mongodb
    static mongocxx::instance _instance;
    mongocxx::pool *_connectionPool;

    // 线程
    KERNEL_NS::LibEventLoopThread *_eventLoopThread;

    // dbready
    std::atomic_bool _isDbReady;
    std::atomic<Int32> _dbFailErrCode;

    // mongodb是不是started
    std::atomic_bool _willStarted;

    // 配置来源
    KERNEL_NS::SourceWrap _sourceWrap;
    KERNEL_NS::LibString _mongoConfigKeyName;
    KERNEL_NS::FileMonitor<MongodbConfig, KERNEL_NS::YamlDeserializer> *_configMonitor;
};

KERNEL_END

#endif
