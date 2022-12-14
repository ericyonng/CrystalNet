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

KERNEL_BEGIN

class LibCpuCounter;

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
    void Drive(Int64 cuTimeMicroseconds);
    void Close();

    // 注册
    void Register(TimeData *timeData, Int64 newExpireTime, Int64 newPeriod);
    // 反注册
    void UnRegister(TimeData *timeData);
    // 微妙
    Int64 GetCurFrameTime() const;
    // 新建定时数据 生成与释放由TimerMgr管理
    TimeData *NewTimeData(LibTimer *timer);
    // 定时器销毁
    void OnTimerDestroy(TimeData *timeData);
    // 微妙
    const Int64 GetNowMicroSecond() const;
    // 获取即将处理的时间间隔时间 单位ms -1 表示没有
    Int64 GetTimeoutIntervalRecently(Int64 nowMs) const;

    // 负载情况
    UInt64 GetTimerLoaded() const;

    // 有没有超时的
    bool HasExpired() const;

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
    Int64 _curTime;                 // 当前时间戳 微妙

    // 过期定时队列
    std::set<TimeData *, TimeDataComp> _expireQueue;
    Int64 _curMaxId;                // 定时id
    bool _hasExpired;

    std::set<TimeData *> _allTimeData;
    std::set<AsynTimeData *> _asynDirty;

    // 唤醒线程回调
    IDelegate<void> *_wakeupCb;
};

inline void TimerMgr::Register(TimeData *timeData, Int64 newExpireTime, Int64 newPeriod)
{
    if (_driving > 0)
    {
        _AsynRegister(timeData, newPeriod, newExpireTime);
        return;
    }

    _Register(timeData, newPeriod, newExpireTime);
}

inline void TimerMgr::UnRegister(TimeData *timeData)
{
    if(_driving > 0)
    {
        _AsynUnRegister(timeData);
        return;
    }

    _UnRegister(timeData);
}

inline Int64 TimerMgr::GetCurFrameTime() const
{
    return _curTime;
}

inline TimeData *TimerMgr::NewTimeData(LibTimer *timer)
{
    auto newData = TimeData::NewThreadLocal_TimeData(++_curMaxId, timer);
    _allTimeData.insert(newData);
    return newData;
}

inline void TimerMgr::OnTimerDestroy(TimeData *timeData)
{
    if(_driving > 0)
    {
        _AsynDestroy(timeData);
        return;
    }

    _Destroy(timeData);
}

inline Int64 TimerMgr::GetTimeoutIntervalRecently(Int64 nowMs) const
{
    if(UNLIKELY(_allTimeData.empty()))
        return -1;

    TimeData *recently = *_allTimeData.begin();
    Int64 diff = recently->_expiredTime - nowMs;
    return diff > 0 ? diff : 0;    
}

inline UInt64 TimerMgr::GetTimerLoaded() const
{
    return _allTimeData.size();
}

ALWAYS_INLINE bool TimerMgr::HasExpired() const
{
    return _hasExpired;
}

inline void TimerMgr::_BeforeDrive()
{
    ++_driving;
}

inline void TimerMgr::_AfterDrive()
{
    if(--_driving > 0)
        return;

    _driving = 0;

    for(auto iter = _asynDirty.begin(); iter != _asynDirty.end(); )
    {
        auto asynData = *iter;
        auto flag = asynData->_flag;

        if(UNLIKELY(BitUtil::IsSet(flag, AsynOpType::OP_DESTROY)))
        {// 数据销毁
            _Destroy(asynData->_data);
            asynData = NULL;
        }
        else
        {
            // 先执行移除
            if(BitUtil::IsSet(flag, AsynOpType::OP_UNREGISTER))
                _UnRegister(asynData->_data);

            // 最后添加
            if(BitUtil::IsSet(flag, AsynOpType::OP_REGISTER))
                _Register(asynData->_data, asynData->_newPeriod, asynData->_newExpiredTime);

            // 重置数据
            asynData->Reset();
        }
        
        iter = _asynDirty.erase(iter);
    }
}

inline void TimerMgr::_AsynRegister(TimeData *timeData, Int64 newPeriod, Int64 newExpiredTime)
{
    auto asynData = timeData->_asynData;
    asynData->MaskRegister(newExpiredTime, newPeriod);
    timeData->_isScheduing = true;

    _asynDirty.insert(asynData);
}

inline void TimerMgr::_Register(TimeData *timeData, Int64 newPeriod, Int64 newExpiredTime)
{
    timeData->_period = newPeriod;
    timeData->_expiredTime = newExpiredTime;
    timeData->_isScheduing = true;

    _expireQueue.insert(timeData);
}

inline void TimerMgr::_AsynUnRegister(TimeData *timeData)
{
    auto asynData = timeData->_asynData;
    timeData->_isScheduing = false;
    asynData->MaskUnRegister();
    _asynDirty.insert(asynData);
}

inline void TimerMgr::_UnRegister(TimeData *timeData)
{
    timeData->_isScheduing = false;
    _expireQueue.erase(timeData);
}

inline void TimerMgr::_Destroy(TimeData *timeData)
{
    timeData->_isScheduing = false;
    timeData->_owner = NULL;
    _expireQueue.erase(timeData);
    _allTimeData.erase(timeData);
    timeData->Release();
}

inline void TimerMgr::_AsynDestroy(TimeData *timeData)
{
    timeData->_isScheduing = false;
    timeData->_asynData->MaskDestroy();
    _asynDirty.insert(timeData->_asynData);
}

inline bool TimerMgr::_IsInTimerThread()
{
    return _launchThreadId == SystemUtil::GetCurrentThreadId();
}

inline void TimerMgr::_WakeupMgrThread()
{
    if(_wakeupCb)
        _wakeupCb->Invoke();
}

KERNEL_END

#endif

