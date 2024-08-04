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
 * Date: 2024-08-04 17:11:12
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_ASYNC_TASK_QUEUE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_ASYNC_TASK_QUEUE_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <coroutine>
#include <functional>

KERNEL_BEGIN

struct KERNEL_EXPORT AsyncTask 
{
    POOL_CREATE_OBJ_DEFAULT(AsyncTask);

    // 异步任务处理函数类型
    using Handler = std::function<void()>;

    // 异步任务处理函数
    Handler handler;
};

// TODO:需要对整个调度全面的理解, 才能把调度整合到Poller中
class KERNEL_EXPORT AsyncTaskQueue 
{
    POOL_CREATE_OBJ_DEFAULT(AsyncTaskQueue);

public:
    static AsyncTaskQueue& getInstance();

    ALWAYS_INLINE void enqueue(const AsyncTask& item) 
    {
        std::lock_guard<std::mutex> guard(_queueMutex);

        _queue.push_back(item);
    }

    ALWAYS_INLINE bool dequeue(AsyncTask* item) 
    {
        std::lock_guard<std::mutex> guard(_queueMutex);

        if (_queue.size() == 0) {
            return false;
        }

        *item = _queue.back();
        _queue.pop_back();

        return true;
    }

    ALWAYS_INLINE size_t getSize() const 
    {
        return _queue.size();
    }

private:
    // 支持单例模式，通过default修饰符说明构造函数使用默认版本
    AsyncTaskQueue() = default;
    // 支持单例模式，通过delete修饰符说明拷贝构造函数不可调用
    AsyncTaskQueue(const AsyncTaskQueue&) = delete;
    // 支持单例模式，通过delete修饰符说明赋值操作符不可调用
    AsyncTaskQueue& operator=(const AsyncTaskQueue&) = delete;

    // 异步任务队列
    std::vector<AsyncTask> _queue;
    // 异步任务队列互斥锁，用于实现线程同步，确保队列操作的线程安全
    std::mutex _queueMutex;
};

ALWAYS_INLINE AsyncTaskQueue& AsyncTaskQueue::getInstance() {
    static AsyncTaskQueue queue;

    return queue;
}

KERNEL_END

#endif

