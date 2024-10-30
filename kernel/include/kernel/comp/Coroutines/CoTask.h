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
 * Date: 2024-10-21 01:31:10
 * Author: Eric Yonng
 * Description: 
*/


#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_COTASK_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_COTASK_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <coroutine>
#include <functional>
#include <exception>
#include <kernel/comp/Utils/TlsUtil.h>
#include <kernel/comp/Poller/PollerEvent.h>
#include <kernel/comp/Poller/Poller.h>
#include <kernel/comp/Coroutines/AsyncTask.h>

KERNEL_BEGIN

struct NoWaitAtInitialSuspend {};
inline constexpr NoWaitAtInitialSuspend no_wait_at_initial_suspend;

template<typename R = void>
struct CoTask : private NonCopyable
{
    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;
    
    // 协程Promise定义
    struct promise_type
    {
        //std::function<void()> _doneHook;
        //bool _doneHookExecuted = false;
        std::exception_ptr _exception; // 待抛出的异常

        ResultType _result;

        Int64 _refCount = 1;

        CoTask<ResultType> get_return_object() { 
            return {
                ._handle = std::coroutine_handle<CoTask<ResultType>::promise_type>::from_promise(*this) 
            }; 
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_value(ResultType &&t) {
            _result = t;
        }

        void unhandled_exception() {
            _exception = std::current_exception();
        }
    };

    template<typename TaskType, typename... Args>
    static CoTask<ResultType> Run(TaskType task, Args... args)
    {
        return CoTask<ResultType>{
            ._taskHandler = [=](){
                return task(args...);
            }
        };
    }

    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> h)
    {
        // handler将在poller中执行
        auto poller = KERNEL_NS::TlsUtil::GetPoller();
        auto ev = KERNEL_NS::AsyncTaskPollerEvent::New_AsyncTaskPollerEvent();
        auto task = AsyncTask::NewThreadLocal_AsyncTask();
        ev->_asyncTask = task;
        auto &&handler = [h, this] {
                auto newHandle = std::coroutine_handle<CoTask<ResultType>::promise_type>::from_address(h.address());
                newHandle.promise()._result = _taskHandler();

                if(--newHandle.promise()._refCount <= 0)
                {
                    // 恢复后 awaitable 会被最终销毁
                    newHandle.resume();
                }
            };
        task->_handler = KERNEL_CREATE_CLOSURE_DELEGATE(handler, void);
        poller->Push(0, ev);
    }

    ResultType await_resume() {
        return _handle.promise()._result;
    }

    std::coroutine_handle<CoTask<ResultType>::promise_type> _handle;

    // co_await时需要执行的任务，开发者可以在suspend实现中调用该函数执行用户期望的任务
    std::function<ResultType()> _taskHandler;
};

template<>
struct CoTask<void>
{
    // 协程Promise定义
    struct promise_type
    {
        //std::function<void()> _doneHook;
        //bool _doneHookExecuted = false;
        std::exception_ptr _exception; // 待抛出的异常
        Int64 _refCount = 1;

        CoTask<void> get_return_object() { 
            return {
                ._handle = std::coroutine_handle<CoTask<void>::promise_type>::from_promise(*this) 
            }; 
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}

        void unhandled_exception() {
            _exception = std::current_exception();
        }
    };

    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> h)
    {
        // handler将在poller中执行
        auto poller = KERNEL_NS::TlsUtil::GetPoller();
        auto ev = KERNEL_NS::AsyncTaskPollerEvent::New_AsyncTaskPollerEvent();
        auto task = AsyncTask::NewThreadLocal_AsyncTask();
        ev->_asyncTask = task;
        auto &&handler = [h, this] {
                _taskHandler();
                
                auto newHandle = std::coroutine_handle<CoTask<void>::promise_type>::from_address(h.address());

                // 恢复后 awaitable 会被最终销毁
                if(--newHandle.promise()._refCount <= 0)
                    newHandle.resume();
            };
        task->_handler = KERNEL_CREATE_CLOSURE_DELEGATE(handler, void);
        poller->Push(0, ev);
    }

    void await_resume() {}

    template<typename TaskType, typename... Args>
    static CoTask<void> Run(TaskType task, Args... args)
    {
        return CoTask<void>{
            ._taskHandler = [=](){
                task(args);
            }
        };
    }

    std::coroutine_handle<CoTask<void>::promise_type> _handle;

    // co_await时需要执行的任务，开发者可以在suspend实现中调用该函数执行用户期望的任务
    std::function<void()> _taskHandler;
};

KERNEL_END

#endif