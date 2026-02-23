// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2026-02-23 17:02:31
// Author: Eric Yonng
// Description: 生命周期操作


#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_OBJ_LIFE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_OBJ_LIFE_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>
#include <atomic>
#include <kernel/comp/memory/ObjPoolMacro.h>

KERNEL_BEGIN

// 原子
template<typename T>
requires requires (T t)
{
    t.fetch_add(1, std::memory_order_release);
    t.fetch_sub(1, std::memory_order_release);
}
class ObjLife
{
    POOL_CREATE_TEMPLATE_OBJ_DEFAULT(ObjLife,  T);
    
public:
    ObjLife(T &t)
        :_t(t)
    {
        _t.fetch_add(1, std::memory_order_release);
    }
    ObjLife(const ObjLife<T> &other)
    :_t(other._t)
    {
        _t.fetch_add(1, std::memory_order_release);
    }
    ObjLife(ObjLife<T> &&other)
        :_t(other._t)
    {
        _t.fetch_add(1, std::memory_order_release);
    }

    ~ObjLife()
    {
        _t.fetch_sub(1, std::memory_order_release);
    }

    // 禁用赋值
    ObjLife<T> &operator= (const ObjLife<T> &other) = delete;
    ObjLife<T> &operator= (ObjLife<T> &&other) = delete;

    T &_t;
};
KERNEL_END

#endif


