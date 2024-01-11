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
 * Date: 2023-12-31 18:13:44
 * Author: Eric Yonng
 * Description: 
 * 时间轮：
 * 1. 每个tick的时间间隔
 * 2. slot个数
 * 4. 任务队列
 * 原理:
 *  1. 类似钟表, 任务的执行只在FIRST_LEVEL_SLOTS级执行
 *  2. 时间轮的tick时间间隔n, tick时候计算过期的槽位个数, 判定这些槽上的任务都过期，并执行超时回调并从任务队列上移除, 需要重新调度的重新放入
 *  3. FIRST_LEVEL_SLOTS 级都执行完之后判断有没有下级时间轮, 若有则取得下级实践论的第一个槽的数据初始化 FIRST_LEVEL_SLOTS 工作轮, 下一级时间轮指针偏移到下个槽位, 若有多级则递归的进行相应操作
 *  4. 最大支持6级时间轮
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TIME_WHEEL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TIME_WHEEL_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibList.h>

KERNEL_BEGIN

class TimerWheelTask;
class TimeWheelTimer;

class KERNEL_EXPORT TaskList
{
    POOL_CREATE_OBJ_DEFAULT(TaskList);

public:
    TaskList(){}
    ~TaskList();

    TimerWheelTask *_tasks = NULL;
};

// 时间轮级联 5级级联最大可以支持间隔34年左右的定时任务, 超过的全部算最后一级34年间隔时间来算, 精度1ms
class KERNEL_EXPORT TimeWheelLevel
{
public:
    // 每一级256个槽位
    static constexpr Int32 SLOT_BITS = 8;

    enum ENUMS
    {
        // 一级工作轮 最大2^8 slots
        WORKING_WHELL_LEVEL_SLOTS = 1,

        // 2级工作轮 最大2^8 slots
        SECOND_LEVEL_SLOTS,

        // 3级工作轮 最大2^8 slots
        THIRD_LEVEL_SLOTS,

        // 4级工作轮 最大2^8 slots
        FORTH_LEVEL_SLOTS,

        // 5级工作轮 最大2^8 slots
        FIFTH_LEVEL_SLOTS,

        MAX,
    };

    // 各级时间轮表示的时间范围
    enum MARK_ENUM:UInt64
    {
        // 1级工作轮(tick会扫这里的任务)
        WORKING_WHELL_LEVEL_SLOTS_RANGE = (1LLU << SLOT_BITS) - 1,

        // 2级时间轮能表示的最大时间范围
        SECOND_LEVEL_SLOTS_RANGE = (1LLU << (SECOND_LEVEL_SLOTS * SLOT_BITS)) - 1,

        // 3级时间轮能表示的最大时间范围
        THIRD_LEVEL_SLOTS_RANGE = (1LLU << (THIRD_LEVEL_SLOTS * SLOT_BITS)) - 1,

        // 4级时间轮能表示的最大时间范围
        FORTH_LEVEL_SLOTS_RANGE = (1LLU << (FORTH_LEVEL_SLOTS * SLOT_BITS)) - 1,

        // 5级时间轮能表示的最大时间范围
        FIFTH_LEVEL_SLOTS_RANGE = (1LLU << (FIFTH_LEVEL_SLOTS * SLOT_BITS)) - 1,
    };

    // 各级时间轮表示的时间范围(各级的最大值)
    static UInt64 SLOTS_TIME_RANGE_BASE_WHEEL_START[TimeWheelLevel::MAX];
    static UInt64 SLOTS_TIME_MIN_RANGE_BASE_WHEEL_START[TimeWheelLevel::MAX];
    // 各级时间轮的槽位掩码
    static UInt64 SLOT_MASK[TimeWheelLevel::MAX];
    // 各级时间轮的最低起始位数
    static UInt64 SLOT_MIN_START_BITS[TimeWheelLevel::MAX];

    // tickCount是通过CALC_TASK_TICKS计算的剩余tick数量
    static Int32 GetSlotIndex(Int32 level, UInt64 tickCount);
};

ALWAYS_INLINE Int32 TimeWheelLevel::GetSlotIndex(Int32 level, UInt64 tickCount)
{
    return static_cast<Int32>((tickCount & SLOT_MASK[level]) >> SLOT_MIN_START_BITS[level]);
}

class KERNEL_EXPORT TimeWheelLevelSlots
{
    POOL_CREATE_OBJ_DEFAULT(TimeWheelLevelSlots);

public:
    explicit TimeWheelLevelSlots(Int32 level);
    ~TimeWheelLevelSlots();

    void Reset();
    // TimeWheelLevel
    const Int32 _level;
    // 当前级轮指向的槽位(下一个tick超时时候取的槽位)
    Int32 _pos;
    // 槽数量
    Int32 _capacity;
    // 槽位任务列表
    std::vector<TaskList *> _slots;
};

ALWAYS_INLINE void TimeWheelLevelSlots::Reset()
{
    _pos = 0;
}

class KERNEL_EXPORT TimerWheel
{
    POOL_CREATE_OBJ_DEFAULT(TimerWheel);

public:
    TimerWheel();
    ~TimerWheel();

    Int32 Init(Int32 maxLevel, Int64 tickIntervalMs);
    void Destroy();

    Int64 Tick();

    bool IsTicking() const;

    void ScheduleTimer(TimeWheelTimer *timer, Int64 newExpireTime, Int64 newPeriod);
    void CancelTimer(TimeWheelTimer *timer);
    void OnTimerDestroy(TimerWheelTask *timeData);

    // 接管Timer生命周期
    template<typename LambdaType>
    void TakeOverLifeTime(TimeWheelTimer *timer, LambdaType &&cb);

    // 获取当前时间轮已tick过的时间戳
    Int64 GetWheelLastTickTime() const;

    UInt64 GetTimerLoaded() const;

    Int64 GetTickIntervalMs() const;

private:
    // 当执行完最后一个slot的超时任务后从高级时间轮更更新当前工作轮
    bool _UpdateSlotsFromHighLevel(TimeWheelLevelSlots *currentSlot, std::vector<TimeWheelLevelSlots *> &highLevelSlots);
    // 当所有轮转到最后一格时需要重置轮
    void _ResetWheel();

    // 添加定时
    void _DoReAddPendingTimerTaskList(TimerWheelTask *&timerTask);
    void _TryReAddTooLongTaskList(TimerWheelTask *&timerTask);
    void _DoAddOneTimerTask(TimerWheelTask *timerTask);

    // 获取当前时间轮已tick过的时间戳
    Int64 _GetWheelLastTickTime() const;
    Int64 _GetWheelEscapeTimeFromStart() const;

private:
    // tick时间间隔
    Int64 _tickIntervalMs;
    // 时间轮起始时间, 当时间轮最大级轮转完之后更新
    Int64 _startTickMs;
    // 最后一次tick的时间
    Int64 _lastTickMs;
    // 时间轮的最大级数
    Int32 _maxLevel;
    // 最大定时时长
    Int64 _maxTickTime;
    // 是否在tick
    Int32 _ticking;

    // 工作轮
    TimeWheelLevelSlots *_workingLevelSlots;

    // 高级别的时间轮
    std::vector<TimeWheelLevelSlots *> _levelSlots;

    // 超长定时队列
    TimerWheelTask *_tooLongTimeQueue;

    // 任务数量
    Int64 _taskCount;

    // timer的生命接管
    std::map<TimeWheelTimer *, IDelegate<void, TimeWheelTimer *> *> _timerRefDeleteMethod;

    // 临时存放的待注册到队列的定时器任务 在ticking中不宜添加到定时队列中,应该放到pending, 因为可能队列的pos会变
    TimerWheelTask *_pendingAddTimer;
};

ALWAYS_INLINE bool TimerWheel::IsTicking() const
{
    return _ticking != 0;
}

template<typename LambdaType>
ALWAYS_INLINE void TimerWheel::TakeOverLifeTime(TimeWheelTimer *timer, LambdaType &&cb)
{
    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void, TimeWheelTimer *);
    auto iter = _timerRefDeleteMethod.find(timer);
    if(iter != _timerRefDeleteMethod.end())
    {
        iter->second->Release();
        iter->second = delg;
        return;
    }

    _timerRefDeleteMethod.insert(std::make_pair(timer, delg));
}

ALWAYS_INLINE UInt64 TimerWheel::GetTimerLoaded() const
{
    return _taskCount;
}

ALWAYS_INLINE Int64 TimerWheel::GetTickIntervalMs() const
{
    return _tickIntervalMs;
}

KERNEL_END

#endif
