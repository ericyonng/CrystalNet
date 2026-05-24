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

    // 设置数据库名(需要在WillStart之前设置)
    virtual void SetDbName(const KERNEL_NS::LibString &dbName) = 0;
    virtual const KERNEL_NS::LibString &GetDbName() const = 0;
    // 给表设置分片键(需要在WillStart之前设置)
    virtual void SetShardKeyInfo(const KERNEL_NS::LibString &collectionName, const std::vector<ShardKeyInfo> &shardKeyInfos) = 0;
    // 设置索引(支持符合索引, 需要在WillStart之前设置)
    virtual void CreateIndex(const KERNEL_NS::LibString &collectionName, const KERNEL_NS::LibString &indexName, const std::vector<std::pair<KERNEL_NS::LibString, Int32>> &fields, bool unique = false) = 0;

#ifdef CRYSTAL_NET_CPP20
    // 查
    virtual KERNEL_NS::CoTask<bool> Query(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collection, KERNEL_NS::LibString keyName, UInt64 keyValue) = 0;
    virtual KERNEL_NS::CoTask<bool> Query(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collection, KERNEL_NS::LibString keyName, KERNEL_NS::LibString keyValue) = 0;
#endif

    // 增(mongodb 只能Int64 Value)
    virtual KERNEL_NS::CoTask<bool> AddData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collection, KERNEL_NS::LibString keyName, Int64 keyValue) = 0;
    virtual KERNEL_NS::CoTask<bool> AddData(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collection, KERNEL_NS::LibString keyName, KERNEL_NS::LibString keyValue) = 0;
    // 增, 删, 改, 查

};

KERNEL_END

#endif
