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
 * Date: 2024-10-26 20:27:13
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_CODELAY_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_CODELAY_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/common/NonCopyabale.h>
#include <kernel/comp/TimeSlice.h>
#include <kernel/comp/Coroutines/CoTask.h>
#include <kernel/comp/Timer/LibTimer.h>

KERNEL_BEGIN

struct KERNEL_EXPORT CoDelayAwaiter: private NonCopyable 
{
    explicit CoDelayAwaiter(KERNEL_NS::TimeSlice && delay): _delay(delay) {}
    explicit CoDelayAwaiter(const KERNEL_NS::TimeSlice &delay): _delay(delay) {}

    constexpr bool await_ready() noexcept { return false; }
    constexpr void await_resume() const noexcept {}

    template<typename Promise>
    void await_suspend(std::coroutine_handle<Promise> caller) const noexcept 
    {
        auto promise = &caller.promise();
        auto &slice = _delay;

        // 先设置可调度
        promise->SetState(KERNEL_NS::KernelHandle::SCHEDULED);
        KERNEL_NS::PostAsyncTask([promise, copySlice = slice]()
        {
            auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
            timer->SetTimeOutHandler([promise](KERNEL_NS::LibTimer *t)
            {
                // 恢复协程
                promise->Run(KERNEL_NS::KernelHandle::UNSCHEDULED);

                KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
            });
            timer->Schedule(copySlice);
        });
    }

private:
    KERNEL_NS::TimeSlice _delay;
};

ALWAYS_INLINE KERNEL_EXPORT CoTask<> CoDelay(NoWaitAtInitialSuspend, const KERNEL_NS::TimeSlice &delay) 
{
    co_await CoDelayAwaiter {delay};
}

ALWAYS_INLINE KERNEL_EXPORT CoTask<> CoDelay(const KERNEL_NS::TimeSlice &delay) 
{
    return CoDelay(no_wait_at_initial_suspend, delay);
}

KERNEL_END

#endif
