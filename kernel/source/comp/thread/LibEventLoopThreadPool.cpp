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
// Date: 2025-12-22 23:12:31
// Author: Eric Yonng
// Description:


#include <pch.h>
#include <kernel/comp/thread/LibEventLoopThreadPool.h>
#include <kernel/comp/Cpu/LibCpuInfo.h>
#include <kernel/comp/thread/LibEventLoopThread.h>
#include <kernel/comp/Log/log.h>

KERNEL_BEGIN

LibEventLoopThreadPool::LibEventLoopThreadPool()
    :_minNum(0)
,_maxNum(2 * LibCpuInfo::GetInstance()->GetCpuCoreCnt())    // 默认是CPU核数的2倍
,_workingNum{0}
,_rrIndex{0}
,_start{false}
,_close{false}
,_disablePost{false}
,_poolName("LibEventLoopThreadPool")
{
    _threads.resize(_maxNum, NULL);
}

LibEventLoopThreadPool::LibEventLoopThreadPool(Int32 minNum, Int32 maxNum)
    :_minNum(minNum < maxNum ? minNum : maxNum)
,_maxNum(maxNum >= minNum ? maxNum : minNum)
,_workingNum(minNum < maxNum ? minNum : maxNum)
,_rrIndex{0}
,_start{false}
,_close{false}
,_disablePost{false}
{
    _threads.resize(_maxNum, NULL);
    _poolName = "LibEventLoopThreadPool";
}

LibEventLoopThreadPool::~LibEventLoopThreadPool()
{
    Close();
}


void LibEventLoopThreadPool::Start()
{
    if(_start.exchange(true))
        return;

    auto workingNum = _workingNum.load(std::memory_order_acquire);
    if(workingNum == 0)
        return;

    for(Int32 idx = 0; idx < workingNum; ++idx)
    {
        auto t = _threads[idx];
        if(t == NULL)
            continue;
        
        _threads[idx] = new LibEventLoopThread(_poolName);
    }

    for(Int32 idx = 0; idx < workingNum; ++idx)
    {
        _threads[idx]->Start();

        // 等待直到线程poller设置完成
        Int32 count = 0;
        while (!_threads[idx]->GetPollerNoAsync())
        {
            // 让出cpu
            if(++count < 1000)
                continue;

            // 超过1000次让出cpu并打印日志
            KERNEL_NS::SystemUtil::YieldScheduler();
            count = 0;
            if(g_Log->IsEnable(LogLevel::Debug))
                g_Log->Debug(LOGFMT_OBJ_TAG("have no poller too long, newThread:%p in thread pool Start"), _threads[idx]);
        }
    }
}

void LibEventLoopThreadPool::Close()
{
    if(_close.exchange(true))
        return;

    HalfClose();
    FinishClose();
}

void LibEventLoopThreadPool::HalfClose()
{
    if(_disablePost.exchange(true))
        return;

    auto workingNum = _workingNum.load(std::memory_order_acquire);
    for(Int32 idx = 0; idx < workingNum; ++idx)
    {
        auto thread = _threads[idx];
        if(UNLIKELY(!thread))
            continue;
        
        thread->HalfClose();
    }
}

void LibEventLoopThreadPool::FinishClose()
{
    auto workingNum = _workingNum.load(std::memory_order_acquire);
    for(Int32 idx = 0; idx < workingNum; ++idx)
    {
        auto thread = _threads[idx];
        if(UNLIKELY(!thread))
            continue;
        
        thread->FinishClose();
    }

    auto cnt = static_cast<Int32>(_threads.size());
    for(Int32 idx = 0; idx < cnt; ++idx)
    {
        CRYSTAL_RELEASE_SAFE(_threads[idx]);
    }
}

ALWAYS_INLINE static bool _IsThreadIdle(LibEventLoopThread *thread)
{
    auto poller = thread->GetPollerNoAsync();
    if(UNLIKELY(!poller))
        return false;
    
    return poller->IsWaiting();
}

Poller *LibEventLoopThreadPool::_SelectPoller(bool priorityToUsingNewThread)
{
    auto workingNum = _workingNum.load(std::memory_order_acquire);
    // 1. 先寻找空闲的线程
    LibEventLoopThread *thread = NULL;
    if(LIKELY(!priorityToUsingNewThread))
    {
        for(Int32 idx = 0; idx < workingNum; ++idx)
        {
            auto t = _threads[idx];
            if(UNLIKELY(!t))
                continue;

            if(!_IsThreadIdle(t))
                continue;

            thread = t;
            break;
        }
    }

    // 2.如果线程都繁忙则创建新的线程
    if(UNLIKELY(!thread))
    {
        // 必须使用cas, 安全的更新 _workingNum, _workingNum 只允许递增, 不可减少
        auto newValue = workingNum >= _maxNum?_maxNum:(workingNum + 1);
        while (!_workingNum.compare_exchange_weak(workingNum, newValue, std::memory_order_acq_rel))
        {
            newValue = workingNum >= _maxNum ? _maxNum : (workingNum + 1);
        }

        // 线程数量递增, 则创建线程, 并选择新创建的线程
        if(newValue != workingNum)
        {
            auto newThread = new LibEventLoopThread(_poolName);
            newThread->Start();
            // 等待直到线程Poller设置完成
            Int32 count = 0;
            while (!newThread->GetPollerNoAsync())
            {
                // 让出cpu
                if(++count < 1000)
                    continue;

                // 超过1000次让出cpu并打印日志
                KERNEL_NS::SystemUtil::YieldScheduler();
                count = 0;
                if(g_Log->IsEnable(LogLevel::Debug))
                    g_Log->Debug(LOGFMT_OBJ_TAG("have no poller too long, newThread:%p in thread pool"), newThread);
            }
            
            _threads[newValue - 1] = newThread;
            thread = newThread;
        }
    }

    // 如果依然没有可选线程(都忙碌, 且不可创建新线程了, 则使用rr任意选择一个)
    if(UNLIKELY(!thread))
    {
        workingNum = _workingNum.load(std::memory_order_acquire);
        auto rrIndex = _rrIndex.load(std::memory_order_acquire);
        auto newIndex = rrIndex % workingNum;
        while (!_rrIndex.compare_exchange_weak(rrIndex, (newIndex + 1) % workingNum, std::memory_order_acq_rel))
        {
            newIndex = rrIndex % workingNum;
        }

        thread = _threads[newIndex];
    }

    return thread->GetPollerNoAsync();
}


KERNEL_END