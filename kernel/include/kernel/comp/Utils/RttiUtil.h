/*!
 * MIT License
 *  
 * Copyright (c) 2020 Eric Yonng<120453674@qq.com>
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *  
 * 
 * Author: Eric Yonng
 * Date: 2021-01-29 11:58:19
 * Description: 运行时类型识别
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_RTTI_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_RTTI_UTIL_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseType.h>
#include <kernel/common/macro.h>
#include <typeinfo>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class LibString;

class KERNEL_EXPORT RttiUtil
{
public:
    template<typename ObjType>
    static LibString GetByType();

    // 必须有GetObjTypeId方法
    template<typename ObjType>
    requires requires(ObjType o)
    {
        { o.GetObjTypeId() } -> std::convertible_to<UInt64>;
    }
    static LibString GetByObj(ObjType *obj);

    template<typename ObjType>
    static UInt64 GetTypeId();

    template<typename ObjType>
    requires requires(ObjType o)
    {
        { o.GetObjTypeId() } -> std::convertible_to<UInt64>;
    }
    static UInt64 GetTypeIdByObj(ObjType *obj);

    // 存在碰撞的可能性，不建议使用
    template<typename ObjType>
    static UInt64 GetTypeHashCode();

    static UInt64 GetTypIdBy(const LibString &objName);
    static void MakeTypeIdDict(const LibString &objName, UInt64 id);

private:
    static LibString GetByTypeName(const char *rawTypeName);

    template<typename ObjType>
    static UInt64 _GetTypeId();

    static UInt64 _GenTypeId();

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    // 非windows平台下的类型识别
    static const Byte8 *GetCxxDemangle(const char *name);
#endif

};

template<typename ObjType>
ALWAYS_INLINE LibString RttiUtil::GetByType()
{
    auto id = _GetTypeId<ObjType>();
    auto &&name = GetByTypeName(typeid(ObjType).name());
    MakeTypeIdDict(name, id);

    return name;
}

template<typename ObjType>
requires requires(ObjType o)
{
    { o.GetObjTypeId() } -> std::convertible_to<UInt64>;
}
ALWAYS_INLINE LibString RttiUtil::GetByObj(ObjType *obj)
{
    auto &&name = GetByTypeName(typeid(*obj).name());
    UInt64 id = obj->GetObjTypeId();
    if(UNLIKELY(id == 0))
    {
        throw new std::logic_error(KERNEL_NS::LibString().AppendFormat("get obj type id fail objtype:%s", name.c_str()).GetRaw());
    }
    MakeTypeIdDict(name, id);

    return name;
}

template<typename ObjType>
ALWAYS_INLINE UInt64 RttiUtil::GetTypeHashCode()
{
    auto id = _GetTypeId<ObjType>();
    auto name = GetByTypeName(typeid(ObjType).name());
    MakeTypeIdDict(name, id);

    return id;
}

template<typename ObjType>
ALWAYS_INLINE UInt64 RttiUtil::GetTypeId()
{
    auto id = _GetTypeId<ObjType>();
    auto name = GetByTypeName(typeid(ObjType).name());
    MakeTypeIdDict(name, id);

    return id;
}

template<typename ObjType>
requires requires(ObjType o)
{
    { o.GetObjTypeId() } -> std::convertible_to<UInt64>;
}
ALWAYS_INLINE UInt64 RttiUtil::GetTypeIdByObj(ObjType *obj)
{
    UInt64 id = obj->GetObjTypeId();
    auto &&name = GetByTypeName(typeid(*obj).name());
    if(UNLIKELY(id == 0))
    {
        throw new std::logic_error(KERNEL_NS::LibString().AppendFormat("get obj type id fail objtype:%s", name.c_str()).GetRaw());
    }
    MakeTypeIdDict(name, id);

    return id;
}

template<typename ObjType>
ALWAYS_INLINE UInt64 RttiUtil::_GetTypeId()
{
    static UInt64 s_objTypeId = _GenTypeId();
    return s_objTypeId;
}


KERNEL_END

#endif
