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
 * Date: 2024-11-27 23:00:00
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LUA_LUATYPE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LUA_LUATYPE_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/common/BaseType.h>

#include <exception>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <lua/lua.hpp>

#include <kernel/comp/LibString.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Utils/TlsUtil.h>
#include <kernel/comp/memory/ObjPoolMacro.h>

KERNEL_BEGIN

#define INHERIT_TABLE "inherit_table"

struct KERNEL_EXPORT CppVoidT {};

class KERNEL_EXPORT LuaException: public std::exception
{
public:
    explicit LuaException(const char *err)
    :_err(err)
	{

    }

    explicit LuaException(const std::string & err)
    :_err(err)
	{
	}

    ~LuaException() throw () override
    {

    }

    const char* what() const throw () override
    { 
        return _err.c_str(); 
    }

private:
    std::string _err;
};


class KERNEL_EXPORT FFLuaTool
{
public:
    static KERNEL_NS::LibString GetDumpStack(lua_State* ls)
    {
        int i;
        int top = lua_gettop(ls);

        KERNEL_NS::LibString dumpInfo;
        for (i = 1; i <= top; ++i)
        {
            int t = lua_type(ls, i);
            switch (t)
            {
                case LUA_TSTRING:
                {
                    dumpInfo.AppendFormat("`%s'", lua_tostring(ls, i));
                }
                break;
                case LUA_TBOOLEAN:
                {
                    dumpInfo.AppendFormat("%s", lua_toboolean(ls, i) ? "true" : "false");
                }
                break;
                case LUA_TNUMBER:
                {
                    dumpInfo.AppendFormat("`%g`", lua_tonumber(ls, i));
                }
                break;
                case LUA_TTABLE:
                {
                    dumpInfo.AppendFormat("table start:\n");

                    lua_pushnil(ls);
                    while (lua_next(ls, i) != 0) 
                    {
                        dumpInfo.AppendFormat("	%s - %s\n",
                                lua_typename(ls, lua_type(ls, -2)),
                                lua_typename(ls, lua_type(ls, -1)));

                        lua_pop(ls, 1);
                    }

                    dumpInfo.AppendFormat("table end\n");
                }
                break;
                default:
                {
                    dumpInfo.AppendFormat("`%s`", lua_typename(ls, t));
                }
                break;
            }

            dumpInfo.AppendFormat(" ");
        }

        dumpInfo.AppendFormat("\n");

        return dumpInfo;
    }

    static KERNEL_NS::LibString DumpError(lua_State* ls, const char *fmt, ...) LIB_KERNEL_FORMAT_CHECK(2, 3)
    {
        // 计算所需字符串长度
        va_list va;
        va_start(va, fmt);
        auto finalSize = LibString::CheckFormatSize(fmt, va);
        va_end(va);

        va_start(va, fmt);
        KERNEL_NS::LibString info;
        info.AppendFormatWithVaList(finalSize, fmt, va)
        .AppendEnd();
        va_end(va);

        info.AppendFormat("tracback:%s", lua_tostring(ls, -1));
        return info;
    }
};

typedef KERNEL_EXPORT Int32 (*LuaFunction) (lua_State *L);

class KERNEL_EXPORT LuaNil
{

};


template<typename T>
struct UserDataForObject
{
    UserDataForObject(T* p = NULL): _obj(p)
    {

    }
    
    T* _obj;
};

template<typename T>
struct LuaTypeRttiStr
{
    POOL_CREATE_TEMPLATE_OBJ_DEFAULT(LuaTypeRttiStr, T);

    KERNEL_NS::LibString _str;
    KERNEL_NS::LibString _inheritName;
};

template<typename T>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_IMPL(LuaTypeRttiStr, T);

template<typename T>
struct LuaTypeInfo
{
	static void SetName(const std::string &name, std::string &&inheritName = std::string(""))
	{
	    auto &rtti = GetLuaTypeRttiStr();
	    rtti._str = name;
	    rtti._inheritName = inheritName;
	}

    ALWAYS_INLINE static const Byte8 *GetName()
    {
		auto &rtti = GetLuaTypeRttiStr();
		return rtti._str.c_str();
    }

    ALWAYS_INLINE static const char* GetInheritName()
	{
	    auto &rtti = GetLuaTypeRttiStr();
		return rtti._inheritName.c_str();
	}

    ALWAYS_INLINE static bool IsRegisted()
	{
		auto &rtti = GetLuaTypeRttiStr();
		return !rtti._str.empty();
	}

    ALWAYS_INLINE static bool IsInherit()
	{
	    auto &rtti = GetLuaTypeRttiStr();
		return !rtti._inheritName.empty();
	}

    static LuaTypeRttiStr<T> &GetLuaTypeRttiStr()
    {
	    auto rttiStr = KERNEL_NS::TlsUtil::GetOrCreateSmartPtr<LuaTypeRttiStr<T>, KERNEL_NS::AutoDelMethods::CustomDelete>();
	    auto &rttiPtr = rttiStr->GetPtr();
	    if(UNLIKELY(!rttiPtr))
	    {
	        rttiPtr = LuaTypeRttiStr<T>::NewThreadLocal_LuaTypeRttiStr();
	        rttiPtr.SetClosureDelegate([](void *ptr)
            {
                LuaTypeRttiStr<T>::DeleteThreadLocal_LuaTypeRttiStr(KERNEL_NS::KernelCastTo<LuaTypeRttiStr<T>>(ptr));
            });
	    }

	    return *rttiPtr;
    }
};

template<typename ARG_TYPE>
struct BaseTypePtrTraits;

template<>
struct BaseTypePtrTraits<const std::string&>
{
    typedef std::string ArgType;
};

template<>
struct BaseTypePtrTraits<std::string &>
{
    typedef std::string ArgType;
};

template<>
struct BaseTypePtrTraits<std::string>
{
    typedef std::string ArgType;
};

template<>
struct BaseTypePtrTraits<const char *>
{
    typedef char * ArgType;
};

template<>
struct BaseTypePtrTraits<char*>
{
    typedef char* ArgType;
};

template<>
struct BaseTypePtrTraits<char>
{
    typedef char ArgType;
};

template<>
struct BaseTypePtrTraits<unsigned char>
{
    typedef unsigned char ArgType;
};

template<>
struct BaseTypePtrTraits<short>
{
    typedef short ArgType;
};

template<>
struct BaseTypePtrTraits<unsigned short>
{
    typedef unsigned short ArgType;
};

template<>
struct BaseTypePtrTraits<int>
{
    typedef int ArgType;
};

template<>
struct BaseTypePtrTraits<unsigned int>
{
    typedef unsigned int ArgType;
};

template<>
struct BaseTypePtrTraits<long>
{
    typedef long ArgType;
};

template<>
struct BaseTypePtrTraits<unsigned long>
{
    typedef unsigned long ArgType;
};
template<>
struct BaseTypePtrTraits<long long>
{
    typedef long long ArgType;
};
template<>
struct BaseTypePtrTraits<unsigned long long>
{
    typedef unsigned long long ArgType;
};

template<>
struct BaseTypePtrTraits<float>
{
    typedef float ArgType;
};
template<>
struct BaseTypePtrTraits<bool>
{
    typedef bool ArgType;
};

template<>
struct BaseTypePtrTraits<double>
{
    typedef double ArgType;
};
template<typename T>
struct BaseTypePtrTraits<const T&>
{
    typedef T* ArgType;
};
template<typename T>
struct BaseTypePtrTraits<T&>
{
    typedef T* ArgType;
};
template<typename T>
struct BaseTypePtrTraits<T*>
{
    typedef T* ArgType;
};
template<typename T>
struct BaseTypePtrTraits<const T*>
{
    typedef T* ArgType;
};
template<typename T>
struct BaseTypePtrTraits<std::vector<T> >
{
    typedef std::vector<T> ArgType;
};

template<typename T>
struct BaseTypePtrTraits<std::list<T> >
{
    typedef std::list<T> ArgType;
};
template<typename T>
struct BaseTypePtrTraits<std::set<T> >
{
    typedef std::set<T> ArgType;
};

template<typename K, typename V>
struct BaseTypePtrTraits<std::map<K, V>>
{
    typedef std::map<K, V> ArgType;
};

template<typename T>
struct BaseTypePtrTraits<std::vector<T> &>
{
    typedef std::vector<T> ArgType;
};

template<typename T>
struct BaseTypePtrTraits<std::list<T> &>
{
    typedef std::list<T> ArgType;
};

template<typename T>
struct BaseTypePtrTraits<std::set<T> &>
{
    typedef std::set<T> ArgType;
};

template<typename K, typename V>
struct BaseTypePtrTraits<std::map<K, V> &>
{
    typedef std::map<K, V> ArgType;
};

template<typename T>
struct BaseTypePtrTraits<const std::vector<T> &>
{
    typedef std::vector<T> ArgType;
};

template<typename T>
struct BaseTypePtrTraits<const std::list<T> &>
{
    typedef std::list<T> ArgType;
};

template<typename T>
struct BaseTypePtrTraits<const std::set<T> &>
{
    typedef std::set<T> ArgType;
};

template<typename K, typename V>
struct BaseTypePtrTraits<const std::map<K, V> &>
{
    typedef std::map<K, V> ArgType;
};


template<typename ARG_TYPE>
struct LuaPt;

template<typename ARG_TYPE>
struct LuaPt
{
    static ARG_TYPE r(ARG_TYPE a) { return a;  }
    static ARG_TYPE& r(ARG_TYPE* a) { return *a; }
};
template<typename ARG_TYPE>
struct LuaPt<ARG_TYPE&>
{
    static ARG_TYPE& r(ARG_TYPE& a) { return a;  }
    static ARG_TYPE& r(ARG_TYPE* a) { return *a; }
};

template<typename ARG_TYPE>
struct ReferenceTraits;

template<typename ARG_TYPE>
struct ReferenceTraits
{
    typedef ARG_TYPE ArgType;
};

template<>
struct ReferenceTraits<const std::string&>
{
    typedef std::string ArgType;
};

template<>
struct ReferenceTraits<std::string&>
{
    typedef std::string ArgType;
};

template<typename T>
struct ReferenceTraits<const T*>
{
    typedef T* ArgType;
};
template<typename T>
struct ReferenceTraits<const T&>
{
    typedef T ArgType;
};

template<>
struct ReferenceTraits<const char*>
{
    typedef char* ArgType;
};

KERNEL_END

#endif
