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
 * Date: 2023-09-17 16:38:11
 * Author: Eric Yonng
 * Description: 
*/
#include <pch.h>
#include <kernel/kernel.h>
#include <Comps/InviteCode/impl/InviteCodeGlobalStorage.h>
#include <Comps/InviteCode/impl/InviteCodeGlobalStorageFactory.h>
#include <Comps/InviteCode/impl/InviteCodeGlobal.h>

SERVICE_BEGIN


POOL_CREATE_OBJ_DEFAULT_IMPL(InviteCodeGlobalStorage);

const KERNEL_NS::LibString InviteCodeGlobalStorage::ID = "Id";
const KERNEL_NS::LibString InviteCodeGlobalStorage::INVITE_CODE = "InviteCode";


InviteCodeGlobalStorage::InviteCodeGlobalStorage()
:IStorageInfo(KERNEL_NS::RttiUtil::GetTypeId<InviteCodeGlobalStorage>(), KERNEL_NS::RttiUtil::GetByType<InviteCodeGlobal>())
{

}

InviteCodeGlobalStorage::~InviteCodeGlobalStorage()
{

}

void InviteCodeGlobalStorage::Release()
{
    InviteCodeGlobalStorage::DeleteByAdapter_InviteCodeGlobalStorage(InviteCodeGlobalStorageFactory::_buildType.V, this);
}

bool InviteCodeGlobalStorage::RegisterStorages()
{
    AddFlags(StorageFlagType::KEY_VALUE_SYSTEM_FLAG | 
    StorageFlagType::MYSQL_FLAG | 
    StorageFlagType::SYSTEM_DATA_STORAGE_FLAG |
    StorageFlagType::LOAD_DATA_ON_STARTUP_FLAG
    );

    auto newStorageInfo = IStorageInfo::NewThreadLocal_IStorageInfo(0, InviteCodeGlobalStorage::ID);
    newStorageInfo->SetRelease([newStorageInfo](){
        IStorageInfo::DeleteThreadLocal_IStorageInfo(newStorageInfo);
    });
    newStorageInfo->AddFlags(StorageFlagType::INT64_NUMBER_FIELD_FLAG | 
    StorageFlagType::UNSIGNED_NUMBER_FIELD_FLAG |
    StorageFlagType::MYSQL_FLAG | 
    StorageFlagType::PRIMARY_FIELD_FLAG);
    newStorageInfo->SetComment("id");
    if(!AddStorageInfo(newStorageInfo))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("AddStorageInfo fail."));
        return false;
    }

    newStorageInfo = IStorageInfo::NewThreadLocal_IStorageInfo(0, InviteCodeGlobalStorage::INVITE_CODE);
    newStorageInfo->SetRelease([newStorageInfo](){
        IStorageInfo::DeleteThreadLocal_IStorageInfo(newStorageInfo);
    });

    newStorageInfo->AddFlags(StorageFlagType::NORMAL_STRING_FIELD_FLAG | StorageFlagType::AS_UNIQUE_KEY_FIELD_FLAG |
    StorageFlagType::MYSQL_FLAG);
    newStorageInfo->SetComment("INVITE_CODE");
    newStorageInfo->SetCapacitySize(StorageCapacityType::Cap128);
    if(!AddStorageInfo(newStorageInfo))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("AddStorageInfo fail."));
        return false;
    }

    SetComment("");

    return true;
}

SERVICE_END