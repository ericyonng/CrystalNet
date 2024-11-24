testsuit集成了lua静态库

lua 在3rd/lua下

只需要

#include <lua/lua.hpp>

即可



*.lua脚本在执行do_loadfile(L, "*.lua")

lua脚本的路径是相对于c++执行程序的路径的