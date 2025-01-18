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
* Date: 2024-11-25 00:11:29
* Author: Eric Yonng
* Description:
*/

#include "pch.h"
#include "TestLua.h"
#include <lua/lua.hpp>

class TestLuaBase
{
public:
    TestLuaBase()
        :_value(100)
    {
        
    }

    ~TestLuaBase()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("destruct value:%d"), _value);
    }

    void Dump()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("value:%d"), _value);
    }
    
    Int32 _value;
};

class TestFoo : public TestLuaBase
{
public:
    TestFoo(Int32 fooValue)
        :_fooValue(fooValue)
    {
        
    }
    ~TestFoo()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("test foo destruct"));
    }

    void Print(Int32 a, TestLuaBase *p) const
    {
        g_Log->Info(LOGFMT_OBJ_TAG("print a:%d, p:%p"), a, p);
    }

    static void Dumy()
    {
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestFoo, "dumy TestFoo"));
    }

    Int32 _fooValue;
};

//! lua talbe 可以自动转换为stl 对象
static  void Dumy(std::map<std::string, std::string> ret, std::vector<int> a, std::list<std::string> b, std::set<int64_t> c)
{
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLua, "dummy start ------------"));
    
    for (auto it =  ret.begin(); it != ret.end(); ++it)
    {
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLua, "key:%s, value:%s"), it->first.c_str(), it->second.c_str());
    }

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLua, "dummy end ------------"));
}

static void LuaReg(lua_State* ls)
{
    //! 注册基类函数, ctor() 为构造函数的类型
    KERNEL_NS::LuaRegister<TestLuaBase, LUA_CTOR()>(ls, "TestLuaBase")  //! 注册构造函数
                    .Def(&TestLuaBase::Dump, "Dump")     //! 注册基类的函数
                    .Def(&TestLuaBase::_value, "_value");          //! 注册基类的属性

    //! 注册子类，ctor(int) 为构造函数， foo_t为类型名称， base_t为继承的基类名称
    KERNEL_NS::LuaRegister<TestFoo, LUA_CTOR(int)>(ls, "TestFoo", "TestLuaBase")
                .Def(&TestFoo::Print, "Print")        //! 子类的函数
                .Def(&TestFoo::_fooValue, "_fooValue");               //! 子类的字段

    KERNEL_NS::LuaRegister<>(ls)
                .Def(&Dumy, "Dumy");                //! 注册静态函数
}

static  void RetLuaFail(Int32 a)
{
    
}

void TestLua::Run()
{
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLua, "start test lua..."));
    KERNEL_NS::KernelLua kernelLua;
    kernelLua.SetModFuncFlag(true);
    kernelLua.Reg(LuaReg);

    auto &&curPath = KERNEL_NS::SystemUtil::GetCurProgRootPath();
    if(!curPath.IsEndsWith("/") && !curPath.IsEndsWith("\\"))
        curPath += "/";
    curPath += "TestServiceLuaScript/";

    if(curPath.Contain("\\"))
        curPath.findreplace("\\", "\\\\");
    kernelLua.AddPackagePath(curPath.GetRaw());

    // 便利目录并load lua文件
    KERNEL_NS::DirectoryUtil::TraverseDirRecursively(curPath, [&kernelLua](const KERNEL_NS::FindFileInfo &findFileInfo, bool &isParentDirContinue) -> bool
    {
        if(KERNEL_NS::FileUtil::IsDir(findFileInfo))
            return true;                                         

        if(KERNEL_NS::FileUtil::ExtractFileExtension(findFileInfo._fileName) != KERNEL_NS::LibString(".lua"))
            return true;

        kernelLua.LoadFile((findFileInfo._fullName).GetRaw());                                                                           
        
        return true;
    });

    Int32 testGlobal = 0;
    kernelLua.GetGlobalVariable("TestGlobal", testGlobal);
    kernelLua.SetGlobalVariable("TestGlobal", ++testGlobal);
    Int32 testGlobalCopy = 0;
    kernelLua.GetGlobalVariable("TestGlobal", testGlobalCopy);

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLua, "TestGlobal:%d"), testGlobal);
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLua, "testGlobalCopy:%d"), testGlobalCopy);

    // 执行及时脚本
    kernelLua.RunString("print(\"hello dynamic script...\")");

    // 执行lua函数
    auto err = kernelLua.Call<Int32>("DoHelloLua", 1, 2.0, (double)3.0, "4");
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLua, "err:%d"), err);

    auto retBool = kernelLua.Call<bool>("TestTable:funcTest1", 1, "hello test");
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLua, "retBool:%d"), retBool);
    retBool = kernelLua.Call<bool>("TestTable.funcTest2", 2, "hello test");
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLua, "retBool:%d"), retBool);

    {
        std::vector<Int32> vec;
        vec.push_back(100);
        std::vector<float> lt;
        lt.push_back(14.4);
        std::set<std::string> st;
        st.insert("OhNIce");
        std::map<std::string, Int32> dict;
        dict["key"] = 200;
        auto result = kernelLua.Call<std::string>("test_stl", vec, lt, st, dict);
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLua, "result:%s"), result.c_str());
    }

    {
        //! 调用lua 函数返回 talbe，自动转换为stl结构
        auto vec = kernelLua.Call<std::vector<int> >("test_return_stl_vector");
        auto lt  = kernelLua.Call<std::list<float> >("test_return_stl_list");
        auto st  = kernelLua.Call<std::set<std::string> >("test_return_stl_set");
        auto mp  = kernelLua.Call<std::map<std::string, int> >("test_return_stl_map");

        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLua, "vec:%s"), KERNEL_NS::StringUtil::ToString(vec, ",").c_str());
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLua, "lt:%s"), KERNEL_NS::StringUtil::ToString(lt, ",").c_str());
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLua, "st:%s"), KERNEL_NS::StringUtil::ToString(st, ",").c_str());
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLua, "mp:%s"), KERNEL_NS::StringUtil::ToString(mp, ",").c_str());
    }

    // 调用lua函数, c++对象作为参数
    {
        auto foo = new TestFoo(100);
        kernelLua.Call<void>("test_object", foo);

        // foo作为返回值
        auto foo_tmp = kernelLua.Call<TestFoo *>("test_ret_object", foo);
        auto base_ptr = kernelLua.Call<TestLuaBase *>("test_ret_base_object", foo_tmp);
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLua, "foo_tmp:%p, base_ptr:%p"), foo_tmp, base_ptr);
    }

    // Reg 添加了require约束, 以下示范会失败
    // kernelLua.Reg(RetLuaFail);
    
    // auto luaS = luaL_newstate();
    // luaL_openlibs(luaS);
    //
    // auto luaCppFunc = [](lua_State* L)-> Int32
    // {
    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLua, "hello lua c++"));
    //     return 0;
    // };
    // lua_register(luaS, "luaCppFunc", luaCppFunc);
    //
    // // 加载lua脚本
    // luaL_dofile(luaS, "../../testsuit/testsuit/testinst/TestLua/testLua.lua");
    //
    // // 调用Lua脚本中的函数
    // lua_getglobal(luaS, "luaFunc");
    // lua_call(luaS, 0, 0);
    //
    // lua_close(luaS);

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLua, "test lua finish."));
}
