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
 * Author: Eric Yonng
 * Date: 2021-03-17 10:32:03
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TIMER_TIMER_MGR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TIMER_TIMER_MGR_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/Timer/TimeData.h>
#include <kernel/comp/Timer/AsynTimeData.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/Utils/TimeUtil.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/Delegate/Delegate.h>

KERNEL_BEGIN

class LibCpuCounter;
class LibTimer;

// 不支持多线程,请在单线程使用定时器
class KERNEL_EXPORT TimerMgr
{
    POOL_CREATE_OBJ_DEFAULT(TimerMgr);
    
public:
    TimerMgr();
    virtual ~TimerMgr();

public:
    // 在线程中启动
    void Launch(IDelegate<void> *wakeupThreadCb);
    Int64 Drive();
    void SafetyDrive();
    void Close();

    // 注册
    void Register(TimeData *timeData, Int64 newExpireTime, Int64 newPeriod);
    // 反注册
    void UnRegister(TimeData *timeData);
    // 新建定时数据 生成与释放由TimerMgr管理
    TimeData *NewTimeData(LibTimer *timer);
    // 定时器销毁
    void OnTimerDestroy(TimeData *timeData);
    // 获取即将处理的时间间隔时间 单位ms -1 表示没有
    Int64 GetTimeoutIntervalRecently(Int64 nowMs) const;

    // 负载情况
    UInt64 GetTimerLoaded() const;

    // 有没有超时的
    bool HasExpired() const;

    // 是否在处理定时器帧
    bool IsDriving() const;

    // 接管Timer生命周期
    template<typename LambdaType>
    void TakeOverLifeTime(KERNEL_NS::LibTimer *timer, LambdaType &&cb);

private:
    void _BeforeDrive();
    void _AfterDrive();

    void _AsynRegister(TimeData *timeData, Int64 newPeriod, Int64 newExpiredTime);
    void _Register(TimeData *timeData, Int64 newPeriod, Int64 newExpiredTime);
    void _AsynUnRegister(TimeData *timeData);
    void _UnRegister(TimeData *timeData);
    void _Destroy(TimeData *timeData);
    void _AsynDestroy(TimeData *timeData);

    bool _IsInTimerThread();
    void _WakeupMgrThread();

private:
    UInt64 _launchThreadId;         // 启用TimerMgr的线程
    Int64 _driving;                 // 正在驱动
    Int64 _curTime;                 // 当前时间戳 纳秒

    // 过期定时队列
    std::set<TimeData *, TimeDataComp> _expireQueue;
    Int64 _curMaxId;                // 定时id
    bool _hasExpired;

    std::set<TimeData *> _allTimeData;
    std::set<AsynTimeData *> _asynDirty;

    // 唤醒线程回调
    IDelegate<void> *_wakeupCb;

    // timer托管生命周期
    std::map<LibTimer *, IDelegate<void, LibTimer *> *> _timerRefDeleteMethod;
};

ALWAYS_INLINE void TimerMgr::Register(TimeData *timeData, Int64 newExpireTime, Int64 newPeriod)
{
    if (_driving > 0)
    {
        _AsynRegister(timeData, newPeriod, newExpireTime);
        return;
    }

    _Register(timeData, newPeriod, newExpireTime);
}

ALWAYS_INLINE void TimerMgr::UnRegister(TimeData *timeData)
{
    if(_driving > 0)
    {
        _AsynUnRegister(timeData);
        return;
    }

    _UnRegister(timeData);
}

ALWAYS_INLINE TimeData *TimerMgr::NewTimeData(LibTimer *timer)
{
    auto newData = TimeData::NewThreadLocal_TimeData(++_curMaxId, timer);
    _allTimeData.insert(newData);
    return newData;
}


ALWAYS_INLINE Int64 TimerMgr::GetTimeoutIntervalRecently(Int64 nowMs) const
{
    if(UNLIKELY(_allTimeData.empty()))
        return -1;

    TimeData *recently = *_allTimeData.begin();
    Int64 diff = recently->_expiredTime - nowMs;
    return diff > 0 ? diff : 0;    
}

ALWAYS_INLINE UInt64 TimerMgr::GetTimerLoaded() const
{
    return _allTimeData.size();
}

ALWAYS_INLINE bool TimerMgr::HasExpired() const
{
    return _hasExpired;
}

ALWAYS_INLINE bool TimerMgr::IsDriving() const
{
    return _driving > 0;
}

template<typename LambdaType>
ALWAYS_INLINE void TimerMgr::TakeOverLifeTime(LibTimer *timer, LambdaType &&cb)
{
    auto newDelg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void, LibTimer *);
    auto iter = _timerRefDeleteMethod.find(timer);
    if(iter != _timerRefDeleteMethod.end())
    {
        iter->second->Release();
        iter->second = newDelg;
        return;
    }

    _timerRefDeleteMethod.insert(std::make_pair(timer, newDelg));
}

ALWAYS_INLINE void TimerMgr::_BeforeDrive()
{
    ++_driving;
}

ALWAYS_INLINE void TimerMgr::_AsynRegister(TimeData *timeData, Int64 newPeriod, Int64 newExpiredTime)
{
    auto asynData = timeData->_asynData;
    asynData->MaskRegister(newExpiredTime, newPeriod);
    timeData->_isScheduing = true;

    _asynDirty.insert(asynData);
}

ALWAYS_INLINE void TimerMgr::_Register(TimeData *timeData, Int64 newPeriod, Int64 newExpiredTime)
{
    timeData->_period = newPeriod;
    timeData->_expiredTime = newExpiredTime;
    timeData->_isScheduing = true;

    _expireQueue.insert(timeData);
}

ALWAYS_INLINE void TimerMgr::_AsynUnRegister(TimeData *timeData)
{
    auto asynData = timeData->_asynData;
    timeData->_isScheduing = false;
    asynData->MaskUnRegister();
    _asynDirty.insert(asynData);
}

ALWAYS_INLINE void TimerMgr::_UnRegister(TimeData *timeData)
{
    timeData->_isScheduing = false;
    _expireQueue.erase(timeData);
}

ALWAYS_INLINE void TimerMgr::_Destroy(TimeData *timeData)
{
    _expireQueue.erase(timeData);
    timeData->_isScheduing = false;
    timeData->_owner = NULL;
    _allTimeData.erase(timeData);
    timeData->Release();
}

ALWAYS_INLINE void TimerMgr::_AsynDestroy(TimeData *timeData)
{
    timeData->_owner = NULL;
    timeData->_isScheduing = false;
    timeData->_asynData->MaskDestroy();
    _asynDirty.insert(timeData->_asynData);
}

ALWAYS_INLINE bool TimerMgr::_IsInTimerThread()
{
    return _launchThreadId == SystemUtil::GetCurrentThreadId();
}

ALWAYS_INLINE void TimerMgr::_WakeupMgrThread()
{
    if(_wakeupCb)
        _wakeupCb->Invoke();
}

KERNEL_END

#endif

