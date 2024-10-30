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
 * Date: 2024-10-21 11:11:13
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_CONCEPT_AWAITABLE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_CONCEPT_AWAITABLE_H__

#pragma once

#include <coroutine>
#include <type_traits>
#include <forward_list>

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <concepts>

KERNEL_BEGIN

template<typename A>
struct GetAwaiter: std::type_identity<A> { };

template<typename A>
requires requires(A&& a) { std::forward<A>(a).operator co_await(); }
struct GetAwaiter<A>: std::type_identity<decltype(std::declval<A>().operator co_await())> { };

template<typename A>
requires requires(A&& a) 
{
    operator co_await(std::forward<A>(a));
    requires ! (requires { std::forward<A>(a).operator co_await(); });
}
struct GetAwaiter<A>: std::type_identity<decltype(operator co_await(std::declval<A>()))> { };

template<typename A>
using GetAwaiter_t = typename GetAwaiter<A>::type;

// 可等待体需要:await_ready, await_suspend, await_resume三个方法
template<typename A>
concept Awaitable = requires 
{
    typename GetAwaiter_t<A>;
    requires requires (GetAwaiter_t<A> awaiter, std::coroutine_handle<> handle) {
        { awaiter.await_ready() } -> std::convertible_to<bool>;
        awaiter.await_suspend(handle);
        awaiter.await_resume();
    };
};

// 可等待体返回结果类型: AwaitResult
template<Awaitable A>
using AwaitResult = decltype(std::declval<GetAwaiter_t<A>>().await_resume());

// check archtypes
static_assert(Awaitable<std::suspend_always>);
static_assert(Awaitable<std::suspend_never>);

KERNEL_END

#endif
