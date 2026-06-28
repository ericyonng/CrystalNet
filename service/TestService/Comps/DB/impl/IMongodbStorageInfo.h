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
// Date: 2026-06-28 18:06:13
// Author: Eric Yonng
// Description:

#pragma once

#include <service/common/macro.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/CompObject/CompObject.h>

SERVICE_BEGIN

class MongodbIndexFieldValue
{
public:
    enum ENUMS
    {
        // 升序
        ASC = 1,
        // 降序
        DESC = -1,
        // hash索引
        HASHED = -2,
    };
};

// 有需要持久化的, 需要注册信息
class IMongodbStorageInfo : public KERNEL_NS::CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, IMongodbStorageInfo);

public:
    IMongodbStorageInfo(UInt64 objTypeId);
    ~IMongodbStorageInfo() override;
    
    // db名 必须指定否则启动失败
    KERNEL_NS::LibString _dbName;
    // 系统名(表名)如果为空就使用所在宿主的类名
    KERNEL_NS::LibString _collectionName;
    // 索引信息:key,Int32:1:升序, -1降序,-2:hashed MongodbIndexFieldValue 必须指定否则启动失败 唯一索引
    std::vector<std::pair<KERNEL_NS::LibString, Int32>> _uniqueIndexFields;
    // 系统含有多字段, 给每个字段做存储类型定义 必须指定 _fieldNameRefStorageType 和 _storageType之一
    std::map<KERNEL_NS::LibString, Int32> _fieldNameRefStorageType;
    // 系统整体作为一个字段, 存储类型定义(子系统默认只作为母系统的一个字段整体存储)
    Int32 _storageType = 0;
};

#undef CREATE_MONGO_STORAGE_COMP
// 创建mongodb存储信息组件
#define CREATE_MONGO_STORAGE_COMP(VAR, T)                                 \
auto VAR = T::NewByAdapter_##T(_buildType.V);                       \
VAR->SetInterfaceTypeId(KERNEL_NS::RttiUtil::GetTypeId<SERVICE_NS::IMongodbStorageInfo>())

SERVICE_END
