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
 * Date: 2026-07-03 11:45:36
 * Author: Eric Yonng
 * Description: 注册user子系统存储
*/

#include <pch.h>
#include <Comps/User/impl/UserMgrMongoStorage.h>
#include <Comps/User/impl/UserMgr.h>
#include <Comps/User/impl/User.h>
#include <kernel/comp/Utils/RttiUtil.h>

#include "MyTestService.h"
#include "OptionComp/storage/MongoDB/Impl/MongoSerializeInfoType.h"
#include <Comps/UserSys/UserSys.h>

SERVICE_BEGIN

UserMgrMongoStorage::UserMgrMongoStorage()
 :IMongodbStorageInfo(KERNEL_NS::RttiUtil::GetTypeId<UserMgrMongoStorage>(), KERNEL_NS::RttiUtil::GetByType<User>())
{
  // 唯一索引信息(hashed不能作为唯一索引)
  _uniqueIndexFields.emplace_back(KeyName, MongodbIndexFieldValue::ASC);

  // 必要的字段存储(json 文档对象)
  _fieldNameRefStorageType[KeyName] = KERNEL_NS::MongoSerializeInfoType::INT64;
  _fieldNameRefStorageType[SexName] = KERNEL_NS::MongoSerializeInfoType::INT64;
    _fieldNameRefStorageType[LevelName] = KERNEL_NS::MongoSerializeInfoType::INT64;
    _fieldNameRefStorageType[NickNameName] = KERNEL_NS::MongoSerializeInfoType::STRING;
    _fieldNameRefStorageType[AccountNameName] = KERNEL_NS::MongoSerializeInfoType::STRING;

    // 创建索引
    auto &indexInfo = _indexNameRefInfo.emplace("account_name_index", KERNEL_NS::MongoIndexInfo()).first->second;
    indexInfo.Fields.emplace_back(AccountNameName, MongodbIndexFieldValue::ASC);
    indexInfo.Unique = true;    // account 保持唯一

  AsMultiSystem();
}

UserMgrMongoStorage::~UserMgrMongoStorage()
{
 
}

void UserMgrMongoStorage::Release()
{
 
}

Int32 UserMgrMongoStorage::_OnHostInit()
{
  _dbName = GetOwner()->CastTo<UserMgr>()->GetService()->CastTo<MyTestService>()->GetStorageOption()->DbName;
  return Status::Success;
}

void UserMgrMongoStorage::OnRegisterComps()
{
    // 子系统存储注册
    #include <Comps/UserSys/UserStorageInc.h>
}


SERVICE_END