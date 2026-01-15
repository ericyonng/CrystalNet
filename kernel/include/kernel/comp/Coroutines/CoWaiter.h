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

#pragma once

#ifdef CRYSTAL_NET_CPP20

#include <coroutine>
#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/common/status.h>

#include <kernel/common/NonCopyabale.h>

#include <kernel/comp/Coroutines/CoHandle.h>
#include "kernel/comp/SmartPtr.h"
#include <kernel/comp/Coroutines/CoTask.h>
#include <kernel/comp/Timer/LibTimer.h>
#include <kernel/comp/Log/log.h>

KERNEL_BEGIN

struct KERNEL_EXPORT CoWaiter: private NonCopyable 
{
    explicit CoWaiter(){}

    constexpr bool await_ready() noexcept { return false; }
    constexpr void await_resume() const noexcept
    {
        
    }

    template<typename Promise>
    void await_suspend(std::coroutine_handle<Promise> caller) const noexcept 
    {
        CoTaskParam::SetCurrentCoParam(NULL);

        auto promise = &caller.promise();
        auto promiseParam = promise->GetParam();
        auto callerHandleId = promise->GetHandleId();

        promise->SetState(KERNEL_NS::KernelHandle::SCHEDULED);

        // 有过期时间限制, 没有过期时间则一直等待下去
        if(_param && _param->_endTime)
        {
            KERNEL_NS::PostAsyncTask([endTime = this->_param->_endTime, callerHandleId]()
            {
                auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
                timer->SetTimeOutHandler([callerHandleId](KERNEL_NS::LibTimer *t)
                {
                    do
                    {
                        auto callerHandle = reinterpret_cast<CoHandle *>(KERNEL_NS::TlsUtil::GetTlsCoDict()->GetHandle(callerHandleId));
                        if(UNLIKELY(!callerHandle))
                            break;
                        
                        if(callerHandle->IsDone())
                            break;

                        // 设置当前协程
                        CoTaskParam::SetCurrentCoParam(callerHandle->GetParam());

                        // 超时
                        callerHandle->GetParam()->_errCode = Status::CoTaskTimeout;
                        KERNEL_NS::LibString content;
                        callerHandle->GetBacktrace(content);
                        g_Log->Warn(LOGFMT_NON_OBJ_TAG(CoWaiter, "co time out, backtrace:%s"), content.c_str());
                        callerHandle->Run(KERNEL_NS::KernelHandle::UNSCHEDULED);
                    }
                    while (false);

                    KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
                });
                timer->Schedule(endTime - KERNEL_NS::LibTime::Now());
            });
        }
    }
    
    void SetTimeout(const KERNEL_NS::TimeSlice &slice)
    {
        if(!_param)
            _param = KERNEL_NS::CoTaskParam::NewThreadLocal_CoTaskParam();

        _param->_endTime = KERNEL_NS::LibTime::Now() + slice;
    }
    
    SmartPtr<CoTaskParam, AutoDelMethods::Release> _param; 
};

// ALWAYS_INLINE KERNEL_EXPORT CoTask<> CoDelay(NoWaitAtInitialSuspend, const KERNEL_NS::TimeSlice &delay) 
// {
//     co_await CoDelayAwaiter {delay};
// }

// 超时唤醒需要外部手动销毁Waiting产生的协程, 通过GetParam可以获取协程handle
CoTask<> Waiting(KERNEL_NS::TimeSlice slice = KERNEL_NS::TimeSlice::ZeroSlice());
CoTask<> CoCompleted();

// 空协程
#ifndef CRYSTAL_CO_COMPLETED
 // 空协程
 #define CRYSTAL_CO_COMPLETED() co_await KERNEL_NS::CoCompleted()
#endif

KERNEL_END

#endif

#endif
