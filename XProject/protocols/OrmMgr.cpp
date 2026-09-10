/*
 * @Author: ericyonng 120453674@qq.com
 * @Date: 2026-09-07 23:53:10
 * @LastEditors: ericyonng 120453674@qq.com
 * @LastEditTime: 2026-09-11 01:06:58
 * @FilePath: \XProject\protocols\OrmMgr.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
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
#include <service_common/protocol/ORM/IOrmData.h>
#include <protocols/orm_out/AllOrmDatas.h>
#include <kernel/comp/Log/log.h>

#include "kernel/comp/Utils/ContainerUtil.h"

SERVICE_COMMON_BEGIN
    OrmMgr::OrmMgr()
:IOrmMgr(KERNEL_NS::RttiUtil::GetTypeId<OrmMgr>())
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
        CLOG_WARN("exists orm id:%lld, factory", ormId);
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


SERVICE_COMMON_END
