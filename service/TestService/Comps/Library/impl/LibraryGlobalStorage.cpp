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
 * Date: 2023-09-15 11:55:02
 * Author: Eric Yonng
 * Description: 
*/
#include <pch.h>
#include <kernel/kernel.h>
#include <Comps/Library/impl/LibraryGlobalStorage.h>
#include <Comps/Library/impl/LibraryGlobalStorageFactory.h>
#include <Comps/Library/impl/LibraryGlobal.h>

SERVICE_BEGIN


POOL_CREATE_OBJ_DEFAULT_IMPL(LibraryGlobalStorage);

LibraryGlobalStorage::LibraryGlobalStorage()
:IStorageInfo(KERNEL_NS::RttiUtil::GetTypeId<LibraryGlobalStorage>(), KERNEL_NS::RttiUtil::GetByType<LibraryGlobal>())
{

}

LibraryGlobalStorage::~LibraryGlobalStorage()
{

}

void LibraryGlobalStorage::Release()
{
    LibraryGlobalStorage::DeleteByAdapter_LibraryGlobalStorage(LibraryGlobalStorageFactory::_buildType.V, this);
}

bool LibraryGlobalStorage::RegisterStorages()
{
    AddFlags(StorageFlagType::KEY_VALUE_SYSTEM_FLAG | 
    StorageFlagType::MYSQL_FLAG | 
    StorageFlagType::SYSTEM_DATA_STORAGE_FLAG |
    StorageFlagType::LOAD_DATA_ON_STARTUP_FLAG |
    StorageFlagType::NEED_NUMBER_KEY_FLAG
    );

    // auto newStorageInfo = IStorageInfo::NewThreadLocal_IStorageInfo("Id");
    // newStorageInfo->SetRelease([newStorageInfo](){
    //     IStorageInfo::DeleteThreadLocal_IStorageInfo(newStorageInfo);
    // });
    // newStorageInfo->AddFlags(StorageFlagType::INT64_NUMBER_FIELD_FLAG | 
    // StorageFlagType::UNSIGNED_NUMBER_FIELD_FLAG |
    // StorageFlagType::MYSQL_FLAG | 
    // StorageFlagType::PRIMARY_FIELD_FLAG);
    // newStorageInfo->SetComment("id");
    // if(!AddStorageInfo(newStorageInfo))
    // {
    //     g_Log->Warn(LOGFMT_OBJ_TAG("AddStorageInfo fail."));
    //     return false;
    // }

    // newStorageInfo = IStorageInfo::NewThreadLocal_IStorageInfo("LibraryInfo");
    // newStorageInfo->SetRelease([newStorageInfo](){
    //     IStorageInfo::DeleteThreadLocal_IStorageInfo(newStorageInfo);
    // });

    // newStorageInfo->AddFlags(StorageFlagType::NORMAL_TEXT_STRING_FIELD_FLAG |
    // StorageFlagType::MYSQL_FLAG);
    // newStorageInfo->SetComment("LibraryInfo");
    // if(!AddStorageInfo(newStorageInfo))
    // {
    //     g_Log->Warn(LOGFMT_OBJ_TAG("AddStorageInfo fail."));
    //     return false;
    // }

    SetComment("library");

    return true;
}

SERVICE_END
