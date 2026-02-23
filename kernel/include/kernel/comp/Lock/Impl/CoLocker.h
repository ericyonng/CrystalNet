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
// Date: 2026-02-22 13:02:59
// Author: Eric Yonng
// Description: 使用协程方式多线程同步(以便实现单线程复用生产者消费者模型)

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOCK_IMPL_CO_LOCKER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOCK_IMPL_CO_LOCKER_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/comp/Coroutines/CoTask.h>
#include <atomic>
#include <kernel/comp/TimeSlice.h>
#include <kernel/comp/Lock/Impl/SpinLock.h>
#include <unordered_map>
#include <vector>

KERNEL_BEGIN

class Poller;

// 单个线程不支持多个waiter
struct KERNEL_EXPORT CoLockerInfo
{
    POOL_CREATE_OBJ_DEFAULT(CoLockerInfo);

    CoLockerInfo()
        :IsSignal{false}
    ,_poller(NULL)
    {
        
    }
    ~CoLockerInfo()
    {
        
    }
    
    void Release()
    {
        CoLockerInfo::Delete_CoLockerInfo(this);
    }

    alignas(SYSTEM_ALIGN_SIZE) std::atomic_bool IsSignal;

    // 由waiterpoller控制
    KERNEL_NS::SmartPtr<KERNEL_NS::TaskParamRefWrapper, KERNEL_NS::AutoDelMethods::Release> _taskParam;
    Poller *_poller;
};

// 协程式同步(每个线程最多同时一个陷入Wait, 否则Wait函数返回CoLockerMultiWaiter)
// 退出必须先调用Quit, 并等待所有waiter退出, 避免未定义异常
class KERNEL_EXPORT CoLocker
{
public:
    // 最多waiter限制
    CoLocker(Int32 waiterLimit = 16);
    ~CoLocker();
    
    ALWAYS_INLINE void Lock(){}
    ALWAYS_INLINE void Unlock() {}
    ALWAYS_INLINE constexpr bool TryLock() { return true;}

    CoTask<Int32> Wait();
    CoTask<Int32> TimeWait(UInt64 second, UInt64 microSec);
    CoTask<Int32> TimeWait(UInt64 milliSecond);
    CoTask<Int32> TimeWait(TimeSlice slice);

    // >= 200W QPS
    bool Sinal();
    bool HasWaiter() const;
    void Broadcast();
    ALWAYS_INLINE void ResetSinal() {}
    ALWAYS_INLINE void ResetSinalFlag() {}
    bool IsSinal() const;
    bool IsDestroy() const;
    bool IsQuit() const;

    void Destroy();
    void Quit();
    
private:
    alignas(SYSTEM_ALIGN_SIZE) std::atomic_bool _isDestroy;
    alignas(SYSTEM_ALIGN_SIZE) std::atomic_bool _isQuit;
    alignas(SYSTEM_ALIGN_SIZE) std::atomic_bool _isSignal;

    alignas(SYSTEM_ALIGN_SIZE) std::atomic<Int32> _maxWaiterIndex;
    alignas(SYSTEM_ALIGN_SIZE) std::atomic<Int32> _curWaiterCount;

    alignas(SYSTEM_ALIGN_SIZE) std::atomic<UInt32> _incId;
    mutable alignas(SYSTEM_ALIGN_SIZE) std::atomic<Int32> _working;

    alignas(SYSTEM_ALIGN_SIZE) std::vector<std::atomic<CoLockerInfo *> *> _waiters;

    Poller *_ownerPoller;
    SpinLock _lock;
};

ALWAYS_INLINE bool CoLocker::HasWaiter() const
{
    return _curWaiterCount.load(std::memory_order_acquire) != 0;
}

ALWAYS_INLINE bool CoLocker::IsDestroy() const
{
    return _isDestroy.load(std::memory_order_acquire);
}

ALWAYS_INLINE bool CoLocker::IsQuit() const
{
    return _isQuit.load(std::memory_order_acquire);
}

ALWAYS_INLINE void CoLocker::Quit()
{
    _isQuit.store(true, std::memory_order_release);
}



KERNEL_END

#endif


