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

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_INTERFACE_IMONGODB_MGR_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_INTERFACE_IMONGODB_MGR_H__

#pragma once

#include <kernel/comp/CompObject/CompObjectInc.h>
#include <OptionComp/storage/MongoDB/Impl/ShardKeyInfo.h>

KERNEL_BEGIN

struct SourceWrap;

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
    // 设置配置来源
    virtual void SetConfigSource(const KERNEL_NS::SourceWrap &source) = 0;
    virtual void SetConfigKeyName(const KERNEL_NS::LibString &keyName) = 0;

    // 给表设置分片键(需要在WillStart之前设置)
    virtual bool SetShardKeyInfo(const KERNEL_NS::LibString &dbName, const KERNEL_NS::LibString &collectionName, const std::vector<ShardKeyInfo> &shardKeyInfos, bool isUnique = false) = 0;
    // 设置索引(支持符合索引, 需要在WillStart之前设置) fields:字段名, 1:升序, -1降序,-2:hashed
    virtual bool CreateIndex(const KERNEL_NS::LibString &dbName, const KERNEL_NS::LibString &collectionName, const KERNEL_NS::LibString &indexName, const std::vector<std::pair<KERNEL_NS::LibString, Int32>> &fields, bool unique = false) = 0;

#ifdef CRYSTAL_NET_CPP20
    // 查
    virtual KERNEL_NS::CoTask<bool> Query(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collection, KERNEL_NS::LibString keyName, UInt64 keyValue) = 0;
    virtual KERNEL_NS::CoTask<bool> Query(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collection, KERNEL_NS::LibString keyName, KERNEL_NS::LibString keyValue) = 0;

    // jsonString 内部会释放(会自动检查唯一索引)
    virtual KERNEL_NS::CoTask<bool> AddData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, KERNEL_NS::LibString *jsonString) = 0;
    // uniqueKv:唯一索引
    virtual KERNEL_NS::CoTask<bool> AddData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::Variant, KERNEL_NS::Variant> uniqueKv, KERNEL_NS::LibString binaryKeyName, KERNEL_NS::LibStreamTL *binaryData) = 0;
    virtual KERNEL_NS::CoTask<bool> DelData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::Variant, KERNEL_NS::Variant> uniqueKv) = 0;

    virtual KERNEL_NS::CoTask<bool> ReplaceData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::vector<KERNEL_NS::LibString> keyNames, KERNEL_NS::LibString *jsonString) = 0;
    // replaceFields内部释放
    virtual KERNEL_NS::CoTask<bool> ReplaceData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::vector<KERNEL_NS::LibString> keyNames, std::map<KERNEL_NS::Variant, KERNEL_NS::Variant> *replaceFields) = 0;
    // uniqueKv:唯一索引
    // {
    //     uniqueKv ...
    //     BinaryKeyName:BinarayData
    // }
    virtual KERNEL_NS::CoTask<bool> ReplaceData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::map<KERNEL_NS::Variant, KERNEL_NS::Variant> uniqueKv, KERNEL_NS::LibString binaryKeyName, KERNEL_NS::LibStreamTL *binaryData) = 0;
#endif

    // // jsonstring:要改的kv系列
    // virtual KERNEL_NS::CoTask<bool> UpdateData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::vector<std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant>> uniqueKv, KERNEL_NS::LibString *jsonString) = 0;
    // virtual KERNEL_NS::CoTask<bool> UpdateData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::vector<std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant>> uniqueKv, std::vector<std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant>> updateFields) = 0;
    // // uniqueKv:唯一索引
    // virtual KERNEL_NS::CoTask<bool> UpdateData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, std::vector<std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant>> uniqueKv, KERNEL_NS::LibString binaryKeyName, KERNEL_NS::LibStreamTL *binaryData) = 0;
    //
    // 增, 删, 改, 查

    // 被关注的db, 如果不在关注列表, 不提供服务, 在willStart之前需要关注数据库
    virtual bool FocusDb(const KERNEL_NS::LibString &dbName) = 0;
};

KERNEL_END

#endif
