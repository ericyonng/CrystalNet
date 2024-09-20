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
 * Date: 2024-08-04 23:24:08
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_ASYNCIFY_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_ASYNCIFY_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/Coroutines/Invocable.h>
#include <kernel/comp/Coroutines/Coroutine.h>
#include <kernel/comp/Utils/TlsUtil.h>
#include <kernel/comp/Poller/PollerEvent.h>
#include <kernel/comp/Poller/Poller.h>
#include <kernel/comp/Coroutines/AsyncTask.h>

KERNEL_BEGIN


// 默认的AsyncTaskSuspender（当任务函数返回类型不为void时）
template <typename ResultType>
ALWAYS_INLINE void DefaultAsyncAwaitableSuspend(
    Awaitable<ResultType>* awaitable,
    AsyncTaskResumer resumer,
    CoroutineHandle& h
) {
    // handler将在poller中执行
    auto poller = KERNEL_NS::TlsUtil::GetPoller();
    auto ev = KERNEL_NS::AsyncTaskPollerEvent::New_AsyncTaskPollerEvent();
    auto task = AsyncTask::NewThreadLocal_AsyncTask();
    ev->_asyncTask = task;
    auto &&handler = [resumer, awaitable] {
            awaitable->_taskResult = awaitable->_taskHandler();

            // 恢复后 awaitable 会被最终销毁
            resumer();
        };
    task->_handler = KERNEL_CREATE_CLOSURE_DELEGATE(handler, void);
    poller->Push(0, ev);
}

//template <typename ResultType>
//void defaultCoroutineAsyncAwaitableSuspend(
//    Awaitable<ResultType>* awaitable,
//    AsyncTaskResumer resumer,
//    CoroutineHandle& h
//) {
//    Coroutine c{ ._handle = h };
//    if (!c.hasDoneHook()) {
//        c.registerDoneHook([resumer, awaitable]() {
//            //awaitable->_taskResult = awaitable->_taskHandler();
//            // TODO: Assign result from promise to _taskResult
//            resumer();
//        });
//    }

//    if (c.done()) {
//        c.next();
//    }

//    auto& asyncTaskQueue = AsyncTaskQueue::getInstance();
//    asyncTaskQueue.enqueue({
//        .handler = [resumer, awaitable, h] {
//            Coroutine c {._handle = h };
//            if (!c.hasDoneHook()) {
//                c.registerDoneHook([resumer, awaitable]() {
//                    awaitable->_taskResult = awaitable->_taskHandler();
//                    resumer();
//                });
//            }

//            if (c.done()) {
//                
//            }

//        }
//    });
//}

// 默认的AsyncTaskSuspender（当任务函数返回类型为void时）
template <>
ALWAYS_INLINE void DefaultAsyncAwaitableSuspend<void>(
    Awaitable<void>* awaitable,
    AsyncTaskResumer resumer,
    CoroutineHandle& h
) {
    // handler将在poller中执行
    auto poller = KERNEL_NS::TlsUtil::GetPoller();
    auto ev = KERNEL_NS::AsyncTaskPollerEvent::New_AsyncTaskPollerEvent();
    auto task = AsyncTask::NewThreadLocal_AsyncTask();
    ev->_asyncTask = task;
    auto &&handler = [resumer, awaitable] {
            awaitable->_taskHandler();

            // 恢复后 awaitable 会被最终销毁
            resumer();
        };
    task->_handler = KERNEL_CREATE_CLOSURE_DELEGATE(handler, void);
    poller->Push(0, ev);
}

// 异步化工具函数，支持将普通函数f异步化
template<Invocable TaskType>
ALWAYS_INLINE auto AsyncTaskRun(TaskType task, AsyncTaskSuspender<std::invoke_result_t<TaskType>> suspender = DefaultAsyncAwaitableSuspend<std::invoke_result_t<TaskType>>)
{
    return  Awaitable<std::invoke_result_t<TaskType>>{
        ._taskHandler = task,
            ._suspender = suspender
    };
}

KERNEL_END

#endif
