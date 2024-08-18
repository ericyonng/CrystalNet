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
 * Date: 2021-01-29 12:03:10
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Utils/RttiUtil.h>
#include <kernel/comp/Tls/Tls.h>
#include <kernel/comp/Utils/TlsUtil.h>
#include <kernel/comp/LibString.h>

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    // linux下类型识别接口相关
    #include <cxxabi.h>
    // linux下堆栈追踪头文件
    #include <execinfo.h>

#endif


KERNEL_BEGIN


// 从rtti中剔除str
#define __LIB_GET_TYPE_NAME_Trim(str, len)       \
    it = rtti;                                   \
    skipCopy = 0;                                \
    itEnd = rtti + rawTyNameLen - totalSkipCopy; \
    while ((it = strstr(it, str)))               \
    {                                            \
        size_t copyLen = itEnd - it - len;       \
        memmove(it, it + len, copyLen);          \
        itEnd -= len;                            \
        skipCopy += len;                         \
        if (copyLen == 0)                        \
            break;                               \
    }                                            \
    totalSkipCopy += skipCopy;                   \
    rtti[rawTyNameLen - totalSkipCopy] = '\0'    \


#ifdef _WIN32

#define PTR_SUFFIX " __ptr64"

#elif defined(_WIN64)

#define PTR_SUFFIX " __ptr64"

#endif

LibString RttiUtil::GetByTypeName(const char *rawTypeName)
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    auto tlsStack = TlsUtil::GetTlsStack();
    Byte8 *rtti = tlsStack->GetDef()->rtti;

    size_t rawTyNameLen = strlen(rawTypeName);
    memcpy(rtti, rawTypeName, rawTyNameLen);
    rtti[rawTyNameLen] = '\0';

    size_t totalSkipCopy = 0;

    Byte8 *it;
    Byte8 *itEnd;
    size_t skipCopy;
    __LIB_GET_TYPE_NAME_Trim("class ", 6);
    __LIB_GET_TYPE_NAME_Trim("struct ", 7);
    __LIB_GET_TYPE_NAME_Trim(PTR_SUFFIX, 8);
    //__LIB_GET_TYPE_NAME_Trim(" *", 2);

    Byte8 *anonBeg = rtti;
    while((anonBeg = strchr(anonBeg, '`')))
    {
        *anonBeg = '(';
        Byte8 *anonEnd = strchr(anonBeg + 1, '\'');
        *anonEnd = ')';
    }

    return rtti;
#else // Non-Win32
    return GetCxxDemangle(rawTypeName);
#endif
}

#undef __LIB_GET_TYPE_NAME_Trim


#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
const Byte8 *RttiUtil::GetCxxDemangle(const char *name)
{
    auto tlsStack = TlsUtil::GetTlsStack();
    auto defTls = tlsStack->GetDef();

    int status = 0;
    size_t length = sizeof(defTls->rtti);

    // 名字重整技术应用
    abi::__cxa_demangle(name, defTls->rtti, &length, &status);
    if(status != 0)
        return "";

    return defTls->rtti;
}
#endif

static std::unordered_map<KERNEL_NS::LibString, UInt64> &GetRttiTypeDict()
{
    // 泄漏不要紧, 很小, 不会每次调用递增
    DEF_STATIC_THREAD_LOCAL_DECLEAR std::unordered_map<KERNEL_NS::LibString, UInt64> *s_dict = NULL;

    if(UNLIKELY(!s_dict))
        s_dict = new std::unordered_map<KERNEL_NS::LibString, UInt64>();

    return *s_dict;
}
UInt64 RttiUtil::_GenTypeId()
{
    static std::atomic<UInt64> s_inc = {0};
    return ++s_inc;
}

UInt64 RttiUtil::GetTypIdBy(const LibString &objName)
{
    auto &dict = GetRttiTypeDict();
    auto iter = dict.find(objName);
    return iter == dict.end() ? 0 : iter->second;
}

void RttiUtil::MakeTypeIdDict(const LibString &objName, UInt64 id)
{
    auto &dict = GetRttiTypeDict();
    auto iter = dict.find(objName);
    if(UNLIKELY(iter != dict.end()))
        return;

    dict.insert(std::make_pair(objName, id));
}

KERNEL_END

