--[[
Author: ericyonng 120453674@qq.com
Date: 2025-01-02 00:55:05
LastEditors: ericyonng 120453674@qq.com
LastEditTime: 2025-01-02 00:55:49
FilePath: \LusScript\hellolua.lua
Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
--]]
function DoHelloLua(arg1, arg2, arg3, arg4)
    print('hello lua', arg1, arg2, arg3, arg4)

    return 100
end

TestGlobal = 66

TestTable = {}
function TestTable:funcTest1(arg1, arg2)
    print("in TestTable:", self, arg1, arg2)
    return true
end

function TestTable.funcTest2(arg1, arg2)
    print('test table func test2', arg1, arg2)
    
    return false
end

function dump_table(tb, str)
    if nil == str then str = "" end
    for k, v in pairs(tb)
    do
        print(str, k, v)
    end
end

-- 接受stl参数
function test_stl(vec, lt, st, mp)
    print("--------------dump_table begin ----------------")
    dump_table(vec, "vec")
    dump_table(lt, "lt")
    dump_table(st, "st")
    dump_table(mp, "mp")
    print("--------------dump_table end ----------------")
    return "ok"
end


-- 返回stl 参数
function test_return_stl_vector()
    return {1,2,3,4}
end
function test_return_stl_list()
    return {1,2,3,4}
end
function test_return_stl_set()
    return {1,2,3,4}
end
function test_return_stl_map()
    return {
        ["key"] = 124
    }
end

-- 测试接受C++对象
function test_object(foo_obj)
    --测试构造
    base = TestLuaBase:new()
    -- 每个对象都有一个get_pointer获取指针
    print("base ptr:", base:GetPointer())
    -- 测试C++对象函数
    foo_obj:Print(12333, base)
    base:delete()
    --基类的函数
    foo_obj:Dump()
    -- 测试C++ 对象属性
    print("foo property", foo_obj._fooValue)
    print("base property", foo_obj._value)
end

-- 测试返回C++对象
function test_ret_object(foo_obj)
    return foo_obj
end

-- 测试返回C++对象
function test_ret_base_object(foo_obj)
    return foo_obj
end