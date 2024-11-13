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
 * Date: 2024-11-12 17:02:10
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_CO_WAITER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_CO_WAITER_H__


#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>

#include <kernel/common/NonCopyabale.h>
#include <kernel/comp/Coroutines/CoTask.h>

KERNEL_BEGIN

struct KERNEL_EXPORT CoWaiter: private NonCopyable 
{
 explicit CoWaiter(){}

 constexpr bool await_ready() noexcept { return false; }
 constexpr void await_resume() const noexcept {}

 template<typename Promise>
 void await_suspend(std::coroutine_handle<Promise> caller) const noexcept 
 {
  auto promise = &caller.promise();

  // 设置被调度, 后面协程由外部唤醒
  promise->SetState(KERNEL_NS::KernelHandle::SCHEDULED);
 }
};

// ALWAYS_INLINE KERNEL_EXPORT CoTask<> CoDelay(NoWaitAtInitialSuspend, const KERNEL_NS::TimeSlice &delay) 
// {
//     co_await CoDelayAwaiter {delay};
// }

KERNEL_EXPORT CoTask<> Waiting();

KERNEL_END

#endif
