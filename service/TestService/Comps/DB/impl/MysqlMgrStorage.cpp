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
 * Date: 2023-07-23 20:42:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/kernel.h>
#include <service/TestService/Comps/DB/impl/MysqlMgrStorage.h>
#include <service/TestService/Comps/DB/impl/MysqlMgrStorageFactory.h>
#include <service/TestService/Comps/DB/impl/MysqlMgr.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(MysqlMgrStorage);

const KERNEL_NS::LibString MysqlMgrStorage::TABLE_NAME = "TableName";
const KERNEL_NS::LibString MysqlMgrStorage::SIMPLE_INFO = "SimpleInfo";
const KERNEL_NS::LibString MysqlMgrStorage::COUNT = "Count";
    

MysqlMgrStorage::MysqlMgrStorage()
:IStorageInfo(KERNEL_NS::RttiUtil::GetTypeId<MysqlMgrStorage>(), KERNEL_NS::RttiUtil::GetByType<MysqlMgr>())
{

}

MysqlMgrStorage::~MysqlMgrStorage()
{

}

void MysqlMgrStorage::Release()
{
    MysqlMgrStorage::DeleteByAdapter_MysqlMgrStorage(MysqlMgrStorageFactory::_buildType.V, this);
}

bool MysqlMgrStorage::RegisterStorages()
{

    // 当前系统属性设置(kv 系统, mysql存储, 某个系统的数据)
    AddFlags(StorageFlagType::MULTI_FIELD_SYSTEM_FLAG | 
    StorageFlagType::MYSQL_FLAG | 
    StorageFlagType::SYSTEM_DATA_STORAGE_FLAG |
    StorageFlagType::LOAD_DATA_ON_STARTUP_FLAG
    );
    SetComment("mysql global system table manager");

    // TableName string field 主键
    auto newStorageInfo = IStorageInfo::NewThreadLocal_IStorageInfo(0, TABLE_NAME);
    newStorageInfo->SetRelease([newStorageInfo](){
        IStorageInfo::DeleteThreadLocal_IStorageInfo(newStorageInfo);
    });
    newStorageInfo->AddFlags(StorageFlagType::NORMAL_STRING_FIELD_FLAG | 
    StorageFlagType::MYSQL_FLAG | 
    StorageFlagType::PRIMARY_FIELD_FLAG);
    newStorageInfo->SetCapacitySize(StorageCapacityType::Cap64);
    newStorageInfo->SetComment("table name");
    newStorageInfo->SetFieldName(TABLE_NAME);
    if(!AddStorageInfo(newStorageInfo))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("AddStorageInfo fail."));
        return false;
    }

    // simple info
    newStorageInfo = IStorageInfo::NewThreadLocal_IStorageInfo(0, SIMPLE_INFO);
    newStorageInfo->SetRelease([newStorageInfo](){
        IStorageInfo::DeleteThreadLocal_IStorageInfo(newStorageInfo);
    });

    newStorageInfo->AddFlags(StorageFlagType::NORMAL_TEXT_STRING_FIELD_FLAG | 
    StorageFlagType::MYSQL_FLAG);
    newStorageInfo->SetComment("table simple info");
    newStorageInfo->SetFieldName(SIMPLE_INFO);
    if(!AddStorageInfo(newStorageInfo))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("AddStorageInfo fail."));
        return false;
    }

    // 数量
    // newStorageInfo = IStorageInfo::NewThreadLocal_IStorageInfo(COUNT);
    // newStorageInfo->SetRelease([newStorageInfo](){
    //     IStorageInfo::DeleteThreadLocal_IStorageInfo(newStorageInfo);
    // });

    // newStorageInfo->AddFlags(StorageFlagType::INT64_NUMBER_FIELD_FLAG | 
    // StorageFlagType::MYSQL_FLAG);
    // newStorageInfo->SetComment("count");
    // AddStorageInfo(newStorageInfo);

    return true;
}


SERVICE_END
