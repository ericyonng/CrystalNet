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
#include <kernel/comp/Poller/Poller.h>
#include <kernel/comp/Timer/Timer.h>
#include <kernel/comp/thread/thread.h>
#include <kernel/comp/TimeSlice.h>
#include <kernel/comp/Utils/TlsUtil.h>

static ALWAYS_INLINE bool IsPriorityEvenetsQueueEmpty(const std::vector<KERNEL_NS::LibList<KERNEL_NS::PollerEvent *, KERNEL_NS::_Build::MT> *> &queue)
{
    if(UNLIKELY(queue.empty()))
        return true;

    for(auto &evList:queue)
    {
        if(!evList->IsEmpty())
            return false;
    }

    return true;
}

static ALWAYS_INLINE UInt64 GetPriorityEvenetsQueueElemCount(const std::vector<KERNEL_NS::LibList<KERNEL_NS::PollerEvent *, KERNEL_NS::_Build::MT> *> &queue)
{
    if(UNLIKELY(queue.empty()))
        return 0;

    UInt64 count = 0;
    for(auto &evList:queue)
        count += evList->GetAmount();

    return count;
}

static ALWAYS_INLINE void MergePriorityEvenetsQueue(std::vector<KERNEL_NS::LibList<KERNEL_NS::PollerEvent *, KERNEL_NS::_Build::MT> *> &from,std::vector<KERNEL_NS::LibList<KERNEL_NS::PollerEvent *, KERNEL_NS::_Build::MT> *> &to)
{
    if(UNLIKELY(from.empty()))
        return;

    Int32 queueSize = static_cast<Int32>(from.size());
    for(Int32 idx = 0; idx < queueSize; ++idx)
    {
        auto fromQueue = from[idx];
        auto toQueue = to[idx];
        toQueue->MergeTail(fromQueue);
    }
}

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(Poller);

Poller::Poller()
:_maxPieceTime(LibCpuSlice::FromMilliseconds(8))  // ?????????8ms
,_workThreadId{0}
,_isWorking{false}
,_isEnable{true}
,_isQuitLoop{false}
,_waitingNotQuit{0}
,_maxSleepMilliseconds(1)
,_timerMgr(TimerMgr::New_TimerMgr())
,_dirtyHelper(LibDirtyHelper<void *, UInt32>::New_LibDirtyHelper())
,_prepareEventWorkerHandler(NULL)
,_onEventWorkerCloseHandler(NULL)
,_eventHandler(NULL)
,_eventsList(ConcurrentPriorityQueue<PollerEvent *>::New_ConcurrentPriorityQueue())
,_handlingEventCount{0}
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

Int32 Poller::_OnInit()
{
    Int32 ret = CompObject::_OnInit();
    if(ret != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("comp init fail ret:%d"), ret);
        return ret;
    }

    _isWorking = false;
    _isQuitLoop = false;
    _workThreadId = 0;
    _eventsList->Init();

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

    _isQuitLoop = false;
    g_Log->Debug(LOGFMT_OBJ_TAG("poller started %s"), ToString().c_str());
    return Status::Success;
}

void Poller::_OnWillClose()
{
    g_Log->Debug(LOGFMT_OBJ_TAG("poller will close %s"), ToString().c_str());

    _isQuitLoop = true;
    WakeupEventLoop();

    CompObject::_OnWillClose();
}

void Poller::_OnClose()
{
    g_Log->Debug(LOGFMT_OBJ_TAG("poller close %s"), ToString().c_str());
    _Clear();
    CompObject::_OnClose();
}

void Poller::Clear()
{
    _Clear();
    CompObject::Clear();
}

LibString Poller::ToString() const
{
    LibString info;
    info.AppendFormat("%s", CompObject::ToString().c_str());

    info.AppendFormat("_maxPieceTimeInMicroseconds:%llu, ", _maxPieceTime.GetTotalCount())
        .AppendFormat("_workThreadId:%llu, ", _workThreadId.load())
        .AppendFormat("_isWorking:%d, ", _isWorking.load())
        .AppendFormat("_isEnable:%d, ", _isEnable.load())
        .AppendFormat("_isClosed:%d, ", _isQuitLoop.load())
        .AppendFormat("timer loaded:%llu, ", _timerMgr ? _timerMgr->GetTimerLoaded() : 0)
        .AppendFormat("dirty helper loaded:%llu, ", _dirtyHelper ? _dirtyHelper->GetLoaded() : 0)
        .AppendFormat("events count:%llu, ", _eventsList ? _eventsList->GetAmount() : 0)
        ;

    return info;
}

UInt64 Poller::CalcLoadScore() const
{
    const UInt64 timerLoaded = _timerMgr->GetTimerLoaded();
    const UInt64 dirtyLoaded = _dirtyHelper->GetLoaded();
    const UInt64 eventsLoaded = _eventsList->GetAmount() + _handlingEventCount;

    return static_cast<UInt64>((timerLoaded * TIMER_LOADED + dirtyLoaded * DIRTY_LOADED + eventsLoaded * EVENTS_LOADED) / ((TIMER_LOADED + DIRTY_LOADED + EVENTS_LOADED) * 1.0 + 1));
}

void Poller::SetMaxPieceTime(const TimeSlice &piece)
{
    _maxPieceTime.SetMicroseconds(static_cast<UInt64>(piece.GetTotalMicroSeconds()));
}

bool Poller::PrepareLoop()
{
    _workThreadId = SystemUtil::GetCurrentThreadId();
    _timerMgr->Launch(DelegateFactory::Create(this, &Poller::WakeupEventLoop));
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

void Poller::EventLoop()
{
    std::vector<LibList<PollerEvent *, _Build::MT> *> priorityEvents;
    const Int32 priorityQueueSize = _eventsList->GetMaxLevel() + 1;
    priorityEvents.resize(priorityQueueSize);
    for(Int32 idx = 0; idx < priorityQueueSize; ++idx)
        priorityEvents[idx] = LibList<PollerEvent *, _Build::MT>::New_LibList();

    // ??????????????????
    LibString errLog;

    _isWorking = false;

    // ????????????merge????????????
    UInt64 eventsAmount = _eventsList->MergeTailAllTo(priorityEvents);

    g_Log->Debug(LOGFMT_OBJ_TAG("poller event worker ready poller info:%s"), ToString().c_str());

    LibCpuCounter deadline;
    LibCpuCounter nowCounter;

    #ifdef _DEBUG
     LibCpuCounter performaceStart;
    #endif

    const UInt64 pollerId = GetId();
    const UInt64 maxSleepMilliseconds = _maxSleepMilliseconds;

    while (!_isQuitLoop || (eventsAmount != 0) || (_waitingNotQuit > 0) || _dirtyHelper->HasDirty())
    {
        // ???????????????????????????????????????
        if((eventsAmount == 0) && !_dirtyHelper->HasDirty() && !_timerMgr->HasExpired())
        {
            _eventGuard.Lock();
            _eventGuard.TimeWait(maxSleepMilliseconds);
            _eventGuard.Unlock();
        }
        

        // ????????????????????????
        if(!_eventsList->IsEmpty())
            eventsAmount += _eventsList->MergeTailAllTo(priorityEvents);
        _handlingEventCount = eventsAmount;

        // WORKING START
        _isWorking = true;

        deadline.Update() += _maxPieceTime;
        #ifdef _DEBUG
         performaceStart = deadline;
        #endif

        // ????????????
        Int32 idx = 0;
        Int32 loopCount = 0;

        #ifdef _DEBUG
         UInt64 curConsumeEventsCount = 0;
        #endif

        for (;;)
        {
            idx = loopCount++ % priorityQueueSize;

            // ?????????????????????????????????
            auto sunList = priorityEvents[idx];
            auto node = sunList->Begin();
            if(LIKELY(node))
            {
                auto data = node->_data;

                // ????????????
                if(LIKELY(_eventHandler))
                    _eventHandler->Invoke(data);
                data->Release();
                sunList->Erase(node);

                #ifdef _DEBUG
                 ++curConsumeEventsCount;
                #endif

                --eventsAmount;
            }

            // ?????????
            if(UNLIKELY(nowCounter.Update() >=  deadline))
                break;

            // ????????????
            if(eventsAmount == 0)
                break;
        }

        _handlingEventCount = eventsAmount;

        // ?????????
        _dirtyHelper->Purge(nowCounter.Update(), &errLog);
        if(UNLIKELY(!errLog.empty()))
            g_Log->Warn(LOGFMT_OBJ_TAG("poller dirty helper has err:%s, poller id:%llu"), errLog.c_str(), pollerId);      

        // ???????????????
        const auto curTime = TimeUtil::GetMicroTimestamp();
        _timerMgr->Drive(curTime);

        // ???????????????????????????
        #ifdef _DEBUG
            const auto &elapseTime = nowCounter.Update() - performaceStart;
            g_Log->Debug(LOGFMT_OBJ_TAG("poller performance poller id:%llu, \n"
                                        "_maxPieceTimeInMicroseconds:%lld, threadId:%llu, use microseconds:%llu, curConsumeEventsCount:%llu")
                                        , pollerId, _maxPieceTime.GetTotalMicroseconds(), _workThreadId.load(), elapseTime.GetTotalMicroseconds(), curConsumeEventsCount);
        #endif

        // WORKING END
        _isWorking = false;

        // GET EVENTS
        eventsAmount += _eventsList->MergeTailAllTo(priorityEvents);

        _handlingEventCount = eventsAmount;
    }

    const auto leftElemCount = GetPriorityEvenetsQueueElemCount(priorityEvents);
    if(leftElemCount != 0)
        g_Log->Warn(LOGFMT_OBJ_TAG("has unhandled events left:%llu, poller info:%s"), leftElemCount, ToString().c_str());
    
    ContainerUtil::DelContainer(priorityEvents, [this](LibList<PollerEvent *, _Build::MT> *evList){
        ContainerUtil::DelContainer(*evList, [this](PollerEvent *ev){
            g_Log->Warn(LOGFMT_OBJ_TAG("event type:%d, not handled when poller will closed."), ev->_type);
            ev->Release();
        });
        LibList<PollerEvent *, _Build::MT>::Delete_LibList(evList);
    });

    g_Log->Debug(LOGFMT_OBJ_TAG("poller worker down poller info:%s"), ToString().c_str());
}

void Poller::OnLoopEnd()
{
    // worker ????????????
    if(LIKELY(_onEventWorkerCloseHandler))
        _onEventWorkerCloseHandler->Invoke(this);

    if(LIKELY(_dirtyHelper))
        _dirtyHelper->Destroy();

    g_Log->Info(LOGFMT_OBJ_TAG("poller loop end poller info:%s"), ToString().c_str());
}

void Poller::_Clear()
{
    _isEnable = false;
    _isWorking = false;
    _isQuitLoop = false;

    // ???????????????tls??????
    auto defObj = TlsUtil::GetDefTls();
    defObj->_poller = NULL;
    defObj->_pollerTimerMgr = NULL;
    
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
    CRYSTAL_DELETE_SAFE(_eventHandler);

    if(LIKELY(_eventsList))
    {
        if(UNLIKELY(_eventsList->GetAmount() != 0))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("have some unhandled events amount:%llu, poller info:%s"), _eventsList->GetAmount(), ToString().c_str());

            // ????????????
            std::vector<LibList<PollerEvent *> *> allList;
            allList.resize(_eventsList->GetMaxLevel() + 1);
            for(Int32 idx = 0; idx < static_cast<Int32>(allList.size()); ++idx)
                allList[idx] = LibList<PollerEvent *>::New_LibList();

            _eventsList->SwapAll(allList);
            for(Int32 idx = 0; idx < static_cast<Int32>(allList.size()); ++idx)
            {
                auto eventList = allList[idx];
                for(auto node = eventList->Begin(); node;)
                {
                    node->_data->Release();
                    node = eventList->Erase(node);
                }
            }
        }

        _eventsList->Destroy();
        ConcurrentPriorityQueue<PollerEvent *>::Delete_ConcurrentPriorityQueue(_eventsList);
        _eventsList = NULL;
    }
}

KERNEL_END
