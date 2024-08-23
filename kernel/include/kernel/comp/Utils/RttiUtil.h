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
#include <type_traits>

KERNEL_BEGIN

class KERNEL_EXPORT RttiUtil
{
public:
    template<typename ObjType>
    static LibString GetByType();

    // 必须有GetObjTypeId方法
    template<typename ObjType>
    static LibString GetByObj(ObjType *obj);

    template<typename ObjType>
    static UInt64 GetTypeId();

    // static UInt64 GetTypIdBy(const LibString &objName);
    // static void MakeTypeIdDict(const LibString &objName, UInt64 id);

private:
    static const Byte8 *GetByTypeName(const char *rawTypeName);

    static UInt64 _GenTypeId();

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    // 非windows平台下的类型识别
    static const Byte8 *GetCxxDemangle(const char *name);
#endif

};

template<typename ObjType>
ALWAYS_INLINE LibString RttiUtil::GetByType()
{
    return GetByTypeName(typeid(ObjType).name());
}

template<typename ObjType>
ALWAYS_INLINE LibString RttiUtil::GetByObj(ObjType *obj)
{
    return GetByTypeName(typeid(*obj).name());
}

template<typename ObjType>
ALWAYS_INLINE UInt64 RttiUtil::GetTypeId()
{
    static UInt64 s_objTypeId = _GenTypeId();
    return s_objTypeId;
}

KERNEL_END

#endif
