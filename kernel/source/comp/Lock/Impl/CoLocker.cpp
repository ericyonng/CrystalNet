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
// Date: 2026-02-22 14:02:12
// Author: Eric Yonng
// Description:


#include <pch.h>
#include <kernel/comp/Lock/Impl/CoLocker.h>

#include <kernel/comp/Poller/Poller.h>
#include <kernel/comp/ObjLife.h>
#include <kernel/comp/Log/log.h>


KERNEL_BEGIN

CoLocker::CoLocker(Int32 waiterLimit)
    :_isDestroy{false}
,_isQuit{false}
,_isSignal{false}
,_maxWaiterIndex{0}
,_curWaiterCount{0}
,_incId{0}
,_working{0}
,_ownerPoller(KERNEL_NS::TlsUtil::GetPoller())
{
    // 不能为0
    if(waiterLimit <= 0)
        waiterLimit = 16;

    _waiters.reserve(waiterLimit);
    for(Int32 idx = 0; idx < waiterLimit; ++idx)
    {
        _waiters.push_back(new std::atomic<CoLockerInfo *>{NULL});
    }

    _maxWaiterIndex.store(-1, std::memory_order_release);
}

CoLocker::~CoLocker()
{
    Destroy();
}

CoTask<Int32> CoLocker::Wait()
{
   auto poller = KERNEL_NS::TlsUtil::GetPoller();
    
    // 不同poller需要生命周期控制
    KERNEL_NS::SmartPtr<ObjLife<std::atomic<Int32>>, KERNEL_NS::AutoDelMethods::CustomDelete> lifeControl;
    if(poller != _ownerPoller)
    {
        lifeControl = ObjLife<std::atomic<Int32>>::NewThreadLocal_ObjLife(_working);
        if(lifeControl)
        {
            lifeControl.SetClosureDelegate([](void *p)
            {
                ObjLife<std::atomic<Int32>>::DeleteThreadLocal_ObjLife(KERNEL_NS::KernelCastTo<ObjLife<std::atomic<Int32>>>(p));
            });
        }
    }

    if(UNLIKELY(_isQuit.load(std::memory_order_acquire)))
        co_return Status::CoLockerDestroy;
    
    // waiter数量
    ObjLife<std::atomic<Int32>> waiterControl(_curWaiterCount);
    
    KERNEL_NS::SmartPtr<KERNEL_NS::TaskParamRefWrapper, KERNEL_NS::AutoDelMethods::Release> params = KERNEL_NS::TaskParamRefWrapper::NewThreadLocal_TaskParamRefWrapper();
    CoLockerInfo *coLockerInfo = NULL;
    _lock.Lock();
    auto len = static_cast<Int32>(_waiters.size());
    Int32 firstNullIdx = -1;
    Int32 findPollerIdx = -1;
    for(Int32 idx = 0; idx < len; ++idx)
    {
        auto ptr = _waiters[idx]->load(std::memory_order_relaxed);
        if((firstNullIdx == -1) && (ptr == NULL))
            firstNullIdx = idx;

        if(ptr == NULL)
            continue;

        if(ptr->_poller == poller)
        {
            findPollerIdx = idx;
            break;
        }
    }
    if(findPollerIdx == -1)
    {
        coLockerInfo = CoLockerInfo::New_CoLockerInfo();
        coLockerInfo->_taskParam = params;
        coLockerInfo->_poller = poller;
        
        if(firstNullIdx != -1)
        {
            _waiters[firstNullIdx]->store(coLockerInfo, std::memory_order_release);
            auto lastIndex = _maxWaiterIndex.load(std::memory_order_relaxed);

            // 变更最大索引
            while (true)
            {
                if(firstNullIdx <= lastIndex)
                    break;

                if(_maxWaiterIndex.compare_exchange_weak(lastIndex, firstNullIdx,  std::memory_order_acq_rel))
                    break;
            }
        }
        else
        {
            CRYSTAL_TRACE("Co Locker waiter over limit thread id:%lld, limit:%d, curWaiter:%d"
                , KERNEL_NS::SystemUtil::GetCurrentThreadId(), static_cast<Int32>(_waiters.size()), _curWaiterCount.load(std::memory_order_acquire));
            
            _lock.Unlock();
            co_return Status::CoLockerOverMaxWaiterLimit;
        }
    }
    else
    {
        coLockerInfo = _waiters[findPollerIdx]->load(std::memory_order_relaxed);
        
        if(coLockerInfo->_taskParam && coLockerInfo->_taskParam->_params)
        {
            CRYSTAL_TRACE("Co Locker multi waiter thread id:%lld", KERNEL_NS::SystemUtil::GetCurrentThreadId());
            
            _lock.Unlock();
            co_return Status::CoLockerMultiWaiter;
        }

        coLockerInfo->_taskParam = params;
    }
    _lock.Unlock();

    do
    {
        // 有没信号过来
        if(_isSignal.exchange(false, std::memory_order_acq_rel))
            break;
        
        if(coLockerInfo->IsSignal.exchange(false, std::memory_order_acq_rel))
            break;

        // 等待信号
        co_await KERNEL_NS::Waiting().GetParam(coLockerInfo->_taskParam);

        coLockerInfo->Version.fetch_add(1, std::memory_order_release);
    }
    while (false);
    
    coLockerInfo->IsSignal.exchange(false, std::memory_order_release);
    _isSignal.exchange(false, std::memory_order_release);

    if(LIKELY(coLockerInfo->_taskParam && coLockerInfo->_taskParam->_params))
    {
        auto &pa = coLockerInfo->_taskParam->_params; 
        if(pa->_errCode != Status::Success)
        {
            if (g_Log)
            {
                CLOG_WARN("waiting err:%d", pa->_errCode);
            }
        }

        // 销毁waiting协程
        if(pa->_handle)
            pa->_handle->DestroyHandle(pa->_errCode);
    }

    // TODO:测试安全释放param
    _lock.Lock();
    auto count = static_cast<Int32>(_waiters.size());
    for(Int32 idx = 0; idx < count; ++idx)
    {
        auto ptr = _waiters[idx]->load(std::memory_order_relaxed);
        if(ptr == NULL)
            continue;

        if(ptr->_poller == poller)
        {
            if(ptr->_taskParam)
            {
                ptr->_taskParam->_params = NULL;
            }
            break;
        }
    }
    _lock.Unlock();

    co_return IsQuit()? Status::CoLockerDestroy : Status::Success;
}

CoTask<Int32> CoLocker::TimeWait(UInt64 second, UInt64 microSec)
{
    auto poller = KERNEL_NS::TlsUtil::GetPoller();
    
    // 不同poller需要生命周期控制
    KERNEL_NS::SmartPtr<ObjLife<std::atomic<Int32>>, KERNEL_NS::AutoDelMethods::CustomDelete> lifeControl;
    if(poller != _ownerPoller)
    {
        lifeControl = ObjLife<std::atomic<Int32>>::NewThreadLocal_ObjLife(_working);
        if(lifeControl)
        {
            lifeControl.SetClosureDelegate([](void *p)
            {
                ObjLife<std::atomic<Int32>>::DeleteThreadLocal_ObjLife(KERNEL_NS::KernelCastTo<ObjLife<std::atomic<Int32>>>(p));
            });
        }
    }

    if(UNLIKELY(_isQuit.load(std::memory_order_acquire)))
        co_return Status::CoLockerDestroy;
    
    // waiter数量
    ObjLife<std::atomic<Int32>> waiterControl(_curWaiterCount);
    
    KERNEL_NS::SmartPtr<KERNEL_NS::TaskParamRefWrapper, KERNEL_NS::AutoDelMethods::Release> params = KERNEL_NS::TaskParamRefWrapper::NewThreadLocal_TaskParamRefWrapper();
    CoLockerInfo *coLockerInfo = NULL;
    _lock.Lock();
    auto len = static_cast<Int32>(_waiters.size());
    Int32 firstNullIdx = -1;
    Int32 findPollerIdx = -1;
    for(Int32 idx = 0; idx < len; ++idx)
    {
        auto ptr = _waiters[idx]->load(std::memory_order_relaxed);
        if((firstNullIdx == -1) && (ptr == NULL))
            firstNullIdx = idx;

        if(ptr == NULL)
            continue;

        if(ptr->_poller == poller)
        {
            findPollerIdx = idx;
            break;
        }
    }
    if(findPollerIdx == -1)
    {
        coLockerInfo = CoLockerInfo::New_CoLockerInfo();
        coLockerInfo->_taskParam = params;
        coLockerInfo->_poller = poller;
        
        if(firstNullIdx != -1)
        {
            _waiters[firstNullIdx]->store(coLockerInfo, std::memory_order_release);
            auto lastIndex = _maxWaiterIndex.load(std::memory_order_relaxed);

            // 变更最大索引
            while (true)
            {
                if(firstNullIdx <= lastIndex)
                    break;

                if(_maxWaiterIndex.compare_exchange_weak(lastIndex, firstNullIdx,  std::memory_order_acq_rel))
                    break;
            }
        }
        else
        {
            CRYSTAL_TRACE("Co Locker waiter over limit thread id:%lld, limit:%d, curWaiter:%d"
                , KERNEL_NS::SystemUtil::GetCurrentThreadId(), static_cast<Int32>(_waiters.size()), _curWaiterCount.load(std::memory_order_acquire));
            
            _lock.Unlock();
            co_return Status::CoLockerOverMaxWaiterLimit;
        }
    }
    else
    {
        coLockerInfo = _waiters[findPollerIdx]->load(std::memory_order_relaxed);
        
        if(coLockerInfo->_taskParam && coLockerInfo->_taskParam->_params)
        {
            CRYSTAL_TRACE("Co Locker multi waiter thread id:%lld", KERNEL_NS::SystemUtil::GetCurrentThreadId());
            
            _lock.Unlock();
            co_return Status::CoLockerMultiWaiter;
        }

        coLockerInfo->_taskParam = params;
    }
    _lock.Unlock();

    do
    {
        // 有没信号过来
        if(_isSignal.exchange(false, std::memory_order_acq_rel))
            break;
        
        if(coLockerInfo->IsSignal.exchange(false, std::memory_order_acq_rel))
            break;

        // 等待信号
        co_await KERNEL_NS::Waiting(KERNEL_NS::TimeSlice::FromSeconds(static_cast<Int64>(second)) + TimeSlice::FromMicroSeconds(static_cast<Int64>(microSec))).GetParam(coLockerInfo->_taskParam);

        coLockerInfo->Version.fetch_add(1, std::memory_order_release);
    }
    while (false);
    
    coLockerInfo->IsSignal.exchange(false, std::memory_order_release);
    _isSignal.exchange(false, std::memory_order_release);

    if(LIKELY(coLockerInfo->_taskParam && coLockerInfo->_taskParam->_params))
    {
        auto &pa = coLockerInfo->_taskParam->_params; 
        if(pa->_errCode != Status::Success)
        {
            if (g_Log)
            {
                CLOG_WARN("waiting err:%d", pa->_errCode);
            }
        }

        // 销毁waiting协程
        if(pa->_handle)
            pa->_handle->DestroyHandle(pa->_errCode);
    }

    // TODO:测试安全释放param
    _lock.Lock();
    auto count = static_cast<Int32>(_waiters.size());
    for(Int32 idx = 0; idx < count; ++idx)
    {
        auto ptr = _waiters[idx]->load(std::memory_order_relaxed);
        if(ptr == NULL)
            continue;

        if(ptr->_poller == poller)
        {
            if(ptr->_taskParam)
            {
                ptr->_taskParam->_params = NULL;
            }
            break;
        }
    }
    _lock.Unlock();

    co_return IsQuit()? Status::CoLockerDestroy : Status::Success;
}

CoTask<Int32> CoLocker::TimeWait(UInt64 milliSecond)
{
   auto poller = KERNEL_NS::TlsUtil::GetPoller();
    
    // 不同poller需要生命周期控制
    KERNEL_NS::SmartPtr<ObjLife<std::atomic<Int32>>, KERNEL_NS::AutoDelMethods::CustomDelete> lifeControl;
    if(poller != _ownerPoller)
    {
        lifeControl = ObjLife<std::atomic<Int32>>::NewThreadLocal_ObjLife(_working);
        if(lifeControl)
        {
            lifeControl.SetClosureDelegate([](void *p)
            {
                ObjLife<std::atomic<Int32>>::DeleteThreadLocal_ObjLife(KERNEL_NS::KernelCastTo<ObjLife<std::atomic<Int32>>>(p));
            });
        }
    }

    if(UNLIKELY(_isQuit.load(std::memory_order_acquire)))
        co_return Status::CoLockerDestroy;
    
    // waiter数量
    ObjLife<std::atomic<Int32>> waiterControl(_curWaiterCount);
    
    KERNEL_NS::SmartPtr<KERNEL_NS::TaskParamRefWrapper, KERNEL_NS::AutoDelMethods::Release> params = KERNEL_NS::TaskParamRefWrapper::NewThreadLocal_TaskParamRefWrapper();
    CoLockerInfo *coLockerInfo = NULL;
    _lock.Lock();
    auto len = static_cast<Int32>(_waiters.size());
    Int32 firstNullIdx = -1;
    Int32 findPollerIdx = -1;
    for(Int32 idx = 0; idx < len; ++idx)
    {
        auto ptr = _waiters[idx]->load(std::memory_order_relaxed);
        if((firstNullIdx == -1) && (ptr == NULL))
            firstNullIdx = idx;

        if(ptr == NULL)
            continue;

        if(ptr->_poller == poller)
        {
            findPollerIdx = idx;
            break;
        }
    }
    if(findPollerIdx == -1)
    {
        coLockerInfo = CoLockerInfo::New_CoLockerInfo();
        coLockerInfo->_taskParam = params;
        coLockerInfo->_poller = poller;
        
        if(firstNullIdx != -1)
        {
            _waiters[firstNullIdx]->store(coLockerInfo, std::memory_order_release);
            auto lastIndex = _maxWaiterIndex.load(std::memory_order_relaxed);

            // 变更最大索引
            while (true)
            {
                if(firstNullIdx <= lastIndex)
                    break;

                if(_maxWaiterIndex.compare_exchange_weak(lastIndex, firstNullIdx,  std::memory_order_acq_rel))
                    break;
            }
        }
        else
        {
            CRYSTAL_TRACE("Co Locker waiter over limit thread id:%lld, limit:%d, curWaiter:%d"
                , KERNEL_NS::SystemUtil::GetCurrentThreadId(), static_cast<Int32>(_waiters.size()), _curWaiterCount.load(std::memory_order_acquire));
            
            _lock.Unlock();
            co_return Status::CoLockerOverMaxWaiterLimit;
        }
    }
    else
    {
        coLockerInfo = _waiters[findPollerIdx]->load(std::memory_order_relaxed);
        
        if(coLockerInfo->_taskParam && coLockerInfo->_taskParam->_params)
        {
            CRYSTAL_TRACE("Co Locker multi waiter thread id:%lld", KERNEL_NS::SystemUtil::GetCurrentThreadId());
            
            _lock.Unlock();
            co_return Status::CoLockerMultiWaiter;
        }

        coLockerInfo->_taskParam = params;
    }
    _lock.Unlock();

    do
    {
        // 有没信号过来
        if(_isSignal.exchange(false, std::memory_order_acq_rel))
            break;
        
        if(coLockerInfo->IsSignal.exchange(false, std::memory_order_acq_rel))
            break;
        
        // 等待信号
        co_await KERNEL_NS::Waiting(TimeSlice::FromMilliSeconds(static_cast<Int64>(milliSecond))).GetParam(coLockerInfo->_taskParam);

        // 更新版本号
        coLockerInfo->Version.fetch_add(1, std::memory_order_release);
    }
    while (false);

    coLockerInfo->IsSignal.exchange(false, std::memory_order_release);
    _isSignal.exchange(false, std::memory_order_release);

    if(LIKELY(coLockerInfo->_taskParam && coLockerInfo->_taskParam->_params))
    {
        auto &pa = coLockerInfo->_taskParam->_params; 
        if(pa->_errCode != Status::Success)
        {
            if (g_Log)
            {
                CLOG_WARN("waiting err:%d", pa->_errCode);
            }
        }

        // 销毁waiting协程
        if(pa->_handle)
            pa->_handle->DestroyHandle(pa->_errCode);
    }

    // TODO:测试安全释放param
    _lock.Lock();
    auto count = static_cast<Int32>(_waiters.size());
    for(Int32 idx = 0; idx < count; ++idx)
    {
        auto ptr = _waiters[idx]->load(std::memory_order_relaxed);
        if(ptr == NULL)
            continue;

        if(ptr->_poller == poller)
        {
            if(ptr->_taskParam)
            {
                ptr->_taskParam->_params = NULL;
            }
            break;
        }
    }
    _lock.Unlock();

    co_return IsQuit()? Status::CoLockerDestroy : Status::Success;
}

CoTask<Int32> CoLocker::TimeWait(TimeSlice slice)
{
   auto poller = KERNEL_NS::TlsUtil::GetPoller();
    
    // 不同poller需要生命周期控制
    KERNEL_NS::SmartPtr<ObjLife<std::atomic<Int32>>, KERNEL_NS::AutoDelMethods::CustomDelete> lifeControl;
    if(poller != _ownerPoller)
    {
        lifeControl = ObjLife<std::atomic<Int32>>::NewThreadLocal_ObjLife(_working);
        if(lifeControl)
        {
            lifeControl.SetClosureDelegate([](void *p)
            {
                ObjLife<std::atomic<Int32>>::DeleteThreadLocal_ObjLife(KERNEL_NS::KernelCastTo<ObjLife<std::atomic<Int32>>>(p));
            });
        }
    }

    if(UNLIKELY(_isQuit.load(std::memory_order_acquire)))
        co_return Status::CoLockerDestroy;
    
    // waiter数量
    ObjLife<std::atomic<Int32>> waiterControl(_curWaiterCount);
    
    KERNEL_NS::SmartPtr<KERNEL_NS::TaskParamRefWrapper, KERNEL_NS::AutoDelMethods::Release> params = KERNEL_NS::TaskParamRefWrapper::NewThreadLocal_TaskParamRefWrapper();
    CoLockerInfo *coLockerInfo = NULL;
    _lock.Lock();
    auto len = static_cast<Int32>(_waiters.size());
    Int32 firstNullIdx = -1;
    Int32 findPollerIdx = -1;
    for(Int32 idx = 0; idx < len; ++idx)
    {
        auto ptr = _waiters[idx]->load(std::memory_order_relaxed);
        if((firstNullIdx == -1) && (ptr == NULL))
            firstNullIdx = idx;

        if(ptr == NULL)
            continue;

        if(ptr->_poller == poller)
        {
            findPollerIdx = idx;
            break;
        }
    }
    if(findPollerIdx == -1)
    {
        coLockerInfo = CoLockerInfo::New_CoLockerInfo();
        coLockerInfo->_taskParam = params;
        coLockerInfo->_poller = poller;
        
        if(firstNullIdx != -1)
        {
            _waiters[firstNullIdx]->store(coLockerInfo, std::memory_order_release);
            auto lastIndex = _maxWaiterIndex.load(std::memory_order_relaxed);

            // 变更最大索引
            while (true)
            {
                if(firstNullIdx <= lastIndex)
                    break;

                if(_maxWaiterIndex.compare_exchange_weak(lastIndex, firstNullIdx,  std::memory_order_acq_rel))
                    break;
            }
        }
        else
        {
            CRYSTAL_TRACE("Co Locker waiter over limit thread id:%lld, limit:%d, curWaiter:%d"
                , KERNEL_NS::SystemUtil::GetCurrentThreadId(), static_cast<Int32>(_waiters.size()), _curWaiterCount.load(std::memory_order_acquire));
            
            _lock.Unlock();
            co_return Status::CoLockerOverMaxWaiterLimit;
        }
    }
    else
    {
        coLockerInfo = _waiters[findPollerIdx]->load(std::memory_order_relaxed);
        
        if(coLockerInfo->_taskParam && coLockerInfo->_taskParam->_params)
        {
            CRYSTAL_TRACE("Co Locker multi waiter thread id:%lld", KERNEL_NS::SystemUtil::GetCurrentThreadId());
            
            _lock.Unlock();
            co_return Status::CoLockerMultiWaiter;
        }

        coLockerInfo->_taskParam = params;
    }
    _lock.Unlock();

    do
    {
        // 有没信号过来
        if(_isSignal.exchange(false, std::memory_order_acq_rel))
            break;
        
        if(coLockerInfo->IsSignal.exchange(false, std::memory_order_acq_rel))
            break;

        // CLOG_DEBUG("[Wait Slice] will waiting %p version:%llu", coLockerInfo, coLockerInfo->Version.load(std::memory_order_relaxed));

        // 等待信号
        co_await KERNEL_NS::Waiting(slice).GetParam(coLockerInfo->_taskParam);

        coLockerInfo->Version.fetch_add(1, std::memory_order_release);

        // CLOG_DEBUG("[Wait Slice] wake up %p version:%llu", coLockerInfo, coLockerInfo->Version.load(std::memory_order_relaxed));
    }
    while (false);
    
    coLockerInfo->IsSignal.exchange(false, std::memory_order_release);
    _isSignal.exchange(false, std::memory_order_release);

    if(LIKELY(coLockerInfo->_taskParam && coLockerInfo->_taskParam->_params))
    {
        auto &pa = coLockerInfo->_taskParam->_params; 
        if(pa->_errCode != Status::Success)
        {
            if (g_Log)
            {
                CLOG_WARN("waiting err:%d", pa->_errCode);
            }
        }

        // 销毁waiting协程
        if(pa->_handle)
            pa->_handle->DestroyHandle(pa->_errCode);
    }

    // TODO:测试安全释放param
    _lock.Lock();
    auto count = static_cast<Int32>(_waiters.size());
    for(Int32 idx = 0; idx < count; ++idx)
    {
        auto ptr = _waiters[idx]->load(std::memory_order_relaxed);
        if(ptr == NULL)
            continue;

        if(ptr->_poller == poller)
        {
            if(ptr->_taskParam)
            {
                ptr->_taskParam->_params = NULL;
            }
            break;
        }
    }
    _lock.Unlock();

    co_return IsQuit()? Status::CoLockerDestroy : Status::Success;
}


bool CoLocker::Sinal()
{
    // _working生命周期传递
    ObjLife<std::atomic<Int32>> objLifeControl(_working);

    if(UNLIKELY(_isDestroy.load(std::memory_order_acquire)))
        return true;
    
    if(_isSignal.exchange(true, std::memory_order_release))
        return true;

    if(_curWaiterCount.load(std::memory_order_acquire) == 0)
        return true;

    auto maxWaiterIndex = _maxWaiterIndex.load(std::memory_order_acquire);
    if(maxWaiterIndex < 0)
        return true;

    // 唤醒其中之一
    auto magicNum = _incId.fetch_add(1, std::memory_order_acq_rel);
    auto count = maxWaiterIndex + 1;
    auto left = magicNum % count;
    auto atomicPtr = _waiters[left];

    auto coLockerInfo = atomicPtr->load(std::memory_order_acquire);
    if(UNLIKELY(!coLockerInfo))
    {
        // 找不到, 遍历所有找到一个
        for(Int32 idx = 0; idx < count; ++idx)
        {
            auto next = (left + idx) % count;
            auto nextLockerInfo = _waiters[next]->load(std::memory_order_acquire);
            if(!nextLockerInfo)
                continue;

            coLockerInfo = nextLockerInfo;
            break;
        }

        if(UNLIKELY(!coLockerInfo))
            return true;
    }
    
    auto poller = coLockerInfo->_poller;
    if(UNLIKELY(poller == NULL))
        return true;

    // TODO:测试taskParam的生命周期
    coLockerInfo->IsSignal.store(true, std::memory_order_release);
    auto version = coLockerInfo->Version.load(std::memory_order_acquire);

    // CLOG_DEBUG("[Signal] lockerInfo:%p, version:%llu", coLockerInfo, version);

    if(poller == _ownerPoller)
    {
        // 同一线程        
        poller->Push([taskParam = coLockerInfo->_taskParam, version, coLockerInfo]()
        {
            if(taskParam && taskParam->_params)
            {
                auto &pa = taskParam->_params;
                // CLOG_DEBUG_GLOBAL(CoLocker, "coLockerInfo:%p TRACE:%p-%s, handle:%p-%llu timeout:%p-%s version:%llu, curVersion:%llu"
                //     , coLockerInfo, pa->_trace, pa->_trace? (pa->_trace->ToString().c_str()) : ""
                //     , pa->_handle, pa->_handle ? pa->_handle->GetHandleId() : 0
                //     , pa->_timeout, pa->_timeout? pa->_timeout->ToString().c_str(): "", version, coLockerInfo->Version.load(std::memory_order_relaxed));

                // _params不为空, 那么coLockerInfo必然还没销毁, 且必然不存在其他线程销毁coLockerInfo可能性,版本号不同, 那么必然在signal之后一段时间被唤醒过, 不需要再唤醒
                if (coLockerInfo && coLockerInfo->Version.load(std::memory_order_relaxed) != version)
                    return;

                if (pa->_timeout)
                    pa->_timeout->Cancel();
                
                if(pa->_handle)
                    pa->_handle->ForceAwake();
            }
        });
    }
    // 与_ownerPoller不同线程需要控制objLifeControl
    else
    {

        poller->Push([version, coLockerInfo, objLifeControl]()
        {
            if(coLockerInfo->_taskParam && coLockerInfo->_taskParam->_params)
            {
                auto &pa = coLockerInfo->_taskParam->_params;
                // CLOG_DEBUG_GLOBAL(CoLocker, "coLockerInfo:%p, TRACE:%p-%s, handle:%p-%llu timeout:%p-%s version:%llu, curVersion:%llu"
                //     , coLockerInfo, pa->_trace, pa->_trace? (pa->_trace->ToString().c_str()) : ""
                //     , pa->_handle, pa->_handle ? pa->_handle->GetHandleId() : 0
                //     , pa->_timeout, pa->_timeout? pa->_timeout->ToString().c_str(): "", version, coLockerInfo->Version.load(std::memory_order_relaxed));

                // 版本号不同,说明在signal后, waiter已经醒过, 不需要再唤醒了,避免多次唤醒
                if (version == coLockerInfo->Version.load(std::memory_order_relaxed))
                {
                    if (pa->_timeout)
                        pa->_timeout->Cancel();
                
                    if(pa->_handle)
                        pa->_handle->ForceAwake();
                }
            }
        });
    }

    return true;
}

void CoLocker::Broadcast()
{
    // _working生命周期传递
    ObjLife<std::atomic<Int32>> objLifeControl(_working);

    if(UNLIKELY(_isDestroy.load(std::memory_order_acquire)))
        return;

    auto maxWaterIndex = _maxWaiterIndex.load(std::memory_order_acquire);
    if(UNLIKELY(maxWaterIndex < 0))
        return;

    auto count = maxWaterIndex + 1;

    for(Int32 idx = 0; idx < count; ++idx)
    {
        auto atomicPtr = _waiters[idx];

        auto coLockerInfo = atomicPtr->load(std::memory_order_acquire);
        if(!coLockerInfo)
            continue;

        auto poller = coLockerInfo->_poller;
        if(UNLIKELY(poller == NULL))
            continue;

        // TODO:测试taskParam的生命周期
        coLockerInfo->IsSignal.store(true, std::memory_order_release);
        auto version = coLockerInfo->Version.load(std::memory_order_acquire);

        if(poller == _ownerPoller)
        {
            // 同一线程
            poller->Push([taskParam = coLockerInfo->_taskParam, version, coLockerInfo]()
            {
                if(taskParam && taskParam->_params)
                {
                    auto &pa = taskParam->_params;
                    // _params不为空, 那么coLockerInfo必然还没销毁, 且必然不存在其他线程销毁coLockerInfo可能性,版本号不同, 那么必然在signal之后一段时间被唤醒过, 不需要再唤醒
                    if (coLockerInfo && coLockerInfo->Version.load(std::memory_order_relaxed) != version)
                        return;

                    if (pa->_timeout)
                        pa->_timeout->Cancel();
        
                    if(pa->_handle)
                        pa->_handle->ForceAwake();
                }
            });
        }
        // 与_ownerPoller不同线程需要控制objLifeControl
        else
        {
            poller->Push([version, coLockerInfo, objLifeControl]()
            {
                if(coLockerInfo->_taskParam && coLockerInfo->_taskParam->_params)
                {
                    auto &pa = coLockerInfo->_taskParam->_params;
                    // CLOG_DEBUG_GLOBAL(CoLocker, "coLockerInfo:%p, TRACE:%p-%s, handle:%p-%llu timeout:%p-%s version:%llu, curVersion:%llu"
                    //     , coLockerInfo, pa->_trace, pa->_trace? (pa->_trace->ToString().c_str()) : ""
                    //     , pa->_handle, pa->_handle ? pa->_handle->GetHandleId() : 0
                    //     , pa->_timeout, pa->_timeout? pa->_timeout->ToString().c_str(): "", version, coLockerInfo->Version.load(std::memory_order_relaxed));

                    // 版本号不同,说明在signal后, waiter已经醒过, 不需要再唤醒了,避免多次唤醒
                    if (version == coLockerInfo->Version.load(std::memory_order_relaxed))
                    {
                        if (pa->_timeout)
                            pa->_timeout->Cancel();
                
                        if(pa->_handle)
                            pa->_handle->ForceAwake();
                    }
                }
            });
        }
    }
}

bool CoLocker::IsSinal() const
{
    // _working生命周期传递
    ObjLife<std::atomic<Int32>> objLifeControl(_working);

    if(UNLIKELY(_isDestroy.load(std::memory_order_acquire)))
        return true;

    if(_isSignal.load(std::memory_order_acquire))
        return true;

    auto maxWaiterIndex = _maxWaiterIndex.load(std::memory_order_acquire);
    if(UNLIKELY(maxWaiterIndex < 0))
        return true;

    auto count = maxWaiterIndex + 1;
    for(Int32 idx = 0; idx < count; ++idx)
    {
        auto atomicPtr = _waiters[idx];
        auto coLockerInfo = atomicPtr->load(std::memory_order_acquire);
        if(!coLockerInfo)
            continue;

        if(coLockerInfo->IsSignal.load(std::memory_order_acquire))
            return true;
    }

    return false;
}

void CoLocker::Destroy()
{
    _isQuit.store(true, std::memory_order_release);
    
    if(_isDestroy.exchange(true, std::memory_order_acq_rel))
        return;

    auto curPoller = KERNEL_NS::TlsUtil::GetPoller();
    
    CRYSTAL_TRACE("will deconstructing colocker:%p, cur poller:%s, ownerPoller:%s", this, curPoller->ToString().c_str(), _ownerPoller->ToString().c_str());

    if(UNLIKELY(curPoller != _ownerPoller))
    {
        CRYSTAL_TRACE("deconstructing CoLocker not in owner poller cur poller:%s, ownerPoller:%s", curPoller->ToString().c_str(), _ownerPoller->ToString().c_str());
    }
    
    // 先将当前poller唤醒
    auto maxWaiterIndex = _maxWaiterIndex.load(std::memory_order_acquire);
    if(maxWaiterIndex >= 0)
    {
        for(Int32 idx = 0; idx <= maxWaiterIndex; ++idx)
        {
            auto atomicPtr = _waiters[idx];
            auto coLockerInfo = atomicPtr->load(std::memory_order_acquire);
            if(coLockerInfo == NULL)
                continue;

            if(coLockerInfo->_poller != _ownerPoller)
                continue;

            if(curPoller != _ownerPoller)
                continue;

            // 当前线程
            if(coLockerInfo->_taskParam && coLockerInfo->_taskParam->_params)
            {
                auto &pa = coLockerInfo->_taskParam->_params;
                if(pa->_handle)
                    pa->_handle->ForceAwake();

                coLockerInfo->_taskParam->_params = NULL;
            }
        }
    }

    // 等待其他线程退出
    while (_working.load(std::memory_order_acquire) != 0)
    {
        CRYSTAL_TRACE("waiting for waiter quit... working num:%d, co locker:%p", _working.load(std::memory_order_acquire), this);

        Broadcast();
        KERNEL_NS::SystemUtil::ThreadSleep(5);
    }

    _curWaiterCount.exchange(0, std::memory_order_release);
    _maxWaiterIndex.exchange(-1, std::memory_order_release);

    // 销毁 _waiters
    auto waiterCount = static_cast<Int32>(_waiters.size());
    for(Int32 idx = 0; idx < waiterCount; ++idx)
    {
        auto atomicPtr = _waiters[idx];

        auto coLockerInfo = atomicPtr->load(std::memory_order_acquire);
        if(!coLockerInfo)
            continue;
        
        coLockerInfo->Release();
        delete atomicPtr;
    }
    _waiters.clear();
}

KERNEL_END