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
#include <kernel/comp/Coroutines/AsyncTaskQueue.h>

KERNEL_BEGIN


// 默认的AsyncTaskSuspender（当任务函数返回类型不为void时）
template <typename ResultType>
ALWAYS_INLINE void defaultAsyncAwaitableSuspend(
    Awaitable<ResultType>* awaitable,
    AsyncTaskResumer resumer,
    CoroutineHandle& h
) {
    auto& asyncTaskQueue = AsyncTaskQueue::getInstance();
    asyncTaskQueue.enqueue({
        .handler = [resumer, awaitable] {
            awaitable->_taskResult = awaitable->_taskHandler();
            resumer();
        }
    });
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
ALWAYS_INLINE void defaultAsyncAwaitableSuspend<void>(
    Awaitable<void>* awaitable,
    AsyncTaskResumer resumer,
    CoroutineHandle& h
) {
    auto& asyncTaskQueue = AsyncTaskQueue::getInstance();
    asyncTaskQueue.enqueue({
        .handler = [resumer, awaitable] {
            awaitable->_taskHandler();
            resumer();
        }
    });
}

// 异步化工具函数，支持将普通函数f异步化
template <Invocable T>
ALWAYS_INLINE auto asyncify(
    T taskHandler, 
    AsyncTaskSuspender<std::invoke_result_t<T>> suspender = 
        defaultAsyncAwaitableSuspend<std::invoke_result_t<T>>
) {
    return Awaitable<std::invoke_result_t<T>>{
        ._taskHandler = taskHandler,
            ._suspender = suspender
    };
}

KERNEL_END

#endif
