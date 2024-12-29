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
#include <kernel/comp/Utils/StringUtil.h>
#include <kernel/comp/Log/LogTool.h>

KERNEL_BEGIN

#ifndef INHERIT_TABLE
 #define INHERIT_TABLE "inherit_table"
#endif

struct KERNEL_EXPORT CppVoidT {};

struct KERNEL_EXPORT LuaStringTool
{
	inline static const char* c_str(const std::string& s) { return s.c_str(); }
	inline static const char* c_str(const char* s)   { return s; }
};

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

	explicit LuaException(const LibString & err)
	:_err(err.GetRaw())
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
struct KERNEL_EXPORT BaseTypePtrTraits<const std::string&>
{
    typedef std::string ArgType;
};

template<>
struct KERNEL_EXPORT BaseTypePtrTraits<std::string &>
{
    typedef std::string ArgType;
};

template<>
struct KERNEL_EXPORT BaseTypePtrTraits<std::string>
{
    typedef std::string ArgType;
};

template<>
struct KERNEL_EXPORT BaseTypePtrTraits<const char *>
{
    typedef char * ArgType;
};

template<>
struct KERNEL_EXPORT BaseTypePtrTraits<char*>
{
    typedef char* ArgType;
};

template<>
struct KERNEL_EXPORT BaseTypePtrTraits<char>
{
    typedef char ArgType;
};

template<>
struct KERNEL_EXPORT BaseTypePtrTraits<unsigned char>
{
    typedef unsigned char ArgType;
};

template<>
struct KERNEL_EXPORT BaseTypePtrTraits<short>
{
    typedef short ArgType;
};

template<>
struct KERNEL_EXPORT BaseTypePtrTraits<unsigned short>
{
    typedef unsigned short ArgType;
};

template<>
struct KERNEL_EXPORT BaseTypePtrTraits<int>
{
    typedef int ArgType;
};

template<>
struct KERNEL_EXPORT BaseTypePtrTraits<unsigned int>
{
    typedef unsigned int ArgType;
};

template<>
struct KERNEL_EXPORT BaseTypePtrTraits<long>
{
    typedef long ArgType;
};

template<>
struct KERNEL_EXPORT BaseTypePtrTraits<unsigned long>
{
    typedef unsigned long ArgType;
};
template<>
struct KERNEL_EXPORT BaseTypePtrTraits<long long>
{
    typedef long long ArgType;
};
template<>
struct KERNEL_EXPORT BaseTypePtrTraits<unsigned long long>
{
    typedef unsigned long long ArgType;
};

template<>
struct KERNEL_EXPORT BaseTypePtrTraits<float>
{
    typedef float ArgType;
};
template<>
struct KERNEL_EXPORT BaseTypePtrTraits<bool>
{
    typedef bool ArgType;
};

template<>
struct KERNEL_EXPORT BaseTypePtrTraits<double>
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
struct KERNEL_EXPORT ReferenceTraits<const std::string&>
{
    typedef std::string ArgType;
};

template<>
struct KERNEL_EXPORT ReferenceTraits<std::string&>
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
struct KERNEL_EXPORT ReferenceTraits<const char*>
{
    typedef char* ArgType;
};


template <typename T>
struct InitValueTraits;

template <typename T>
struct InitValueTraits
{
    ALWAYS_INLINE static T Value(){ return T(); }
};

template <typename T>
struct InitValueTraits<const T*>
{
    ALWAYS_INLINE static T* Value(){ return NULL; }
};

template <typename T>
struct KERNEL_EXPORT InitValueTraits<const T&>
{
    ALWAYS_INLINE static T Value(){ return T(); }
};

template <>
struct KERNEL_EXPORT InitValueTraits<std::string>
{
    ALWAYS_INLINE static const char* Value(){ return ""; }
};

template <>
struct KERNEL_EXPORT InitValueTraits<const std::string&>
{
    ALWAYS_INLINE static const char* Value(){ return ""; }
};


template<typename T>
struct LuaOp
{
    static void PushStack(lua_State* ls, const char* arg)
    {
        lua_pushstring(ls, arg);
    }
	
    /*
    static int lua_to_value(lua_State* ls_, int pos_, char*& param_)
    {
        const char* str = luaL_checkstring(ls_, pos_);
        param_ = (char*)str;
        return 0;
    }*/
};


template<>
struct KERNEL_EXPORT LuaOp<const char*>
{
    static void PushStack(lua_State* ls, const char* arg)
    {
        lua_pushstring(ls, arg);
    }
    static int LuaToValue(lua_State* ls, int pos, char*& param)
    {
        const char* str = luaL_checkstring(ls, pos);
        param = const_cast<char*>(str);
        return 0;
    }
};

template<>
struct KERNEL_EXPORT LuaOp<char*>
{
    static void PushStack(lua_State* ls, const char* arg)
    {
        lua_pushstring(ls, arg);
    }
    static int LuaToValue(lua_State* ls, int pos, char*& param)
    {
        const char* str = luaL_checkstring(ls, pos);
        param = const_cast<char*>(str);
        return 0;
    }
};

template<>
struct KERNEL_EXPORT LuaOp<LuaNil>
{
    static void PushStack(lua_State* ls, const LuaNil& arg)
    {
        lua_pushnil(ls);
    }

};

template<>
struct KERNEL_EXPORT LuaOp<CppVoidT>
{
    static int GetRetValue(lua_State* ls, int pos, CppVoidT& param)
    {
        return 0;
    }
};

template<>
struct KERNEL_EXPORT LuaOp<Int64>
{
    static void PushStack(lua_State* ls, Int64 arg)
    {
#if LUA_VERSION_NUM >= 503
		lua_pushinteger(ls, arg);
#else
    	const auto &ss = KERNEL_NS::StringUtil::Num2Str(arg);
    	lua_pushlstring(ls, ss.c_str(), ss.length());
#endif
    }

    static int GetRetValue(lua_State* ls, int pos, Int64& param)
    {
#if LUA_VERSION_NUM >= 503
		if (!lua_isinteger(ls, pos))
		{
			return -1;
		}
		param = lua_tointeger(ls, pos);
    	
#else
        if (!lua_isstring(ls, pos))
        {
            return -1;
        }

    	size_t len  = 0;
    	const char* src = lua_tolstring(ls, pos, &len);
    	param = KERNEL_NS::StringUtil::StringToInt64(src);
#endif
        return 0;
    }

    static int LuaToValue(lua_State* ls, int pos, Int64& param)
    {    	
#if LUA_VERSION_NUM >= 503
		param = luaL_checkinteger(ls, pos);
#else
    	size_t len = 0;
    	const char* str = luaL_checklstring(ls, pos, &len);
    	param = KERNEL_NS::StringUtil::StringToInt64(str);
#endif
        return 0;
    }
};

template<> struct KERNEL_EXPORT LuaOp<UInt64>
{
    static void PushStack(lua_State* ls, UInt64 arg)
    {
    	const auto &ss = KERNEL_NS::StringUtil::Num2Str(arg);
		lua_pushlstring(ls, ss.c_str(), ss.length());
    }

    static int GetRetValue(lua_State* ls, int pos, UInt64& param)
    {
        if (!lua_isstring(ls, pos))
        {
            return -1;
        }

        size_t len  = 0;
        const char* src = lua_tolstring(ls, pos, &len);
        param = KERNEL_NS::StringUtil::StringToUInt64(src);
        return 0;
    }

    static int LuaToValue(lua_State* ls, int pos, UInt64& param)
    {
        size_t len = 0;
        const char* str = luaL_checklstring(ls, pos, &len);
        param = KERNEL_NS::StringUtil::StringToUInt64(str);
        return 0;
    }
};

template<> struct KERNEL_EXPORT LuaOp<Byte8>
{
	static void PushStack(lua_State* ls, Byte8 arg)
	{
		lua_pushnumber(ls, (lua_Number)arg);
	}

	static int GetRetValue(lua_State* ls, int pos, Byte8& param)
	{
		if (!lua_isnumber(ls, pos))
		{
			return -1;
		}
		param = static_cast<Byte8>(lua_tonumber(ls, pos));
		return 0;
	}
	
	static int LuaToValue(lua_State* ls, int pos, Byte8& param)
	{
		param = static_cast<Byte8>(luaL_checknumber(ls, pos));
		return 0;
	}
};

template<>
struct KERNEL_EXPORT LuaOp<U8>
{
	static void PushStack(lua_State* ls, U8 arg)
	{
		lua_pushnumber(ls, (lua_Number)arg);
	}

	static int GetRetValue(lua_State* ls, int pos, U8& param)
	{
		if (!lua_isnumber(ls, pos))
			return -1;

		param =  static_cast<U8>(lua_tonumber(ls, pos));
		return 0;
	}
	
	static int LuaToValue(lua_State* ls, int pos, U8& param)
	{
		param =  static_cast<U8>(luaL_checknumber(ls, pos));
		return 0;
	}
};

template<> struct KERNEL_EXPORT LuaOp<Int16>
{
	static void PushStack(lua_State* ls, Int16 arg)
	{
		lua_pushnumber(ls, (lua_Number)arg);
	}

	static int GetRetValue(lua_State* ls, int pos, Int16& param)
	{
		if (!lua_isnumber(ls, pos))
			return -1;
		
		param = static_cast<Int16>(lua_tonumber(ls, pos));
		return 0;
	}
	
	static int LuaToValue(lua_State* ls, int pos, Int16& param)
	{
		param = static_cast<Int16>(luaL_checknumber(ls, pos));
		return 0;
	}
};

template<> struct KERNEL_EXPORT LuaOp<UInt16>
{
	static void PushStack(lua_State* ls, UInt16 arg)
	{
		lua_pushnumber(ls, (lua_Number)arg);
	}
	
	static int GetRetValue(lua_State* ls, int pos, UInt16& param)
	{
		if (!lua_isnumber(ls, pos))
			return -1;
		
		param = static_cast<UInt16>(lua_tonumber(ls, pos));
		return 0;
	}
	
	static int LuaToValue(lua_State* ls, int pos, UInt16& param)
	{
		param = static_cast<UInt16>(luaL_checknumber(ls, pos));
		return 0;
	}
};

template<> struct KERNEL_EXPORT LuaOp<Int32>
{
	static void PushStack(lua_State* ls, Int32 arg)
	{
		lua_pushnumber(ls, (lua_Number)arg);
	}
	
	static int GetRetValue(lua_State* ls, int pos, Int32& param)
	{
		if (!lua_isnumber(ls, pos))
			return -1;
		
		param = static_cast<Int32>(lua_tonumber(ls, pos));

		return 0;
	}
	
	static int LuaToValue(lua_State* ls, int pos, Int32& param)
	{
		param = static_cast<Int32>(luaL_checknumber(ls, pos));
		return 0;
	}
};

template<> struct KERNEL_EXPORT LuaOp<UInt32>
{
	static void PushStack(lua_State* ls, UInt32 arg)
	{
		lua_pushnumber(ls, (lua_Number)arg);
	}
	
	static int GetRetValue(lua_State* ls, int pos, UInt32& param)
	{
		if (!lua_isnumber(ls, pos))
			return -1;

		param = static_cast<UInt32>(lua_tonumber(ls, pos));
		return 0;
	}
	
	static int LuaToValue(lua_State* ls, int pos, UInt32& param)
	{
		param = static_cast<UInt32>(luaL_checknumber(ls, pos));
		
		return 0;
	}
};

template<>
struct KERNEL_EXPORT LuaOp<bool>
{
    static void PushStack(lua_State* ls, bool arg)
    {
        lua_pushboolean(ls, arg);
    }

    static int GetRetValue(lua_State* ls, int pos, bool& param)
    {
    	//! nil 自动转换为false
    	if (lua_isnil(ls, pos))
    	{
    		param = false;
    		return 0;
    	}
    	
        if (!lua_isboolean(ls, pos))
            return -1;

        param = (0 != lua_toboolean(ls, pos));
        return 0;
    }
	
    static int LuaToValue(lua_State* ls, int pos, bool& param)
    {
		luaL_checktype(ls, pos,  LUA_TBOOLEAN);
        param = (0 != lua_toboolean(ls, pos));
        return 0;
    }
};

template<>
struct KERNEL_EXPORT LuaOp<std::string>
{
    static void PushStack(lua_State* ls, const std::string& arg)
    {
        lua_pushlstring(ls, arg.c_str(), arg.length());
    }

    static int GetRetValue(lua_State* ls, int pos, std::string& param)
    {
        if (!lua_isstring(ls, pos))
            return -1;

        lua_pushvalue(ls, pos);
        size_t len  = 0;
        const char* src = lua_tolstring(ls, -1, &len);
        param.assign(src, len);
        lua_pop(ls, 1);

        return 0;
    }
	
    static int LuaToValue(lua_State* ls, int pos, std::string& param)
    {
        size_t len = 0;
        const char* str = luaL_checklstring(ls, pos, &len);
        param.assign(str, len);
        return 0;
    }
};

template<>
struct KERNEL_EXPORT LuaOp<const std::string&>
{
    static void PushStack(lua_State* ls, const std::string& arg)
    {
        lua_pushlstring(ls, arg.c_str(), arg.length());
    }

    static int GetRetValue(lua_State* ls, int pos, std::string& param)
    {
        if (!lua_isstring(ls, pos))
            return -1;

        lua_pushvalue(ls, pos);
        size_t len  = 0;
        const char* src = lua_tolstring(ls, -1, &len);
        param.assign(src, len);
        lua_pop(ls, 1);

        return 0;
    }
	
    static int LuaToValue(lua_State* ls, int pos, std::string& param)
    {
        size_t len = 0;
        const char* str = luaL_checklstring(ls, pos, &len);
        param.assign(str, len);
        return 0;
    }
};

template<> struct KERNEL_EXPORT LuaOp<Float>
{
	static void PushStack(lua_State* ls, Float arg)
	{
		lua_pushnumber(ls, (lua_Number)arg);
	}
	
	static int GetRetValue(lua_State* ls, int pos, Float& param)
	{
		if (!lua_isnumber(ls, pos))
			return -1;
		
		param = static_cast<Float>(lua_tonumber(ls, pos));
		return 0;
	}
	
	static int LuaToValue(lua_State* ls, int pos, Float& param)
	{
		param =  static_cast<Float>(luaL_checknumber(ls, pos));
		return 0;
	}
};

template<> struct KERNEL_EXPORT LuaOp<Double>
{
	static void PushStack(lua_State* ls, Double arg)
	{
		lua_pushnumber(ls, (lua_Number)arg);
	}
	
	static int GetRetValue(lua_State* ls, int pos, Double& param)
	{
		if (!lua_isnumber(ls, pos))
			return -1;
		
		param = (double)lua_tonumber(ls, pos);
		return 0;
	}
	
	static int LuaToValue(lua_State* ls, int pos, Double& param)
	{
		param = (double)luaL_checknumber(ls, pos);
		return 0;
	}
};

template<>
struct KERNEL_EXPORT LuaOp<void *>
{
    static void PushStack(lua_State* ls, void* arg)
    {
        lua_pushlightuserdata(ls, arg);
    }

    static int GetRetValue(lua_State* ls, int pos, void* & param)
    {
        if (!lua_isuserdata(ls, pos))
        {
        	LogTool::Warn(LOGFMT_NON_OBJ_TAG(LuaOp<void *>, "userdata param expected, but type<%s> provided"), lua_typename(ls, lua_type(ls, pos)));
            return -1;
        }

        param = lua_touserdata(ls, pos);
        return 0;
    }

    static int LuaToValue(lua_State* ls, int pos, void*& param)
    {
        if (!lua_isuserdata(ls, pos))
        {
            luaL_argerror (ls, 1, "userdata param expected");
            return -1;
        }
    	
        param = lua_touserdata(ls, pos);
        return 0;
    }
};

template<typename T>
struct LuaOp<T*>
{
    static void PushStack(lua_State* ls, T& arg)
    {
        void* ptr = lua_newuserdata(ls, sizeof(UserDataForObject<T>));
        new (ptr) UserDataForObject<T>(&arg);

        luaL_getmetatable(ls, LuaTypeInfo<T>::GetName());
        lua_setmetatable(ls, -2);
    }
	
    static void PushStack(lua_State* ls, const T& arg)
    {
        void* ptr = lua_newuserdata(ls, sizeof(UserDataForObject<const T>));
        new (ptr) UserDataForObject<const T>(&arg);

        luaL_getmetatable(ls, LuaTypeInfo<T>::GetName());
        lua_setmetatable(ls, -2);
    }
	
    static void PushStack(lua_State* ls, T* arg)
    {
        void* ptr = lua_newuserdata(ls, sizeof(UserDataForObject<T>));
        new (ptr) UserDataForObject<T>(arg);

        luaL_getmetatable(ls, LuaTypeInfo<T>::GetName());
        lua_setmetatable(ls, -2);
    }

    static int GetRetValue(lua_State* ls, int pos, T* &param)
    {
        if (false == LuaTypeInfo<T>::IsRegisted())
        {
            luaL_argerror(ls, pos, "type not supported");
        }

        void *arg_data = lua_touserdata(ls, pos);

        if (NULL == arg_data)
        {
        	LogTool::Warn(LOGFMT_NON_OBJ_TAG(LuaOp<T *>, "expect<%s> but <%s> NULL"), LuaTypeInfo<T>::GetName(), lua_typename(ls, lua_type(ls, pos)));
            return -1;
        }

        if (0 == lua_getmetatable(ls, pos))
        {
            return -1;
        }

        luaL_getmetatable(ls, LuaTypeInfo<T>::GetName());
        if (0 == lua_rawequal(ls, -1, -2))
        {
        	lua_getfield(ls, -2, INHERIT_TABLE);
        	if (0 == lua_rawequal(ls, -1, -2))
        	{
        		LogTool::Warn(LOGFMT_NON_OBJ_TAG(LuaOp<T *>,"expect<%s> but <%s> not equal"), LuaTypeInfo<T>::GetName(), lua_typename(ls, lua_type(ls, pos)));
				lua_pop(ls, 3);
				return -1;
        	}
        	
            lua_pop(ls, 3);
        }
        else
        {
        	lua_pop(ls, 2);
        }
    	
        T* ret_ptr = (KERNEL_NS::KernelCastTo<UserDataForObject<T> *>(arg_data))->_obj;
        if (NULL == ret_ptr)
        {
            return -1;
        }

        param = ret_ptr;
        return 0;
    }

    static int LuaToValue(lua_State* ls, int pos, T* &param)
    {
        if (false == LuaTypeInfo<T>::IsRegisted())
            luaL_argerror(ls, pos, "type not supported");

    	void *arg_data = lua_touserdata(ls, pos);
        if (NULL == arg_data || 0 == lua_getmetatable(ls, pos))
		{
        	KERNEL_NS::LibString info;
        	info.AppendFormat("`%s` arg1 connot be null", LuaTypeInfo<T>::GetName());
        	
			luaL_argerror(ls, pos, info.c_str());
		}

		luaL_getmetatable(ls, LuaTypeInfo<T>::GetName());
    	
		if (0 == lua_rawequal(ls, -1, -2))
		{
			lua_getfield(ls, -2, INHERIT_TABLE);
			if (0 == lua_rawequal(ls, -1, -2))
			{
				lua_pop(ls, 3);
				KERNEL_NS::LibString info;
				info.AppendFormat("`%s` arg1 type not equal", LuaTypeInfo<T>::GetName());
				
				luaL_argerror(ls, pos, info.c_str());
			}
			
			lua_pop(ls, 3);
		}
		else
		{
			lua_pop(ls, 2);
		}

        T* ret_ptr = (KERNEL_NS::KernelCastTo<UserDataForObject<T> *>(arg_data))->_obj;
        if (NULL == ret_ptr)
        {
        	KERNEL_NS::LibString info;
        	info.AppendFormat("`%s` object ptr can't be null", LuaTypeInfo<T>::GetName());

            luaL_argerror(ls, pos, info.c_str());
        }

        param = ret_ptr;
        return 0;
    }
};

template<typename T>
struct LuaOp<const T*>
{
    static void PushStack(lua_State* ls, const T* arg)
    {
        LuaOp<T*>::PushStack(ls, const_cast<T*>(arg));
    }

    static int GetRetValue(lua_State* ls, int pos, T* &param)
    {
       return LuaOp<T*>::GetRetValue(ls, pos, param);
    }

    static int LuaToValue(lua_State* ls, int pos, T*& param)
    {
        return LuaOp<T*>::LuaToValue(ls, pos, param);
    }
};

template<typename T>
struct LuaOp<std::vector<T>>
{
    static void PushStack(lua_State* ls, const std::vector<T> &arg)
    {
    	lua_newtable(ls);
    	typename std::vector<T>::const_iterator it = arg.begin();
    	for (int i = 1; it != arg.end(); ++it, ++i)
    	{
    		LuaOp<int>::PushStack(ls, i);
    		LuaOp<T>::PushStack(ls, *it);
			lua_settable(ls, -3);
    	}
    }

    static int GetRetValue(lua_State* ls, int pos, std::vector<T>& param)
    {
    	if (0 == lua_istable(ls, pos))
    		return -1;
    	
    	lua_pushnil(ls);
    	int real_pos = pos;
    	if (pos < 0) real_pos = real_pos - 1;

		while (lua_next(ls, real_pos) != 0)
		{
			param.push_back(T());
			if (LuaOp<T>::GetRetValue(ls, -1, param[param.size() - 1]) < 0)
				return -1;
			
			lua_pop(ls, 1);
		}
    	
       return 0;
    }

    static int LuaToValue(lua_State* ls, int pos, std::vector<T> &param)
    {
    	luaL_checktype(ls, pos, LUA_TTABLE);

		lua_pushnil(ls);
    	int real_pos = pos;
    	if (pos < 0) real_pos = real_pos - 1;
		while (lua_next(ls, real_pos) != 0)
		{
			param.push_back(T());
			if (LuaOp<T>::LuaToValue(ls, -1, param[param.size() - 1]) < 0)
				luaL_argerror(ls, (pos > 0) ? pos : -pos, "convert to vector failed");
			
			lua_pop(ls, 1);
		}
    	
		return 0;
    }
};

template<typename T>
struct LuaOp<std::list<T>>
{
    static void PushStack(lua_State* ls, const std::list<T> &arg)
    {
    	lua_newtable(ls);
    	typename std::list<T>::const_iterator it = arg.begin();
    	for (int i = 1; it != arg.end(); ++it, ++i)
    	{
    		LuaOp<int>::PushStack(ls, i);
    		LuaOp<T>::PushStack(ls, *it);
			lua_settable(ls, -3);
    	}
    }

    static int GetRetValue(lua_State* ls, int pos, std::list<T> &param)
    {
    	if (0 == lua_istable(ls, pos))
    		return -1;
    	
    	lua_pushnil(ls);
    	int real_pos = pos;
    	if (pos < 0) real_pos = real_pos - 1;

		while (lua_next(ls, real_pos) != 0)
		{
			param.push_back(T());
			if (LuaOp<T>::GetRetValue(ls, -1, (param.back())) < 0)
				return -1;
			
			lua_pop(ls, 1);
		}
    	
       return 0;
    }

    static int LuaToValue(lua_State* ls, int pos, std::list<T> &param)
    {
    	luaL_checktype(ls, pos, LUA_TTABLE);

		lua_pushnil(ls);
    	int real_pos = pos;
    	if (pos < 0) real_pos = real_pos - 1;
		while (lua_next(ls, real_pos) != 0)
		{
			param.push_back(T());
			if (LuaOp<T>::LuaToValue(ls, -1, (param.back())) < 0)
				luaL_argerror(ls, (pos>0)?pos:-pos, "convert to vector failed");
			
			lua_pop(ls, 1);
		}
    	
		return 0;
    }
};

template<typename T>
struct LuaOp<std::set<T> >
{
    static void PushStack(lua_State* ls, const std::set<T>& arg)
    {
    	lua_newtable(ls);
    	typename std::set<T>::const_iterator it = arg.begin();
    	for (int i = 1; it != arg.end(); ++it, ++i)
    	{
    		LuaOp<int>::PushStack(ls, i);
    		LuaOp<T>::PushStack(ls, *it);
			lua_settable(ls, -3);
    	}
    }

    static int GetRetValue(lua_State* ls, int pos, std::set<T>& param)
    {
    	if (0 == lua_istable(ls, pos))
    		return -1;

    	lua_pushnil(ls);
    	int real_pos = pos;

    	if(pos < 0)
    	{
    		real_pos = real_pos - 1;
    	}
    	
		while(lua_next(ls, real_pos) != 0)
		{
			T val = InitValueTraits<T>::Value();
			if (LuaOp<T>::GetRetValue(ls, -1, val) < 0)
				return -1;
			
			param.insert(val);
			lua_pop(ls, 1);
		}
    	
       return 0;
    }

    static int LuaToValue(lua_State* ls, int pos, std::set<T>& param)
    {
    	luaL_checktype(ls, pos, LUA_TTABLE);

		lua_pushnil(ls);
    	int real_pos = pos;
    	if(pos < 0)
    	{
    		real_pos = real_pos - 1;
    	}
    	
		while(lua_next(ls, real_pos) != 0)
		{
			T val = InitValueTraits<T>::Value();
			if(LuaOp<T>::LuaToValue(ls, -1, val) < 0)
				luaL_argerror(ls, (pos>0) ? pos : -pos, "convert to vector failed");
			
			param.insert(val);
			lua_pop(ls, 1);
		}
    	
		return 0;
    }
};

template<typename K, typename V>
struct LuaOp<std::map<K, V> >
{
    static void PushStack(lua_State* ls, const std::map<K, V>& arg)
    {
    	lua_newtable(ls);
    	typename std::map<K, V>::const_iterator it = arg.begin();
    	for (; it != arg.end(); ++it)
    	{
    		LuaOp<K>::PushStack(ls, it->first);
    		LuaOp<V>::PushStack(ls, it->second);
			lua_settable(ls, -3);
    	}
    }

    static int GetRetValue(lua_State* ls, int pos, std::map<K, V> &param)
    {
    	if (0 == lua_istable(ls, pos))
    		return -1;

    	lua_pushnil(ls);
    	int real_pos = pos;

    	if(pos < 0)
    	{
    		real_pos = real_pos - 1;
    	}

    	while(lua_next(ls, real_pos) != 0)
		{
			K key = InitValueTraits<K>::Value();
			V val = InitValueTraits<V>::Value();

			if((LuaOp<K>::GetRetValue(ls, -2, key) < 0) ||
				(LuaOp<V>::GetRetValue(ls, -1, val) < 0))
				return -1;
			
			param.insert(std::make_pair(key, val));

			lua_pop(ls, 1);
		}
    	
       return 0;
    }

    static int LuaToValue(lua_State* ls, int pos, std::map<K, V> &param)
    {
    	luaL_checktype(ls, pos, LUA_TTABLE);

		lua_pushnil(ls);
    	int real_pos = pos;
    	if(pos < 0)
    	{
    		real_pos = real_pos - 1;
    	}

    	while(lua_next(ls, real_pos) != 0)
		{
			K key = InitValueTraits<K>::Value();
			V val = InitValueTraits<V>::Value();
    		
			if((LuaOp<K>::LuaToValue(ls, -2, key) < 0) ||
				(LuaOp<V>::LuaToValue(ls, -1, val) < 0))
				luaL_argerror(ls, pos>0 ? pos:-pos, "convert to vector failed");
    		
			param.insert(std::make_pair(key, val));
			lua_pop(ls, 1);
		}
    	
		return 0;
    }
};

KERNEL_END

#endif
