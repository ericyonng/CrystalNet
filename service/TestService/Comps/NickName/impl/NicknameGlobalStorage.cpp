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
 * Date: 2023-08-05 21:45:02
 * Author: Eric Yonng
 * Description: 
*/
#include <pch.h>
#include <Comps/NickName/impl/NicknameGlobalStorage.h>
#include <Comps/NickName/impl/NicknameGlobalStorageFactory.h>
#include <Comps/NickName/impl/NicknameGlobal.h>

SERVICE_BEGIN


POOL_CREATE_OBJ_DEFAULT_IMPL(NicknameGlobalStorage);

NicknameGlobalStorage::NicknameGlobalStorage()
:IStorageInfo(KERNEL_NS::RttiUtil::GetByType<NicknameGlobal>())
{

}

NicknameGlobalStorage::~NicknameGlobalStorage()
{

}

void NicknameGlobalStorage::Release()
{
    NicknameGlobalStorage::DeleteByAdapter_NicknameGlobalStorage(NicknameGlobalStorageFactory::_buildType.V, this);
}

bool NicknameGlobalStorage::RegisterStorages()
{
    AddFlags(StorageFlagType::KEY_VALUE_SYSTEM_FLAG | 
    StorageFlagType::MYSQL_FLAG | 
    StorageFlagType::SYSTEM_DATA_STORAGE_FLAG |
    StorageFlagType::LOAD_DATA_ON_STARTUP_FLAG
    );

    auto newStorageInfo = IStorageInfo::NewThreadLocal_IStorageInfo(NicknameGlobalStorage::ID);
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

    newStorageInfo = IStorageInfo::NewThreadLocal_IStorageInfo(NicknameGlobalStorage::NICK_NAME);
    newStorageInfo->SetRelease([newStorageInfo](){
        IStorageInfo::DeleteThreadLocal_IStorageInfo(newStorageInfo);
    });

    newStorageInfo->AddFlags(StorageFlagType::NORMAL_STRING_FIELD_FLAG |
    StorageFlagType::MYSQL_FLAG);
    newStorageInfo->SetComment("nickname");
    newStorageInfo->SetCapacitySize(StorageCapacityType::Cap256);
    if(!AddStorageInfo(newStorageInfo))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("AddStorageInfo fail."));
        return false;
    }

    SetComment("");

    return true;
}

SERVICE_END
