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
 * Date: 2023-12-16 22:09:46
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <protocols/OrmMgr.h>
#include <protocols/OrmMgrFactory.h>
#include <kernel/kernel.h>
#include <service_common/protocol/ORM/IOrmData.h>
#include <protocols/orm_out/AllOrmDatas.h>

SERVICE_COMMON_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IOrmMgr);
POOL_CREATE_OBJ_DEFAULT_IMPL(OrmMgr);

OrmMgr::OrmMgr()
{

}

OrmMgr::~OrmMgr()
{
    _Clear();
}

void OrmMgr::Release()
{
    OrmMgr::DeleteByAdapter_OrmMgr(OrmMgrFactory::_buildType.V, this);
}

const std::unordered_map<Int64, IOrmDataFactory *> &OrmMgr::GetAllOrmFactorys() const
{
    return _ormIdRefOrmFactory;
}

IOrmData *OrmMgr::CreateOrmData(Int64 ormId) const
{
    auto iter = _ormIdRefOrmFactory.find(ormId);
    if(UNLIKELY(iter == _ormIdRefOrmFactory.end()))
        return NULL;

    return iter->second->Create();
}

void OrmMgr::AddOrmFactory(IOrmDataFactory *factory)
{
    const auto ormId = factory->GetOrmId();
    auto iter = _ormIdRefOrmFactory.find(ormId);
    if(UNLIKELY(iter != _ormIdRefOrmFactory.end()))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("exists orm id:%lld, factory"), ormId);
        iter->second->Release();
        _ormIdRefOrmFactory.erase(iter);
    }

    _ormIdRefOrmFactory.insert(std::make_pair(ormId, factory));
}

Int32 OrmMgr::_OnInit()
{
    #include <protocols/orm_out/RegisterAllOrmFactory.hpp>

    return Status::Success;
}

void OrmMgr::_OnClose() 
{
    _Clear();
}

void OrmMgr::_Clear()
{
    KERNEL_NS::ContainerUtil::DelContainer2(_ormIdRefOrmFactory);
}

OBJ_GET_OBJ_TYPEID_IMPL(OrmMgr)

SERVICE_COMMON_END
