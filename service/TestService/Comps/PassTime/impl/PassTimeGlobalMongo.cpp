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
// Date: 2026-06-28 23:06:28
// Author: Eric Yonng
// Description:


#include <pch.h>
#include <Comps/PassTime/impl/PassTimeGlobalMongo.h>
#include <Comps/PassTime/impl/PassTimeGlobalMongoFactory.h>
#include <kernel/comp/Utils/RttiUtil.h>
#include <OptionComp/storage/MongoDB/Impl/MongoSerializeInfoType.h>
#include <protocols/cplusplus/com_passtime.pb.h>
#include <kernel/common/status.h>
#include <Comps/PassTime/impl/PassTimeGlobal.h>

#include "MyTestService.h"
#include <service/common/Configs/Configs.h>

SERVICE_BEGIN

PassTimeGlobalMongo::PassTimeGlobalMongo()
    :KERNEL_NS::IMongodbStorageInfo(KERNEL_NS::RttiUtil::GetTypeId<PassTimeGlobalMongo>()
        , KERNEL_NS::RttiUtil::GetByType<PassTimeGlobal>()
        , KERNEL_NS::RttiUtil::GetTypeId<PassTimeGlobal>())
{
    // 唯一索引信息(hashed不能作为唯一索引)
    _uniqueIndexFields.emplace_back(KeyName, KERNEL_NS::MongodbIndexFieldValue::ASC);

    // 存储类型(json 文档对象)
    _fieldNameRefStorageType[KeyName] = KERNEL_NS::MongoSerializeInfoType::INT64;
    _fieldNameRefStorageType[ValueName] = KERNEL_NS::MongoSerializeInfoType::JSON;

    // 设置kv系统
    AsKvSystem(KeyName, ValueName);
}

PassTimeGlobalMongo::~PassTimeGlobalMongo()
{
    
}

void PassTimeGlobalMongo::Release()
{
    PassTimeGlobalMongo::DeleteByAdapter_PassTimeGlobalMongo(PassTimeGlobalMongoFactory::_buildType.V, this);
}

Int32 PassTimeGlobalMongo::_OnHostInit()
{
    _dbName = GetOwner()->CastTo<PassTimeGlobal>()->GetService()->CastTo<MyTestService>()->GetStorageOption()->DbName;

    // 设置持久化回调
    SetOnSaveCb<PassTimeGlobal>(&PassTimeGlobal::OnSave);
    return Status::Success;
}


SERVICE_END