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
 * Date: 2024-08-04 16:53:13
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_COROUTINE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_COROUTINE_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <coroutine>
#include <functional>

KERNEL_BEGIN

// 协程类
struct KERNEL_EXPORT Coroutine
{
    POOL_CREATE_OBJ_DEFAULT(Coroutine);

    // 协程Promise定义
    struct promise_type
    {
        //std::function<void()> _doneHook;
        //bool _doneHookExecuted = false;

        Coroutine get_return_object() { 
            return {
                ._handle = std::coroutine_handle<promise_type>::from_promise(*this) 
            }; 
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };

    // 协程的句柄，可用于构建Coroutine类，并在业务代码中调用接口进行相关操作
    std::coroutine_handle<promise_type> _handle;

    //bool done() const {
    //    return _handle.done();
    //}

    //void destroy() {
    //    _handle.destroy();
    //}

    //bool next() {
    //    if (!_handle.done()) {
    //        _handle.resume();

    //        return true;
    //    }

    //    auto& promise = _handle.promise();
    //    if (promise._doneHookExecuted) {
    //        return false;
    //    }

    //    if (promise._doneHook) {
    //        promise._doneHook();
    //    }

    //    promise._doneHookExecuted = true;

    //    return false;
    //}

    //bool hasDoneHook() const{
    //    return static_cast<bool>(_handle.promise()._doneHook);
    //}

    //void registerDoneHook(std::function<void()> doneHook) {
    //    _handle.promise()._doneHook = doneHook;
    //}
};


// AsyncTaskSuspender类型声明
template <typename ResultType>
struct Awaitable;

// 唤醒函数
using AsyncTaskResumer = std::function<void()>;

// 定义协程句柄
using CoroutineHandle = std::coroutine_handle<Coroutine::promise_type>;

// 协程挂起
template <typename ResultType>
using AsyncTaskSuspender = std::function<void(
    Awaitable<ResultType>*, AsyncTaskResumer, CoroutineHandle&
)>;


// Awaitable类型定义（当任务函数返回类型不为void时）
template <typename ResultType>
struct Awaitable
{
    POOL_CREATE_TEMPLATE_OBJ_DEFAULT(Awaitable);

    // co_await时需要执行的任务，开发者可以在suspend实现中调用该函数执行用户期望的任务
    std::function<ResultType()> _taskHandler;

    // 存储任务执行的结果，会在await_resume中作为co_await表达式的值返回。
    ResultType _taskResult;

    // 存储开发者自定义的await_suspend实现，会在await_suspend中调用
    AsyncTaskSuspender<ResultType> _suspender;

    bool await_ready() { return false; }
    void await_suspend(CoroutineHandle h)
    {
        _suspender(this, [h] { h.resume(); }, h);
    }

    ResultType await_resume() {
        return _taskResult;
    }
};

template <typename ResultType>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_IMPL(Awaitable, ResultType);

// Awaitable类型定义（当任务函数返回类型为void时）
template <>
struct KERNEL_EXPORT Awaitable<void>
{
    // co_await时需要执行的任务，开发者可以在suspend实现中调用该函数执行用户期望的任务
    std::function<void()> _taskHandler;

    // 存储开发者自定义的await_suspend实现，会在await_suspend中调用
    AsyncTaskSuspender<void> _suspender;

    bool await_ready() { return false; }
    void await_suspend(CoroutineHandle h)
    {
        _suspender(this, [h] { h.resume(); }, h);
    }

    void await_resume() {}
};

KERNEL_END

#endif
