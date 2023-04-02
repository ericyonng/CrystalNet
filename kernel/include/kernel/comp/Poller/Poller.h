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

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/CompObject/CompObject.h>
#include <kernel/comp/LibDirtyHelper.h>
#include <kernel/comp/Delegate/Delegate.h>
#include <kernel/comp/Poller/PollerEvent.h>
#include <kernel/comp/LibList.h>
#include <kernel/comp/Lock/Lock.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/ConcurrentPriorityQueue/ConcurrentPriorityQueue.h>

KERNEL_BEGIN

class TimerMgr;
class TimeSlice;

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
    virtual void Release();

public:
    virtual void Clear() override;
    LibString ToString() const override;

    // 负载
    virtual UInt64 CalcLoadScore() const;
    
    // 是否可用
    bool IsEnable() const;

    // worker线程id
    UInt64 GetWorkerThreadId() const;

    // 消息队列数量
    Int64 GetEventAmount() const;
    // 获得消费的数量
    Int64 GetAndResetConsumCount();

    // 获取脏助手
   LibDirtyHelper<void *, UInt32> *GetDirtyHelper();
   const LibDirtyHelper<void *, UInt32> *GetDirtyHelper() const;

   // 设置worker handler
   void SetPepareEventWorkerHandler(IDelegate<bool, Poller *> *handler);
   template<typename ObjType>
   void SetPepareEventWorkerHandler(ObjType *obj, bool (ObjType::*handler)(Poller *));

   // poller线程退出前销毁
   void SetEventWorkerCloseHandler(IDelegate<void, Poller *> *handler);
   template<typename ObjType>
   void SetEventWorkerCloseHandler(ObjType *obj, void (ObjType::*handler)(Poller *));

    // 设置事件处理 PollerEvent统一在poller层释放,handler不可以释放
    void SetEventHandler(IDelegate<void, PollerEvent *> *handler);
    template<typename ObjType>
    void SetEventHandler(ObjType *obj, void (ObjType::*handler)(PollerEvent *ev));

    void SetEventHandler(void (*f)(PollerEvent *));

    // 设置poller事件循环休眠时最大等待时长
    void SetMaxSleepMilliseconds(UInt64 maxMilliseconds);

    // 设置处理超时时长
    void SetMaxPieceTime(const TimeSlice &piece);

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
    void QuickEventLoop();
    void OnLoopEnd();
    void WakeupEventLoop();
    void QuitLoop();

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
  IDelegate<void, PollerEvent *> *_eventHandler;            // 事件处理回调
  PollerHandler _quickEventHandler;                         // 事件处理回调
  ConditionLocker _eventGuard;                              // 空闲挂起等待
  ConcurrentPriorityQueue<PollerEvent *> *_eventsList;      // 优先级事件队列
  std::atomic<Int64> _eventAmountLeft;
  std::atomic<Int64> _consumEventCount;
};

ALWAYS_INLINE bool Poller::IsEnable() const
{
    return _isEnable;
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

ALWAYS_INLINE void Poller::SetEventHandler(IDelegate<void, PollerEvent *> *handler)
{
    CRYSTAL_DELETE_SAFE(_eventHandler);
    _eventHandler = handler;
}

template<typename ObjType>
ALWAYS_INLINE void Poller::SetEventHandler(ObjType *obj, void (ObjType::*handler)(PollerEvent *ev))
{
    auto delg = DelegateFactory::Create(obj, handler);
    SetEventHandler(delg);
}

ALWAYS_INLINE void Poller::SetEventHandler(void (*f)(PollerEvent *))
{
    _quickEventHandler = f;
}

ALWAYS_INLINE void Poller::SetMaxSleepMilliseconds(UInt64 maxMilliseconds)
{
    _maxSleepMilliseconds = maxMilliseconds;
}

ALWAYS_INLINE void Poller::SetMaxPriorityLevel(Int32 level)
{
    _eventsList->SetMaxLevel(level);
}

ALWAYS_INLINE Int32 Poller::GetMaxPriorityLevel() const
{
    return _eventsList->GetMaxLevel();
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

ALWAYS_INLINE void Poller::Push(Int32 level, PollerEvent *ev)
{
    if(UNLIKELY(!_isEnable))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("poller is destroying obj id:%llu, ev:%s"), GetId(), ev->ToString().c_str());
        ev->Release();
        return;
    }
    
    ++_eventAmountLeft;
    _eventsList->PushQueue(level, ev);
    WakeupEventLoop();
}

ALWAYS_INLINE void Poller::Push(Int32 level, LibList<PollerEvent *> *evList)
{
    if(UNLIKELY(!_isEnable))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("poller is destroying obj id:%llu, evList count:%llu")
                        , GetId(), evList->GetAmount());

        ContainerUtil::DelContainer(*evList, [](PollerEvent *ev){
            ev->Release();
        });
        LibList<PollerEvent *>::Delete_LibList(evList);
        return;
    }

    _eventAmountLeft += static_cast<Int64>(evList->GetAmount());
    _eventsList->PushQueue(level, evList);
    LibList<PollerEvent *>::Delete_LibList(evList);
    WakeupEventLoop();
}

ALWAYS_INLINE void Poller::Push(Int32 level, Int32 specifyActionType, IDelegate<void> *action)
{
    if(UNLIKELY(!_isEnable))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("poller is destroying poller obj id:%llu"), GetId());
        action->Release();
        return;
    }

    auto ev = ActionPollerEvent::New_ActionPollerEvent(specifyActionType);
    ev->_action = action;
    ++_eventAmountLeft;
    _eventsList->PushQueue(level, ev);
    WakeupEventLoop();
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

KERNEL_END

#endif
