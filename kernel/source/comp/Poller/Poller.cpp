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
#include <kernel/comp/Utils/SignalHandleUtil.h>

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

static ALWAYS_INLINE UInt64 GetPriorityEvenetsQueueElemCount(const KERNEL_NS::LibList<KERNEL_NS::LibList<KERNEL_NS::PollerEvent *, KERNEL_NS::_Build::MT> *> &queue)
{
    if(UNLIKELY(queue.IsEmpty()))
        return 0;

    UInt64 count = 0;
    for(auto node = queue.Begin(); node; node = node->_next)
        count += node->_data->GetAmount();

    return count;
}

static ALWAYS_INLINE UInt64 GetPriorityEvenetsQueueElemCount(const std::vector<KERNEL_NS::LibList<KERNEL_NS::PollerEvent *, KERNEL_NS::_Build::MT> *> &queue)
{
    if(UNLIKELY(queue.empty()))
        return 0;

    UInt64 count = 0;
    for(auto node : queue)
        count += node->GetAmount();

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
:_maxPieceTime(LibCpuSlice::FromMilliseconds(8))  // 经验值8ms
,_workThreadId{0}
,_isEnable{true}
,_isQuitLoop{false}
,_maxSleepMilliseconds(1)
,_loopDetectTimeout(1000)   // 默认1000次循环检查一次
,_timerMgr(TimerMgr::New_TimerMgr())
,_dirtyHelper(LibDirtyHelper<void *, UInt32>::New_LibDirtyHelper())
,_prepareEventWorkerHandler(NULL)
,_onEventWorkerCloseHandler(NULL)
,_eventHandler(NULL)
,_quickEventHandler(NULL)
,_eventsList(ConcurrentPriorityQueue<PollerEvent *>::New_ConcurrentPriorityQueue())
,_eventAmountLeft{0}
,_genEventAmount{0}
,_consumEventCount{0}
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
    Int64 eventsLoaded = _eventAmountLeft;
    eventsLoaded = (eventsLoaded <= 0) ? 0 : eventsLoaded;

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
    LibList<LibList<PollerEvent *, _Build::MT> *> *priorityEvents = LibList<LibList<PollerEvent *, _Build::MT> *>::New_LibList();
    const Int64 priorityQueueSize = static_cast<Int64>(_eventsList->GetMaxLevel() + 1);
    for(Int64 idx = 0; idx < priorityQueueSize; ++idx)
        priorityEvents->PushBack(LibList<PollerEvent *, _Build::MT>::New_LibList());

    // 部分数据准备
    LibString errLog;

    g_Log->Debug(LOGFMT_OBJ_TAG("poller event worker ready poller info:%s"), ToString().c_str());

    LibCpuCounter deadline;
    LibCpuCounter nowCounter;

    #ifdef _DEBUG
     LibCpuCounter performaceStart;
    #endif

    const UInt64 pollerId = GetId();
    const UInt64 maxSleepMilliseconds = _maxSleepMilliseconds;

    UInt64 mergeNumber = 0;
    for(;;)
    {
        // 没有事件且没有脏处理则等待
        if((_eventAmountLeft == 0) && !_dirtyHelper->HasDirty())
        {
            // quit仅考虑消息是否处理完,以及脏是否处理完,定时器不需要考虑,否则如果过期时间设置了0则无法退出
            if(UNLIKELY(_isQuitLoop))
                break;

            if(!_timerMgr->HasExpired())
            {
                _eventGuard.Lock();
                _eventGuard.TimeWait(maxSleepMilliseconds);
                _eventGuard.Unlock();
            }
        }

        // 队列有消息就合并
        if(LIKELY(_eventAmountLeft != 0))
            mergeNumber += _eventsList->MergeTailAllTo(priorityEvents);

        // 处理事件
        #ifdef _DEBUG
         UInt64 curConsumeEventsCount = 0;
        #endif

        Int32 detectTimeoutLoopCount = _loopDetectTimeout;
        deadline.Update() += _maxPieceTime;
        #ifdef _DEBUG
         performaceStart = deadline;
        #endif

        for (auto listNode = priorityEvents->Begin(); LIKELY(mergeNumber != 0);)
        {
            // 切换不同优先级消息队列
            auto node = listNode->_data->Begin();
            if(LIKELY(node))
            {
                auto data = node->_data;

                // 事件处理
                if(LIKELY(_eventHandler))
                    _eventHandler->Invoke(data);
                data->Release();
                listNode->_data->Erase(node);
                --_eventAmountLeft;
                --mergeNumber;
                ++_consumEventCount;

                #ifdef _DEBUG
                 ++curConsumeEventsCount;
                #endif
            }

            listNode = (listNode->_next != NULL) ? listNode->_next : priorityEvents->Begin();
            
            // 片超时
            if(UNLIKELY(--detectTimeoutLoopCount <= 0))
            {
                detectTimeoutLoopCount = _loopDetectTimeout;

                if(UNLIKELY(nowCounter.Update() >=  deadline))
                    break;
            }
        }

        // 脏处理
        if(UNLIKELY(_dirtyHelper->HasDirty()))
        {
            _dirtyHelper->Purge(nowCounter.Update(), &errLog);
            if(UNLIKELY(!errLog.empty()))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("poller dirty helper has err:%s, poller id:%llu"), errLog.c_str(), pollerId);      
                errLog.clear();
            }
        }

        // 处理定时器
        _timerMgr->Drive();

        // 当前帧性能信息记录
        #ifdef _DEBUG
            const auto &elapseTime = nowCounter.Update() - performaceStart;
            g_Log->Debug(LOGFMT_OBJ_TAG("poller performance poller id:%llu, \n"
                                        "_maxPieceTimeInMicroseconds:%lld, threadId:%llu, use microseconds:%llu, curConsumeEventsCount:%llu")
                                        , pollerId, _maxPieceTime.GetTotalMicroseconds(), _workThreadId.load(), elapseTime.GetTotalMicroseconds(), curConsumeEventsCount);
        #endif
    }

    const auto leftElemCount = GetPriorityEvenetsQueueElemCount(*priorityEvents);
    if(leftElemCount != 0)
        g_Log->Warn(LOGFMT_OBJ_TAG("has unhandled events left:%llu, poller info:%s"), leftElemCount, ToString().c_str());
    
    ContainerUtil::DelContainer(*priorityEvents, [this](LibList<PollerEvent *, _Build::MT> *evList){
        ContainerUtil::DelContainer(*evList, [this](PollerEvent *ev){
            g_Log->Warn(LOGFMT_OBJ_TAG("event type:%d, not handled when poller will closed."), ev->_type);
            ev->Release();
        });
        LibList<PollerEvent *, _Build::MT>::Delete_LibList(evList);
    });
    LibList<LibList<PollerEvent *, _Build::MT> *>::Delete_LibList(priorityEvents);
    priorityEvents = NULL;

    g_Log->Debug(LOGFMT_OBJ_TAG("poller worker down poller info:%s"), ToString().c_str());
}

void Poller::SafetyEventLoop()
{
    LibList<LibList<PollerEvent *, _Build::MT> *> *priorityEvents = LibList<LibList<PollerEvent *, _Build::MT> *>::New_LibList();
    const Int64 priorityQueueSize = static_cast<Int64>(_eventsList->GetMaxLevel() + 1);
    for(Int64 idx = 0; idx < priorityQueueSize; ++idx)
        priorityEvents->PushBack(LibList<PollerEvent *, _Build::MT>::New_LibList());

    // 部分数据准备
    LibString errLog;

    g_Log->Debug(LOGFMT_OBJ_TAG("poller event worker ready poller info:%s"), ToString().c_str());

    LibCpuCounter deadline;
    LibCpuCounter nowCounter;

    #ifdef _DEBUG
     LibCpuCounter performaceStart;
    #endif

    const UInt64 pollerId = GetId();
    const UInt64 maxSleepMilliseconds = _maxSleepMilliseconds;

    // TODO:如何安全的恢复栈帧
    // jmp_buf stackFrame;
    UInt64 mergeNumber = 0;
    for(;;)
    {
        // 没有事件且没有脏处理则等待
        if((_eventAmountLeft == 0) && !_dirtyHelper->HasDirty())
        {
            // quit仅考虑消息是否处理完,以及脏是否处理完,定时器不需要考虑,否则如果过期时间设置了0则无法退出
            if(UNLIKELY(_isQuitLoop))
                break;

            if(!_timerMgr->HasExpired())
            {
                _eventGuard.Lock();
                _eventGuard.TimeWait(maxSleepMilliseconds);
                _eventGuard.Unlock();
            }
        }

        // 队列有消息就合并
        if(LIKELY(_eventAmountLeft != 0))
            mergeNumber += _eventsList->MergeTailAllTo(priorityEvents);

        // 处理事件
        #ifdef _DEBUG
         UInt64 curConsumeEventsCount = 0;
        #endif

        Int32 detectTimeoutLoopCount = _loopDetectTimeout;
        deadline.Update() += _maxPieceTime;
        #ifdef _DEBUG
         performaceStart = deadline;
        #endif

        // #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        // auto err = SignalHandleUtil::PushRecoverPoint(&stackFrame);
        // if(LIKELY(err == 0))
        // {
        // #endif
            for (auto listNode = priorityEvents->Begin(); LIKELY(mergeNumber != 0);)
            {
                // 切换不同优先级消息队列
                auto node = listNode->_data->Begin();
                if(LIKELY(node))
                {
                    UniqueSmartPtr<PollerEvent, AutoDelMethods::Release> data = node->_data;
                    listNode->_data->Erase(node);
                    --_eventAmountLeft;
                    --mergeNumber;
                    ++_consumEventCount;

                    #ifdef _DEBUG
                    ++curConsumeEventsCount;
                    #endif

                    // 事件处理
                        if(LIKELY(_eventHandler))
                            _eventHandler->Invoke(data);
                    
                }

                listNode = (listNode->_next != NULL) ? listNode->_next : priorityEvents->Begin();
                
                // 片超时
                if(UNLIKELY(--detectTimeoutLoopCount <= 0))
                {
                    detectTimeoutLoopCount = _loopDetectTimeout;

                    if(UNLIKELY(nowCounter.Update() >=  deadline))
                        break;
                }
            }

        // #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        // }
        // else
        // {
        //     g_Log->Error(LOGFMT_OBJ_TAG("event handler error happen:"));
        // }
        // #endif

        // 脏处理
        if(UNLIKELY(_dirtyHelper->HasDirty()))
        {
        // #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        //     SignalHandleUtil::PopRecoverPoint();
        //     auto err = SignalHandleUtil::PushRecoverPoint(&stackFrame);
        //     if(LIKELY(err == 0))
        //     {
        // #endif
                _dirtyHelper->Purge(nowCounter.Update(), &errLog);
                if(UNLIKELY(!errLog.empty()))
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("poller dirty helper has err:%s, poller id:%llu"), errLog.c_str(), pollerId);      
                    errLog.clear();
                }
        // #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        //     }
        //     else
        //     {
        //         g_Log->Error(LOGFMT_OBJ_TAG("dirty helper purge error"));
        //     }
        // #endif
        }

        // 处理定时器
        _timerMgr->SafetyDrive(stackFrame);

        // 当前帧性能信息记录
        #ifdef _DEBUG
            const auto &elapseTime = nowCounter.Update() - performaceStart;
            g_Log->Debug(LOGFMT_OBJ_TAG("poller performance poller id:%llu, \n"
                                        "_maxPieceTimeInMicroseconds:%lld, threadId:%llu, use microseconds:%llu, curConsumeEventsCount:%llu")
                                        , pollerId, _maxPieceTime.GetTotalMicroseconds(), _workThreadId.load(), elapseTime.GetTotalMicroseconds(), curConsumeEventsCount);
        #endif
    }

    const auto leftElemCount = GetPriorityEvenetsQueueElemCount(*priorityEvents);
    if(leftElemCount != 0)
        g_Log->Warn(LOGFMT_OBJ_TAG("has unhandled events left:%llu, poller info:%s"), leftElemCount, ToString().c_str());
    
    ContainerUtil::DelContainer(*priorityEvents, [this](LibList<PollerEvent *, _Build::MT> *evList){
        ContainerUtil::DelContainer(*evList, [this](PollerEvent *ev){
            g_Log->Warn(LOGFMT_OBJ_TAG("event type:%d, not handled when poller will closed."), ev->_type);
            ev->Release();
        });
        LibList<PollerEvent *, _Build::MT>::Delete_LibList(evList);
    });
    LibList<LibList<PollerEvent *, _Build::MT> *>::Delete_LibList(priorityEvents);
    priorityEvents = NULL;

    g_Log->Debug(LOGFMT_OBJ_TAG("poller worker down poller info:%s"), ToString().c_str());
}

void Poller::QuickEventLoop()
{
    std::vector<LibList<PollerEvent *, _Build::MT> *> priorityEvents;
    const Int64 priorityQueueSize = static_cast<Int64>(_eventsList->GetMaxLevel() + 1);
    for(Int64 idx = 0; idx < priorityQueueSize; ++idx)
        priorityEvents.push_back(LibList<PollerEvent *, _Build::MT>::New_LibList());

    // 部分数据准备
    LibString errLog;

    g_Log->Debug(LOGFMT_OBJ_TAG("poller event worker ready poller info:%s"), ToString().c_str());

    LibCpuCounter deadline;
    LibCpuCounter nowCounter;

    #ifdef _DEBUG
     LibCpuCounter performaceStart;
    #endif

    for(;!_isQuitLoop;)
    {

        _eventsList->MergeTailAllTo(priorityEvents);

        for (auto eventList :  priorityEvents)
        {
            for(auto dataNode = eventList->Begin(); dataNode; )
            {
                auto data = dataNode->_data;
                // 事件处理
                if(LIKELY(_quickEventHandler))
                    (*_quickEventHandler)(data);

                --_eventAmountLeft;
                ++_consumEventCount;
                data->Release();
                dataNode = eventList->Erase(dataNode);
            }
        }
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
    // worker 线程关闭
    if(LIKELY(_onEventWorkerCloseHandler))
        _onEventWorkerCloseHandler->Invoke(this);

    if(LIKELY(_dirtyHelper))
        _dirtyHelper->Destroy();

    if(LIKELY(_timerMgr))
        _timerMgr->Close();

    g_Log->Info(LOGFMT_OBJ_TAG("poller loop end poller info:%s"), ToString().c_str());
}

void Poller::_Clear()
{
    _isEnable = false;
    _isQuitLoop = false;

    // 清理设置的tls资源
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

            // 释放内存
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
