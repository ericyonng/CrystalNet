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
 * Date: 2026-07-03 14:42:36
 * Author: Eric Yonng
 * Description: 
*/


#include <pch.h>
#include <Comps/UserSys/Login//impl/LoginMgrMongoStorage.h>
#include <Comps/UserSys/Login//impl/LoginMgrMongoStorageFactory.h>
#include <Comps/UserSys/Login/impl/LoginMgr.h>
#include <OptionComp/storage/MongoDB/MongoDBComp.h>

SERVICE_BEGIN

LoginMgrMongoStorage::LoginMgrMongoStorage()
 :IMongodbStorageInfo(KERNEL_NS::RttiUtil::GetTypeId<LoginMgrMongoStorage>(), KERNEL_NS::RttiUtil::GetByType<LoginMgr>())
{
 AsFieldSystem(KERNEL_NS::MongoSerializeInfoType::JSON);
}

LoginMgrMongoStorage::~LoginMgrMongoStorage()
{
 
}

void LoginMgrMongoStorage::Release()
{
 LoginMgrMongoStorage::DeleteByAdapter_LoginMgrMongoStorage(LoginMgrMongoStorageFactory::_buildType.V, this);
}

Int32 LoginMgrMongoStorage::_OnHostInit()
{
    // 设置持久化回调
    SetOnSaveCb<LoginMgr>(&LoginMgr::OnSave);
    return Status::Success;
}

SERVICE_END