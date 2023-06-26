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
#include <kernel/comp/Timer/TimerMgr.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/Timer/LibTimer.h>
#include <kernel/comp/Timer/TimerDefs.h>
#include <kernel/comp/Variant/variant_inc.h>
#include <kernel/comp/Cpu/LibCpuCounter.h>
#include <kernel/comp/Log/log.h>

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

void TimerMgr::Drive()
{
    _BeforeDrive();

    if(LIKELY(!_expireQueue.empty()))
    {
        _curTime = TimeUtil::GetFastMicroTimestamp();
        TimeData *expiredHead = NULL;
        TimeData *expiredEnd = NULL;

        // 判断头节点过期
        auto iter = _expireQueue.begin();
        if((*iter)->_expiredTime <= _curTime)
        {
            expiredHead = *iter;
            expiredEnd = expiredHead;
            expiredEnd->_next = NULL;
            iter = _expireQueue.erase(iter);
        }

        // 第一个有过期就判断其他有没有过期
        if(expiredHead)
        {
            for(; iter != _expireQueue.end();)
            {
                // 未过期的为止
                auto timeData = *iter;
                if(timeData->_expiredTime > _curTime)
                    break;

                expiredEnd->_next = timeData;
                expiredEnd = timeData;
                expiredEnd->_next = NULL;

                iter = _expireQueue.erase(iter);
            }
        }

        for (;expiredHead;)
        {
            auto timeData = expiredHead;
            expiredHead = expiredHead->_next;

            if(timeData->_isScheduing)
            {
                timeData->_owner->OnTimeOut();

                if(LIKELY(timeData->_owner))
                {
                    // 重新被调度
                    if(timeData->_isScheduing)
                        _Register(timeData, timeData->_period, timeData->_expiredTime + timeData->_period);
                }
            }
        }

        // // 重算过期标记
        // if(UNLIKELY(!_expireQueue.empty()))
        // {
        //     auto timeData =  *_expireQueue.begin();
        //     _hasExpired = TimeUtil::GetFastMicroTimestamp() >= timeData->_expiredTime;
        // }
        // else
        // {
        //     _hasExpired = false;
        // }
    }

    _AfterDrive();

    // 重算过期标记
    if(UNLIKELY(!_expireQueue.empty()))
    {
        auto timeData =  *_expireQueue.begin();
        _hasExpired = (_curTime = TimeUtil::GetFastMicroTimestamp()) >= timeData->_expiredTime;
    }
    else
    {
        _hasExpired = false;
    }
}

void TimerMgr::SafetyDrive()
{
   _BeforeDrive();

    if(LIKELY(!_expireQueue.empty()))
    {
        _curTime = TimeUtil::GetFastMicroTimestamp();
        UniqueSmartPtr<LibList<TimeData *, _Build::TL>, AutoDelMethods::CustomDelete> timeDataList = LibList<TimeData *, _Build::TL>::NewThreadLocal_LibList();
        timeDataList.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<LibList<TimeData *, _Build::TL> *>(p);
            LibList<TimeData *, _Build::TL>::DeleteThreadLocal_LibList(ptr);
        });

        try
        {
            for(auto iter = _expireQueue.begin(); iter != _expireQueue.end();)
            {
                // 未过期的为止
                auto timeData = *iter;
                if(timeData->_expiredTime > _curTime)
                    break;

                timeDataList->PushBack(timeData);
                iter = _expireQueue.erase(iter);
            }

            for(auto node = timeDataList->Begin(); node;)
            {
                auto timeData = node->_data;
                if(timeData->_isScheduing)
                {
                    timeData->_owner->OnTimeOut();

                    if(LIKELY(timeData->_owner))
                    {
                        // 重新被调度
                        if(timeData->_isScheduing)
                            _Register(timeData, timeData->_period, timeData->_expiredTime + timeData->_period);
                    }
                }

                node = timeDataList->Erase(node);
            }
        }
        catch(const std::exception& e)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("eception when drive timer:%s"), e.what());
        }
        catch(...)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("unknown eception when drive timer."));
            throw;
        }
    }

    _AfterDrive();

    // 重算过期标记
    if(UNLIKELY(!_expireQueue.empty()))
    {
        auto timeData =  *_expireQueue.begin();
        _hasExpired = (_curTime = TimeUtil::GetFastMicroTimestamp()) >= timeData->_expiredTime;
    }
    else
    {
        _hasExpired = false;
    }
}

void TimerMgr::Close()
{
    _expireQueue.clear();
    ContainerUtil::DelContainer<TimeData *, AutoDelMethods::Release>(_allTimeData);
    ContainerUtil::DelContainer<AsynTimeData *, AutoDelMethods::Release>(_asynDirty);

    if(LIKELY(_wakeupCb))
    {
        _wakeupCb->Release();
        _wakeupCb = NULL;
    }
}


KERNEL_END