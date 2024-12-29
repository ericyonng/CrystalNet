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
#include <kernel/comp/Log/LogTool.h>

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

#ifndef LUA_TO_METATABLE_NAME
#define LUA_TO_METATABLE_NAME(x) LuaOpTool::ToMetaTableName(x).c_str()
#endif

template<typename T>
struct LuaClassPropertyInfo
{
    LuaClassPropertyInfo():PropertyPos(NULL)
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
    typedef LuaClassPropertyInfo<PropertyType> RealClassPropertyInfo;
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

template <typename CLASS_TYPE, typename FUNC_TYPE>
struct LuaNewTraits;

template <typename CLASS_TYPE>
struct LuaDeleteTraits
{
    static Int32 LuaFunction(lua_State* ls)
    {
        CLASS_TYPE** obj_ptr = MetaTableRegisterImpl<CLASS_TYPE>::UserDataToObjectPtrAddress(ls);

        delete *obj_ptr;
        *obj_ptr = NULL;
        return 0;
    }
};

template <typename FUNC_TYPE>
struct LuaClassFunctionTraits;

template <typename PROPERTY_TYPE, typename RET>
struct LuaClassPropertyTraits;

template <typename FUNC_TYPE>
struct LuaFunctionTraits;

//! CLASS_TYPE 为要注册的类型, CTOR_TYPE为构造函数类型
template<typename T>
struct LuaRegisterRouter;

template<typename T>
struct LuaRegisterRouter
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE *reg, T arg, const std::string &s)
	{
		reg->DefClassProperty(arg, s);
	}
};
template<typename CLASS_TYPE = LuaOpTool, typename CTOR_TYPE = void()>
class LuaRegister
{
public:
	LuaRegister(lua_State *ls):_ls(ls){}
	LuaRegister(lua_State *ls, const std::string &className, std::string inherit_name = "");

	template<typename FUNC_TYPE>
	LuaRegister& Def(FUNC_TYPE func, const std::string& s)
	{
		LuaRegisterRouter<FUNC_TYPE>::Call(this, func, s);
		return *this;
	}
	
	template<typename FUNC_TYPE>
	LuaRegister& DefClassFunc(FUNC_TYPE func, const std::string& func_name)
	{
		LuaFunction class_function = &LuaClassFunctionTraits<FUNC_TYPE>::LuaFunction;
		typedef typename LuaClassFunctionTraits<FUNC_TYPE>::UserdataForFunctionInfo UserdataForFunction;
		void* user_data_ptr = lua_newuserdata(_ls, sizeof(UserdataForFunction));
		new(user_data_ptr) UserdataForFunction(func);
		lua_pushcclosure(_ls, class_function, 1);

		luaL_getmetatable(_ls, LUA_TO_METATABLE_NAME(_className));
		lua_pushstring(_ls, func_name.c_str());
		lua_pushvalue(_ls, -3);
		lua_settable(_ls, -3);

		lua_pop(_ls, 2);
		return *this;
	}
	template<typename RET>
	LuaRegister& DefClassProperty(RET CLASS_TYPE::* p, const std::string &propertyName)
	{
		typedef typename LuaClassPropertyTraits<CLASS_TYPE, RET>::ProcessIndexFunc ProcessIndexFunc;
		typedef typename LuaClassPropertyTraits<CLASS_TYPE, RET>::ProcessNewindexFunc PprocessNewindexFunc;
		ProcessIndexFunc process_index       = &LuaClassPropertyTraits<CLASS_TYPE, RET>::ProcessIndex;
		PprocessNewindexFunc process_newindex = &LuaClassPropertyTraits<CLASS_TYPE, RET>::ProcessNewindex;

		typedef UserDataForClassProperty<RET CLASS_TYPE::*> udata_t;

		udata_t* pu 					= KernelCastTo<udata_t>(lua_newuserdata(_ls, sizeof(udata_t)));
		pu->PropertyInfo.PropertyPos 	= p;
		int udata_index               	= lua_gettop(_ls);
		pu->_indexImplFunc    			= process_index;
		pu->_newIndexImplFunc 			= process_newindex;
		pu->_propertyPos          		= reinterpret_cast<void*>((&(pu->PropertyInfo)));

		luaL_getmetatable(_ls, LuaTypeInfo<CLASS_TYPE>::GetName());
		lua_pushstring(_ls, propertyName.c_str());
		lua_pushvalue(_ls, udata_index);
		lua_settable(_ls, -3);

		lua_pop(_ls, 1);
		lua_remove(_ls, udata_index);
		return *this;
	}
	
	template<typename FUNC>
	LuaRegister& DefFunc(FUNC func, const std::string &funcName)
	{
	    if (_className.empty())
	    {
    		LuaFunction lua_func = LuaFunctionTraits<FUNC>::LuaFunction;
    
    		void* user_data_ptr = lua_newuserdata(_ls, sizeof(func));
    		new(user_data_ptr) FUNC(func);
    
    		lua_pushcclosure(_ls, lua_func, 1);
    		lua_setglobal(_ls, funcName.c_str());
    	}
    	else
	    {
	        LuaFunction lua_func = LuaFunctionTraits<FUNC>::LuaFunction;
    
    		void* user_data_ptr = lua_newuserdata(_ls, sizeof(func));
    		new(user_data_ptr) FUNC(func);
    
    		lua_pushcclosure(_ls, lua_func, 1);
    		//lua_setglobal(m_ls, func_name_.c_str());
    		
	        lua_getglobal(_ls, (_className).c_str());
    		lua_pushstring(_ls, funcName.c_str());
    		lua_pushvalue(_ls, -3);
    		lua_settable(_ls, -3);
    
    		lua_pop(_ls, 2);
	    }
		return *this;
	}
	
private:
	lua_State *_ls;
	std::string _className;
};

template<typename CLASS_TYPE, typename CTOR_TYPE>
ALWAYS_INLINE LuaRegister<CLASS_TYPE, CTOR_TYPE>::LuaRegister(lua_State* ls, const std::string &className, std::string inheritName)
:_ls(ls)
,_className(className)
{
	LuaTypeInfo<CLASS_TYPE>::SetName(LUA_TO_METATABLE_NAME(className), LUA_TO_METATABLE_NAME(inheritName));
	luaL_newmetatable(ls, LUA_TO_METATABLE_NAME(className));
	
	Int32 metatable_index = lua_gettop(ls);
	if (false == inheritName.empty())//! 设置基类
	{
		luaL_getmetatable(ls, LUA_TO_METATABLE_NAME(inheritName));
		if (lua_istable(ls, -1))
		{
			lua_setfield(ls, metatable_index, INHERIT_TABLE);
		}
		else
		{
			lua_pop(ls, 1);
		}
	}
	lua_pushstring(ls, "__index");
	LuaFunction index_function = &MetaTableRegisterImpl<CLASS_TYPE>::MtIndexFunction;
	lua_pushcclosure(ls, index_function, 0);
	lua_settable(ls, -3);

	lua_pushstring(ls, "GetPointer");
	LuaFunction pointer_function = &MetaTableRegisterImpl<CLASS_TYPE>::GetPointer;
	lua_pushcclosure(ls, pointer_function, 0);
	lua_settable(ls, -3);

	lua_pushstring(ls, "__newindex");
	LuaFunction newindex_function = &MetaTableRegisterImpl<CLASS_TYPE>::MtNewIndexFunction;
	lua_pushcclosure(ls, newindex_function, 0);
	lua_settable(ls, -3);

	LuaFunction function_for_new = &LuaNewTraits<CLASS_TYPE, CTOR_TYPE>::LuaFunction;
	lua_pushcclosure(ls, function_for_new, 0);

	lua_newtable(ls);
	lua_pushstring(ls, "new");
	lua_pushvalue(ls, -3);
	lua_settable(ls, -3);

	lua_setglobal(ls, className.c_str());
	
	lua_pop(ls, 1);

	LuaFunction function_for_delete = &LuaDeleteTraits<CLASS_TYPE>::LuaFunction;
	lua_pushcclosure(ls, function_for_delete, 0);

	lua_pushstring(ls, "delete");
	lua_pushvalue(ls, -2);
	lua_settable(ls, metatable_index);

	lua_pop(ls, 2);
}

#pragma region // LuaNewTraits

template <typename CLASS_TYPE>
struct LuaNewTraits<CLASS_TYPE, int()>
{
    static int LuaFunction(lua_State* ls)
    {
        return 0;
    }
};

template <typename CLASS_TYPE>
struct LuaNewTraits<CLASS_TYPE, void()>
{
    static int LuaFunction(lua_State* ls)
    {
        void* user_data_ptr = lua_newuserdata(ls, sizeof(UserDataForObject<CLASS_TYPE>));
        luaL_getmetatable(ls, LuaTypeInfo<CLASS_TYPE>::GetName());
        lua_setmetatable(ls, -2);

        new(user_data_ptr) UserDataForObject<CLASS_TYPE>(new CLASS_TYPE());
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1>
struct LuaNewTraits<CLASS_TYPE, void(ARG1)>
{
    static int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);

        void* user_data_ptr = lua_newuserdata(ls, sizeof(UserDataForObject<CLASS_TYPE>));
        luaL_getmetatable(ls, LuaTypeInfo<CLASS_TYPE>::GetName());
        lua_setmetatable(ls, -2);

        new(user_data_ptr) UserDataForObject<CLASS_TYPE>(new CLASS_TYPE(arg1));
        return 1;
    }
};


template <typename CLASS_TYPE, typename ARG1, typename ARG2>
struct LuaNewTraits<CLASS_TYPE, void(ARG1, ARG2)>
{
    static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);

        void* user_data_ptr = lua_newuserdata(ls, sizeof(UserDataForObject<CLASS_TYPE>));
        luaL_getmetatable(ls, LuaTypeInfo<CLASS_TYPE>::GetName());
        lua_setmetatable(ls, -2);

        new(user_data_ptr) UserDataForObject<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2));
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3>
struct LuaNewTraits<CLASS_TYPE, void(ARG1, ARG2, ARG3)>
{
    static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);

        void* user_data_ptr = lua_newuserdata(ls, sizeof(UserDataForObject<CLASS_TYPE>));

        luaL_getmetatable(ls, LuaTypeInfo<CLASS_TYPE>::GetName());
        lua_setmetatable(ls, -2);

        new(user_data_ptr) UserDataForObject<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2, arg3));
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename ARG4>
struct LuaNewTraits<CLASS_TYPE, void(ARG1, ARG2, ARG3, ARG4)>
{
    static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);

        void* user_data_ptr = lua_newuserdata(ls, sizeof(UserDataForObject<CLASS_TYPE>));
        luaL_getmetatable(ls, LuaTypeInfo<CLASS_TYPE>::GetName());
        lua_setmetatable(ls, -2);

        new(user_data_ptr) UserDataForObject<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2, arg3, arg4));
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename ARG4, typename ARG5>
struct LuaNewTraits<CLASS_TYPE, void(ARG1, ARG2, ARG3, ARG4,
                                               ARG5)>
{
    static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);

        void* user_data_ptr = lua_newuserdata(ls, sizeof(UserDataForObject<CLASS_TYPE>));
        luaL_getmetatable(ls, LuaTypeInfo<CLASS_TYPE>::GetName());
        lua_setmetatable(ls, -2);

        new(user_data_ptr) UserDataForObject<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2, arg3, arg4, arg5));
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename ARG4, typename ARG5, typename ARG6>
struct LuaNewTraits<CLASS_TYPE, void(ARG1, ARG2, ARG3,
                                               ARG4, ARG5, ARG6)>
{
    static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg6);

        void* user_data_ptr = lua_newuserdata(ls, sizeof(UserDataForObject<CLASS_TYPE>));
        luaL_getmetatable(ls, LuaTypeInfo<CLASS_TYPE>::GetName());
        lua_setmetatable(ls, -2);

        new(user_data_ptr) UserDataForObject<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2, arg3, arg4, arg5, arg6));
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename ARG4, typename ARG5, typename ARG6, typename ARG7>
struct LuaNewTraits<CLASS_TYPE, void(ARG1, ARG2, ARG3,
                                               ARG4, ARG5, ARG6, ARG7)>
{
    static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG7>::ArgType arg7 = InitValueTraits<typename BaseTypePtrTraits<ARG7>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg6);
        LuaOp<typename BaseTypePtrTraits<ARG7>::ArgType>::LuaToValue(ls, LUA_ARG_POS(8), arg7);

        void* user_data_ptr = lua_newuserdata(ls, sizeof(UserDataForObject<CLASS_TYPE>));

        luaL_getmetatable(ls, LuaTypeInfo<CLASS_TYPE>::GetName());
        lua_setmetatable(ls, -2);

        new(user_data_ptr) UserDataForObject<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2, arg3, arg4, arg5,
                                                        arg6, arg7));
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct LuaNewTraits<CLASS_TYPE, void(ARG1, ARG2, ARG3,
                                               ARG4, ARG5, ARG6, ARG7, ARG8)>
{
    static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG7>::ArgType arg7 = InitValueTraits<typename BaseTypePtrTraits<ARG7>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG8>::ArgType arg8 = InitValueTraits<typename BaseTypePtrTraits<ARG8>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg6);
        LuaOp<typename BaseTypePtrTraits<ARG7>::ArgType>::LuaToValue(ls, LUA_ARG_POS(8), arg7);
        LuaOp<typename BaseTypePtrTraits<ARG8>::ArgType>::LuaToValue(ls, LUA_ARG_POS(9), arg8);

        void* user_data_ptr = lua_newuserdata(ls, sizeof(UserDataForObject<CLASS_TYPE>));
        luaL_getmetatable(ls, LuaTypeInfo<CLASS_TYPE>::GetName());
        lua_setmetatable(ls, -2);

        new(user_data_ptr) UserDataForObject<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2, arg3, arg4, arg5, arg6,
                                                        arg7, arg8));
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8,
          typename ARG9>
struct LuaNewTraits<CLASS_TYPE, void(ARG1, ARG2, ARG3,
                                               ARG4, ARG5, ARG6, ARG7, ARG8, ARG9)>
{
    static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG7>::ArgType arg7 = InitValueTraits<typename BaseTypePtrTraits<ARG7>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG8>::ArgType arg8 = InitValueTraits<typename BaseTypePtrTraits<ARG8>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG9>::ArgType arg9 = InitValueTraits<typename BaseTypePtrTraits<ARG9>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg6);
        LuaOp<typename BaseTypePtrTraits<ARG7>::ArgType>::LuaToValue(ls, LUA_ARG_POS(8), arg7);
        LuaOp<typename BaseTypePtrTraits<ARG8>::ArgType>::LuaToValue(ls, LUA_ARG_POS(9), arg8);
        LuaOp<typename BaseTypePtrTraits<ARG9>::ArgType>::LuaToValue(ls, LUA_ARG_POS(10), arg9);

        void* user_data_ptr = lua_newuserdata(ls, sizeof(UserDataForObject<CLASS_TYPE>));
        luaL_getmetatable(ls, LuaTypeInfo<CLASS_TYPE>::GetName());
        lua_setmetatable(ls, -2);

        new(user_data_ptr) UserDataForObject<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2, arg3, arg4, arg5, arg6,
                                                        arg7, arg8, arg9));

        return 1;
    }
};

#pragma endregion // LuaNewTraits

#pragma region // LuaClassFunctionTraits

template <typename FUNC_CLASS_TYPE>
struct LuaClassFunctionTraits<void (FUNC_CLASS_TYPE::*)()>
{
    typedef void (FUNC_CLASS_TYPE::*DestFunc)();
    typedef UserDataForFunction<DestFunc> UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
    	void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo &registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));

        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);
        (obj_ptr->*(registed_data._realFunc))();
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1>
struct LuaClassFunctionTraits<void (FUNC_CLASS_TYPE::*)(ARG1)>
{
    typedef void (FUNC_CLASS_TYPE::*DestFunc)(ARG1);
    typedef UserDataForFunction<DestFunc> UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);

        (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1));
        return 0;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2>
struct LuaClassFunctionTraits<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2)>
{
    typedef void (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2);
    typedef UserDataForFunction<DestFunc> UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
    	UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);

        (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3>
struct LuaClassFunctionTraits<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3)>
{
    typedef void (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3);
    typedef UserDataForFunction<DestFunc> UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
    	UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
    	FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct LuaClassFunctionTraits<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4)>
{
    typedef void (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4);
    typedef UserDataForFunction<DestFunc> UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
    	UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
    	FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);

        (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5>
struct LuaClassFunctionTraits<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5)>
{
    typedef void (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5);
	typedef UserDataForFunction<DestFunc> UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
    	UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
    	FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);

        (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4), LuaPt<ARG5>::r(arg5));
        return 0;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6>
struct LuaClassFunctionTraits<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
                                                              ARG6)>
{
    typedef void (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    typedef UserDataForFunction<DestFunc> UserDataForFunctionInfo;

    static int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata(ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));

        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg6);

        (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4), LuaPt<ARG5>::r(arg5), LuaPt<ARG6>::r(arg6));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6, typename ARG7>
struct LuaClassFunctionTraits<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
                                                              ARG6, ARG7)>
{
    typedef void (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    typedef UserDataForFunction<DestFunc> UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));

        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG7>::ArgType arg7 = InitValueTraits<typename BaseTypePtrTraits<ARG7>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg6);
        LuaOp<typename BaseTypePtrTraits<ARG7>::ArgType>::LuaToValue(ls, LUA_ARG_POS(8), arg7);

        (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4), LuaPt<ARG5>::r(arg5), LuaPt<ARG6>::r(arg6), LuaPt<ARG7>::r(arg7));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct LuaClassFunctionTraits<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
                                                                   ARG6, ARG7, ARG8)>
{
    typedef void (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8);
    typedef UserDataForFunction<DestFunc> UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG7>::ArgType arg7 = InitValueTraits<typename BaseTypePtrTraits<ARG7>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG8>::ArgType arg8 = InitValueTraits<typename BaseTypePtrTraits<ARG8>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg6);
        LuaOp<typename BaseTypePtrTraits<ARG7>::ArgType>::LuaToValue(ls, LUA_ARG_POS(8), arg7);
        LuaOp<typename BaseTypePtrTraits<ARG8>::ArgType>::LuaToValue(ls, LUA_ARG_POS(9), arg8);

        (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4), LuaPt<ARG5>::r(arg5), LuaPt<ARG6>::r(arg6), LuaPt<ARG7>::r(arg7), LuaPt<ARG8>::r(arg8));
        return 0;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct LuaClassFunctionTraits<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
                                                              ARG6, ARG7, ARG8, ARG9)>
{
    typedef void (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
    typedef UserDataForFunction<DestFunc> UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);
        
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG7>::ArgType arg7 = InitValueTraits<typename BaseTypePtrTraits<ARG7>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG8>::ArgType arg8 = InitValueTraits<typename BaseTypePtrTraits<ARG8>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG9>::ArgType arg9 = InitValueTraits<typename BaseTypePtrTraits<ARG9>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg6);
        LuaOp<typename BaseTypePtrTraits<ARG7>::ArgType>::LuaToValue(ls, LUA_ARG_POS(8), arg7);
        LuaOp<typename BaseTypePtrTraits<ARG8>::ArgType>::LuaToValue(ls, LUA_ARG_POS(9), arg8);
        LuaOp<typename BaseTypePtrTraits<ARG9>::ArgType>::LuaToValue(ls, LUA_ARG_POS(10), arg9);

        (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4), LuaPt<ARG5>::r(arg5), LuaPt<ARG6>::r(arg6), LuaPt<ARG7>::r(arg7), LuaPt<ARG8>::r(arg8), LuaPt<ARG9>::r(arg9));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE>
struct LuaClassFunctionTraits<void (FUNC_CLASS_TYPE::*)() const>
{
    typedef void (FUNC_CLASS_TYPE::*DestFunc)() const;
    typedef UserDataForFunction<DestFunc> UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
    	void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);
        
        (obj_ptr->*(registed_data._realFunc))();
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1>
struct LuaClassFunctionTraits<void (FUNC_CLASS_TYPE::*)(ARG1) const>
{
    typedef void (FUNC_CLASS_TYPE::*dest_func_t)(ARG1) const;
    typedef UserDataForFunction<dest_func_t> UserDataForFunctionInfo;

    static int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata(ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);

        (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1)); 
        return 0;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2>
struct LuaClassFunctionTraits<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2) const>
{
    typedef void (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2) const;
    typedef UserDataForFunction<DestFunc> UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);

        (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3>
struct LuaClassFunctionTraits<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3) const>
{
    typedef void (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3) const;
    typedef UserDataForFunction<DestFunc> UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        
        (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct LuaClassFunctionTraits<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4) const>
{
    typedef void (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4) const;
    typedef UserDataForFunction<DestFunc>        UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);

        (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5>
struct LuaClassFunctionTraits<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5) const>
{
    typedef void (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5) const;
    typedef UserDataForFunction<DestFunc> UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);

        (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4), LuaPt<ARG5>::r(arg5));
        return 0;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6>
struct LuaClassFunctionTraits<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
                                                              ARG6) const>
{
    typedef void (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) const;
    typedef UserDataForFunction<DestFunc>        UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg6);

        (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4), LuaPt<ARG5>::r(arg5), LuaPt<ARG6>::r(arg6));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6, typename ARG7>
struct LuaClassFunctionTraits<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
                                                              ARG6, ARG7) const>
{
    typedef void (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) const;
    typedef UserDataForFunction<DestFunc> UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata(ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG7>::ArgType arg7 = InitValueTraits<typename BaseTypePtrTraits<ARG7>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg6);
        LuaOp<typename BaseTypePtrTraits<ARG7>::ArgType>::LuaToValue(ls, LUA_ARG_POS(8), arg7);

        (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4), LuaPt<ARG5>::r(arg5), LuaPt<ARG6>::r(arg6), LuaPt<ARG7>::r(arg7));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct LuaClassFunctionTraits<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
                                                                   ARG6, ARG7, ARG8) const>
{
    typedef void (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8) const;
    typedef UserDataForFunction<DestFunc> UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG7>::ArgType arg7 = InitValueTraits<typename BaseTypePtrTraits<ARG7>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG8>::ArgType arg8 = InitValueTraits<typename BaseTypePtrTraits<ARG8>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg6);
        LuaOp<typename BaseTypePtrTraits<ARG7>::ArgType>::LuaToValue(ls, LUA_ARG_POS(8), arg7);
        LuaOp<typename BaseTypePtrTraits<ARG8>::ArgType>::LuaToValue(ls, LUA_ARG_POS(9), arg8);

        (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4), LuaPt<ARG5>::r(arg5), LuaPt<ARG6>::r(arg6), LuaPt<ARG7>::r(arg7), LuaPt<ARG8>::r(arg8));
        return 0;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct LuaClassFunctionTraits<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
                                                              ARG6, ARG7, ARG8, ARG9) const>
{
    typedef void (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9) const;
    typedef UserDataForFunction<DestFunc>        UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG7>::ArgType arg7 = InitValueTraits<typename BaseTypePtrTraits<ARG7>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG8>::ArgType arg8 = InitValueTraits<typename BaseTypePtrTraits<ARG8>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG9>::ArgType arg9 = InitValueTraits<typename BaseTypePtrTraits<ARG9>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg6);
        LuaOp<typename BaseTypePtrTraits<ARG7>::ArgType>::LuaToValue(ls, LUA_ARG_POS(8), arg7);
        LuaOp<typename BaseTypePtrTraits<ARG8>::ArgType>::LuaToValue(ls, LUA_ARG_POS(9), arg8);
        LuaOp<typename BaseTypePtrTraits<ARG9>::ArgType>::LuaToValue(ls, LUA_ARG_POS(10), arg9);

        (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4), LuaPt<ARG5>::r(arg5), LuaPt<ARG6>::r(arg6), LuaPt<ARG7>::r(arg7), LuaPt<ARG8>::r(arg8), LuaPt<ARG9>::r(arg9));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename RET>
struct LuaClassFunctionTraits<RET (FUNC_CLASS_TYPE::*)()>
{
    typedef RET (FUNC_CLASS_TYPE::*DestFunc)();
    typedef UserDataForFunction<DestFunc> UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);
        
        RET ret = (obj_ptr->*(registed_data._realFunc))();
        
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename RET>
struct LuaClassFunctionTraits<RET (FUNC_CLASS_TYPE::*)(ARG1)>
{
    typedef RET (FUNC_CLASS_TYPE::*DestFunc)(ARG1);
    typedef UserDataForFunction<DestFunc>        UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);
        
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);

        RET ret = (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1));
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2,
          typename RET>
struct LuaClassFunctionTraits<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2)>
{
    typedef RET (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2);
    typedef UserDataForFunction<DestFunc>        UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);
        
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);

        RET ret = (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2));
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename RET>
struct LuaClassFunctionTraits<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3)>
{
    typedef RET (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3);
    typedef UserDataForFunction<DestFunc>        UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata(ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);
        
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);

        RET ret = (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2), LuaPt<ARG3>::r(arg3));
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename RET>
struct LuaClassFunctionTraits<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4)>
{
    typedef RET (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4);
    typedef UserDataForFunction<DestFunc>        UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
         UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
         FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);
         
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);

        RET ret = (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4));
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename RET>
struct LuaClassFunctionTraits<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5)>
{
    typedef RET (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5);
    typedef UserDataForFunction<DestFunc>        UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);
        
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);

        RET ret = (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4), LuaPt<ARG5>::r(arg5));
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6, typename RET>
struct LuaClassFunctionTraits<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
                                                              ARG6)>
{
    typedef RET (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    typedef UserDataForFunction<DestFunc>        UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);
        
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg6);

        RET ret = (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4), LuaPt<ARG5>::r(arg5), LuaPt<ARG6>::r(arg6));
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6, typename ARG7, typename RET>
struct LuaClassFunctionTraits<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
                                                              ARG6, ARG7)>
{
    typedef RET (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    typedef UserDataForFunction<DestFunc>        UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);
        
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG7>::ArgType arg7 = InitValueTraits<typename BaseTypePtrTraits<ARG7>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg6);
        LuaOp<typename BaseTypePtrTraits<ARG7>::ArgType>::LuaToValue(ls, LUA_ARG_POS(8), arg7);

        RET ret = (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4), LuaPt<ARG5>::r(arg5), LuaPt<ARG6>::r(arg6), LuaPt<ARG7>::r(arg7));
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename RET>
struct LuaClassFunctionTraits<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
                                                                   ARG6, ARG7, ARG8)>
{
    typedef RET (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8);
    typedef UserDataForFunction<DestFunc>        UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);
        
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG7>::ArgType arg7 = InitValueTraits<typename BaseTypePtrTraits<ARG7>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG8>::ArgType arg8 = InitValueTraits<typename BaseTypePtrTraits<ARG8>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg6);
        LuaOp<typename BaseTypePtrTraits<ARG7>::ArgType>::LuaToValue(ls, LUA_ARG_POS(8), arg7);
        LuaOp<typename BaseTypePtrTraits<ARG8>::ArgType>::LuaToValue(ls, LUA_ARG_POS(9), arg8);

        RET ret = (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4), LuaPt<ARG5>::r(arg5), LuaPt<ARG6>::r(arg6), LuaPt<ARG7>::r(arg7), LuaPt<ARG8>::r(arg8));
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9, typename RET>
struct LuaClassFunctionTraits<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
                                                              ARG6, ARG7, ARG8, ARG9)>
{
    typedef RET (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
    typedef UserDataForFunction<DestFunc>        UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);
        
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG7>::ArgType arg7 = InitValueTraits<typename BaseTypePtrTraits<ARG7>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG8>::ArgType arg8 = InitValueTraits<typename BaseTypePtrTraits<ARG8>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG9>::ArgType arg9 = InitValueTraits<typename BaseTypePtrTraits<ARG9>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg6);
        LuaOp<typename BaseTypePtrTraits<ARG7>::ArgType>::LuaToValue(ls, LUA_ARG_POS(8), arg7);
        LuaOp<typename BaseTypePtrTraits<ARG8>::ArgType>::LuaToValue(ls, LUA_ARG_POS(9), arg8);
        LuaOp<typename BaseTypePtrTraits<ARG9>::ArgType>::LuaToValue(ls, LUA_ARG_POS(10), arg9);

        RET ret = (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4), LuaPt<ARG5>::r(arg5), LuaPt<ARG6>::r(arg6), LuaPt<ARG7>::r(arg7), LuaPt<ARG8>::r(arg8), LuaPt<ARG9>::r(arg9));
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};


template <typename FUNC_CLASS_TYPE, typename RET>
struct LuaClassFunctionTraits<RET (FUNC_CLASS_TYPE::*)() const>
{
    typedef RET (FUNC_CLASS_TYPE::*DestFunc)() const;
    typedef UserDataForFunction<DestFunc>        UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);
        
        RET ret = (obj_ptr->*(registed_data._realFunc))();
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename RET>
struct LuaClassFunctionTraits<RET (FUNC_CLASS_TYPE::*)(ARG1) const>
{
    typedef RET (FUNC_CLASS_TYPE::*DestFunc)(ARG1) const;
    typedef UserDataForFunction<DestFunc>        UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);

        RET ret = (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1));
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2,
          typename RET>
struct LuaClassFunctionTraits<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2) const>
{
    typedef RET (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2) const;
    typedef UserDataForFunction<DestFunc>        UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);

        RET ret = (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2));
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3,
          typename RET>
struct LuaClassFunctionTraits<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3) const>
{
    typedef RET (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3) const;
    typedef UserDataForFunction<DestFunc>        UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);
        
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);

        RET ret = (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3));
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename RET>
struct LuaClassFunctionTraits<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4) const>
{
    typedef RET (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4) const;
    typedef UserDataForFunction<DestFunc>        UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);

        RET ret = (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4));
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename RET>
struct LuaClassFunctionTraits<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5) const>
{
    typedef RET (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5) const;
    typedef UserDataForFunction<DestFunc>        UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);

        RET ret = (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4), LuaPt<ARG5>::r(arg5));
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6, typename RET>
struct LuaClassFunctionTraits<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
                                                              ARG6) const>
{
    typedef RET (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) const;
    typedef UserDataForFunction<dest_func_t>        UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg6);

        RET ret = (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4), LuaPt<ARG5>::r(arg5), LuaPt<ARG6>::r(arg6));
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6, typename ARG7, typename RET>
struct LuaClassFunctionTraits<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
                                                              ARG6, ARG7) const>
{
    typedef RET (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) const;
    typedef UserDataForFunction<DestFunc>        UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG7>::ArgType arg7 = InitValueTraits<typename BaseTypePtrTraits<ARG7>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg6);
        LuaOp<typename BaseTypePtrTraits<ARG7>::ArgType>::LuaToValue(ls, LUA_ARG_POS(8), arg7);

        RET ret = (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4), LuaPt<ARG5>::r(arg5), LuaPt<ARG6>::r(arg6), LuaPt<ARG7>::r(arg7));
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename RET>
struct LuaClassFunctionTraits<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
                                                                   ARG6, ARG7, ARG8) const>
{
    typedef RET (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8) const;
    typedef UserDataForFunction<DestFunc>        UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG7>::ArgType arg7 = InitValueTraits<typename BaseTypePtrTraits<ARG7>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG8>::ArgType arg8 = InitValueTraits<typename BaseTypePtrTraits<ARG8>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg6);
        LuaOp<typename BaseTypePtrTraits<ARG7>::ArgType>::LuaToValue(ls, LUA_ARG_POS(8), arg7);
        LuaOp<typename BaseTypePtrTraits<ARG8>::ArgType>::LuaToValue(ls, LUA_ARG_POS(9), arg8);

        RET ret = (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4), LuaPt<ARG5>::r(arg5), LuaPt<ARG6>::r(arg6), LuaPt<ARG7>::r(arg7), LuaPt<ARG8>::r(arg8));
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};


template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9, typename RET>
struct LuaClassFunctionTraits<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5,
                                                              ARG6, ARG7, ARG8, ARG9) const>
{
    typedef RET (FUNC_CLASS_TYPE::*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9) const;
    typedef UserDataForFunction<DestFunc>        UserDataForFunctionInfo;

    static  int LuaFunction(lua_State* ls)
    {
        void* dest_data = lua_touserdata (ls, lua_upvalueindex(1));
        UserDataForFunctionInfo& registed_data = *(KernelCastTo<UserDataForFunctionInfo>(dest_data));
        FUNC_CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<FUNC_CLASS_TYPE>::UserDataToObject(ls);

        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG7>::ArgType arg7 = InitValueTraits<typename BaseTypePtrTraits<ARG7>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG8>::ArgType arg8 = InitValueTraits<typename BaseTypePtrTraits<ARG8>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG9>::ArgType arg9 = InitValueTraits<typename BaseTypePtrTraits<ARG9>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg6);
        LuaOp<typename BaseTypePtrTraits<ARG7>::ArgType>::LuaToValue(ls, LUA_ARG_POS(8), arg7);
        LuaOp<typename BaseTypePtrTraits<ARG8>::ArgType>::LuaToValue(ls, LUA_ARG_POS(9), arg8);
        LuaOp<typename BaseTypePtrTraits<ARG9>::ArgType>::LuaToValue(ls, LUA_ARG_POS(10), arg9);

        RET ret = (obj_ptr->*(registed_data._realFunc))(LuaPt<ARG1>::r(arg1), LuaPt<ARG2>::r(arg2),  LuaPt<ARG3>::r(arg3), LuaPt<ARG4>::r(arg4), LuaPt<ARG5>::r(arg5), LuaPt<ARG6>::r(arg6), LuaPt<ARG7>::r(arg7), LuaPt<ARG8>::r(arg8), LuaPt<ARG9>::r(arg9));
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};

#pragma endregion // LuaClassFunctionTraits

template <typename CLASS_TYPE, typename RET>
struct LuaClassPropertyTraits
{
	typedef int (*ProcessIndexFunc)(lua_State*, void*, const char*);
	typedef int (*ProcessNewindexFunc)(lua_State*, void*, const char*, int);

	typedef RET Property;
	typedef RET CLASS_TYPE::* PropertyPtr;
	using ThisType = LuaClassPropertyTraits<CLASS_TYPE, RET>;
	
	static int ProcessIndex(lua_State* ls, void* fieldInfo, const char* key)
	{
		typedef LuaClassPropertyInfo<PropertyPtr> ClassPropertyInfo;
		CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<CLASS_TYPE>::UserDataToObject(ls);

		ClassPropertyInfo* reg = KERNEL_NS::KernelCastTo<ClassPropertyInfo>(fieldInfo);
		PropertyPtr ptr = reg->PropertyPos;

		if (ptr)
		{
			LuaOp<Property>::PushStack(ls, (obj_ptr->*ptr));
			return 1;
		}
		else
		{
        	LogTool::Warn(LOGFMT_NON_OBJ_TAG(ThisType, "none this field:%s"), key);
			return 0;
		}

		return 0;
	}

	static int ProcessNewindex(lua_State* ls, void* fieldInfo, const char* key, int valueIndex)
	{
		typedef LuaClassPropertyInfo<PropertyPtr> ClassPropertyInfo;
		CLASS_TYPE* obj_ptr = MetaTableRegisterImpl<CLASS_TYPE>::UserDataToObject(ls);

		ClassPropertyInfo* reg = KERNEL_NS::KernelCastTo<ClassPropertyInfo>(fieldInfo);
		PropertyPtr ptr = reg->PropertyPos;

		if (ptr)
		{
			Property  v = InitValueTraits<Property>::Value();
			LuaOp<Property>::LuaToValue(ls, valueIndex, v);
			(obj_ptr->*ptr) = v;
			return 0;
		}
		else
		{
        	LogTool::Warn(LOGFMT_NON_OBJ_TAG(ThisType, "none this field:%s"), key);
			return 0;
		}

		return 0;
	}
};

#pragma region // LuaFunctionTraits

template<>
struct LuaFunctionTraits<void(*)()>
{
    typedef void (*DestFunc)();
    static  int LuaFunction(lua_State* ls)
    {
        void* user_data = lua_touserdata (ls, lua_upvalueindex(1));

        DestFunc& registed_func = *(KernelCastTo<DestFunc>(user_data));
        registed_func();
        return 0;
    }
};

template <typename ARG1>
struct LuaFunctionTraits<void(*)(ARG1)>
{
    typedef void (*DestFunc)(ARG1);
    static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(1), arg1);

        void* user_data = lua_touserdata (ls, lua_upvalueindex(1));
        DestFunc& registed_func = *(KernelCastTo<DestFunc>(user_data));

        registed_func(arg1);
        return 0;
    }
};

template <typename ARG1, typename ARG2>
struct LuaFunctionTraits<void(*)(ARG1, ARG2)>
{
    typedef void (*DestFunc)(ARG1, ARG2);
    static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(1), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg2);

        void* user_data = lua_touserdata (ls, lua_upvalueindex(1));
    	DestFunc& registed_func = *(KernelCastTo<DestFunc>(user_data));

        registed_func(arg1, arg2);
        return 0;
    }
};

template <typename ARG1, typename ARG2, typename ARG3>
struct LuaFunctionTraits<void(*)(ARG1, ARG2, ARG3)>
{
    typedef void (*DestFunc)(ARG1, ARG2, ARG3);
    static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(1), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg3);

        void* user_data = lua_touserdata (ls, lua_upvalueindex(1));
    	DestFunc& registed_func = *(KernelCastTo<DestFunc>(user_data));

        registed_func(arg1, arg2, arg3);
        return 0;
    }
};

template <typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct LuaFunctionTraits<void (*)(ARG1, ARG2, ARG3, ARG4)>
{
    typedef void (*DestFunc)(ARG1, ARG2, ARG3, ARG4);
    static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(1), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg4);

        void* user_data = lua_touserdata (ls, lua_upvalueindex(1));
    	DestFunc& registed_func = *(KernelCastTo<DestFunc>(user_data));

        registed_func(arg1, arg2, arg3, arg4);
        return 0;
    }
};

template <typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5>
struct LuaFunctionTraits<void (*)(ARG1, ARG2, ARG3, ARG4, ARG5)>
{
    typedef void (*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5);
    static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(1), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg5);

        void* user_data = lua_touserdata (ls, lua_upvalueindex(1));
    	DestFunc& registed_func = *(KernelCastTo<DestFunc>(user_data));

        registed_func(arg1, arg2, arg3, arg4, arg5);
        return 0;
    }
};
template <typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6>
struct LuaFunctionTraits<void (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6)>
{
    typedef void (*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(1), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg6);

        void* user_data = lua_touserdata (ls, lua_upvalueindex(1));
    	DestFunc& registed_func = *(KernelCastTo<DestFunc>(user_data));

        registed_func(arg1, arg2, arg3, arg4, arg5, arg6);
        return 0;
    }
};

template <typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6, typename ARG7>
struct LuaFunctionTraits<void (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)>
{
    typedef void (*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG7>::ArgType arg7 = InitValueTraits<typename BaseTypePtrTraits<ARG7>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(1), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg6);
        LuaOp<typename BaseTypePtrTraits<ARG7>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg7);

        void* user_data = lua_touserdata (ls, lua_upvalueindex(1));
    	DestFunc& registed_func = *(KernelCastTo<DestFunc>(user_data));

        registed_func(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
        return 0;
    }
};

template <typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct LuaFunctionTraits<void (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8)>
{
    typedef void (*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8);
    static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG7>::ArgType arg7 = InitValueTraits<typename BaseTypePtrTraits<ARG7>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG8>::ArgType arg8 = InitValueTraits<typename BaseTypePtrTraits<ARG8>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(1), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg6);
        LuaOp<typename BaseTypePtrTraits<ARG7>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg7);
        LuaOp<typename BaseTypePtrTraits<ARG8>::ArgType>::LuaToValue(ls, LUA_ARG_POS(8), arg8);

        void* user_data = lua_touserdata (ls, lua_upvalueindex(1));
    	DestFunc& registed_func = *(KernelCastTo<DestFunc>(user_data));

        registed_func(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
        return 0;
    }
};


template <typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct LuaFunctionTraits<void (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9)>
{
    typedef void (*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
    static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG7>::ArgType arg7 = InitValueTraits<typename BaseTypePtrTraits<ARG7>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG8>::ArgType arg8 = InitValueTraits<typename BaseTypePtrTraits<ARG8>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG9>::ArgType arg9 = InitValueTraits<typename BaseTypePtrTraits<ARG9>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(1), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg6);
        LuaOp<typename BaseTypePtrTraits<ARG7>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg7);
        LuaOp<typename BaseTypePtrTraits<ARG8>::ArgType>::LuaToValue(ls, LUA_ARG_POS(8), arg8);
        LuaOp<typename BaseTypePtrTraits<ARG9>::ArgType>::LuaToValue(ls, LUA_ARG_POS(9), arg9);

        void* user_data = lua_touserdata (ls, lua_upvalueindex(1));
    	DestFunc& registed_func = *(KernelCastTo<DestFunc>(user_data));

        registed_func(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
        return 0;
    }
};


template <typename RET>
struct LuaFunctionTraits<RET (*)()>
{
    typedef RET (*DestFunc)();
    static  int LuaFunction(lua_State* ls)
    {
        void* user_data = lua_touserdata (ls, lua_upvalueindex(1));
    	DestFunc& registed_func = *(KernelCastTo<DestFunc>(user_data));

        RET ret = registed_func();
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);

        return 1;
    }
};

template <typename RET, typename ARG1>
struct LuaFunctionTraits<RET (*)(ARG1)>
{
    typedef RET (*DestFunc)(ARG1);
    static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();

    	LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(1), arg1);

        void* user_data = lua_touserdata (ls, lua_upvalueindex(1));
    	DestFunc& registed_func = *(KernelCastTo<DestFunc>(user_data));

        RET ret = registed_func(arg1);
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};

template <typename RET, typename ARG1, typename ARG2>
struct LuaFunctionTraits<RET (*)(ARG1, ARG2)>
{
    typedef RET (*DestFunc)(ARG1, ARG2);
    static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(1), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg2);

        void* user_data = lua_touserdata (ls, lua_upvalueindex(1));
    	DestFunc& registed_func = *(KernelCastTo<DestFunc>(user_data));

        RET ret = registed_func(arg1, arg2);
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3>
struct LuaFunctionTraits<RET (*)(ARG1, ARG2, ARG3)>
{
    typedef RET (*DestFunc)(ARG1, ARG2, ARG3);
    static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(1), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg3);

        void* user_data = lua_touserdata (ls, lua_upvalueindex(1));
    	DestFunc& registed_func = *(KernelCastTo<DestFunc>(user_data));

        RET ret = registed_func(arg1, arg2, arg3);
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct LuaFunctionTraits<RET (*)(ARG1, ARG2, ARG3, ARG4)>
{
    typedef RET (*DestFunc)(ARG1, ARG2, ARG3, ARG4);
    static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(1), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg4);

        void* user_data = lua_touserdata (ls, lua_upvalueindex(1));
    	DestFunc& registed_func = *(KernelCastTo<DestFunc>(user_data));

        RET ret = registed_func(arg1, arg2, arg3, arg4);
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5>
struct LuaFunctionTraits<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5)>
{
    typedef RET (*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5);
	static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(1), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg5);

        void* user_data = lua_touserdata (ls, lua_upvalueindex(1));
		DestFunc& registed_func = *(KernelCastTo<DestFunc>(user_data));

        RET ret = registed_func(arg1, arg2, arg3, arg4, arg5);
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};


template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6>
struct LuaFunctionTraits<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6)>
{
    typedef RET (*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
	static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(1), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg6);

        void* user_data = lua_touserdata (ls, lua_upvalueindex(1));
		DestFunc& registed_func = *(KernelCastTo<DestFunc>(user_data));

        RET ret = registed_func(arg1, arg2, arg3, arg4, arg5, arg6);
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6, typename ARG7>
struct LuaFunctionTraits<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)>
{
    typedef RET (*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
	static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG7>::ArgType arg7 = InitValueTraits<typename BaseTypePtrTraits<ARG7>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(1), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg6);
        LuaOp<typename BaseTypePtrTraits<ARG7>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg7);

        void* user_data = lua_touserdata (ls, lua_upvalueindex(1));
		DestFunc& registed_func = *(KernelCastTo<DestFunc>(user_data));

        RET ret = registed_func(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct LuaFunctionTraits<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8)>
{
    typedef RET (*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8);
	static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG7>::ArgType arg7 = InitValueTraits<typename BaseTypePtrTraits<ARG7>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG8>::ArgType arg8 = InitValueTraits<typename BaseTypePtrTraits<ARG8>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(1), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg6);
        LuaOp<typename BaseTypePtrTraits<ARG7>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg7);
        LuaOp<typename BaseTypePtrTraits<ARG8>::ArgType>::LuaToValue(ls, LUA_ARG_POS(8), arg8);

        void* user_data = lua_touserdata (ls, lua_upvalueindex(1));
		DestFunc& registed_func = *(KernelCastTo<DestFunc>(user_data));

        RET ret = registed_func(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4,
          typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct LuaFunctionTraits<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9)>
{
    typedef RET (*DestFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
	static  int LuaFunction(lua_State* ls)
    {
        typename BaseTypePtrTraits<ARG1>::ArgType arg1 = InitValueTraits<typename BaseTypePtrTraits<ARG1>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG2>::ArgType arg2 = InitValueTraits<typename BaseTypePtrTraits<ARG2>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG3>::ArgType arg3 = InitValueTraits<typename BaseTypePtrTraits<ARG3>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG4>::ArgType arg4 = InitValueTraits<typename BaseTypePtrTraits<ARG4>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG5>::ArgType arg5 = InitValueTraits<typename BaseTypePtrTraits<ARG5>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG6>::ArgType arg6 = InitValueTraits<typename BaseTypePtrTraits<ARG6>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG7>::ArgType arg7 = InitValueTraits<typename BaseTypePtrTraits<ARG7>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG8>::ArgType arg8 = InitValueTraits<typename BaseTypePtrTraits<ARG8>::ArgType>::Value();
        typename BaseTypePtrTraits<ARG9>::ArgType arg9 = InitValueTraits<typename BaseTypePtrTraits<ARG9>::ArgType>::Value();

        LuaOp<typename BaseTypePtrTraits<ARG1>::ArgType>::LuaToValue(ls, LUA_ARG_POS(1), arg1);
        LuaOp<typename BaseTypePtrTraits<ARG2>::ArgType>::LuaToValue(ls, LUA_ARG_POS(2), arg2);
        LuaOp<typename BaseTypePtrTraits<ARG3>::ArgType>::LuaToValue(ls, LUA_ARG_POS(3), arg3);
        LuaOp<typename BaseTypePtrTraits<ARG4>::ArgType>::LuaToValue(ls, LUA_ARG_POS(4), arg4);
        LuaOp<typename BaseTypePtrTraits<ARG5>::ArgType>::LuaToValue(ls, LUA_ARG_POS(5), arg5);
        LuaOp<typename BaseTypePtrTraits<ARG6>::ArgType>::LuaToValue(ls, LUA_ARG_POS(6), arg6);
        LuaOp<typename BaseTypePtrTraits<ARG7>::ArgType>::LuaToValue(ls, LUA_ARG_POS(7), arg7);
        LuaOp<typename BaseTypePtrTraits<ARG8>::ArgType>::LuaToValue(ls, LUA_ARG_POS(8), arg8);
        LuaOp<typename BaseTypePtrTraits<ARG9>::ArgType>::LuaToValue(ls, LUA_ARG_POS(9), arg9);

        void* user_data = lua_touserdata (ls, lua_upvalueindex(1));
		DestFunc& registed_func = *(KernelCastTo<DestFunc>(user_data));

        RET ret = registed_func(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
        LuaOp<typename BaseTypePtrTraits<RET>::ArgType>::PushStack(ls, ret);
        return 1;
    }
};

#pragma endregion // LuaFunctionTraits

#pragma region // LuaRegisterRouter

template<typename RET>
struct LuaRegisterRouter<RET (*)()>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (*arg)(), const std::string& s)
	{
		reg->DefFunc(arg, s);
	}
};
template<typename RET, typename ARG1>
struct LuaRegisterRouter<RET (*)(ARG1)>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (*arg)(ARG1), const std::string& s)
	{
		reg->DefFunc(arg, s);
	}
};
template<typename RET, typename ARG1, typename ARG2>
struct LuaRegisterRouter<RET (*)(ARG1, ARG2)>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (*arg)(ARG1, ARG2), const std::string& s)
	{
		reg->DefFunc(arg, s);
	}
};
template<typename RET, typename ARG1, typename ARG2, typename ARG3>
struct LuaRegisterRouter<RET (*)(ARG1, ARG2, ARG3)>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (*arg)(ARG1, ARG2, ARG3), const std::string& s)
	{
		reg->DefFunc(arg, s);
	}
};
template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct LuaRegisterRouter<RET (*)(ARG1, ARG2, ARG3, ARG4)>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (*arg)(ARG1, ARG2, ARG3, ARG4), const std::string& s)
	{
		reg->DefFunc(arg, s);
	}
};
template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
struct LuaRegisterRouter<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5)>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (*arg)(ARG1, ARG2, ARG3, ARG4, ARG5), const std::string& s)
	{
		reg->DefFunc(arg, s);
	}
};
template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
struct LuaRegisterRouter<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6)>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (*arg)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6), const std::string& s)
	{
		reg->DefFunc(arg, s);
	}
};
template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
struct LuaRegisterRouter<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (*arg)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7), const std::string& s)
	{
		reg->DefFunc(arg, s);
	}
};

template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct LuaRegisterRouter<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8)>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (*arg)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8), const std::string& s)
	{
		reg->DefFunc(arg, s);
	}
};

template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct LuaRegisterRouter<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9)>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (*arg)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9), const std::string& s)
	{
		reg->DefFunc(arg, s);
	}
};

template<typename RET, typename CLASS_TYPE>
struct LuaRegisterRouter<RET (CLASS_TYPE::*)()>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (CLASS_TYPE::*arg)() , const std::string& s)
	{
		reg->DefClassFunc(arg, s);
	}
};

template<typename RET, typename CLASS_TYPE, typename ARG1>
struct LuaRegisterRouter<RET (CLASS_TYPE::*)(ARG1)>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (CLASS_TYPE::*arg)(ARG1), const std::string& s)
	{
		reg->DefClassFunc(arg, s);
	}
};
template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2>
struct LuaRegisterRouter<RET (CLASS_TYPE::*)(ARG1, ARG2)>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (CLASS_TYPE::*arg)(ARG1, ARG2), const std::string& s)
	{
		reg->DefClassFunc(arg, s);
	}
};
template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3>
struct LuaRegisterRouter<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3)>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (CLASS_TYPE::*arg)(ARG1, ARG2, ARG3), const std::string& s)
	{
		reg->DefClassFunc(arg, s);
	}
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct LuaRegisterRouter<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4)>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (CLASS_TYPE::*arg)(ARG1, ARG2, ARG3, ARG4), const std::string& s)
	{
		reg->DefClassFunc(arg, s);
	}
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
struct LuaRegisterRouter<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5)>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (CLASS_TYPE::*arg)(ARG1, ARG2, ARG3, ARG4, ARG5), const std::string& s)
	{
		reg->DefClassFunc(arg, s);
	}
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
struct LuaRegisterRouter<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6)>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (CLASS_TYPE::*arg)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6), const std::string& s)
	{
		reg->DefClassFunc(arg, s);
	}
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
struct LuaRegisterRouter<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (CLASS_TYPE::*arg)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7), const std::string& s)
	{
		reg->DefClassFunc(arg, s);
	}
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct LuaRegisterRouter<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8)>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (CLASS_TYPE::*arg)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8), const std::string& s)
	{
		reg->DefClassFunc(arg, s);
	}
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct LuaRegisterRouter<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9)>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (CLASS_TYPE::*arg)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9), const std::string& s)
	{
		reg->DefClassFunc(arg, s);
	}
};

template<typename RET, typename CLASS_TYPE>
struct LuaRegisterRouter<RET (CLASS_TYPE::*)() const>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (CLASS_TYPE::*arg)() const, const std::string& s)
	{
		reg->DefClassFunc(arg, s);
	}
};

template<typename RET, typename CLASS_TYPE, typename ARG1>
struct LuaRegisterRouter<RET (CLASS_TYPE::*)(ARG1) const>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (CLASS_TYPE::*arg)(ARG1) const, const std::string& s)
	{
		reg->DefClassFunc(arg, s);
	}
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2>
struct LuaRegisterRouter<RET (CLASS_TYPE::*)(ARG1, ARG2) const>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (CLASS_TYPE::*arg)(ARG1, ARG2) const, const std::string& s)
	{
		reg->DefClassFunc(arg, s);
	}
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3>
struct LuaRegisterRouter<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3) const>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (CLASS_TYPE::*arg)(ARG1, ARG2, ARG3) const, const std::string& s)
	{
		reg->DefClassFunc(arg, s);
	}
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct LuaRegisterRouter<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4) const>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (CLASS_TYPE::*arg)(ARG1, ARG2, ARG3, ARG4) const, const std::string& s)
	{
		reg->DefClassFunc(arg, s);
	}
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
struct LuaRegisterRouter<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5) const>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (CLASS_TYPE::*arg)(ARG1, ARG2, ARG3, ARG4, ARG5) const, const std::string& s)
	{
		reg->DefClassFunc(arg, s);
	}
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
struct LuaRegisterRouter<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) const>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (CLASS_TYPE::*arg)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) const, const std::string& s)
	{
		reg->DefClassFunc(arg, s);
	}
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
struct LuaRegisterRouter<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) const>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (CLASS_TYPE::*arg)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) const, const std::string& s)
	{
		reg->DefClassFunc(arg, s);
	}
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct LuaRegisterRouter<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8) const>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (CLASS_TYPE::*arg)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8) const, const std::string& s)
	{
		reg->DefClassFunc(arg, s);
	}
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct LuaRegisterRouter<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9) const>
{
	template<typename REG_TYPE>
	static void Call(REG_TYPE* reg, RET (CLASS_TYPE::*arg)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9) const, const std::string& s)
	{
		reg->DefClassFunc(arg, s);
	}
};

#pragma endregion // LuaRegisterRouter

KERNEL_END

#endif
