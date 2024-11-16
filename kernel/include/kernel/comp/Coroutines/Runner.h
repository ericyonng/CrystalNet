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
 * Date: 2024-10-26 20:13:10
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_RUNNER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_RUNNER_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/comp/Coroutines/Concept/Future.h>

#include <kernel/comp/Poller/PollerInc.h>
#include <kernel/comp/Utils/TlsUtil.h>
#include <kernel/comp/Coroutines/CoHandle.h>
#include <kernel/comp/Coroutines/CoTask.h>
#include <kernel/comp/Coroutines/CoTools.h>

KERNEL_BEGIN

// 异步运行任务, 不会立刻运行
template<Future Fut>
ALWAYS_INLINE void PostRun(Fut&& task) 
{
    if (task.Valid() && ! task.Done()) 
    {
        task.GetHandle().promise().SetState(KERNEL_NS::KernelHandle::SCHEDULED);
        task.SetDisableSuspend(true);

        // handler将在poller中执行(lambda 绑定移动语义)
        KERNEL_NS::SmartPtr<Fut> moveTask(new Fut(std::forward<Fut>(task)));
        PostAsyncTask([moveTask] () mutable -> void
        {
            if(moveTask->Valid() && !moveTask->Done())
            {
                moveTask->GetHandle().promise().Run(KERNEL_NS::KernelHandle::UNSCHEDULED);
            }
        });
    }
}

// t 是lambda注意, lambda捕获的成员将在被调用中无法使用, 所以如果需要参数不应该捕获外部参数而应该在Caller的时候传递参数，因为会被作为协程参数被保存在协程堆中
template<typename T, typename... Args>
ALWAYS_INLINE void PostCaller(T &&t, Args... args)
{
    auto &&lamb = [t](Args... argsLamb)->CoTask<> 
    {        
        // 此处不挂起
        co_await t(argsLamb...).SetDisableSuspend(true);
    };
    PostRun(lamb(args...));
}

KERNEL_END

#endif
