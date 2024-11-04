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
 * Date: 2024-11-02 21:29:10
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_COTOOLS_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_COTOOLS_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>

#include <kernel/comp/Poller/PollerInc.h>
#include <kernel/comp/Utils/TlsUtil.h>
#include <kernel/comp/Coroutines/AsyncTask.h>

KERNEL_BEGIN

// AsyncTask事件：需要对cb执行时对协程进行有效性检查(做一个字典, 检查id是否存在) TODO:
template<typename CallerType>
ALWAYS_INLINE void PostAsyncTask(CallerType &&cb, Int32 level = 0)
{
    // handler将在poller中执行
    auto poller = KERNEL_NS::TlsUtil::GetPoller();
    auto ev = KERNEL_NS::AsyncTaskPollerEvent::New_AsyncTaskPollerEvent();
    auto task = AsyncTask::NewThreadLocal_AsyncTask();
    ev->_asyncTask = task;
    auto delg = KERNEL_NS::DelegateFactory::Create<decltype(cb), void>(std::forward<CallerType>(cb));
    task->_handler = delg;
    poller->Push(level, ev);
}

KERNEL_END

#endif