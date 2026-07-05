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
 * Date: 2026-07-03 12:31:36
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <Comps/UserSys/Notify/impl/NotifyMgrMongoStorage.h>
#include <Comps/UserSys/Notify/impl/NotifyMgrMongoStorageFactory.h>
#include <Comps/UserSys/Notify/impl/NotifyMgr.h>
#include <OptionComp/storage/MongoDB/MongoDBComp.h>

SERVICE_BEGIN

NotifyMgrMongoStorage::NotifyMgrMongoStorage()
 :IMongodbStorageInfo(KERNEL_NS::RttiUtil::GetTypeId<NotifyMgrMongoStorage>()
     , KERNEL_NS::RttiUtil::GetByType<NotifyMgr>(), KERNEL_NS::RttiUtil::GetTypeId<NotifyMgr>())
{
  AsFieldSystem(KERNEL_NS::MongoSerializeInfoType::JSON);
}

NotifyMgrMongoStorage::~NotifyMgrMongoStorage()
{
 
}

void NotifyMgrMongoStorage::Release()
{
    NotifyMgrMongoStorage::DeleteByAdapter_NotifyMgrMongoStorage(NotifyMgrMongoStorageFactory::_buildType.V, this);
}

Int32 NotifyMgrMongoStorage::_OnHostInit()
{
    // 设置持久化回调
    SetOnSaveCb<NotifyMgr>(&NotifyMgr::OnSave);
    return Status::Success;
}
SERVICE_END
