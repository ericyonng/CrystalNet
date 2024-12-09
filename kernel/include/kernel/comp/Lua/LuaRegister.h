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
 * Date: 2024-12-07 13:29:00
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LUA_REGISTER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LUA_REGISTER_H__

#pragma once

#include <kernel/comp/Lua/LuaType.h>
#include <kernel/comp/Utils/RttiUtil.h>

KERNEL_BEGIN

// 基类构造
#ifndef LUA_VIRTUAL_CTOR
 #define LUA_VIRTUAL_CTOR Int32
#endif
// 构造
#ifndef LUA_CTOR
 #define LUA_CTOR void
#endif

#ifndef LUA_ARG_POS
 #define  LUA_ARG_POS(x) (x)
#endif

typedef Int32 (*MtIndexFunc)(lua_State*, void*, const Byte8*);
typedef Int32 (*MtNewIndexFunc)(lua_State*, void*, const Byte8*, Int32);

// 原表名
struct KERNEL_EXPORT LuaOpTool
{
  static std::string ToMetaTableName(const std::string &name)
  {
    return std::string("Lua.") + name;
  }
};

template<typename T>
struct ClassPropertyInfo
{
    ClassPropertyInfo():PropertyPos(NULL)
    {
        
    }
    
  T PropertyPos;
};

// 记录类中字段的指针
struct KERNEL_EXPORT RealClassPropertyProcessor
{
    RealClassPropertyProcessor()
        :_indexImplFunc(NULL)
    ,_newIndexImplFunc(NULL)
    ,_propertyPos(NULL)
    {
        
    }
    
    MtIndexFunc _indexImplFunc;
    MtNewIndexFunc _newIndexImplFunc;
    void* _propertyPos;
};

template<typename FuncType>
struct UserDataForFunction
{
    UserDataForFunction(FuncType funcType)
        :_realFunc(funcType)
    {
        
    }

    FuncType _realFunc;
};

template<typename PropertyType>
struct UserDataForClassProperty : public RealClassPropertyProcessor
{
    typedef ClassPropertyInfo<PropertyType> RealClassPropertyInfo;
    RealClassPropertyInfo PropertyInfo;
};


//!  生成构造函数new,delete,index的实现类
template<typename CLASS_TYPE>
struct MetaTableRegisterImpl
{
	static Int32 MtIndexFunction(lua_State *ls)
	{
		const char* key = luaL_checkstring(ls, LUA_ARG_POS(2));

		luaL_getmetatable(ls, LuaTypeInfo<CLASS_TYPE>::GetName());
		Int32 mtIndex = lua_gettop(ls);

		lua_getfield(ls, -1, key);
		lua_remove(ls, mtIndex);

		//! 没有这个字段，查找基类
		if (lua_isnil(ls, -1) && LuaTypeInfo<CLASS_TYPE>::IsInherit())
		{
			lua_pop(ls, 1);
			luaL_getmetatable(ls, LuaTypeInfo<CLASS_TYPE>::GetInheritName());
			mtIndex = lua_gettop(ls);
			lua_getfield(ls, -1, key);
			lua_remove(ls, mtIndex);
		}

		if (lua_isuserdata(ls, -1))//! 获取属性
		{
			RealClassPropertyProcessor* p = KernelCastTo<RealClassPropertyProcessor>(lua_touserdata(ls, -1));
			lua_pop(ls, 1);
			return (*(p->_indexImplFunc))(ls, p->_propertyPos, key);
		}
		
		return 1;
	}
	
	static Int32 MtNewIndexFunction(lua_State* ls)
	{
		const char* key = luaL_checkstring(ls, LUA_ARG_POS(2));

		luaL_getmetatable(ls, LuaTypeInfo<CLASS_TYPE>::GetName());
		int mtIndex = lua_gettop(ls);

		lua_getfield(ls, -1, key);
		lua_remove(ls, mtIndex);

		//! 没有这个字段，查找基类
		if (lua_isnil(ls, -1) && LuaTypeInfo<CLASS_TYPE>::IsInherit())
		{
			lua_pop(ls, 1);
			luaL_getmetatable(ls, LuaTypeInfo<CLASS_TYPE>::GetInheritName());
			mtIndex = lua_gettop(ls);
			lua_getfield(ls, -1, key);
			lua_remove(ls, mtIndex);
		}
		if (lua_isuserdata(ls, -1))
		{
			RealClassPropertyProcessor* p = KernelCastTo<RealClassPropertyProcessor>(lua_touserdata(ls, -1));
			lua_pop(ls, 1);
			return (*(p->_newIndexImplFunc))(ls, p->_propertyPos, key, LUA_ARG_POS(3));
		}

		return 1;
	}

	static CLASS_TYPE** UserDataToObjectPtrAddress(lua_State* ls)
	{
	    if (false == LuaTypeInfo<CLASS_TYPE>::IsRegisted())
	    {
	    	KERNEL_NS::LibString info;
	    	info.AppendFormat("arg 1 can't convert to class*:%s, because the class has not registed to Lua", RttiUtil::GetByType<CLASS_TYPE>().c_str());

	        luaL_argerror(ls, 1, info.c_str());
	    }

	    void* arg_data = luaL_checkudata(ls, 1, LuaTypeInfo<CLASS_TYPE>::GetName());
	    if (NULL == arg_data)
	    {
	    	KERNEL_NS::LibString info;
	        info.AppendFormat("`%s` expect arg 1, but arg == null", LuaTypeInfo<CLASS_TYPE>::GetName());
	        luaL_argerror(ls, 1, info.c_str());
	    }

	    CLASS_TYPE** ret_ptr = &( (KernelCastTo<UserDataForObject<CLASS_TYPE>>(arg_data))->_obj );
	    return ret_ptr;
	}
	
	static CLASS_TYPE* UserDataToObject(lua_State* ls)
	{
	    if (false == LuaTypeInfo<CLASS_TYPE>::IsRegisted())
	    {
	        luaL_argerror(ls, 1, "arg 1 can't convert to class*, because the class has not registed to Lua");
	    }
		
	    void *arg_data = lua_touserdata(ls, 1);
		if (NULL == arg_data)
		{
			luaL_argerror(ls, 1, "arg 1 need userdsata(can't be null) param");
		}
		if (0 == lua_getmetatable(ls, 1))
		{
			luaL_argerror(ls, 1, "arg 1 has no metatable, it is not cpp type");
		}

		luaL_getmetatable(ls, LuaTypeInfo<CLASS_TYPE>::GetName());
		if (0 == lua_rawequal(ls, -1, -2))
		{
			//! 查找基类
			lua_getfield(ls, -2, INHERIT_TABLE);
			if (0 == lua_rawequal(ls, -1, -2))
			{
				lua_pop(ls, 3);
				luaL_argerror(ls, 1, "type convert failed");
			}
			lua_pop(ls, 3);
		}
		else
		{
			lua_pop(ls, 2);
		}
	    CLASS_TYPE* ret_ptr = (KernelCastTo<UserDataForObject<CLASS_TYPE>>(arg_data))->_obj;
	    if (NULL == ret_ptr)
	    {
	    	LibString info;
	        info.AppendFormat("`%s` object ptr can't be null", LuaTypeInfo<CLASS_TYPE>::GetName());
	        luaL_argerror(ls, 1, info.c_str());
	    }
		
	    return ret_ptr;
	}
	
	static int GetPointer(lua_State* ls)
	{
		CLASS_TYPE** obj_ptr = UserDataToObjectPtrAddress(ls);
		Int64 addr = static_cast<Int64>(*obj_ptr);
		LuaOp<Int64>::PushStack(ls, addr);
		return 1;
	}
};

KERNEL_END

#endif
