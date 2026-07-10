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
 * 1. 默认开启read/write重试, 通过srv连接会选择新的mongos, 只重试一次, 如果仍然失败那么接口返回失败, 业务自行处理失败情况
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_INTERFACE_IMONGODB_MGR_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_INTERFACE_IMONGODB_MGR_H__

#pragma once

#include <kernel/comp/CompObject/CompObjectInc.h>
#include <OptionComp/storage/MongoDB/Impl/ShardKeyInfo.h>
#include <kernel/comp/TimeSlice.h>
#include <OptionComp/storage/MongoDB/Impl/MongoSerializeInfo.h>
#include <OptionComp/storage/MongoDB/Impl/MongodbConfig.h>
#include <kernel/comp/FileMonitor/FileMonitor.h>
#include <kernel/comp/ConcurrentPriorityQueue/MPMCQueue.h>


KERNEL_BEGIN

struct SourceWrap;

struct QueryRoundContinue
{
    KERNEL_NS::CoLocker *_roundWaiter = NULL;
    std::atomic_bool *_isContinue = NULL;
    std::atomic_bool *_isCompleted = NULL;
    // 100条后暂停等待
    Int32 _pauseCount = 100;
    // 当前查询总条数
    std::atomic<Int64> *_curTotal = NULL;
};

class IMongoDbMgr : public CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, IMongoDbMgr);

public:
    IMongoDbMgr(UInt64 typeId): CompHostObject(typeId) {}

    // 可以外部传入uri支持自行定制连接
    virtual  void SetUri(const KERNEL_NS::LibString &uri) = 0;

    // 默认如果不传入uri就是srv格式连接mongos(自动发现, 动态解析域名，支持断开重连新ip)
    // 账号密码
    virtual void SetAccountPwd(const KERNEL_NS::LibString &account, const KERNEL_NS::LibString &pwd) = 0;
    // 设置srv连接的域名
    virtual void SetSrvHostName(const KERNEL_NS::LibString &hostName) = 0;
    // 手动设置复制集名, 有配置项优先使用配置
    virtual void SetReplicaSetName(const KERNEL_NS::LibString &rs) = 0;

    // 设置配置来源
    virtual void SetConfigSource(const KERNEL_NS::SourceWrap &source) = 0;
    virtual void SetConfigKeyName(const KERNEL_NS::LibString &keyName) = 0;
    virtual const KERNEL_NS::SourceWrap &GetConfigSource() const = 0;
    virtual const KERNEL_NS::LibString &GetConfigSourceKeyName() const = 0;

    // 给表设置分片键(需要在WillStart之前设置)
    virtual bool SetShardKeyInfo(const KERNEL_NS::LibString &dbName, const KERNEL_NS::LibString &collectionName, const std::vector<ShardKeyInfo> &shardKeyInfos, bool isUnique = false) = 0;
    // 设置索引(支持符合索引, 需要在WillStart之前设置) fields:字段名, 1:升序, -1降序,-2:hashed hashed不能做唯一索引否则会失败
    virtual bool CreateIndex(const KERNEL_NS::LibString &dbName, const KERNEL_NS::LibString &collectionName, const KERNEL_NS::LibString &indexName, const std::vector<std::pair<KERNEL_NS::LibString, Int32>> &fields, bool unique = false) = 0;

    // 分布式锁(表级)
    // 分布式锁(行级)

#ifdef CRYSTAL_NET_CPP20
    // 尝试获取分布式锁 (使用 findOneAndUpdate + upsert 原子操作)
    // 逻辑: 如果锁不存在则创建; 如果锁存在但已过期则更新; 如果锁存在且未过期则失败 lockName:目标(可以组合不同的字符串, 来实现不同功能的分布式锁), ownerId:锁id
    virtual CoTask<bool> TryAcquireLock(KERNEL_NS::LibString lockTargetId, KERNEL_NS::LibString lockId, KERNEL_NS::TimeSlice slice = KERNEL_NS::TimeSlice::FromSeconds(30)) = 0;
    
    // 释放分布式锁
    virtual CoTask<> ReleaseLock(const KERNEL_NS::LibString &lockTargetId, const KERNEL_NS::LibString &lockId) = 0;
    
    // 查json数据 查一条数据 fieldNameRefVariant 外部释放,query内部不释放
    // 特殊转换: 
    // 如果存储的是简单的数据例如整数浮点数字符串等, variant返回的就是对应的简单数据
    // 如果存储的是二进制数据, 那么variant也是二进制数据通过LibStreamTl返回(判断Variant IsSteamTL/或者IsBinary)
    // 如果存的是数组, 那么Variant就是个Variant数组, 查到数据后业务层自行解析该数组
    // 如果存的是字典, 那么Variant就是Variant字典, 解析时候按照实际数据解析
    // 具体的转换规则可以看:_TurnVariant
    // Variant 释放的时, 如果是二进制数据需要自己手动释放LibStreamTL,避免内存泄露
    // 若fieldNameRefVariant有指定字段名的, 只会查询数据的指定的几个字段,如果是空的, 则查完整数据
    // ignoreOid:是否忽略返回MongoDb _id字段
    virtual KERNEL_NS::CoTask<bool> Query(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> *fieldNameRefVariant, bool ignoreOid = false) = 0;

    // jsonString 内部会释放(会自动检查唯一索引)
    virtual KERNEL_NS::CoTask<bool> AddData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, KERNEL_NS::LibString *jsonString) = 0;
    // uniqueKv:唯一索引 添加一条数据
    virtual KERNEL_NS::CoTask<bool> AddData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> uniqueKv,  std::map<KERNEL_NS::LibString, KERNEL_NS::LibStreamTL *> *binaryKeyNameRefData) = 0;
    // 添加一条数据, Variant包含二进制数据等
    virtual KERNEL_NS::CoTask<bool> AddData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> uniqueKv, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> *keyRefVariant) = 0;

    // 删一条数据
    virtual KERNEL_NS::CoTask<bool> DelData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> uniqueKv) = 0;

    // 覆盖一条数据
    virtual KERNEL_NS::CoTask<bool> ReplaceData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::vector<KERNEL_NS::LibString> keyNames, KERNEL_NS::LibString *jsonString) = 0;
    // replaceFields内部释放 覆盖一条数据
    virtual KERNEL_NS::CoTask<bool> ReplaceData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::vector<KERNEL_NS::LibString> keyNames, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> *replaceFields) = 0;
    // uniqueKv:唯一索引
    // {
    //     uniqueKv ...
    //     BinaryKeyName:BinarayData
    // }
    // 覆盖一条数据
    virtual KERNEL_NS::CoTask<bool> ReplaceData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> uniqueKv, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStreamTL *> *binaryKeyNameRefData) = 0;

    // 增量更新, kv 可以是非唯一索引, updateFields:序列化的json字符串, createIfNotExists:为true, 那么kv和updateFields会自动合并生成新的数据 更新一条数据
    virtual KERNEL_NS::CoTask<bool> UpdateData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv, KERNEL_NS::LibString *jsonFields, bool createIfNotExists = false) = 0;
    virtual KERNEL_NS::CoTask<bool> UpdateData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> *updateFields, bool createIfNotExists = false) = 0;
    virtual KERNEL_NS::CoTask<bool> UpdateData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStreamTL *> *binaryKeyNameRefData, bool createIfNotExists = false) = 0;
    // condition : 传入 bsoncxx::builder::basic::document 条件, UpdateDataIf 用于原子操作,返回结果:jsonResult(json),外部释放, 返回旧的文档 Condition带上唯一id, updateFields就不用带唯一id
    virtual KERNEL_NS::CoTask<bool> UpdateDataIf(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> *updateFields, void *condition) = 0;

    // 序列化发序列化:MongoDataSerialize, 序列化反序列化数据定义:MongoSerializeInfo keyNameRefData内部释放
    virtual KERNEL_NS::CoTask<bool> UpdateData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv, std::map<KERNEL_NS::LibString, MongoSerializeInfo> *keyNameRefData, bool createIfNotExists = false) = 0;
    // 序列化发序列化:MongoDataSerialize, 序列化反序列化数据定义:MongoSerializeInfo keyNameRefData内部释放
    virtual KERNEL_NS::CoTask<bool> ReplaceData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> uniqueKv, std::map<KERNEL_NS::LibString, MongoSerializeInfo> *keyNameRefData) = 0;
    // 序列化发序列化:MongoDataSerialize, 序列化反序列化数据定义:MongoSerializeInfo keyNameRefData内部释放
    virtual KERNEL_NS::CoTask<bool> AddData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> uniqueKv,  std::map<KERNEL_NS::LibString, MongoSerializeInfo> *keyNameRefData) = 0;
    // 序列化发序列化:MongoDataSerialize, 序列化反序列化数据定义:MongoSerializeInfo keyNameRefData 外部释放 若fieldNameRefVariant有指定字段名的, 只会查询数据的指定的几个字段,如果是空的, 则查完整数据
    virtual KERNEL_NS::CoTask<bool> Query(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv, std::map<KERNEL_NS::LibString, MongoSerializeInfo> *fieldNameRefData, bool ignoreOid = false) = 0;

    // roundWaiter: 查询一轮游标后等待请求者下一步是否继续
    virtual KERNEL_NS::CoTask<bool> Query(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv, MPMCQueue<std::map<KERNEL_NS::LibString, MongoSerializeInfo> *, 1024> *dataQueue, QueryRoundContinue roundContinueInfo = {}, Int32 batchSize = 512, bool ignoreOid = false) = 0;

#endif

    // // jsonstring:要改的kv系列
    // virtual KERNEL_NS::CoTask<bool> UpdateData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::vector<std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant>> uniqueKv, KERNEL_NS::LibString *jsonString) = 0;
    // virtual KERNEL_NS::CoTask<bool> UpdateData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::vector<std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant>> uniqueKv, std::vector<std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant>> updateFields) = 0;
    // // uniqueKv:唯一索引
    // virtual KERNEL_NS::CoTask<bool> UpdateData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::vector<std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant>> uniqueKv, KERNEL_NS::LibString binaryKeyName, KERNEL_NS::LibStreamTL *binaryData) = 0;
    //
    // 增, 删, 改, 查
    // find_one_and... 系列函数用于原子操作场景(比如比对版本号是否一致, 一致才能更新, 后续有需要的再迭代)
    
    // 多少个请求未完成
    virtual Int64 GetPendingRequestCount() const = 0;

    // 获取配置
    virtual const KERNEL_NS::FileMonitor<MongodbConfig, KERNEL_NS::YamlDeserializer> *GetConfig() const = 0;
};


KERNEL_END

#endif
