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
 * Date: 2022-09-29 23:13:14
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Utils/AllocUtil.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

void *&AllocUtil::GetStaticObjNoFree(std::function<void *()> &&createFactory)
{
    static void *s_obj = createFactory();
    return s_obj;
}

void *&AllocUtil::GetThreadLocalStaticObjNoFree(std::function<void *()> &&createFactory)
{
    static void *s_obj = createFactory();
    return s_obj;
}

void *AllocUtil::GetThreadLocalStaticObjNoFreeByDict(const KERNEL_NS::LibString &objName, std::function<void *()> &&createFactory)
{
    DEF_STATIC_THREAD_LOCAL_DECLEAR SmartPtr<std::unordered_map<LibString, void *>> s_objTypeRefPtr;
    if(UNLIKELY(!s_objTypeRefPtr))
    {
        s_objTypeRefPtr = new std::unordered_map<LibString, void *>;
    }

    auto iter = s_objTypeRefPtr->find(objName);
    if(iter == s_objTypeRefPtr->end())
    {
        iter = s_objTypeRefPtr->insert(std::make_pair(objName, createFactory())).first;
        // g_Log->Info(LOGFMT_NON_OBJ_TAG(AllocUtil, "thread id:%llu, thread local create new obj:%s:%p")
        //     , SystemUtil::GetCurrentThreadId(), objName.c_str(), iter->second);
    }
    // else
    // {
    //     // g_Log->Info(LOGFMT_NON_OBJ_TAG(AllocUtil, "thread id:%llu, get thread local obj:%s:%p")
    //     //     , SystemUtil::GetCurrentThreadId(), objName.c_str(), iter->second);
    // }

    return iter->second;
}

KERNEL_END
