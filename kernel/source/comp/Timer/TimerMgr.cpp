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
 * Date: 2021-03-17 17:48:56
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Timer/TimeData.h>
#include <kernel/comp/Timer/TimerMgr.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/Timer/LibTimer.h>
#include <kernel/comp/Cpu/LibCpuCounter.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/PerformanceRecord.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/Utils/TimeUtil.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(TimerMgr);

TimerMgr::TimerMgr() 
:_launchThreadId(0)
,_driving(0)
,_curTime(0)
,_curMaxId(0)
,_hasExpired(false)
,_wakeupCb(NULL)
{
    
}

TimerMgr::~TimerMgr()
{
    Close();

    ContainerUtil::DelContainer<TimeData *, AutoDelMethods::Release>(_allTimeData);

    if(LIKELY(_wakeupCb))
    {
        _wakeupCb->Release();
        _wakeupCb = NULL;
    }
}

void TimerMgr::Launch(IDelegate<void> *wakeupThreadCb)
{
    if(UNLIKELY(_launchThreadId))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("already Launch before _launchThreadId = [%llu], current thread id = [%llu]"), _launchThreadId, SystemUtil::GetCurrentThreadId());
        CRYSTAL_DELETE_SAFE(wakeupThreadCb);
        return;
    }

    _launchThreadId = SystemUtil::GetCurrentThreadId();
    if(_wakeupCb)
        CRYSTAL_DELETE_SAFE(_wakeupCb);
    _wakeupCb = wakeupThreadCb;
}

Int64 TimerMgr::Drive()
{
    Int64 handled = 0;
    _BeforeDrive();

    if(!_expireQueue.empty())
    {
        #ifdef ENABLE_PERFORMANCE_RECORD
            KERNEL_NS::TimeData *timerData = NULL;
            auto &&outputLogFunc = [&timerData](UInt64 ms){
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(TimerMgr, "timer time out cost:%llu, timer data:%s"), ms, timerData ? timerData->ToString().c_str() : "");
            };
        #endif

        _curTime = TimeUtil::GetFastNanoTimestamp();

        // 为了避免在TimeOut执行过程中定时器重新注册进去导致队列顺序失效, 以及可能注册进去的定时器每次都超时导致死循环, 所以一定是需要异步处理注册定时, 反注册, 以及销毁定时器等操作
        // TimeOut中有可能继续加入定时器，这个定时器可能在一个循环中刚好超时, 所以添加移除定时器操作一定得是异步在_AfterDrive做
        for(auto iter = _expireQueue.begin(); iter != _expireQueue.end();)
        {
            // 未过期的为止
            auto timeData = *iter;
            if(timeData->_expiredTime > _curTime)
                break;

            // 必须先移除
            iter = _expireQueue.erase(iter);
            if(timeData->_isScheduing)
            {
                #ifdef ENABLE_PERFORMANCE_RECORD
                    timerData = timeData;
                    PERFORMANCE_RECORD_DEF(pr, outputLogFunc, 10);
                #endif

                timeData->_owner->OnTimeOut();
                ++handled;

                if(LIKELY(timeData->_owner && timeData->_isScheduing))
                {
                    // 重新被调度
                    if(timeData->_asynData && (!BitUtil::IsSet(timeData->_asynData->_flag, AsynOpType::OP_REGISTER)))
                        _AsynRegister(timeData, timeData->_period, timeData->_expiredTime + timeData->_period);
                }
            }
        }
    }

    _AfterDrive();

    // 重算过期标记
    if(!_expireQueue.empty())
    {
        auto timeData =  *_expireQueue.begin();
        _hasExpired = (_curTime = TimeUtil::GetFastNanoTimestamp()) >= timeData->_expiredTime;
    }
    else
    {
        _hasExpired = false;
    }

    return handled;
}

void TimerMgr::SafetyDrive()
{
   _BeforeDrive();

    if(LIKELY(!_expireQueue.empty()))
    {
        _curTime = TimeUtil::GetFastNanoTimestamp();
        // #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        // SignalHandleUtil::PopRecoverPoint();
        // auto err = SignalHandleUtil::PushRecoverPoint(&stackFrame);
        // if(LIKELY(err == 0))
        // {
        // #endif
            // 为了避免在TimeOut执行过程中定时器重新注册进去导致队列顺序失效, 以及可能注册进去的定时器每次都超时导致死循环, 所以一定是需要异步处理注册定时, 反注册, 以及销毁定时器等操作
            // TimeOut中有可能继续加入定时器，这个定时器可能在一个循环中刚好超时, 所以添加移除定时器操作一定得是异步在_AfterDrive做
            for(auto iter = _expireQueue.begin(); iter != _expireQueue.end();)
            {
                // 未过期的为止
                auto timeData = *iter;
                if(timeData->_expiredTime > _curTime)
                    break;

                // 必须先移除
                iter = _expireQueue.erase(iter);

                if(timeData->_isScheduing)
                {
                    timeData->_owner->OnTimeOut();

                    if(LIKELY(timeData->_owner))
                    {
                        // 重新被调度
                        if(timeData->_isScheduing)
                        {
                            if(timeData->_asynData && (!BitUtil::IsSet(timeData->_asynData->_flag, AsynOpType::OP_REGISTER)))
                                _AsynRegister(timeData, timeData->_period, timeData->_expiredTime + timeData->_period);
                        }
                    }
                }
            }
        // #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        // }
        // else
        // {
        //     g_Log->Error(LOGFMT_OBJ_TAG("timer mgr handle timer event error"));
        // }
        // #endif
    }

    _AfterDrive();

    // 重算过期标记
    if(UNLIKELY(!_expireQueue.empty()))
    {
        auto timeData =  *_expireQueue.begin();
        _hasExpired = (_curTime = TimeUtil::GetFastNanoTimestamp()) >= timeData->_expiredTime;
    }
    else
    {
        _hasExpired = false;
    }
}

TimeData *TimerMgr::NewTimeData(LibTimer *timer)
{
    auto newData = TimeData::NewThreadLocal_TimeData(++_curMaxId, timer);
    _allTimeData.insert(newData);
    return newData;
}

Int64 TimerMgr::GetTimeoutIntervalRecently(Int64 nowMs) const
{
    if(UNLIKELY(_allTimeData.empty()))
        return -1;

    TimeData *recently = *_allTimeData.begin();
    Int64 diff = recently->_expiredTime - nowMs;
    return diff > 0 ? diff : 0;    
}

void TimerMgr::_AsynRegister(TimeData *timeData, Int64 newPeriod, Int64 newExpiredTime)
{
    auto asynData = timeData->_asynData;
    asynData->MaskRegister(newExpiredTime, newPeriod);
    timeData->_isScheduing = true;

    _asynDirty.insert(asynData);
}

void TimerMgr::_Register(TimeData *timeData, Int64 newPeriod, Int64 newExpiredTime)
{
    timeData->_period = newPeriod;
    timeData->_expiredTime = newExpiredTime;
    timeData->_isScheduing = true;

    _expireQueue.insert(timeData);
}

void TimerMgr::_AsynUnRegister(TimeData *timeData)
{
    auto asynData = timeData->_asynData;
    timeData->_isScheduing = false;
    asynData->MaskUnRegister();
    _asynDirty.insert(asynData);
}

void TimerMgr::_UnRegister(TimeData *timeData)
{
    timeData->_isScheduing = false;
    _expireQueue.erase(timeData);
}

void TimerMgr::_Destroy(TimeData *timeData)
{
    _expireQueue.erase(timeData);
    timeData->_isScheduing = false;
    timeData->_owner = NULL;
    _allTimeData.erase(timeData);
    timeData->Release();
}

void TimerMgr::_AsynDestroy(TimeData *timeData)
{
    timeData->_owner = NULL;
    timeData->_isScheduing = false;
    timeData->_asynData->MaskDestroy();
    _asynDirty.insert(timeData->_asynData);
}

bool TimerMgr::_IsInTimerThread()
{
    return _launchThreadId == SystemUtil::GetCurrentThreadId();
}

void TimerMgr::Close()
{
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
    
    _expireQueue.clear();
    _asynDirty.clear();
}

void TimerMgr::OnTimerDestroy(TimeData *timeData)
{
    // 先移除
    if(timeData->_owner)
    {
        auto iter = _timerRefDeleteMethod.find(timeData->_owner);
        if(iter != _timerRefDeleteMethod.end())
        {
            iter->second->Release();
            _timerRefDeleteMethod.erase(iter);
        }
    }

    // 在Drive中的时候由Drive函数释放
    if(_driving <= 0)
    {
        _asynDirty.erase(timeData->_asynData);
        _Destroy(timeData);
    }
    else
    {
        _AsynDestroy(timeData);
    }
}

void TimerMgr::_AfterDrive()
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
            if(BitUtil::IsSet(flag, AsynOpType::OP_REGISTER) && asynData->_data->_owner)
                _Register(asynData->_data, asynData->_newPeriod, asynData->_newExpiredTime);

            // 重置数据
            asynData->Reset();
        }
        
        iter = _asynDirty.erase(iter);
    }
}

KERNEL_END