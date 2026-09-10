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

#include "UserMgrMongoStorageFactory.h"
#include <protocols/cplusplus/com_user.pb.h>

SERVICE_BEGIN
    UserMgrMongoStorage::UserMgrMongoStorage()
 :IMongodbStorageInfo(KERNEL_NS::RttiUtil::GetTypeId<UserMgrMongoStorage>()
     , KERNEL_NS::RttiUtil::GetByType<User>(), KERNEL_NS::RttiUtil::GetTypeId<UserMgr>())
{
    // 唯一索引信息(hashed不能作为唯一索引)
    _uniqueIndexFields.emplace_back(GetKeyName(), KERNEL_NS::MongodbIndexFieldValue::ASC);

    // 必要的字段存储(json 文档对象)
    _fieldNameRefStorageType[GetKeyName()] = KERNEL_NS::MongoSerializeInfoType::INT64;
    _fieldNameRefStorageType[GetAccountName()] = KERNEL_NS::MongoSerializeInfoType::STRING;
    _fieldNameRefStorageType[GetNickNameName()] = KERNEL_NS::MongoSerializeInfoType::STRING;
    _fieldNameRefStorageType[GetLastLoginTimeName()] = KERNEL_NS::MongoSerializeInfoType::INT64;
    _fieldNameRefStorageType[GetLastLoginIpName()] = KERNEL_NS::MongoSerializeInfoType::STRING;
    _fieldNameRefStorageType[GetCreateIpName()] = KERNEL_NS::MongoSerializeInfoType::STRING;
    _fieldNameRefStorageType[GetCreateTimeName()] = KERNEL_NS::MongoSerializeInfoType::INT64;
    _fieldNameRefStorageType[GetLastPassDayTimeName()] = KERNEL_NS::MongoSerializeInfoType::INT64;

    // 创建索引
    auto &indexInfo = _indexNameRefInfo.emplace("account_name_index", KERNEL_NS::MongoIndexInfo()).first->second;
    indexInfo.Fields.emplace_back(GetAccountName(), KERNEL_NS::MongodbIndexFieldValue::ASC);
    indexInfo.Unique = true;    // account 保持唯一

    AsMultiSystem();
}

UserMgrMongoStorage::~UserMgrMongoStorage()
{
 
}

void UserMgrMongoStorage::Release()
{
    UserMgrMongoStorage::DeleteByAdapter_UserMgrMongoStorage(UserMgrMongoStorageFactory::_buildType.V, this);
}

Int32 UserMgrMongoStorage::_OnHostInit()
{
    _dbName = GetOwner()->CastTo<UserMgr>()->GetService()->CastTo<MyTestService>()->GetStorageOption()->DbName;
    SetOnSaveCb<UserMgr>(&UserMgr::OnSave);
    return Status::Success;
}

void UserMgrMongoStorage::OnRegisterComps()
{
    // 子系统存储注册
    #include <Comps/UserSys/UserStorageInc.h>
}

KERNEL_NS::LibString UserMgrMongoStorage::GetKeyName()
{
    return UserBaseInfo::descriptor()->FindFieldByNumber(UserBaseInfo::kUserIdFieldNumber)->name();
}

KERNEL_NS::LibString UserMgrMongoStorage::GetAccountName()
{
    return UserBaseInfo::descriptor()->FindFieldByNumber(UserBaseInfo::kAccountNameFieldNumber)->name();
}


KERNEL_NS::LibString UserMgrMongoStorage::GetNickNameName()
{
    return UserBaseInfo::descriptor()->FindFieldByNumber(UserBaseInfo::kNicknameFieldNumber)->name();
}

KERNEL_NS::LibString UserMgrMongoStorage::GetLastLoginTimeName()
{
    return UserBaseInfo::descriptor()->FindFieldByNumber(UserBaseInfo::kLastLoginTimeFieldNumber)->name();
}

KERNEL_NS::LibString UserMgrMongoStorage::GetLastLoginIpName()
{
    return UserBaseInfo::descriptor()->FindFieldByNumber(UserBaseInfo::kLastLoginIpFieldNumber)->name();
}

KERNEL_NS::LibString UserMgrMongoStorage::GetCreateIpName()
{
    return UserBaseInfo::descriptor()->FindFieldByNumber(UserBaseInfo::kCreateIpFieldNumber)->name();
}

KERNEL_NS::LibString UserMgrMongoStorage::GetCreateTimeName()
{
    return UserBaseInfo::descriptor()->FindFieldByNumber(UserBaseInfo::kCreateTimeFieldNumber)->name();
}

KERNEL_NS::LibString UserMgrMongoStorage::GetLastPassDayTimeName()
{
    return UserBaseInfo::descriptor()->FindFieldByNumber(UserBaseInfo::kLastPassDayTimeFieldNumber)->name();
}

SERVICE_END