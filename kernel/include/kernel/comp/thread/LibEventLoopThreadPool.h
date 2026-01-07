// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2025-12-22 23:12:05
// Author: Eric Yonng
// Description:

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_THREAD_LIB_EVENT_LOOP_THREAD_POOL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_THREAD_LIB_EVENT_LOOP_THREAD_POOL_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>
#include <vector>
#include <kernel/comp/LibString.h>

#include "kernel/comp/Coroutines/CoTask.h"
#include <kernel/comp/Poller/Poller.h>

KERNEL_BEGIN

class LibEventLoopThread;

class KERNEL_EXPORT LibEventLoopThreadPool
{
public:
    LibEventLoopThreadPool();
    LibEventLoopThreadPool(Int32 minNum, Int32 maxNum);
    ~LibEventLoopThreadPool();
    
    void Release() const
    {
        delete this;
    }

    void Start();
    void Close();
    void HalfClose();
    void FinishClose();

    // 线程池执行行为
    template<typename LambdaType>
    requires requires (LambdaType lam) 
    {
        {lam()} -> std::convertible_to<void>;
    }
    void Send(LambdaType &&lambda, bool priorityToUsingNewThread = false)
    {
        if(UNLIKELY(_disablePost.load(std::memory_order_acquire)))
        {
            return;
        }
        
        auto poller = _SelectPoller(priorityToUsingNewThread);
        poller->Push(std::forward<LambdaType>(lambda));
    }

    // 线程池中执行lambda行为并返回结果到调用线程
    template<typename ResType, typename LambdaType>
    requires requires(LambdaType lambda, ResType res)
    {
        // 可移动
        requires std::move_constructible<ResType>;
        
        {lambda()} -> std::convertible_to<ResType>;

        // res必须有Release接口
        res.Release();
    
        // res必须有ToString接口
        res.ToString();
    }
    CoTask<KERNEL_NS::SmartPtr<ResType, AutoDelMethods::Release>> SendAsync(LambdaType &&lamb, bool priorityToUsingNewThread = false)
    {
        if(UNLIKELY(_disablePost.load(std::memory_order_acquire)))
        {
            co_return KERNEL_NS::SmartPtr<ResType, AutoDelMethods::Release>();
        }

        auto poller = _SelectPoller(priorityToUsingNewThread);

        co_return co_await poller->template SendAsync<ResType, LambdaType>(std::forward<LambdaType>(lamb));
    }
    
    Int32 GetWorkerNum() const;
    Int32 GetMaxWorkerNum() const;
    Int32 GetRRIndex() const;
    bool IsDisablePost() const;

    LibString ToString() const;

    Poller *SelectPoller(bool priorityToUsingNewThread = false);

private:
    Poller *_SelectPoller(bool priorityToUsingNewThread = false);
    
private:
    const Int32 _minNum;
    const Int32 _maxNum;
    // 只允许递增, 不可减少
    std::atomic<Int32> _workingNum;
    std::vector<LibEventLoopThread *> _threads;
    std::atomic<Int32> _rrIndex;

    std::atomic_bool _start;
    std::atomic_bool _close;
    std::atomic_bool _disablePost;

    LibString _poolName;                // 池名
};

ALWAYS_INLINE Int32 LibEventLoopThreadPool::GetWorkerNum() const
{
    return _workingNum.load(std::memory_order_relaxed);
}

ALWAYS_INLINE Int32 LibEventLoopThreadPool::GetMaxWorkerNum() const
{
    return _maxNum;
}

ALWAYS_INLINE Int32 LibEventLoopThreadPool::GetRRIndex() const
{
    return _rrIndex.load(std::memory_order_relaxed);
}

ALWAYS_INLINE bool LibEventLoopThreadPool::IsDisablePost() const
{
    return _disablePost.load(std::memory_order_relaxed);
}

ALWAYS_INLINE LibString LibEventLoopThreadPool::ToString() const
{
    return LibString().AppendFormat("%s: workingNum:%d, maxNum:%d, rrIndex:%d, IsDisable:%d",
        _poolName.c_str(), GetWorkerNum(), GetMaxWorkerNum(), GetRRIndex(), IsDisablePost() ? 1 : 0);
}

ALWAYS_INLINE Poller *LibEventLoopThreadPool::SelectPoller(bool priorityToUsingNewThread)
{
    return _SelectPoller(priorityToUsingNewThread);
}


KERNEL_END                                      

#endif
