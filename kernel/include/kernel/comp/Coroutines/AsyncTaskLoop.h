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
 * Date: 2024-08-04 23:04:37
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_ASYNC_TASK_LOOP_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_ASYNC_TASK_LOOP_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/Coroutines/AsyncTaskQueue.h>

KERNEL_BEGIN

class KERNEL_EXPORT AsyncTaskLoop 
{
public:
    // 常量，定义了任务循环的等待间隔时间（单位为毫秒）
    static constexpr Int32 SLEEP_MS = 1000;

    static AsyncTaskLoop& getInstance();
    static void start() 
    {
        getInstance().startLoop();
    }

private:
    // 支持单例模式，通过default修饰符说明构造函数使用默认版本
    AsyncTaskLoop() = default;
    // 支持单例模式，通过delete修饰符说明拷贝构造函数不可调用
    AsyncTaskLoop(const AsyncTaskLoop&) = delete;
    // 支持单例模式，通过delete修饰符说明赋值操作符不可调用
    AsyncTaskLoop& operator=(const AsyncTaskLoop&) = delete;

    void startLoop() 
    {
        while (true) 
        {
            loopExecution();
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));
        }
    }

    void loopExecution() 
    {
        AsyncTask asyncEvent;
        if (!AsyncTaskQueue::getInstance().dequeue(&asyncEvent)) 
        {
            return;
        }

        asyncEvent.handler();
    }
};

ALWAYS_INLINE AsyncTaskLoop& AsyncTaskLoop::getInstance() 
{
    static AsyncTaskLoop eventLoop;

    return eventLoop;
}

KERNEL_END


#endif
