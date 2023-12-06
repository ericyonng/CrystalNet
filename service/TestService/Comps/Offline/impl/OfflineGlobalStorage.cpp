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
 * Date: 2023-10-21 22:29:11
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/kernel.h>
#include <Comps/Offline/impl/OfflineGlobalStorage.h>
#include <Comps/Offline/impl/OfflineGlobalStorageFactory.h>
#include <Comps/Offline/impl/OfflineGlobal.h>


SERVICE_BEGIN


POOL_CREATE_OBJ_DEFAULT_IMPL(OfflineGlobalStorage);

OfflineGlobalStorage::OfflineGlobalStorage()
:IStorageInfo(KERNEL_NS::RttiUtil::GetByType<OfflineGlobal>())
{

}

OfflineGlobalStorage::~OfflineGlobalStorage()
{

}

void OfflineGlobalStorage::Release()
{
    OfflineGlobalStorage::DeleteByAdapter_OfflineGlobalStorage(OfflineGlobalStorageFactory::_buildType.V, this);
}

bool OfflineGlobalStorage::RegisterStorages()
{
    AddFlags(StorageFlagType::KEY_VALUE_SYSTEM_FLAG | 
    StorageFlagType::MYSQL_FLAG | 
    StorageFlagType::SYSTEM_DATA_STORAGE_FLAG |
    StorageFlagType::LOAD_DATA_ON_STARTUP_FLAG |
    StorageFlagType::NEED_NUMBER_KEY_FLAG
    );

    SetComment("Offline");

    return true;
}

SERVICE_END
