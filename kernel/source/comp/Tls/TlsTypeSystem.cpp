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
 * Date: 2024-08-21 21:12:14
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Tls/TlsTypeSystem.h>
#include <kernel/comp/Utils/RttiUtil.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/memory/ObjPoolWrap.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(TlsTypeSystem);

TlsTypeSystem::TlsTypeSystem()
:CompObject(RttiUtil::GetTypeId<TlsTypeSystem>())
{
    // 将自己添加进去
    CheckAddTypeInfo(this);
}

void TlsTypeSystem::Release()
{
    TlsTypeSystem::DeleteByAdapter_TlsTypeSystem(TlsTypeSystemFactory::_buildType.V, this);
}

bool TlsTypeSystem::CheckAddTypeInfo(IObject *obj)
{
    if(UNLIKELY(!obj))
    {
        if (g_Log)
            g_Log->Error(LOGFMT_OBJ_TAG("obj is null"));
        return false;
    }

    Int32 errCount = 0;
    const auto &objName = RttiUtil::GetByObj(obj);
    auto typeId = obj->GetObjTypeId();
    if(UNLIKELY(typeId == 0))
    {
        if (g_Log)
            g_Log->Error(LOGFMT_OBJ_TAG("typeId zero:{typeId:%llu, name:%s}")
            , typeId, objName.c_str());
        return false;
    }

    {
        auto iter = _typeIdRefTypeName.find(typeId);
        if((iter != _typeIdRefTypeName.end()) && (iter->second != objName))
        {
            if (g_Log)
                g_Log->Error(LOGFMT_OBJ_TAG("invalid typeId,type name pair:{typeId:%llu, name:%s}, exists:{typeId:%llu, name:%s}")
            , typeId, objName.c_str(), typeId, iter->second.c_str());

            ++errCount;
        }
    }

    {
        auto iter = _typeNameRefTypeId.find(objName);
        if(iter != _typeNameRefTypeId.end() && iter->second != typeId)
        {
            if (g_Log)
                g_Log->Error(LOGFMT_OBJ_TAG("invalid name => typeid pair:{name:%s, typeId:%llu}, exists:{name:%s, typeId:%llu}")
            , objName.c_str(), typeId, iter->first.c_str(), typeId);

            ++errCount;
        }
    }

    if(UNLIKELY(errCount != 0))
        return false;

    _typeNameRefTypeId[objName] = typeId;
    _typeIdRefTypeName[typeId] = objName;

    return true;
}

Int32 TlsTypeSystem::_OnCreated()
{
    // 将owner添加进去
    auto owner = GetOwner();
    if(!CheckAddTypeInfo(owner))
    {
        if (g_Log)
            g_Log->Error(LOGFMT_OBJ_TAG("CheckAddTypeInfo owner:%s, fail."), owner? owner->GetObjName().c_str() : "");
        return Status::Failed;
    }

    return Status::Success;
}

KERNEL_NS::CompFactory *TlsTypeSystemFactory::FactoryCreate()
{
    return KERNEL_NS::ObjPoolWrap<TlsTypeSystemFactory>::NewByAdapter(_buildType.V);
}

void TlsTypeSystemFactory::Release()
{
    KERNEL_NS::ObjPoolWrap<TlsTypeSystemFactory>::DeleteByAdapter(_buildType.V, this);
}

CompObject *TlsTypeSystemFactory::Create() const
{
    return TlsTypeSystem::NewByAdapter_TlsTypeSystem(_buildType.V);
}


KERNEL_END
