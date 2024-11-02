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

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_SCHEDULEDTASK_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_SCHEDULEDTASK_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/comp/Coroutines/Concept/Future.h>
#include <kernel/common/NonCopyabale.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Poller/PollerInc.h>
#include <kernel/comp/Utils/TlsUtil.h>
#include <kernel/comp/Coroutines/CoHandle.h>

KERNEL_BEGIN

template<Future Task>
struct ScheduledTask: private NonCopyable 
{
    template<Future Fut>
    explicit ScheduledTask(Fut&& fut): _task(std::forward<Fut>(fut)) 
    {
        if (_task.Valid() && ! _task.Done()) 
        {
            _task._handle.promise().Schedule();
        }
    }

    void Cancel() { _task.Destroy(); }

    decltype(auto) operator co_await() const & noexcept 
    {
        return _task.operator co_await();
    }

    auto operator co_await() const && noexcept 
    {
        return _task.operator co_await();
    }

    decltype(auto) GetResult() & 
    {
        return _task.GetResult();
    }

    decltype(auto) GetResult() && 
    {
        return std::move(_task).GetResult();
    }

    bool Valid() const { return _task.Valid(); }
    bool Done() const { return _task.Done(); }

private:
    Task _task;
};

template<Future Fut>
ScheduledTask(Fut&&) -> ScheduledTask<Fut>;

template<Future Fut>
[[nodiscard("discard(detached) a task will not schedule to run")]]
ScheduledTask<Fut> *schedule_task(Fut&& fut) 
{
    
    return new ScheduledTask { std::forward<Fut>(fut) };
}

KERNEL_END

#endif