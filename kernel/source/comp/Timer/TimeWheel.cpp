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
 * Date: 2023-12-31 18:34:04
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Timer/TimeWheel.h>
#include <kernel/comp/Timer/TimerWheelTask.h>
#include <kernel/comp/Timer/TimeWheelTimer.h>
#include <kernel/comp/LibTime.h>
#include <kernel/comp/Log/log.h>
#include <kernel/common/statics.h>
#include <kernel/comp/Utils/ContainerUtil.h>

#define CALC_TASK_TICKS(task)         \
((task->_expiredTime - _startTickMs) / _tickIntervalMs)

KERNEL_BEGIN

static ALWAYS_INLINE void RemoveFromTaskList(TimerWheelTask *task)
{
    auto pre = task->_pre;
    auto next = task->_next;
    if(pre)
        pre->_next = next;

    if(next)
        next->_pre = pre;

    if(task->_head && task == *task->_head)
        *task->_head = next;

    task->_pre = NULL;
    task->_next = NULL;
    task->_head = NULL;
}

static ALWAYS_INLINE void AddToTaskList(TimerWheelTask *&head, TimerWheelTask *task)
{
    task->_head = &head;
    task->_next = head;
    
    if (LIKELY(head))
        head->_pre = task;

    head = task;
}

static ALWAYS_INLINE void InitSlots(TimeWheelLevelSlots *slots, Int32 initPos = 0)
{
    slots->_pos = initPos;
    slots->_capacity = 1 << TimeWheelLevel::SLOT_BITS;
    slots->_slots.resize(slots->_capacity);
    for(Int32 idx = 0; idx < slots->_capacity; ++idx)
        slots->_slots[idx] = TaskList::NewThreadLocal_TaskList();
}


UInt64 TimeWheelLevel::SLOTS_TIME_RANGE_BASE_WHEEL_START[TimeWheelLevel::MAX] = {
    0,
    TimeWheelLevel::WORKING_WHELL_LEVEL_SLOTS_RANGE,
    TimeWheelLevel::SECOND_LEVEL_SLOTS_RANGE,
    TimeWheelLevel::THIRD_LEVEL_SLOTS_RANGE,
    TimeWheelLevel::FORTH_LEVEL_SLOTS_RANGE,
    TimeWheelLevel::FIFTH_LEVEL_SLOTS_RANGE,
};

UInt64 TimeWheelLevel::SLOTS_TIME_MIN_RANGE_BASE_WHEEL_START[TimeWheelLevel::MAX] = 
{
    0,
    0,
    TimeWheelLevel::WORKING_WHELL_LEVEL_SLOTS_RANGE + 1,
    TimeWheelLevel::SECOND_LEVEL_SLOTS_RANGE + 1,
    TimeWheelLevel::THIRD_LEVEL_SLOTS_RANGE + 1,
    TimeWheelLevel::FORTH_LEVEL_SLOTS_RANGE + 1,
};


UInt64 TimeWheelLevel::SLOT_MASK[TimeWheelLevel::MAX] = 
{
    0,
    TimeWheelLevel::WORKING_WHELL_LEVEL_SLOTS_RANGE,
    TimeWheelLevel::SECOND_LEVEL_SLOTS_RANGE & (~TimeWheelLevel::WORKING_WHELL_LEVEL_SLOTS_RANGE),
    TimeWheelLevel::THIRD_LEVEL_SLOTS_RANGE & (~TimeWheelLevel::SECOND_LEVEL_SLOTS_RANGE),
    TimeWheelLevel::FORTH_LEVEL_SLOTS_RANGE & (~TimeWheelLevel::THIRD_LEVEL_SLOTS_RANGE),
    TimeWheelLevel::FIFTH_LEVEL_SLOTS_RANGE & (~TimeWheelLevel::FORTH_LEVEL_SLOTS_RANGE),
};

UInt64 TimeWheelLevel::SLOT_MIN_START_BITS[TimeWheelLevel::MAX] = {
    0,
    0,
    (TimeWheelLevel::SECOND_LEVEL_SLOTS - TimeWheelLevel::WORKING_WHELL_LEVEL_SLOTS) * TimeWheelLevel::SLOT_BITS,
    (TimeWheelLevel::THIRD_LEVEL_SLOTS - TimeWheelLevel::WORKING_WHELL_LEVEL_SLOTS) * TimeWheelLevel::SLOT_BITS,
    (TimeWheelLevel::FORTH_LEVEL_SLOTS - TimeWheelLevel::WORKING_WHELL_LEVEL_SLOTS) * TimeWheelLevel::SLOT_BITS,
    (TimeWheelLevel::FIFTH_LEVEL_SLOTS - TimeWheelLevel::WORKING_WHELL_LEVEL_SLOTS) * TimeWheelLevel::SLOT_BITS,
};

TaskList::~TaskList()
{
    if(_tasks)
    {
        for(auto head = _tasks; head; head = _tasks)
        {
            RemoveFromTaskList(head);

            // 被tiemr绑定由timer来释放
            if(head->_attachTimer)
                continue;

            TimerWheelTask::DeleteThreadLocal_TimerWheelTask(head);
        }

        _tasks = NULL;
    }
}

TimeWheelLevelSlots::TimeWheelLevelSlots(Int32 level)
:_level(level)
,_pos(0)
,_capacity(0)
{

}

TimeWheelLevelSlots::~TimeWheelLevelSlots()
{
    ContainerUtil::DelContainer(_slots, [](TaskList *taskList){
        TaskList::DeleteThreadLocal_TaskList(taskList);
    });
}

ALWAYS_INLINE Int64 TimerWheel::_GetWheelEscapeTimeFromStart() const
{
    Int64 escapeMs = 0;
    for(Int32 idx = TimeWheelLevel::WORKING_WHELL_LEVEL_SLOTS; idx <= _maxLevel; ++idx)
    {
        auto levelSlots = _levelSlots[idx];
        // 当前级时间轮偏移时间 = 偏移指针pos * (一格代表的时间基数)
        escapeMs += (levelSlots->_pos * (_tickIntervalMs << TimeWheelLevel::SLOT_MIN_START_BITS[idx]));
    }

    return escapeMs;
}

ALWAYS_INLINE Int64 TimerWheel::_GetWheelLastTickTime() const
{    
    return  _startTickMs + _GetWheelEscapeTimeFromStart();
}

ALWAYS_INLINE void TimerWheel::_DoAddOneTimerTask(TimerWheelTask *timerTask)
{
    // 超长定时放在超长队列
    if(timerTask->_expiredTime > (_startTickMs + _maxTickTime))
    {
        AddToTaskList(_tooLongTimeQueue, timerTask);
        return;
    }

    // 至少一个tick（保证在下一个tick之后）
    auto delayTicks = (timerTask->_expiredTime - _startTickMs) / _tickIntervalMs;
    delayTicks = delayTicks > 0 ? delayTicks : 1;
    UInt64 ticks = static_cast<UInt64>(delayTicks);

    Int32 level = 0;
    if(ticks <= TimeWheelLevel::WORKING_WHELL_LEVEL_SLOTS_RANGE)
    {
        level = TimeWheelLevel::WORKING_WHELL_LEVEL_SLOTS;
    }
    else if(ticks <= TimeWheelLevel::SECOND_LEVEL_SLOTS_RANGE)
    {
        level = TimeWheelLevel::SECOND_LEVEL_SLOTS;
    }
    else if(ticks <= TimeWheelLevel::THIRD_LEVEL_SLOTS_RANGE)
    {
        level = TimeWheelLevel::THIRD_LEVEL_SLOTS;
    }
    else if(ticks <= TimeWheelLevel::FORTH_LEVEL_SLOTS_RANGE)
    {
        level = TimeWheelLevel::FORTH_LEVEL_SLOTS;
    }
    else
    {
        AddToTaskList(_tooLongTimeQueue, timerTask);
        return;
    }

    if(level > _maxLevel)
    {
        AddToTaskList(_tooLongTimeQueue, timerTask);
        return;
    }

    auto levelSlots = _levelSlots[level];
    auto slotIndex = TimeWheelLevel::GetSlotIndex(level, ticks);
    slotIndex = slotIndex < levelSlots->_pos ? levelSlots->_pos : slotIndex;

    // 放到指定的任务队列中
    AddToTaskList(levelSlots->_slots[slotIndex]->_tasks, timerTask);
}


TimerWheel::TimerWheel()
:_tickIntervalMs(1)
,_startTickMs(0)
,_lastTickMs(0)
,_maxLevel(TimeWheelLevel::WORKING_WHELL_LEVEL_SLOTS)
,_maxTickTime(0)
,_ticking(0)
,_workingLevelSlots(NULL)
,_tooLongTimeQueue(NULL)
,_taskCount(0)
,_pendingAddTimer(NULL)
{

}

TimerWheel::~TimerWheel()
{
    Destroy();
}

Int32 TimerWheel::Init(Int32 maxLevel, Int64 tickIntervalMs)
{
    if(maxLevel >= TimeWheelLevel::MAX || maxLevel < TimeWheelLevel::WORKING_WHELL_LEVEL_SLOTS)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("param error maxlevel:%d"), maxLevel);
        return Status::ParamError;
    }

    if(tickIntervalMs <= 0)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("param error tickIntervalMs:%lld"), tickIntervalMs);
        return Status::ParamError;
    }

    _maxLevel = maxLevel;
    _tickIntervalMs = tickIntervalMs;
    _pendingAddTimer = NULL;

    _ticking = 0;
    _startTickMs = LibTime::NowMilliTimestamp();
    _lastTickMs = _startTickMs;
    _taskCount = 0;

    _maxTickTime = TimeWheelLevel::SLOTS_TIME_RANGE_BASE_WHEEL_START[_maxLevel] * _tickIntervalMs;

    _levelSlots.resize(_maxLevel + 1);
    for(Int32 level = TimeWheelLevel::WORKING_WHELL_LEVEL_SLOTS; level <= _maxLevel; ++level)
    {
        auto newSlots = TimeWheelLevelSlots::NewThreadLocal_TimeWheelLevelSlots(level);
        _levelSlots[level] = newSlots;
        InitSlots(newSlots);
    }

    _workingLevelSlots = _levelSlots[TimeWheelLevel::WORKING_WHELL_LEVEL_SLOTS];
    return Status::Success;
}

void TimerWheel::Destroy()
{
    if(_pendingAddTimer)
    {
        for(auto head = _pendingAddTimer; head; head = _pendingAddTimer)
        {
            RemoveFromTaskList(head);

            // 被tiemr绑定由timer来释放
            if(head->_attachTimer)
                continue;

            TimerWheelTask::DeleteThreadLocal_TimerWheelTask(head);
        }

        _pendingAddTimer = NULL;
    }

    if(_tooLongTimeQueue)
    {
        for(auto head = _tooLongTimeQueue; head; head = _tooLongTimeQueue)
        {
            RemoveFromTaskList(head);

            // 被tiemr绑定由timer来释放
            if(head->_attachTimer)
                continue;

            TimerWheelTask::DeleteThreadLocal_TimerWheelTask(head);
        }

        _pendingAddTimer = NULL;
    }

    ContainerUtil::DelContainer(_levelSlots, [](TimeWheelLevelSlots *slots){
        if(!slots)
            return;

        TimeWheelLevelSlots::DeleteThreadLocal_TimeWheelLevelSlots(slots);
    });

    _workingLevelSlots = NULL;
    _taskCount = 0;

    for(auto iter = _timerRefDeleteMethod.begin(); iter != _timerRefDeleteMethod.end();)
    {
        auto timer = iter->first;
        auto delg = iter->second;
        iter->second = NULL;
        iter = _timerRefDeleteMethod.erase(iter);

        if(LIKELY(delg))
        {
            delg->Invoke(timer);
            delg->Release();
        }
    }
}

Int64 TimerWheel::Tick()
{
    ++_ticking;

    auto nowMs = LibTime::NowMilliTimestamp();
    // 判断是否有定时任务
    if(_taskCount <= 0)
    {
        --_ticking;
        return 0;
    }

    // 计算出过期了多少给slots
    auto expireSlotsCount = (nowMs - _lastTickMs) / _tickIntervalMs;
    if(expireSlotsCount <= 0)
    {
        --_ticking;
        return 0;
    }
    _lastTickMs = nowMs;

    #ifdef ENABLE_PERFORMANCE_RECORD
        TimerWheelTask *timerData = NULL;
        auto &&outputLogFunc = [&timerData](UInt64 ms){
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(TimerWheel, "time wheel time out cost:%llu, timer task data:%s"), ms, timerData ? timerData->_timerInfo.c_str() : "");
        };
    #endif

    // 取出过期的任务并执行过期回调
    Int64 handled = 0;
    auto &pendingAddTimer = _pendingAddTimer;
    auto workingLevlSlots = _workingLevelSlots;
    for(Int32 idx = 0; idx < expireSlotsCount; ++idx)
    {
        auto taskList = workingLevlSlots->_slots[workingLevlSlots->_pos++];
        for(auto head = taskList->_tasks; head; head = taskList->_tasks)
        {
            auto curTask = head;
            RemoveFromTaskList(curTask);
            --_taskCount;

            if(curTask->_isScheduling)
            {
                #ifdef ENABLE_PERFORMANCE_RECORD
                    timerData = curTask;
                    PERFORMANCE_RECORD_DEF(pr, outputLogFunc, 10);
                #endif

                curTask->_isInTicking = true;
                curTask->_attachTimer->OnTimeOut();
                curTask->_isInTicking = false;
                ++handled;

                // 重新添加到槽中
                if(curTask->_attachTimer && curTask->_isScheduling)
                {
                    // 不在队列中才需要添加到pending队列
                    if(!curTask->_head)
                    {
                        curTask->_expiredTime = nowMs + curTask->_period;
                        ++_taskCount;
                        AddToTaskList(pendingAddTimer, curTask);
                    }
                }
                else if(!curTask->_attachTimer)
                {// 销毁
                    TimerWheelTask::DeleteThreadLocal_TimerWheelTask(curTask);
                }
            }
        }

        // 队列结束, 从高级别的slots初始化工作轮
        if(UNLIKELY(workingLevlSlots->_pos >= workingLevlSlots->_capacity))
        {
            // 时间轮转到头了
            if(UNLIKELY(!_UpdateSlotsFromHighLevel(workingLevlSlots, _levelSlots)))
            {
                _ResetWheel();

                // 将pending队列的定时器重新加入队列
                if(pendingAddTimer)
                   _DoReAddPendingTimerTaskList(pendingAddTimer);

                // 尝试添加超长定时
                if(_tooLongTimeQueue)
                   _TryReAddTooLongTaskList(_tooLongTimeQueue);
            }
        }
    }

    if((--_ticking <= 0) && pendingAddTimer)
    {
        _DoReAddPendingTimerTaskList(pendingAddTimer);
    }

    return handled;
}

void TimerWheel::ScheduleTimer(TimeWheelTimer *timer, Int64 newExpireTime, Int64 newPeriod)
{
    if(UNLIKELY(!timer->_attachTimerTask))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("have no attach timer task please check timer:%s"), timer->ToString().c_str());
        return;
    }

    // 如果在其他队列中从队列中移除
    if(timer->_attachTimerTask->_head)
    {
        --_taskCount;
        RemoveFromTaskList(timer->_attachTimerTask);
    }

    // 更新新的过期时间和周期
    timer->_attachTimerTask->_expiredTime = newExpireTime;
    timer->_attachTimerTask->_period = newPeriod;
    timer->_attachTimerTask->_isScheduling = true;

    if(_ticking > 0)
    {
        ++_taskCount;
        AddToTaskList(_pendingAddTimer, timer->_attachTimerTask);
        return;
    }

     ++_taskCount;
    _DoAddOneTimerTask(timer->_attachTimerTask);
}

void TimerWheel::CancelTimer(TimeWheelTimer *timer)
{
    if(UNLIKELY(!timer->_attachTimerTask))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("have no attach timer task please check timer:%s"), timer->ToString().c_str());
        return;
    }

    timer->_attachTimerTask->_isScheduling = false;
    if(timer->_attachTimerTask->_head)
    {
        --_taskCount;
        RemoveFromTaskList(timer->_attachTimerTask);
    }
}

void TimerWheel::OnTimerDestroy(TimerWheelTask *timeData)
{
    if(timeData->_head)
        --_taskCount;

    // 先移除
    if(timeData->_attachTimer)
    {
        auto iter = _timerRefDeleteMethod.find(timeData->_attachTimer);
        if(iter != _timerRefDeleteMethod.end())
        {
            iter->second->Release();
            _timerRefDeleteMethod.erase(iter);
        }
    }

    timeData->_isScheduling = false;
    timeData->_attachTimer = NULL;

    // 在tick中由tick释放
    if(_ticking <= 0)
    {
        RemoveFromTaskList(timeData);
        TimerWheelTask::DeleteThreadLocal_TimerWheelTask(timeData);
    }
}

Int64 TimerWheel::GetWheelLastTickTime() const
{
    return _GetWheelLastTickTime();
}

bool TimerWheel::_UpdateSlotsFromHighLevel(TimeWheelLevelSlots *currentSlot, std::vector<TimeWheelLevelSlots *> &highLevelSlots)
{
    auto fromLevel = currentSlot->_level + 1;
    if(fromLevel > _maxLevel)
    {// 此时所有级别轮pos都指向最高格子
        return false;
    }

    auto highLevelSlot = highLevelSlots[fromLevel];
    if(highLevelSlot->_pos >= highLevelSlot->_capacity)
    {
        if(!_UpdateSlotsFromHighLevel(highLevelSlot, highLevelSlots))
            return false;
    }
    
    auto slots = highLevelSlot->_slots[highLevelSlot->_pos++];
    currentSlot->Reset();

    const auto currentLevelMask = TimeWheelLevel::SLOT_MASK[currentSlot->_level];
    const auto rightShiftBits = TimeWheelLevel::SLOT_MIN_START_BITS[currentSlot->_level];
    for(auto head = slots->_tasks; head; head = slots->_tasks)
    {
        RemoveFromTaskList(head);

        // 计算剩余过期时间的slot位置
        auto currentSlotIndex = CALC_TASK_TICKS(head) & currentLevelMask >> rightShiftBits;
        auto currentTaskList = currentSlot->_slots[currentSlotIndex];

        AddToTaskList(currentTaskList->_tasks, head);
    }
    slots->_tasks = NULL;

    return true;
}

void TimerWheel::_ResetWheel()
{
    for(auto slots : _levelSlots)
    {
        if(!slots)
            continue;

        slots->Reset();
    }

    _startTickMs = KERNEL_NS::LibTime::NowMilliTimestamp();
}

void TimerWheel::_DoReAddPendingTimerTaskList(TimerWheelTask *&timerTask)
{
    for(auto head = timerTask; head; head = timerTask)
    {
        RemoveFromTaskList(head);

        // 超长定时放在超长队列
        if(head->_expiredTime > (_startTickMs + _maxTickTime))
        {
            AddToTaskList(_tooLongTimeQueue, head);
            continue;
        }

        // 至少一个tick（保证在下一个tick之后） 变负数 TODO:
        auto delayTicks = (head->_expiredTime - _startTickMs) / _tickIntervalMs;
        delayTicks = delayTicks > 0 ? delayTicks : 1;
        auto ticks = static_cast<UInt64>(delayTicks);

        Int32 level = 0;
        if(ticks <= TimeWheelLevel::WORKING_WHELL_LEVEL_SLOTS_RANGE)
        {
            level = TimeWheelLevel::WORKING_WHELL_LEVEL_SLOTS;
        }
        else if(ticks <= TimeWheelLevel::SECOND_LEVEL_SLOTS_RANGE)
        {
            level = TimeWheelLevel::SECOND_LEVEL_SLOTS;
        }
        else if(ticks <= TimeWheelLevel::THIRD_LEVEL_SLOTS_RANGE)
        {
            level = TimeWheelLevel::THIRD_LEVEL_SLOTS;
        }
        else if(ticks <= TimeWheelLevel::FORTH_LEVEL_SLOTS_RANGE)
        {
            level = TimeWheelLevel::FORTH_LEVEL_SLOTS;
        }
        else
        {
            AddToTaskList(_tooLongTimeQueue, head);
            continue;
        }

        if(level > _maxLevel)
        {
            AddToTaskList(_tooLongTimeQueue, head);
            continue;
        }

        auto levelSlots = _levelSlots[level];
        auto slotIndex = TimeWheelLevel::GetSlotIndex(level, ticks);
        slotIndex = slotIndex < levelSlots->_pos ? levelSlots->_pos : slotIndex;

        // 放到指定的任务队列中
        AddToTaskList(levelSlots->_slots[slotIndex]->_tasks, head);
    }
}

void TimerWheel::_TryReAddTooLongTaskList(TimerWheelTask *&timerTask)
{
    TimerWheelTask *pendingTail = NULL;
    TimerWheelTask *pendingHead = NULL;
    for(auto head = timerTask; head; head = timerTask)
    {
        RemoveFromTaskList(head);

        // 超长定时放在超长队列
        if(head->_expiredTime > (_startTickMs + _maxTickTime))
        {            
            if(!pendingTail)
                pendingTail= head;

            AddToTaskList(pendingHead, head);

            head->_head = &timerTask;
            continue;
        }

        // 至少一个tick（保证在下一个tick之后）
        auto delayTicks = (head->_expiredTime - _startTickMs) / _tickIntervalMs;
        delayTicks = delayTicks > 0 ? delayTicks : 1;
        auto ticks = static_cast<UInt64>(delayTicks);

        Int32 level = 0;
        if(ticks <= TimeWheelLevel::WORKING_WHELL_LEVEL_SLOTS_RANGE)
        {
            level = TimeWheelLevel::WORKING_WHELL_LEVEL_SLOTS;
        }
        else if(ticks <= TimeWheelLevel::SECOND_LEVEL_SLOTS_RANGE)
        {
            level = TimeWheelLevel::SECOND_LEVEL_SLOTS;
        }
        else if(ticks <= TimeWheelLevel::THIRD_LEVEL_SLOTS_RANGE)
        {
            level = TimeWheelLevel::THIRD_LEVEL_SLOTS;
        }
        else if(ticks <= TimeWheelLevel::FORTH_LEVEL_SLOTS_RANGE)
        {
            level = TimeWheelLevel::FORTH_LEVEL_SLOTS;
        }
        else
        {
            if(!pendingTail)
                pendingTail= head;

            AddToTaskList(pendingHead, head);

            head->_head = &timerTask;
            continue;
        }

        if(level > _maxLevel)
        {
            if(!pendingTail)
                pendingTail= head;

            AddToTaskList(pendingHead, head);

            head->_head = &timerTask;
            continue;
        }

        auto levelSlots = _levelSlots[level];
        auto slotIndex = TimeWheelLevel::GetSlotIndex(level, ticks);
        slotIndex = slotIndex < levelSlots->_pos ? levelSlots->_pos : slotIndex;

        // 放到指定的任务队列中
        AddToTaskList(levelSlots->_slots[slotIndex]->_tasks, head);
    }

    // 队列合并
    if(pendingHead)
    {
        pendingTail->_next = timerTask;
        if(timerTask)
            timerTask->_pre = pendingTail;
        timerTask = pendingHead;
    }
}

KERNEL_END
