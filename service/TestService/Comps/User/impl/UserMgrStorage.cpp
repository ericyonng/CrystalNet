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
 * Date: 2023-08-02 13:23:59
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <Comps/User/impl/UserMgrStorage.h>
#include <Comps/User/impl/UserMgrStorageFactory.h>
#include <Comps/User/impl/UserMgr.h>
#include <protocols/protocols.h>
#include <Comps/UserSys/UserSys.h>


SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(UserMgrStorage);

UserMgrStorage::UserMgrStorage()
:IStorageInfo(KERNEL_NS::RttiUtil::GetByType<UserMgr>())
{

}

UserMgrStorage::~UserMgrStorage()
{

}

void UserMgrStorage::Release()
{
    UserMgrStorage::DeleteByAdapter_UserMgrStorage(UserMgrStorageFactory::_buildType.V, this);
}

// 所有user系统存储都需要在这里注册Storage, 因为启动要自动建表
bool UserMgrStorage::RegisterStorages()
{
    // 当前系统属性设置(kv 系统, mysql存储, 某个系统的数据)
    AddFlags(StorageFlagType::MULTI_FIELD_SYSTEM_FLAG | 
    StorageFlagType::MYSQL_FLAG
    );
    SetTableName("tbl_user");
    SetComment("user data");
    // 热加载100个
    SetDataCountLimit(1000);

    // SetDataCountLimit(10);
    auto descriptor = UserBaseInfo::GetDescriptor();
    auto userIdfield = descriptor->FindFieldByNumber(UserBaseInfo::kUserIdFieldNumber);
    const KERNEL_NS::LibString userIdName = userIdfield->name();
    auto accountNamefield = descriptor->FindFieldByNumber(UserBaseInfo::kAccountNameFieldNumber);
    const KERNEL_NS::LibString accountNameName = accountNamefield->name();
    if(!StorageHelper::AddMysqlStorageInfoWithPb(this, descriptor, &userIdName, {accountNameName}))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("AddMysqlStorageInfoWithPb fail"));
        return false;
    }

    // login mgr
    if(!RegisterStorage<LoginMgrStorageFactory>())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("register LoginMgrStorageFactory fail"));
        return false;
    }

    // librarymgr
    if(!RegisterStorage<LibraryMgrStorageFactory>())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("register LibraryMgrStorageFactory fail"));
        return false;
    }

    return true;
}
SERVICE_END
