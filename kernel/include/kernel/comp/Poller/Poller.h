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

#include <unordered_map>
#include <atomic>

KERNEL_BEGIN

struct PollerEvent;

class TimerMgr;
class TimeSlice;

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
    void Push(Int32 level, Int32 specifyActionType, IDelegate<void> *action);

    template<typename LamvadaType>
    void Push(Int32 level, Int32 specifyActionType, LamvadaType &&lambdaType);

    // 事件循环接口
    bool PrepareLoop();
    void EventLoop();
    void OnLoopEnd();
    void WakeupEventLoop();
    void QuitLoop();
    bool CanQuit() const;

    LibString OnMonitor();

    // TODO:假release, 不会Delete Poller,暂时性处理当处wait状态,中间收到信号导致Poller在被释放的时候调用条件变量的析构并调用destroy销毁条件变量时导致死锁
    void SetDummyRelease();

protected:
    virtual Int32 _OnInit() override;
    virtual Int32 _OnStart() override;
    virtual void _OnWillClose() override;
    virtual void _OnClose() override;

private:
    void _Clear();

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

  std::atomic_bool _isDummyRelease;
};

ALWAYS_INLINE bool Poller::IsEnable() const
{
    return _isEnable;
}

ALWAYS_INLINE void Poller::Disable()
{
    _isEnable = false;
}

ALWAYS_INLINE UInt64 Poller::GetWorkerThreadId() const
{
    return _workThreadId;
}

ALWAYS_INLINE Int64 Poller::GetEventAmount() const
{
    return _eventAmountLeft;
}

ALWAYS_INLINE Int64 Poller::GetAndResetConsumCount()
{
    const Int64 consumCount = _consumEventCount;
    _consumEventCount -= consumCount;
    return consumCount;
}

ALWAYS_INLINE Int64 Poller::GetGenEventAmount() const
{
    return _genEventAmount;
}

ALWAYS_INLINE Int64 Poller::GetAndResetGenCount()
{
    const Int64 count = _genEventAmount;
    _genEventAmount -= count;
    return count;
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

template<typename LamvadaType>
ALWAYS_INLINE void Poller::Push(Int32 level, Int32 specifyActionType, LamvadaType &&lambdaType)
{
    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(lambdaType, void);
    Push(level, specifyActionType, delg);
}

ALWAYS_INLINE void Poller::WakeupEventLoop()
{
    _eventGuard.Sinal();
}

ALWAYS_INLINE void Poller::QuitLoop()
{
    _isQuitLoop = true;
    WakeupEventLoop();
}

ALWAYS_INLINE void Poller::SetDummyRelease()
{
    _isDummyRelease = true;
}


KERNEL_END

#endif
