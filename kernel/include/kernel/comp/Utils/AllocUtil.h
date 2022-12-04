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
 * Date: 2021-01-13 01:06:25
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_ALLOC_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_ALLOC_UTIL_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/Utils/RttiUtil.h>

KERNEL_BEGIN

class KERNEL_EXPORT AllocUtil
{
public:
    template<typename ObjType, typename... Args>
    static ObjType *NewByPtr(void *ptr, Args&&... args);
    template<typename ObjType>
    static ObjType *NewByPtrNoConstructorParams(void *ptr);

    static void *&GetStaticObjNoFree(std::function<void *()> &&createFactory);
    static void *&GetThreadLocalStaticObjNoFree(std::function<void *()> &&createFactory);

    template<typename ObjType>
    static ObjType *&GetStaticTemplateObjNoFree(std::function<ObjType *()> &&createFactory);

    template<typename ObjType>
    static ObjType *GetStaticThreadLocalTemplateObjNoFree(std::function<void *()> &&createFactory);

    static void *GetThreadLocalStaticObjNoFreeByDict(const KERNEL_NS::LibString &objName, std::function<void *()> &&createFactory);
};

template<typename ObjType, typename... Args>
inline ObjType *AllocUtil::NewByPtr(void *ptr, Args&&... args)
{
    return ::new(ptr)ObjType(std::forward<Args>(args)...);
}

template<typename ObjType>
inline ObjType *AllocUtil::NewByPtrNoConstructorParams(void *ptr)
{
    return ::new(ptr)ObjType();
}

template<typename ObjType>
inline ObjType *&AllocUtil::GetStaticTemplateObjNoFree(std::function<ObjType *()> &&createFactory)
{
    static ObjType *s_obj = createFactory();
    return s_obj;
}

template<typename ObjType>
inline ObjType *AllocUtil::GetStaticThreadLocalTemplateObjNoFree(std::function<void *()> &&createFactory)
{
    DEF_STATIC_THREAD_LOCAL_DECLEAR ObjType *s_obj = NULL;
    if(UNLIKELY(s_obj == NULL))
    {
        s_obj = reinterpret_cast<ObjType *>( GetThreadLocalStaticObjNoFreeByDict(RttiUtil::GetByType<ObjType>()
        , std::forward<std::function<void *()> &&>(createFactory)));
    }

    return s_obj;
}

KERNEL_END


#endif
