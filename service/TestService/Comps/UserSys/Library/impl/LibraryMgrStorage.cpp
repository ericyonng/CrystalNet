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
 * Date: 2023-09-16 20:55:00
 * Author: Eric Yonng
 * Description: 
*/
#include <pch.h>
#include <Comps/UserSys/Library/impl/LibraryMgrStorage.h>
#include <Comps/UserSys/Library/impl/LibraryMgrStorageFactory.h>
#include <Comps/UserSys/Library/impl/LibraryMgr.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(LibraryMgrStorage);

LibraryMgrStorage::LibraryMgrStorage()
:IStorageInfo(KERNEL_NS::RttiUtil::GetByType<LibraryMgr>())
{

}

LibraryMgrStorage::~LibraryMgrStorage()
{

}


void LibraryMgrStorage::Release()
{
    LibraryMgrStorage::DeleteByAdapter_LibraryMgrStorage(LibraryMgrStorageFactory::_buildType.V, this);
}

bool LibraryMgrStorage::RegisterStorages()
{
    // 当前系统属性设置(kv 系统, mysql存储, 某个系统的数据)
    AddFlags(StorageFlagType::NORMAL_TEXT_STRING_FIELD_FLAG | 
    StorageFlagType::MYSQL_FLAG | 
    StorageFlagType::SYSTEM_DATA_STORAGE_FLAG
    );

    SetComment("librarymgr");

    return true;
}


SERVICE_END