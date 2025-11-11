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
 * Date: 2022-03-23 11:59:36
 * Author: Eric Yonng
 * Description: 
 * 请遵循one thread one loop 原则，一个线程不可出现多个Poller，只能有一个
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_POLLER_POLLER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_POLLER_POLLER_H__

#pragma once

#include <kernel/comp/CompObject/CompObject.h>
#include <kernel/comp/Delegate/LibDelegate.h>
#include <kernel/common/LibObject.h>

#include <kernel/comp/Lock/Impl/ConditionLocker.h>
#include <kernel/comp/Lock/Impl/SpinLock.h>
#include <kernel/comp/LibList.h>
#include <kernel/comp/ConcurrentPriorityQueue/ConcurrentPriorityQueue.h>
#include <kernel/comp/Cpu/LibCpuCounter.h>
#include <kernel/comp/LibList.h>

#include <unordered_map>
#include <atomic>
#include <kernel/comp/Coroutines/CoTask.h>

#include "kernel/comp/Coroutines/CoWaiter.h"
#include <kernel/comp/SmartPtr.h>


KERNEL_BEGIN
    struct PollerEvent;

class TimerMgr;
class TimeSlice;
struct PollerCompStatistics;

struct AsyncTask;

template<typename KeyType, typename MaskValue>
class LibDirtyHelper;

class KERNEL_EXPORT Poller : public CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, Poller);

    typedef void (*PollerHandler)(PollerEvent *);

public:
    enum LoadRateDefault
    {
        TIMER_LOADED = 30,      // timer 30%记入
        DIRTY_LOADED = 20,      // dirty 20%记入
        EVENTS_LOADED = 50,     // events 50%记入
    };
public:
    Poller();
    ~Poller();
    virtual void Release() override;

public:
    virtual void Clear() override;
    LibString ToString() const override;

    // 负载
    virtual UInt64 CalcLoadScore() const;
    
    // 是否可用
    bool IsEnable() const;
    void Disable();

    // worker线程id
    UInt64 GetWorkerThreadId() const;

    // 消息队列数量
    Int64 GetEventAmount() const;
    // 获得消费的数量
    Int64 GetAndResetConsumCount();
    // 消息队列生产数量
    Int64 GetGenEventAmount() const;
    // 获得生产的数量并重置
    Int64 GetAndResetGenCount();

    // 获取脏助手
   LibDirtyHelper<void *, UInt32> *GetDirtyHelper();
   const LibDirtyHelper<void *, UInt32> *GetDirtyHelper() const;

   // 设置worker handler
   void SetPepareEventWorkerHandler(IDelegate<bool, Poller *> *handler);
   template<typename ObjType>
   void SetPepareEventWorkerHandler(ObjType *obj, bool (ObjType::*handler)(Poller *));

   // 设置帧末执行
   template<typename ObjType>
   void SetFrameTick(ObjType *obj, void(ObjType::*handler)());
   void SetFrameTick(IDelegate<void> *handler);

   // poller线程退出前销毁
   void SetEventWorkerCloseHandler(IDelegate<void, Poller *> *handler);
   template<typename ObjType>
   void SetEventWorkerCloseHandler(ObjType *obj, void (ObjType::*handler)(Poller *));

    // 订阅消息处理
    template<typename ObjType>
    void Subscribe(Int32 eventType, ObjType *obj, void (ObjType::*handler)(KERNEL_NS::PollerEvent *));
    void Subscribe(Int32 eventType, void (*handler)(KERNEL_NS::PollerEvent *));
    void Subscribe(Int32 eventType, KERNEL_NS::IDelegate<void, KERNEL_NS::PollerEvent *> *deleg);
    void UnSubscribe(Int32 eventType);

    // 订阅对象消息（对象消息只能本进程, 因为对象id是本地生成, 如果要实现远程，必须使用字符串映射到本地的对象id）
    template<typename ObjectType>
    requires requires(ObjectType obj)
    {
        obj.Release();
        obj.ToString();
    }
    void SubscribeObjectEvent(IDelegate<void, StubPollerEvent *> *cb);
    template<typename ObjectType, typename ObjType>
    requires requires(ObjectType obj)
    {
        obj.Release();
        obj.ToString();
    }
    void SubscribeObjectEvent(ObjType *obj, void (ObjType::*handler)(KERNEL_NS::StubPollerEvent *));
    template<typename ObjectType>
    requires requires(ObjectType obj)
    {
        obj.Release();
        obj.ToString();
    }
    void SubscribeObjectEvent(void (*handler)(KERNEL_NS::StubPollerEvent *));
    template<typename ObjectType, typename  LambType>
    requires requires(ObjectType obj, LambType lamb, KERNEL_NS::StubPollerEvent *ev)
    {
        obj.Release();
        obj.ToString();
        lamb(ev);
    }
    void SubscribeObjectEvent(LambType &&lamb);
    void SubscribeObjectEvent(UInt64 objectTypeId, IDelegate<void, StubPollerEvent *> *cb);

    // 取消订阅对象消息
    template<typename ObjectType>
    requires requires(ObjectType obj)
    {
        obj.Release();
        obj.ToString();
    }
    void UnSubscribeObjectEvent();
    void UnSubscribeObjectEvent(UInt64 objectTypeId);

    // 设置poller事件循环休眠时最大等待时长
    void SetMaxSleepMilliseconds(UInt64 maxMilliseconds);

    // 设置处理超时时长
    void SetMaxPieceTime(const TimeSlice &piece);
    const LibCpuSlice &GetMaxPieceTime() const;

    // 设置事件优先级队列最大等级
    void SetMaxPriorityLevel(Int32 level);
    Int32 GetMaxPriorityLevel() const;

    // n次循环检查一次超时
    void SetLoopDetectTimeout(Int32 loopCount);
    Int32 GetLoopDetectTimeout() const;

    // 获取定时器
    TimerMgr *GetTimerMgr();
    const TimerMgr *GetTimerMgr() const;

    // 投递事件
    void Push(Int32 level, PollerEvent *ev);
    void Push(Int32 level, LibList<PollerEvent *> *evList);
    void Push(Int32 level, IDelegate<void> *action);

    template<typename LamvadaType>
    requires requires (LamvadaType lam) 
    {
        {lam()} -> std::convertible_to<void>;
    }
    ALWAYS_INLINE void Push(Int32 level, LamvadaType &&lambdaType)
    {
        IDelegate<void> *delg = KERNEL_NS::DelegateFactory::Create<decltype(lambdaType), void>(lambdaType);
        Push(level, delg);
    }

    // 事件循环接口
    bool PrepareLoop();
    // Debug下是QuicklyLoop, 不使用try/catch让问题充分暴露, release下是SafeEventLoop, 保证稳定性
    void EventLoop();
    // 不适用try/catch 应对高效的简单的场景
    void QuicklyLoop();
    // 使用try/catch保证稳定性
    void SafeEventLoop();
    void OnLoopEnd();
    void WakeupEventLoop();
    void QuitLoop();
    bool CanQuit() const;

    void OnMonitor(PollerCompStatistics &statistics);

    // 调用者当前线程投递req给this
    // req暂时只能传指针，而且会在otherChannel（可能不同线程）释放
    // req/res 必须实现Release, ToString接口
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
    CoTask<KERNEL_NS::SmartPtr<ResType, AutoDelMethods::Release>> SendAsync(ReqType *req)
    {
        co_return KERNEL_NS::TlsUtil::GetPoller()->SendToAsync<ResType, ReqType>(*this, req);
    }
    
    // 跨线程协程消息(otherPoller也可以是自己)
    // req暂时只能传指针，而且会在otherChannel（可能不同线程）释放
    // req/res 必须实现Release, ToString接口
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
    CoTask<KERNEL_NS::SmartPtr<ResType, AutoDelMethods::Release>> SendToAsync(Poller &otherPoller, ReqType *req)
    {
        // 1.ptr用来回传ResType
        KERNEL_NS::SmartPtr<ResType *,  KERNEL_NS::AutoDelMethods::CustomDelete> ptr(KERNEL_NS::KernelCastTo<ResType *>(
            kernel::KernelAllocMemory<KERNEL_NS::_Build::TL>(sizeof(ResType **))));
        ptr.SetClosureDelegate([](void *p)
        {
            // 释放packet
            auto castP = KERNEL_NS::KernelCastTo<ResType*>(p);
            if(*castP)
                (*castP)->Release();

            KERNEL_NS::KernelFreeMemory<KERNEL_NS::_Build::TL>(castP);
        });
        *ptr = NULL;

        // 设置stub => ResType的事件回调
        UInt64 stub = ++_maxStub;
        KERNEL_NS::SmartPtr<KERNEL_NS::TaskParamRefWrapper, KERNEL_NS::AutoDelMethods::Release> params = KERNEL_NS::TaskParamRefWrapper::NewThreadLocal_TaskParamRefWrapper();
        SubscribeStubEvent(stub, [ptr, params](KERNEL_NS::StubPollerEvent *ev) mutable 
        {
            KERNEL_NS::ObjectPollerEvent<ResType> *finalEv = KernelCastTo<KERNEL_NS::ObjectPollerEvent<ResType>>(ev);
            // 将结果带出去
            *ptr = finalEv->_obj;
            finalEv->_obj = NULL;
            
            // 唤醒Waiter
            auto &coParam = params->_params;
            if(coParam && coParam->_handle)
                coParam->_handle->ForceAwake();
        });

        // 发送对象事件 ObjectPollerEvent到 other
        auto objEvent = ObjectPollerEvent<ReqType>::New_ObjectPollerEvent(stub, false);
        objEvent->_obj = req;
        otherPoller.Push(otherPoller.GetMaxPriorityLevel(), objEvent);
        
        // 等待 ObjectPollerEvent 的返回消息唤醒
        co_await KERNEL_NS::Waiting().SetDisableSuspend().GetParam(params);
        if(LIKELY(params->_params))
        {
            auto &pa = params->_params; 
            if(pa->_errCode != Status::Success)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("waiting err:%d, stub:%llu, req:%p")
                    , pa->_errCode, stub, req);
                
                UnSubscribeStubEvent(stub);
            }

            // 销毁waiting协程
            if(pa->_handle)
                pa->_handle->DestroyHandle(pa->_errCode);
        }
        
        // 3.将消息回调中的ResType引用设置成空
        auto res = *ptr;
        *ptr = NULL;
        co_return KERNEL_NS::SmartPtr<ResType,  KERNEL_NS::AutoDelMethods::Release>(res);
    }
    
protected:
    virtual Int32 _OnInit() override;
    virtual Int32 _OnStart() override;
    virtual void _OnWillClose() override;
    virtual void _OnClose() override;

protected:
    // 异步任务
    void _OnAsyncTaskEvent(PollerEvent *ev);
    void _OnObjectEvent(PollerEvent *ev);

private:
    void _Clear();

    // 跨线程异步实现
#pragma region

private:
    // 事件返回处理
    void _OnObjectEventResponse(StubPollerEvent *ev);

    // 事件请求处理
    void _OnObjectEventRequest(StubPollerEvent *ev);

    // 订阅Stub事件
    template<typename LambType>
    requires requires(LambType lam, StubPollerEvent * ev)
    {
        lam(ev);
    }
    void SubscribeStubEvent(UInt64 stub, LambType &&cb);

    // 取消订阅回调
    void UnSubscribeStubEvent(UInt64 stub);
#pragma endregion 

private:
  LibCpuSlice _maxPieceTime;                                 // 每个事务时间片
  std::atomic<UInt64> _workThreadId;                        // 事件处理线程id
  std::atomic_bool _isEnable;                               // 销毁的时候是disable
  std::atomic_bool _isQuitLoop;                               // 是否关闭
  UInt64 _maxSleepMilliseconds;                             // poller等待的最大时长一般毫秒级即可
  Int32 _loopDetectTimeout;                                 // n次循环检测一次超时

  TimerMgr *_timerMgr;                                      // 定时器管理
  LibDirtyHelper<void *, UInt32> *_dirtyHelper;             // 事件脏标记

  IDelegate<bool, Poller *> *_prepareEventWorkerHandler;    // 事件处理线程初始准备
  IDelegate<void, Poller *> *_onEventWorkerCloseHandler;    // 事件处理线程结束销毁
  ConditionLocker _eventGuard;                              // 空闲挂起等待
  ConcurrentPriorityQueue<PollerEvent *> *_eventsList;      // 优先级事件队列
  std::atomic<Int64> _eventAmountLeft;
  std::atomic<Int64> _genEventAmount;
  std::atomic<Int64> _consumEventCount;
  IDelegate<void> *_onTick;                                 // 帧末执行

  // poller event handler
  std::unordered_map<Int32, KERNEL_NS::IDelegate<void, KERNEL_NS::PollerEvent *> *> _pollerEventHandler;

    // 跨线程异步实现
#pragma region // async implement
    UInt64 _maxStub;

    // 订阅对象事件 stub => callback
    std::map<UInt64, IDelegate<void, StubPollerEvent *> *> _stubRefCb;

    // 订阅对象请求消息
    std::map<UInt64, IDelegate<void, StubPollerEvent *> *> _objTypeIdRefCallback;
#pragma endregion 

};

ALWAYS_INLINE bool Poller::IsEnable() const
{
    return _isEnable.load(std::memory_order_acquire);
}

ALWAYS_INLINE void Poller::Disable()
{
    _isEnable.store(false, std::memory_order_release);
}

ALWAYS_INLINE UInt64 Poller::GetWorkerThreadId() const
{
    return _workThreadId.load(std::memory_order_acquire);
}

ALWAYS_INLINE Int64 Poller::GetEventAmount() const
{
    return _eventAmountLeft.load(std::memory_order_acquire);
}

ALWAYS_INLINE Int64 Poller::GetAndResetConsumCount()
{
    return _consumEventCount.exchange(0, std::memory_order_acq_rel);
}

ALWAYS_INLINE Int64 Poller::GetGenEventAmount() const
{
    return _genEventAmount.load(std::memory_order_acquire);
}

ALWAYS_INLINE Int64 Poller::GetAndResetGenCount()
{
    return _genEventAmount.exchange(0, std::memory_order_acq_rel);
}

ALWAYS_INLINE LibDirtyHelper<void *, UInt32> *Poller::GetDirtyHelper()
{
    return _dirtyHelper;
}

ALWAYS_INLINE const LibDirtyHelper<void *, UInt32> *Poller::GetDirtyHelper() const
{
    return _dirtyHelper;
}

ALWAYS_INLINE void Poller::SetPepareEventWorkerHandler(IDelegate<bool, Poller *> *handler)
{
    CRYSTAL_DELETE_SAFE(_prepareEventWorkerHandler);
    _prepareEventWorkerHandler = handler;
}

template<typename ObjType>
ALWAYS_INLINE void Poller::SetPepareEventWorkerHandler(ObjType *obj, bool (ObjType::*handler)(Poller *))
{
    auto delg = DelegateFactory::Create(obj, handler);
    SetPepareEventWorkerHandler(delg);
}

template<typename ObjType>
ALWAYS_INLINE void Poller::SetFrameTick(ObjType *obj, void(ObjType::*handler)())
{
    auto delg = DelegateFactory::Create(obj, handler);
    SetFrameTick(delg);
}

ALWAYS_INLINE void Poller::SetFrameTick(IDelegate<void> *handler)
{
    CRYSTAL_RELEASE_SAFE(_onTick);
    _onTick = handler;
}

ALWAYS_INLINE void Poller::SetEventWorkerCloseHandler(IDelegate<void, Poller *> *handler)
{
    CRYSTAL_DELETE_SAFE(_onEventWorkerCloseHandler);
    _onEventWorkerCloseHandler = handler;
}

template<typename ObjType>
ALWAYS_INLINE void Poller::SetEventWorkerCloseHandler(ObjType *obj, void (ObjType::*handler)(Poller *))
{
    auto delg = DelegateFactory::Create(obj, handler);
    SetEventWorkerCloseHandler(delg);
}

template<typename ObjType>
ALWAYS_INLINE void Poller::Subscribe(Int32 eventType, ObjType *obj, void (ObjType::*handler)(KERNEL_NS::PollerEvent *))
{
    auto delg = DelegateFactory::Create(obj, handler);
    Subscribe(eventType, delg);
}

ALWAYS_INLINE void Poller::Subscribe(Int32 eventType, void (*handler)(KERNEL_NS::PollerEvent *))
{
    auto delg = DelegateFactory::Create(handler);
    Subscribe(eventType, delg);
}

ALWAYS_INLINE void Poller::UnSubscribe(Int32 eventType)
{
    auto iter = _pollerEventHandler.find(eventType);
    if(UNLIKELY(iter == _pollerEventHandler.end()))
        return;

    iter->second->Release();
    _pollerEventHandler.erase(iter);
}

ALWAYS_INLINE void Poller::SetMaxSleepMilliseconds(UInt64 maxMilliseconds)
{
    _maxSleepMilliseconds = maxMilliseconds;
}

ALWAYS_INLINE void Poller::SetLoopDetectTimeout(Int32 loopCount)
{
    _loopDetectTimeout = loopCount;
}

ALWAYS_INLINE Int32 Poller::GetLoopDetectTimeout() const
{
    return _loopDetectTimeout;
}

ALWAYS_INLINE TimerMgr *Poller::GetTimerMgr()
{
    return _timerMgr;
}

ALWAYS_INLINE const TimerMgr *Poller::GetTimerMgr() const
{
    return _timerMgr;
}

#ifdef _DEBUG

// Debug情况下不使用try{}catch(){}让问题充分暴露
ALWAYS_INLINE void Poller::EventLoop()
{
  QuicklyLoop();
}

#else

ALWAYS_INLINE void Poller::EventLoop()
{
  SafeEventLoop();
}

#endif

// template<typename LamvadaType>
// ALWAYS_INLINE void Poller::Push(Int32 level, LamvadaType &&lambdaType)
// {
//     IDelegate<void> *delg = KERNEL_NS::DelegateFactory::Create<decltype(lambdaType), void>(lambdaType);
//     PushAction(level, delg);
// }

ALWAYS_INLINE void Poller::WakeupEventLoop()
{
    _eventGuard.Sinal();
}

ALWAYS_INLINE void Poller::QuitLoop()
{
    _isQuitLoop.store(true, std::memory_order_release);
    WakeupEventLoop();
}

template<typename ObjectType>
requires requires(ObjectType obj)
{
    obj.Release();
    obj.ToString();
}
ALWAYS_INLINE void Poller::SubscribeObjectEvent(IDelegate<void, StubPollerEvent *> *cb)
{
    auto objectTypeId = KERNEL_NS::RttiUtil::GetTypeId<ObjectType>();
    SubscribeObjectEvent(objectTypeId, cb);
}

template<typename ObjectType, typename ObjType>
requires requires(ObjectType obj)
{
    obj.Release();
    obj.ToString();
}
ALWAYS_INLINE void Poller::SubscribeObjectEvent(ObjType *obj, void (ObjType::*handler)(KERNEL_NS::StubPollerEvent *))
{
    auto delg = DelegateFactory::Create(obj, handler);
    auto objectTypeId = KERNEL_NS::RttiUtil::GetTypeId<ObjectType>();
    SubscribeObjectEvent(objectTypeId, delg);
}

template<typename ObjectType>
requires requires(ObjectType obj)
{
    obj.Release();
    obj.ToString();
}
ALWAYS_INLINE void Poller::SubscribeObjectEvent(void (*handler)(KERNEL_NS::StubPollerEvent *))
{
    auto delg = DelegateFactory::Create(handler);
    auto objectTypeId = KERNEL_NS::RttiUtil::GetTypeId<ObjectType>();
    SubscribeObjectEvent(objectTypeId, delg);
}

template<typename ObjectType, typename  LambType>
requires requires(ObjectType obj, LambType lamb, KERNEL_NS::StubPollerEvent *ev)
{
    obj.Release();
    obj.ToString();
    lamb(ev);
}
ALWAYS_INLINE void Poller::SubscribeObjectEvent(LambType &&lamb)
{
    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(lamb, void, KERNEL_NS::StubPollerEvent *);
    auto objectTypeId = KERNEL_NS::RttiUtil::GetTypeId<ObjectType>();
    SubscribeObjectEvent(objectTypeId, delg);
}

ALWAYS_INLINE void Poller::SubscribeObjectEvent(UInt64 objectTypeId, IDelegate<void, StubPollerEvent *> *cb)
{
    auto iter = _objTypeIdRefCallback.find(objectTypeId);
    if (iter != _objTypeIdRefCallback.end())
    {
        if (g_Log->IsEnable(LogLevel::Warn))
            g_Log->Warn(LOGFMT_OBJ_TAG("repeat object event, object type id:%llu, older cb:%s, older cb owner:%s, new cb:%s, new cb owner:%s")
                , objectTypeId, iter->second->GetCallbackRtti().c_str(), iter->second->GetOwnerRtti().c_str(), cb->GetCallbackRtti().c_str(), cb->GetOwnerRtti().c_str());

        iter->second->Release();
        _objTypeIdRefCallback.erase(iter);
    }

    _objTypeIdRefCallback.insert(std::make_pair(objectTypeId, cb));
}

template<typename ObjectType>
requires requires(ObjectType obj)
{
    obj.Release();
    obj.ToString();
}
ALWAYS_INLINE void Poller::UnSubscribeObjectEvent()
{
    auto objectTypeId = KERNEL_NS::RttiUtil::GetTypeId<ObjectType>();
    UnSubscribeObjectEvent(objectTypeId);
}

ALWAYS_INLINE void Poller::UnSubscribeObjectEvent(UInt64 objectTypeId)
{
    auto iter = _objTypeIdRefCallback.find(objectTypeId);
    if (iter == _objTypeIdRefCallback.end())
        return;

    if (g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
        g_Log->Debug(LOGFMT_OBJ_TAG("UnSubscribeObjectEvent objectTypeId:%llu => callback:(%s) owner:%s.")
            , objectTypeId, iter->second->GetCallbackRtti().c_str(), iter->second->GetOwnerRtti().c_str());

    iter->second->Release();
    _objTypeIdRefCallback.erase(iter);
}

template<typename LambType>
requires requires(LambType lam, StubPollerEvent * ev)
{
    lam(ev);
}
ALWAYS_INLINE void Poller::SubscribeStubEvent(UInt64 stub, LambType &&cb)
{
    auto iter = _stubRefCb.find(stub);
    if (iter != _stubRefCb.end())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("repeated stub:%llu => callback, and will release older cb:%s, older cb owner:%s.")
            , stub, iter->second->GetCallbackRtti().c_str(), iter->second->GetOwnerRtti().c_str());
            
        iter->second->Release();
        _stubRefCb.erase(iter);
    }
        
    auto deleg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void, StubPollerEvent *);
    _stubRefCb.insert(std::make_pair(stub, deleg));
}

ALWAYS_INLINE void Poller::UnSubscribeStubEvent(UInt64 stub)
{
    auto iter = _stubRefCb.find(stub);
    if (iter == _stubRefCb.end())
        return;

    if (g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
        g_Log->Debug(LOGFMT_OBJ_TAG("unsubscribe stub:%llu => callback:(%s) owner:%s."), stub, iter->second->GetCallbackRtti().c_str(), iter->second->GetOwnerRtti().c_str());

    iter->second->Release();
    _stubRefCb.erase(iter);
}

KERNEL_END

#endif
