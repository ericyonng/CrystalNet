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
 * Date: 2022-03-23 14:49:04
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/LibDirtyHelper.h>
#include <kernel/comp/Poller/PollerEvent.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/Utils/ContainerUtil.h>

#include <kernel/comp/Poller/Poller.h>
#include <kernel/comp/Timer/Timer.h>
#include <kernel/comp/thread/thread.h>
#include <kernel/comp/TimeSlice.h>
#include <kernel/comp/Utils/TlsUtil.h>
#include <kernel/comp/Tls/Tls.h>
#include <kernel/comp/Poller/PollerEventInternalType.h>

#include <kernel/comp/Poller/PollerCompStatistics.h>
#include <kernel/comp/Coroutines/AsyncTask.h>

#include "kernel/comp/Poller/Channel.h"
#include "kernel/comp/Utils/StringUtil.h"

// static ALWAYS_INLINE bool IsPriorityEvenetsQueueEmpty(const std::vector<KERNEL_NS::LibList<KERNEL_NS::PollerEvent *, KERNEL_NS::_Build::MT> *> &queue)
// {
//     if(UNLIKELY(queue.empty()))
//         return true;

//     for(auto &evList:queue)
//     {
//         if(!evList->IsEmpty())
//             return false;
//     }

//     return true;
// }

// static ALWAYS_INLINE UInt64 GetPriorityEvenetsQueueElemCount(const KERNEL_NS::LibList<KERNEL_NS::LibList<KERNEL_NS::PollerEvent *, KERNEL_NS::_Build::MT> *> &queue)
// {
//     if(UNLIKELY(queue.IsEmpty()))
//         return 0;
//
//     UInt64 count = 0;
//     for(auto node = queue.Begin(); node; node = node->_next)
//         count += node->_data->GetAmount();
//
//     return count;
// }

// static ALWAYS_INLINE UInt64 GetPriorityEvenetsQueueElemCount(const std::vector<KERNEL_NS::LibList<KERNEL_NS::PollerEvent *, KERNEL_NS::_Build::MT> *> &queue)
// {
//     if(UNLIKELY(queue.empty()))
//         return 0;

//     UInt64 count = 0;
//     for(auto node : queue)
//         count += node->GetAmount();

//     return count;
// }

// static ALWAYS_INLINE void MergePriorityEvenetsQueue(std::vector<KERNEL_NS::LibList<KERNEL_NS::PollerEvent *, KERNEL_NS::_Build::MT> *> &from,std::vector<KERNEL_NS::LibList<KERNEL_NS::PollerEvent *, KERNEL_NS::_Build::MT> *> &to)
// {
//     if(UNLIKELY(from.empty()))
//         return;

//     Int32 queueSize = static_cast<Int32>(from.size());
//     for(Int32 idx = 0; idx < queueSize; ++idx)
//     {
//         auto fromQueue = from[idx];
//         auto toQueue = to[idx];
//         toQueue->MergeTail(fromQueue);
//     }
// }

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(Poller);

Poller::Poller()
:CompObject(KERNEL_NS::RttiUtil::GetTypeId<Poller>())
,_maxPieceTime(LibCpuSlice::FromMilliseconds(8))  // 经验值8ms
,_workThreadId{0}
,_isEnable{true}
,_isQuitLoop{false}
,_maxSleepMilliseconds(1)
,_loopDetectTimeout(1000)   // 默认1000次循环检查一次
,_timerMgr(TimerMgr::New_TimerMgr())
,_dirtyHelper(LibDirtyHelper<void *, UInt32>::New_LibDirtyHelper())
,_prepareEventWorkerHandler(NULL)
,_onEventWorkerCloseHandler(NULL)
,_isWaiting{true}
,_eventAmountLeft{0}
,_genEventAmount{0}
,_consumEventCount{0}
,_onTick(NULL)
,_maxStub(0)
,_commonEvents(MPMCQueue<PollerEvent *, 16*16*2*1024>::NewThreadLocal_MPMCQueue())
,_localEvents(LibList<PollerEvent *, KERNEL_NS::_Build::TL>::NewThreadLocal_LibList())
{
    // auto defObj = TlsUtil::GetDefTls();
    // if(UNLIKELY(defObj->_poller))
    //     g_Log->Error(LOGFMT_OBJ_TAG("please follow the rule of one thread one loop, one thread cant support multi poller, there is already a poller existed. and will cover it with new poller,  old poller:%s"), defObj->_poller->ToString().c_str());

    // defObj->_poller = this;
    // defObj->_pollerTimerMgr = _timerMgr;
}

Poller::~Poller()
{
    _Clear();
}

void Poller::Release()
{
    Poller::Delete_Poller(this);
}

#ifdef CRYSTAL_NET_CPP20
CoTask<Channel *> Poller::ApplyChannel()
{
    // 如果已经存在了不需要再申请直接返回
    auto myPoller = TlsUtil::GetPoller();
    auto iter = myPoller->_targetPollerRefChannel.find(this);
    if(UNLIKELY(iter != myPoller->_targetPollerRefChannel.end()))
    {
        co_return iter->second;
    }

    // 向target poller 申请spsc消息队列和channel id, 并创建给TlsUtil::Poller
    auto req = ApplyChannelEvent::New_ApplyChannelEvent();
    auto res = co_await SendAsync<ApplyChannelEventResponse, ApplyChannelEvent>(req);

    // 给当前线程创建通向target poller的channel
    auto channel = Channel::NewThreadLocal_Channel(res->_result._channelId, this, res->_result._queue);
    myPoller->_idRefChannel.insert(std::make_pair(channel->GetChannelId(), channel));
    myPoller->_targetPollerRefChannel.insert(std::make_pair(this, channel));

    if(g_Log->IsEnable(LogLevel::Info))
        g_Log->Info(LOGFMT_OBJ_TAG("new channel created src:%s => target:%s, channel id:%llu")
            , myPoller->ToString().c_str(), ToString().c_str(), channel->GetChannelId());

    co_return channel;
}
#endif

void Poller::SendByChannel(UInt64 channelId, PollerEvent *ev)
{
    auto iter = _idRefChannel.find(channelId);
    if(UNLIKELY(iter == _idRefChannel.end()))
    {
        ev->Release();
        if(UNLIKELY(g_Log->IsEnable(LogLevel::Debug)))
            g_Log->Debug(LOGFMT_OBJ_TAG("channel not found:%llu, ev:%s, poller:%s"), channelId, ev->ToString().c_str(), ToString().c_str());
        return;
    }

    iter->second->Send(ev);
}

void Poller::SendByChannel(UInt64 channelId, LibList<PollerEvent *> *evs)
{
    auto iter = _idRefChannel.find(channelId);
    if(UNLIKELY(iter == _idRefChannel.end()))
    {
        KERNEL_NS::LibString info;
        for(auto node = evs->Begin(); node; node = node->_next)
        {
            info.AppendFormat("%s\n", node->_data->ToString().c_str());
        }
        
        if(UNLIKELY(g_Log->IsEnable(LogLevel::Debug)))
            g_Log->Debug(LOGFMT_OBJ_TAG("channel not found:%llu, ev:%s, poller:%s"), channelId, info.c_str(), ToString().c_str());
        
        ContainerUtil::DelContainer(*evs, [](PollerEvent *ev)
        {
            ev->Release();
        });
        LibList<PollerEvent *>::Delete_LibList(evs);
        return;
    }

    iter->second->Send(evs);
}

Int32 Poller::_OnInit()
{
    Int32 ret = CompObject::_OnInit();
    if(ret != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("comp init fail ret:%d"), ret);
        return ret;
    }

    _isQuitLoop.store(false, std::memory_order_release);
    _workThreadId.store(0, std::memory_order_release);

    // AsyncTask消息
    Subscribe(PollerEventInternalType::AsyncTaskType, this, &Poller::_OnAsyncTaskEvent);
    // 对象消息
    Subscribe(PollerEventInternalType::ObjectPollerEventType, this, &Poller::_OnObjectEvent);
    // 批量消息
    Subscribe(PollerEventInternalType::BatchPollerEventType, this, &Poller::_OnBatchPollerEvent);
    // action
    Subscribe(PollerEventInternalType::ActionPollerEventType, this, &Poller::_OnActionPollerEvent);
    // 订阅创建Channel消息
    SubscribeObjectEvent<ApplyChannelEvent>(this, &Poller::_OnApplyChannelEvent);
    // 订阅销毁Channel消息（废弃）
    // SubscribeObjectEvent<DestroyChannelEvent>(this, &Poller::_OnDestroyChannelEvent);
    
    // TODO:测试是不是在Poller所在线程
    _workThreadId.store(SystemUtil::GetCurrentThreadId(), std::memory_order_release);
    _timerMgr->Launch(DelegateFactory::Create(this, &Poller::WakeupEventLoop));
    
    g_Log->Debug(LOGFMT_OBJ_TAG("poller inited %s"), ToString().c_str());
    return Status::Success;
}

Int32 Poller::_OnStart()
{
    auto ret = CompObject::_OnStart();
    if(ret != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("comp obj _OnStart fail ret:%d"), ret);
        return ret;
    }

    _isQuitLoop.store(false, std::memory_order_release);
    g_Log->Debug(LOGFMT_OBJ_TAG("poller started %s"), ToString().c_str());
    return Status::Success;
}

void Poller::_OnWillClose()
{
    g_Log->Debug(LOGFMT_OBJ_TAG("poller will close %s"), ToString().c_str());

    _isQuitLoop.store(true, std::memory_order_release);
    WakeupEventLoop();

    CompObject::_OnWillClose();
}

void Poller::_OnClose()
{
    // TODO:销毁channel src channel/target spsc queue
    g_Log->Debug(LOGFMT_OBJ_TAG("poller close %s"), ToString().c_str());
    // _Clear();
    CompObject::_OnClose();
}

void Poller::_OnAsyncTaskEvent(PollerEvent *ev)
{
    auto asyncTaskEv = ev->CastTo<AsyncTaskPollerEvent>();

    if(asyncTaskEv->_asyncTask && asyncTaskEv->_asyncTask->_handler)
        asyncTaskEv->_asyncTask->_handler->Invoke();
}

void Poller::_OnObjectEvent(PollerEvent *ev)
{
    auto stubEv = ev->CastTo<StubPollerEvent>();

    // 是个对象事件的返回消息
    if (stubEv->_isResponse)
    {
        _OnObjectEventResponse(stubEv);
        return;
    }

    _OnObjectEventRequest(stubEv);
}

void Poller::_OnBatchPollerEvent(PollerEvent *ev)
{
    auto batchEv = ev->CastTo<BatchPollerEvent>();

    if(LIKELY(batchEv->_events))
    {
        for(auto node = batchEv->_events->Begin(); node; )
        {
            auto subEv = node->_data;
            node = batchEv->_events->Erase(node);

#if _DEBUG
            // 事件处理
            try
            {
                auto iter = _pollerEventHandler.find(subEv->_type);
                if(LIKELY(iter != _pollerEventHandler.end()))
                    iter->second->Invoke(subEv);
            }
            catch(const std::exception& e)
            {
                if(g_Log->IsEnable(LogLevel::Error))
                    g_Log->Error(LOGFMT_OBJ_TAG("Poller handler err:%s, data:%s, poller:%s"), e.what(), subEv->ToString().c_str(), ToString().c_str());
            }
            catch(...)
            {
                if(g_Log->IsEnable(LogLevel::Error))
                    g_Log->Error(LOGFMT_OBJ_TAG("Poller handler unknown err, data:%s, poller:%s"), subEv->ToString().c_str(), ToString().c_str());
            }
#else
            // 事件处理
            auto iter = _pollerEventHandler.find(subEv->_type);
            if(LIKELY(iter != _pollerEventHandler.end()))
                iter->second->Invoke(subEv);
#endif

            subEv->Release();
        }
    }
}

void Poller::_OnActionPollerEvent(PollerEvent *ev)
{
    auto actionEv = ev->CastTo<ActionPollerEvent>();
    if(LIKELY(actionEv->_action))
        actionEv->_action->Invoke();
}

void Poller::_OnApplyChannelEvent(StubPollerEvent *ev)
{
    auto applyEv = ev->CastTo<ObjectPollerEvent<ApplyChannelEvent>>();

    auto queue = SPSCQueue<PollerEvent *>::NewThreadLocal_SPSCQueue();
    auto channelId = Channel::GenChannelId();
    _channelIdRefQueue.insert(std::make_pair(channelId, queue));
    _msgQueues.push_back(queue);

    if(g_Log->IsEnable(LogLevel::Debug))
        g_Log->Debug(LOGFMT_OBJ_TAG("apply channel applyEv: %s, channelId:%llu, src poller:%s => target poller:%s,")
            , applyEv->ToString().c_str(), channelId, applyEv->_srcPoller->ToString().c_str(), ToString().c_str());

    // 返回包
    auto res = ApplyChannelEventResponse::New_ApplyChannelEventResponse();
    res->_result._channelId = channelId;
    res->_result._queue = queue;
    applyEv->_srcPoller->SendResponse(ev->_stub, res);
}
//
// void Poller::_OnDestroyChannelEvent(StubPollerEvent *ev)
// {
//     auto destroyEv = ev->CastTo<ObjectPollerEvent<DestroyChannelEvent>>();
//
//     auto iter = _idRefChannel.find(destroyEv->_obj->_channelId);
//     if(UNLIKELY(iter == _idRefChannel.end()))
//         return;
//
//     if(g_Log->IsEnable(LogLevel::Debug))
//         g_Log->Debug(LOGFMT_OBJ_TAG("destroy channel id:%llu"), iter->first);
//
//     _targetPollerRefChannel.erase(iter->second->GetTarget());
//     Channel::DeleteThreadLocal_Channel(iter->second);
//     _idRefChannel.erase(iter);
// }

void Poller::Clear()
{
    // _Clear();
    CompObject::Clear();
}

LibString Poller::ToString() const
{
    LibString info;
    info.AppendFormat("%s", CompObject::ToString().c_str());

    info.AppendFormat("_maxPieceTimeInMicroseconds:%llu, ", _maxPieceTime.GetTotalCount())
        .AppendFormat("_workThreadId:%llu, ", _workThreadId.load(std::memory_order_acquire))
        .AppendFormat("_isEnable:%d, ", _isEnable.load(std::memory_order_acquire))
        .AppendFormat("_isClosed:%d, ", _isQuitLoop.load(std::memory_order_acquire))
        .AppendFormat("timer loaded:%llu, ", _timerMgr ? _timerMgr->GetTimerLoaded() : 0)
        .AppendFormat("dirty helper loaded:%llu, ", _dirtyHelper ? _dirtyHelper->GetLoaded() : 0)
        .AppendFormat("common events count:%llu, ", _commonEvents ? _commonEvents->Size() : 0)
        .AppendFormat("_eventAmountLeft:%llu, ", _eventAmountLeft.load(std::memory_order_acquire))
        ;

    return info;
}

UInt64 Poller::CalcLoadScore() const
{
    const UInt64 timerLoaded = _timerMgr->GetTimerLoaded();
    const UInt64 dirtyLoaded = _dirtyHelper->GetLoaded();
    Int64 eventsLoaded = _eventAmountLeft.load(std::memory_order_acquire);
    eventsLoaded = (eventsLoaded <= 0) ? 0 : eventsLoaded;

    return static_cast<UInt64>((timerLoaded * TIMER_LOADED + dirtyLoaded * DIRTY_LOADED + eventsLoaded * EVENTS_LOADED) / ((TIMER_LOADED + DIRTY_LOADED + EVENTS_LOADED) * 1.0 + 1));
}

void Poller::Subscribe(Int32 eventType, KERNEL_NS::IDelegate<void, KERNEL_NS::PollerEvent *> *deleg)
{
    auto iter = _pollerEventHandler.find(eventType);
    if(iter != _pollerEventHandler.end())
    {
        auto cb = iter->second;
        g_Log->Warn(LOGFMT_OBJ_TAG("repeat eventType:%d callback, old callback owner:%s, callback:%s , and will replace with new one: owner:%s, callback:%s")
        ,eventType, cb->GetOwnerRtti().c_str(), cb->GetCallbackRtti().c_str(), deleg->GetOwnerRtti().c_str(), deleg->GetCallbackRtti().c_str());
        cb->Release();

        iter->second = deleg;
        return;
    }

    _pollerEventHandler.insert(std::make_pair(eventType, deleg));
}

void Poller::SetMaxPieceTime(const TimeSlice &piece)
{
    _maxPieceTime.SetMicroseconds(static_cast<UInt64>(piece.GetTotalMicroSeconds()));
}

const LibCpuSlice &Poller::GetMaxPieceTime() const
{
    return _maxPieceTime;
}

bool Poller::PrepareLoop()
{
    // _workThreadId.store(SystemUtil::GetCurrentThreadId(), std::memory_order_release);
    // _timerMgr->Launch(DelegateFactory::Create(this, &Poller::WakeupEventLoop));
    if(LIKELY(_prepareEventWorkerHandler))
    {
        if(!_prepareEventWorkerHandler->Invoke(this))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("prepare event worker fail poller info:%s"), ToString().c_str());
            SetErrCode(NULL, Status::PollerFail);
            return false;
        }
    }

    g_Log->Debug(LOGFMT_OBJ_TAG("poller prepare loop poller info:%s"), ToString().c_str());

    return true;
}

void Poller::QuicklyLoop()
{
    // 部分数据准备
    LibString errLog;

    g_Log->Debug(LOGFMT_OBJ_TAG("poller event worker ready poller info:%s"), ToString().c_str());

    const UInt64 pollerId = GetId();
    
    #if ENABLE_POLLER_PERFORMANCE
    const UInt64 curThreadId = _workThreadId.load(std::memory_order_acquire);
    #endif

    const UInt64 maxSleepMilliseconds = _maxSleepMilliseconds;

    PollerEvent *ev = NULL;
    for(;;)
    {
        // 没有事件且没有脏处理则等待
        if((_eventAmountLeft.load(std::memory_order_acquire) == 0) && !_dirtyHelper->HasDirty())
        {
            // quit仅考虑消息是否处理完,以及脏是否处理完,定时器不需要考虑,否则如果过期时间设置了0则无法退出
            if(UNLIKELY(_isQuitLoop.load(std::memory_order_acquire)))
                break;

            if(!_timerMgr->HasExpired())
            {
                _eventGuard.Lock();
                _eventGuard.TimeWait(maxSleepMilliseconds);
                _eventGuard.Unlock();
            }
        }

        // 处理公共消息
        ev = NULL;
        if (_commonEvents->TryPop(ev))
        {
            _eventAmountLeft.fetch_sub(1, std::memory_order_release);
            _consumEventCount.fetch_add(1, std::memory_order_release);

            // 事件处理
            auto iter = _pollerEventHandler.find(ev->_type);
            if(LIKELY(iter != _pollerEventHandler.end()))
                iter->second->Invoke(ev);

            ev->Release();
        }

        // 处理每个poller的消息
        const Int32 queueCount = static_cast<Int32>(_msgQueues.size());
        for (Int32 idx = 0; idx < queueCount; ++idx)
        {
            auto queue = _msgQueues[idx];
            ev = NULL;
            if (queue->TryPop(ev))
            {
                _eventAmountLeft.fetch_sub(1, std::memory_order_release);
                _consumEventCount.fetch_add(1, std::memory_order_release);

                // 事件处理
                auto iter = _pollerEventHandler.find(ev->_type);
                if(LIKELY(iter != _pollerEventHandler.end()))
                    iter->second->Invoke(ev);

                ev->Release();
            }
        }

        auto curSize = _localEvents->GetAmount();
        for (UInt64 idx = 0; idx < curSize; ++idx)
        {
            auto iter = _localEvents->Begin();
            ev = iter->_data;
            _localEvents->Erase(iter);

            _eventAmountLeft.fetch_sub(1, std::memory_order_release);
            _consumEventCount.fetch_add(1, std::memory_order_release);

            // 事件处理
            auto iterHandler = _pollerEventHandler.find(ev->_type);
            if(LIKELY(iterHandler != _pollerEventHandler.end()))
                iterHandler->second->Invoke(ev);

            ev->Release();
        }

        if(UNLIKELY(_dirtyHelper->HasDirty()))
        {
            #if ENABLE_POLLER_PERFORMANCE
            dirtyHandled = _dirtyHelper->Purge(&errLog);
            #else
            _dirtyHelper->Purge(&errLog);
            #endif
            if(UNLIKELY(!errLog.empty()))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("poller dirty helper has err:%s, poller id:%llu"), errLog.c_str(), pollerId);      
                errLog.clear();
            }
        }

        // 处理定时器
        #if ENABLE_POLLER_PERFORMANCE
        auto handled = _timerMgr->Drive();
        #else
        _timerMgr->Drive();
        #endif

        // if(_onTick)
        //     _onTick->Invoke();
    }

    g_Log->Debug(LOGFMT_OBJ_TAG("poller worker down poller info:%s"), ToString().c_str());
}

void Poller::SafeEventLoop()
{
    // 部分数据准备
    LibString errLog;

    g_Log->Debug(LOGFMT_OBJ_TAG("poller event worker ready poller info:%s"), ToString().c_str());

    const UInt64 pollerId = GetId();
    
    #if ENABLE_POLLER_PERFORMANCE
    const UInt64 curThreadId = _workThreadId.load(std::memory_order_acquire);
    #endif

    const UInt64 maxSleepMilliseconds = _maxSleepMilliseconds;
    PollerEvent *ev = NULL;

    EVENTLOOP_BEGIN:

    // #ifndef _DEBUG
    try
    {
    // #endif

        for(;;)
        {
            // 没有事件且没有脏处理则等待
            if((_eventAmountLeft.load(std::memory_order_acquire) == 0) && !_dirtyHelper->HasDirty())
            {
                // quit仅考虑消息是否处理完,以及脏是否处理完,定时器不需要考虑,否则如果过期时间设置了0则无法退出
                if(UNLIKELY(_isQuitLoop.load(std::memory_order_acquire)))
                    break;

                if(!_timerMgr->HasExpired())
                {
                    // 第一个超时的时间间隔
                    auto firstExpiredNano = _timerMgr->GetFirstExpiredNano();

                    _eventGuard.Lock();
                    _isWaiting.store(true, std::memory_order_release);
                    if(firstExpiredNano > 0)
                    {
                        firstExpiredNano /= KERNEL_NS::TimeDefs::NANO_SECOND_PER_MILLI_SECOND;
                        _eventGuard.TimeWait(firstExpiredNano  > maxSleepMilliseconds ? firstExpiredNano : maxSleepMilliseconds);
                    }
                    else
                    {
                        _eventGuard.Wait();
                    }
                    _isWaiting.store(false, std::memory_order_release);
                    _eventGuard.Unlock();
                }
            }

            
            // 处理公共消息
            ev = NULL;
            if (_commonEvents->TryPop(ev))
            {
                _eventAmountLeft.fetch_sub(1, std::memory_order_release);
                _consumEventCount.fetch_add(1, std::memory_order_release);

                try
                {
                    // 事件处理
                    auto iter = _pollerEventHandler.find(ev->_type);
                    if(LIKELY(iter != _pollerEventHandler.end()))
                        iter->second->Invoke(ev);
                }
                catch(const std::exception& e)
                {
                    if(g_Log->IsEnable(LogLevel::Error))
                        g_Log->Error(LOGFMT_OBJ_TAG("Poller handler err:%s, data:%s, poller:%s"), e.what(), ev->ToString().c_str(), ToString().c_str());
                }
                catch(...)
                {
                    if(g_Log->IsEnable(LogLevel::Error))
                        g_Log->Error(LOGFMT_OBJ_TAG("Poller handler unknown err, data:%s, poller:%s"), ev->ToString().c_str(), ToString().c_str());
                }

                ev->Release();
            }

            // 处理每个poller的消息
            const Int32 queueCount = static_cast<Int32>(_msgQueues.size());
            for (Int32 idx = 0; idx < queueCount; ++idx)
            {
                auto queue = _msgQueues[idx];
                ev = NULL;
                if (queue->TryPop(ev))
                {
                    _eventAmountLeft.fetch_sub(1, std::memory_order_release);
                    _consumEventCount.fetch_add(1, std::memory_order_release);

                    try
                    {
                        // 事件处理
                        auto iter = _pollerEventHandler.find(ev->_type);
                        if(LIKELY(iter != _pollerEventHandler.end()))
                            iter->second->Invoke(ev);
                    }
                    catch(const std::exception& e)
                    {
                        if(g_Log->IsEnable(LogLevel::Error))
                            g_Log->Error(LOGFMT_OBJ_TAG("Poller handler err:%s, data:%s, poller:%s"), e.what(), ev->ToString().c_str(), ToString().c_str());
                    }
                    catch(...)
                    {
                        if(g_Log->IsEnable(LogLevel::Error))
                            g_Log->Error(LOGFMT_OBJ_TAG("Poller handler unknown err, data:%s, poller:%s"), ev->ToString().c_str(), ToString().c_str());
                    }

                    ev->Release();
                }
            }

            // 只处理有限个消息
            auto curSize = _localEvents->GetAmount();
            for (UInt64 idx = 0; idx < curSize; ++idx)
            {
                auto iter = _localEvents->Begin();
                ev = iter->_data;
                _localEvents->Erase(iter);

                _eventAmountLeft.fetch_sub(1, std::memory_order_release);
                _consumEventCount.fetch_add(1, std::memory_order_release);

                // 事件处理
                try
                {
                    auto iterHandler = _pollerEventHandler.find(ev->_type);
                    if(LIKELY(iterHandler != _pollerEventHandler.end()))
                        iterHandler->second->Invoke(ev);
                }
                catch(const std::exception& e)
                {
                    if(g_Log->IsEnable(LogLevel::Error))
                        g_Log->Error(LOGFMT_OBJ_TAG("Poller handler err:%s, data:%s, poller:%s"), e.what(), ev->ToString().c_str(), ToString().c_str());
                }
                catch(...)
                {
                    if(g_Log->IsEnable(LogLevel::Error))
                        g_Log->Error(LOGFMT_OBJ_TAG("Poller handler unknown err, data:%s, poller:%s"), ev->ToString().c_str(), ToString().c_str());
                }

                ev->Release();
            }
            
            // 脏处理
            #if ENABLE_POLLER_PERFORMANCE
            Int64 dirtyHandled = 0;
            #endif

            if(UNLIKELY(_dirtyHelper->HasDirty()))
            {
                try
                {
                    #if ENABLE_POLLER_PERFORMANCE
                    dirtyHandled = _dirtyHelper->Purge(&errLog);
                    #else
                    _dirtyHelper->Purge(&errLog);
                    #endif
                    if(UNLIKELY(!errLog.empty()))
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("poller dirty helper has err:%s, poller id:%llu"), errLog.c_str(), pollerId);      
                        errLog.clear();
                    }
                }
                catch(const std::exception& e)
                {
                    if(LIKELY(g_Log->IsEnable(LogLevel::Error)))
                       g_Log->Error(LOGFMT_OBJ_TAG("Poller Dirty Purge error:%s, poller:%s"), e.what(), ToString().c_str());
                }
                catch(...)
                {
                    if(LIKELY(g_Log->IsEnable(LogLevel::Error)))
                       g_Log->Error(LOGFMT_OBJ_TAG("Poller Dirty Purge unknown error, poller:%s"), ToString().c_str());
                }
            }

            try
            {
                // 处理定时器
                #if ENABLE_POLLER_PERFORMANCE
                auto handled = _timerMgr->Drive();
                #else
                _timerMgr->Drive();
                #endif

                // if(_onTick)
                //     _onTick->Invoke();
            }
            catch(const std::exception& e)
            {
                if(LIKELY(g_Log->IsEnable(LogLevel::Error)))
                    g_Log->Error(LOGFMT_OBJ_TAG("Poller timer or tick error:%s, poller:%s"), e.what(), ToString().c_str());
            }
            catch(...)
            {
                if(LIKELY(g_Log->IsEnable(LogLevel::Error)))
                    g_Log->Error(LOGFMT_OBJ_TAG("Poller timer or tick unknown error poller:%s"), ToString().c_str());
            }
            
            // 当前帧性能信息记录
            #ifdef ENABLE_POLLER_PERFORMANCE
                const auto &elapseTime = nowCounter.Update() - performaceStart;
                if(UNLIKELY(elapseTime >= _maxPieceTime))
                {
                    if(g_Log->IsEnable(LogLevel::NetInfo))
                        g_Log->Info(LOGFMT_OBJ_TAG("[poller performance] poller id:%llu thread id:%llu, use time over max piece time, use time:%llu(ms), max piece time:%llu(ms), consume event count:%llu, time out handled count:%lld, dirty handled count:%lld")
                    , pollerId, curThreadId, elapseTime.GetTotalMilliseconds(), _maxPieceTime.GetTotalMilliseconds(), curConsumeEventsCount, handled, dirtyHandled);
                }
            #endif
        }

        // #ifndef _DEBUG
    }
    catch(const std::exception& e)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("exception:%s happen"), e.what());
        goto EVENTLOOP_BEGIN;
    }
    catch(...)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("unknown eception happen..."));
        goto EVENTLOOP_BEGIN;
    }
    // #endif
    
    g_Log->Debug(LOGFMT_OBJ_TAG("poller worker down poller info:%s"), ToString().c_str());
}

void Poller::OnLoopEnd()
{
    // worker 线程关闭
    if(LIKELY(_onEventWorkerCloseHandler))
        _onEventWorkerCloseHandler->Invoke(this);

    g_Log->Info(LOGFMT_OBJ_TAG("poller loop end poller info:%s"), ToString().c_str());
}

void Poller::OnMonitor(PollerCompStatistics &statistics)
{
    statistics._loadedScore = CalcLoadScore();
    statistics._pollerGenQps = GetAndResetGenCount();
    statistics._pollerConsumeQps = GetAndResetConsumCount();
    statistics._pollerBacklog = GetEventAmount();
    statistics._isEnable = IsEnable();
    statistics._pollerId = GetId();
}

bool Poller::CanQuit() const
{
    if(_eventAmountLeft.load(std::memory_order_acquire) != 0)
        return false;

    if(_dirtyHelper && _dirtyHelper->HasDirty())
        return false;

    // 不需要考虑定时器事件,因为当要quit的时候应该所有模块的定时器都是停止的
    // if(_timerMgr && !_timerMgr->IsDriving() && _timerMgr->HasExpired())
    //     return false;

    return true;
}

void Poller::Push(LibList<PollerEvent *> *evList)
{
    if(UNLIKELY(!_isEnable.load(std::memory_order_acquire)))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("poller is destroying obj id:%llu, evList count:%llu")
                        , GetId(), evList->GetAmount());

        ContainerUtil::DelContainer(*evList, [](PollerEvent *ev){
            ev->Release();
        });
        LibList<PollerEvent *>::Delete_LibList(evList);
        return;
    }


    auto batchEv = BatchPollerEvent::New_BatchPollerEvent();
    batchEv->_events = evList;

    _Push(batchEv);
}

void Poller::Push(IDelegate<void> *action)
{
    if(UNLIKELY(!_isEnable.load(std::memory_order_acquire)))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("poller is destroying poller obj id:%llu"), GetId());
        action->Release();
        return;
    }

    auto ev = ActionPollerEvent::New_ActionPollerEvent();
    ev->_action = action;
    _Push(ev);
}

void Poller::Push(PollerEvent *ev)
{
    if(UNLIKELY(!_isEnable.load(std::memory_order_acquire)))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("poller is destroying obj id:%llu, ev:%s"), GetId(), ev->ToString().c_str());
        ev->Release();
        return;
    }

    _Push(ev);
}

void Poller::_Clear()
{
    _isEnable.store(false, std::memory_order_release);
    _isQuitLoop.store(false, std::memory_order_release);

    if(LIKELY(_timerMgr))
    {
        TimerMgr::Delete_TimerMgr(_timerMgr);
        _timerMgr = NULL;
    }

    if(LIKELY(_dirtyHelper))
    {
        LibDirtyHelper<void *, UInt32>::Delete_LibDirtyHelper(_dirtyHelper);
        _dirtyHelper = NULL;
    }

    CRYSTAL_DELETE_SAFE(_prepareEventWorkerHandler);
    CRYSTAL_DELETE_SAFE(_onEventWorkerCloseHandler);

    g_Log->Info(LOGFMT_OBJ_TAG("will destroy poller events list %s"), ToString().c_str());

    ContainerUtil::DelContainer2(_pollerEventHandler);

    CRYSTAL_RELEASE_SAFE(_onTick);

    _targetPollerRefChannel.clear();
    ContainerUtil::DelContainer(_idRefChannel, [](Channel *channel)
    {
        Channel::DeleteThreadLocal_Channel(channel);
    });
    ContainerUtil::DelContainer(_msgQueues, [this](SPSCQueue<PollerEvent *> *queue)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have some unhandled events amount:%llu, poller info:%s"), queue->Size(), ToString().c_str());

        while (auto elem = queue->Front())
        {
            g_Log->Info(LOGFMT_OBJ_TAG("event:%s, PollerId:%llu"), (*elem)->ToString().c_str(), GetId());
            queue->Pop();
        }
        
        SPSCQueue<PollerEvent *>::Delete_SPSCQueue(queue);
    });
    _channelIdRefQueue.clear();

    if (LIKELY(_commonEvents))
    {
        if (!_commonEvents->Empty())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("have some unhandled mpmc events amount:%llu, poller info:%s"), _commonEvents->Size(), ToString().c_str());

            while (!_commonEvents->Empty())
            {
                PollerEvent *ev = NULL;
                if (_commonEvents->TryPop(ev))
                {
                    g_Log->Info(LOGFMT_OBJ_TAG("mpmc event:%s, PollerId:%llu"), ev->ToString().c_str(), GetId());
                }
            }
        }

        MPMCQueue<PollerEvent *, 16*16*2*1024>::DeleteThreadLocal_MPMCQueue(_commonEvents);
    }
    _commonEvents = NULL;

    if (_localEvents)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have some unhandled local events amount:%llu, poller info:%s"), _localEvents->GetAmount(), ToString().c_str());

        for (auto iter = _localEvents->Begin(); iter; )
        {
            auto data = iter->_data;
            iter = _localEvents->Erase(iter);

            g_Log->Info(LOGFMT_OBJ_TAG("local event:%s, PollerId:%llu"), data->ToString().c_str(), GetId());

            data->Release();
        }

        LibList<PollerEvent *, KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibList(_localEvents);
    }
    _localEvents = NULL;
    
    g_Log->Info(LOGFMT_OBJ_TAG("destroyed poller events list %s"), ToString().c_str());
}

void Poller::_OnObjectEventResponse(StubPollerEvent *ev)
{
    auto stub = ev->_stub;
    auto iter = _stubRefCb.find(stub);
    if (UNLIKELY(iter == _stubRefCb.end()))
    {
        if (g_Log->IsEnable(KERNEL_NS::LogLevel::Warn))
            g_Log->Warn(LOGFMT_OBJ_TAG("stub:%llu, not found, ev:%s"), ev->_stub, ev->ToString().c_str());
        return;
    }

    iter->second->Invoke(ev);

    // 在Invoke中可能被移除
    iter = _stubRefCb.find(stub);
    if(iter != _stubRefCb.end())
    {
        iter->second->Release();
        _stubRefCb.erase(iter);
    }

}

// 事件请求处理
void Poller::_OnObjectEventRequest(StubPollerEvent *ev)
{
    auto iter = _objTypeIdRefCallback.find(ev->_objTypeId);
    if (iter == _objTypeIdRefCallback.end())
    {
        if (g_Log->IsEnable(KERNEL_NS::LogLevel::Warn))
            g_Log->Warn(LOGFMT_OBJ_TAG("objTypeId:%llu, have no request callback, ev:%s"), ev->_objTypeId, ev->ToString().c_str());
        return;
    }

    iter->second->Invoke(ev);
}

KERNEL_END
