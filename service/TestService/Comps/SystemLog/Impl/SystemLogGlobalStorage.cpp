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
 * Date: 2023-11-19 00:03:49
 * Author: Eric Yonng
 * Description: 
*/


#include <pch.h>
#include <kernel/kernel.h>
#include <Comps/SystemLog/Impl/SystemLogGlobalStorage.h>
#include <Comps/SystemLog/Impl/SystemLogGlobalStorageFactory.h>
#include <Comps/SystemLog/Impl/SystemLogGlobal.h>

SERVICE_BEGIN

const KERNEL_NS::LibString SystemLogGlobalStorage::ID = "Id";
const KERNEL_NS::LibString SystemLogGlobalStorage::LIBRARY_ID_NAME = "LibraryId";
const KERNEL_NS::LibString SystemLogGlobalStorage::LOG_DATA_NAME = "LogData";

POOL_CREATE_OBJ_DEFAULT_IMPL(SystemLogGlobalStorage);

SystemLogGlobalStorage::SystemLogGlobalStorage()
:IStorageInfo(KERNEL_NS::RttiUtil::GetTypeId<SystemLogGlobalStorage>(), KERNEL_NS::RttiUtil::GetByType<SystemLogGlobal>())
{

}

SystemLogGlobalStorage::~SystemLogGlobalStorage()
{

}

void SystemLogGlobalStorage::Release()
{
    SystemLogGlobalStorage::DeleteByAdapter_SystemLogGlobalStorage(SystemLogGlobalStorageFactory::_buildType.V, this);
}

bool SystemLogGlobalStorage::RegisterStorages()
{
    AddFlags(StorageFlagType::MULTI_FIELD_SYSTEM_FLAG | 
    StorageFlagType::MYSQL_FLAG | 
    StorageFlagType::SYSTEM_DATA_STORAGE_FLAG|
    StorageFlagType::DISABLE_LOAD_DATA_ON_STARTUP_FLAG
    );

    auto newStorageInfo = IStorageInfo::NewThreadLocal_IStorageInfo(0, SystemLogGlobalStorage::ID);
    newStorageInfo->SetRelease([newStorageInfo](){
        IStorageInfo::DeleteThreadLocal_IStorageInfo(newStorageInfo);
    });
    newStorageInfo->AddFlags(StorageFlagType::INT64_NUMBER_FIELD_FLAG | 
    StorageFlagType::UNSIGNED_NUMBER_FIELD_FLAG |
    StorageFlagType::MYSQL_FLAG | 
    StorageFlagType::PRIMARY_FIELD_FLAG);
    if(!AddStorageInfo(newStorageInfo))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("AddStorageInfo fail."));
        return false;
    }

    newStorageInfo = IStorageInfo::NewThreadLocal_IStorageInfo(0, SystemLogGlobalStorage::LIBRARY_ID_NAME);
    newStorageInfo->SetRelease([newStorageInfo](){
        IStorageInfo::DeleteThreadLocal_IStorageInfo(newStorageInfo);
    });
    newStorageInfo->AddFlags(StorageFlagType::INT64_NUMBER_FIELD_FLAG | 
    StorageFlagType::UNSIGNED_NUMBER_FIELD_FLAG |
    StorageFlagType::MYSQL_FLAG | 
    StorageFlagType::AS_INDEX_KEY_FIELD_FLAG);
    if(!AddStorageInfo(newStorageInfo))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("AddStorageInfo fail."));
        return false;
    }

    newStorageInfo = IStorageInfo::NewThreadLocal_IStorageInfo(0, SystemLogGlobalStorage::LOG_DATA_NAME);
    newStorageInfo->SetRelease([newStorageInfo](){
        IStorageInfo::DeleteThreadLocal_IStorageInfo(newStorageInfo);
    });
    newStorageInfo->AddFlags(StorageFlagType::VARBINARY_FIELD_FLAG | 
            StorageFlagType::MYSQL_FLAG);
    newStorageInfo->SetCapacitySize(StorageCapacityType::Cap1K);
    
    if(!AddStorageInfo(newStorageInfo))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("AddStorageInfo fail."));
        return false;
    }

    SetComment("");

    return true;
}

SERVICE_END
