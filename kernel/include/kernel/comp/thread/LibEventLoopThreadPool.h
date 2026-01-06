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

    void Start();
    void Close();
    void HalfClose();
    void FinishClose();

    // 调用者当前线程投递req给this
    // req暂时只能传指针，而且会在otherChannel（可能不同线程）释放
    // req/res 必须实现Release, ToString接口
    template<typename ReqType>
    requires requires(ReqType req)
    {
        // req/res必须有Release接口
        req.Release();
    
        // req/res必须有ToString接口
        req.ToString();
    }
    void Send(ReqType *req)
    {
        if(UNLIKELY(_disablePost.load(std::memory_order_acquire)))
        {
            req->Release();
            return;
        }
        
        auto poller = _SelectPoller();
        poller->Send(req);
    }

    template<typename ReqType>
    requires requires(ReqType req)
    {
        // req/res必须有Release接口
        req.Release();
    
        // req/res必须有ToString接口
        req.ToString();
    }
    CoTask<> SendAsync(ReqType *req)
    {
        if(UNLIKELY(_disablePost.load(std::memory_order_acquire)))
        {
            req->Release();
            co_return;
        }
        
        CRYSTAL_CO_COMPLETED();
        
        auto poller = _SelectPoller();
        poller->Send(req);
    }
    
    template<typename ResType, typename ReqType>
    requires requires(ReqType req, ResType res)
    {
        // req/res必须有Release接口
        req.Release();
        res.Release();
    
        // req/res必须有ToString接口
        req.ToString();
        res.ToString();
    }
    CoTask<KERNEL_NS::SmartPtr<ResType, AutoDelMethods::Release>> SendAsync2(ReqType *req)
    {
        // 不允许发送了
        if(UNLIKELY(_disablePost.load(std::memory_order_acquire)))
        {
            req->Release();
            co_return KERNEL_NS::SmartPtr<ResType, AutoDelMethods::Release>(NULL);
        }
        
        auto poller = _SelectPoller();
        co_return co_await poller->template SendAsync<ResType, ReqType>(req);
    }

    Int32 GetWorkerNum() const;
    Int32 GetMaxWorkerNum() const;
    Int32 GetRRIndex() const;
    bool IsDisablePost() const;

    LibString ToString() const;

private:
    Poller *_SelectPoller();
    
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

KERNEL_END                                      

#endif
