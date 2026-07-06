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
// Date: 2026-07-07 01:07:49
// Author: Eric Yonng
// Description:

#include <pch.h>
#include <OptionComp/GlobalId/Impl/GlobalIdMgrMongo.h>
#include <OptionComp/GlobalId/Impl/GlobalIdMgrMongoFactory.h>
#include <OptionComp/GlobalId/Impl/GlobalIdMgr.h>

#include "OptionComp/storage/MongoDB/Impl/MongoSerializeInfoType.h"

KERNEL_BEGIN

GlobalIdMgrMongo::GlobalIdMgrMongo()
    :KERNEL_NS::IMongodbStorageInfo(KERNEL_NS::RttiUtil::GetTypeId<GlobalIdMgrMongo>()
        , KERNEL_NS::RttiUtil::GetByType<GlobalIdMgr>()
        , KERNEL_NS::RttiUtil::GetTypeId<GlobalIdMgr>())
{
    // 唯一索引信息(hashed不能作为唯一索引)
    _uniqueIndexFields.emplace_back(KeyName, KERNEL_NS::MongodbIndexFieldValue::ASC);

    // 存储类型(json 文档对象)
    _fieldNameRefStorageType[KeyName] = KERNEL_NS::MongoSerializeInfoType::INT64;
    _fieldNameRefStorageType[TimePartName] = KERNEL_NS::MongoSerializeInfoType::INT64;
    _fieldNameRefStorageType[HeartbeatTimeName] = KERNEL_NS::MongoSerializeInfoType::INT64;
    _fieldNameRefStorageType[CurOwnerName] = KERNEL_NS::MongoSerializeInfoType::STRING;

    // 设置kv系统
    AsMultiSystem();

    // 不需要Save接口
    _noSaveCb = true;

    _dbName = "GlobalIdDb";
}

GlobalIdMgrMongo::~GlobalIdMgrMongo()
{
    
}

void GlobalIdMgrMongo::Release()
{
    GlobalIdMgrMongo::DeleteByAdapter_GlobalIdMgrMongo(GlobalIdMgrMongoFactory::_buildType.V, this);
}

Int32 GlobalIdMgrMongo::_OnHostInit()
{
    return Status::Success;
}


KERNEL_END