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
 * Date: 2024-10-26 17:02:00
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_CONCEPT_PROMISE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_CONCEPT_PROMISE_H__

#pragma once

#include <kernel/comp/Coroutines/Concept/Future.h>

KERNEL_BEGIN

template<typename P>
concept Promise = requires (P p) 
{
    // 需要有 get_return_object 方法且, 返回值需要符合Future约束
    { p.get_return_object() } -> Future;
    // 需要有initial_suspend方法, 且返回值需要符合 Awaitable 约束
    { p.initial_suspend() } -> Awaitable;
    // 需要有final_suspend方法，且不抛异常, 返回值需要符合 Awaitable 约束
    { p.final_suspend() } noexcept -> Awaitable;
    // 需要有unhandled_exception方法
    p.unhandled_exception();
    // 需要有return_value或return_void方法
    requires (requires(int v) { p.return_value(v); } ||
              requires        { p.return_void();   });
};

KERNEL_END

#endif