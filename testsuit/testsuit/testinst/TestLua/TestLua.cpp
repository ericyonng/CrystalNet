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

void TestLua::Run()
{
    auto luaS = luaL_newstate();
    luaL_openlibs(luaS);

    auto luaCppFunc = [](lua_State* L)-> Int32
    {
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLua, "hello lua c++"));
        return 0;
    };
    lua_register(luaS, "luaCppFunc", luaCppFunc);

    // 加载lua脚本
    luaL_dofile(luaS, "../../testsuit/testsuit/testinst/TestLua/testLua.lua");

    // 调用Lua脚本中的函数
    lua_getglobal(luaS, "luaFunc");
    lua_call(luaS, 0, 0);

    lua_close(luaS);

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLua, "test lua finish."));
}
