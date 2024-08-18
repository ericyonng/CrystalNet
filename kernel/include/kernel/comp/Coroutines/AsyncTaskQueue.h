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
#include <kernel/comp/Lock/Lock.h>
#include <kernel/comp/Utils/TlsUtil.h>

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
    AsyncTaskQueue() {}
    ~AsyncTaskQueue();

    static AsyncTaskQueue& getInstance();

    ALWAYS_INLINE void enqueue(const AsyncTask& item) 
    {        
        // auto defs = KERNEL_NS::TlsUtil::GetDefTls();
        // // TODO:添加到poller队列中
        // if(LIKELY(defs->_poller != NULL && defs->_poller->IsEnable()))
        // {
        //     // defs->_poller->Push(defs->_poller->GetMaxPriorityLevel(), )
        //     return;
        // }
        // _queue.push_back(item);
    }

    ALWAYS_INLINE bool dequeue(AsyncTask *&item) 
    {
        if (_queue.size() == 0) {
            return false;
        }

        item = _queue.back();
        _queue.pop_back();

        return true;
    }

    ALWAYS_INLINE size_t getSize() const 
    {
        return _queue.size();
    }

private:

    // 异步任务队列
    std::vector<AsyncTask *> _queue;
};

ALWAYS_INLINE AsyncTaskQueue& AsyncTaskQueue::getInstance() 
{
    static AsyncTaskQueue queue;
    return queue;
    // auto tlsStack = KERNEL_NS::TlsUtil::GetDefTls();
    // return *tlsStack->_taskQueue;
}

KERNEL_END

#endif

