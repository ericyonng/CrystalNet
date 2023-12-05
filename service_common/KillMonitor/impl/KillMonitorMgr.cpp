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
 * Date: 2023-07-01 14:01:29
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Timer/Timer.h>
#include <service_common/KillMonitor/impl/KillMonitorMgr.h>
#include <service_common/KillMonitor/impl/KillMonitorMgrFactory.h>

SERVICE_COMMON_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(KillMonitorMgr);

KillMonitorMgr::KillMonitorMgr()
:_isReadyToDie(false)
,_enableDetection(false)
,_detectionInterval(10)
,_timerMgr(NULL)
,_timer(NULL)
{

}

KillMonitorMgr::~KillMonitorMgr()
{
    _Clear();
}

void KillMonitorMgr::Release()
{
    KillMonitorMgr::DeleteByAdapter_KillMonitorMgr(KillMonitorMgrFactory::_buildType.V, this);
}

bool KillMonitorMgr::IsReadyToDie() const
{
    return _isReadyToDie;
}

void KillMonitorMgr::SetDeadthDetectionFile(const KERNEL_NS::LibString &detectionFile)
{
    _deadthDetectionFile = detectionFile;
}

void KillMonitorMgr::SetTimerMgr(KERNEL_NS::TimerMgr *timerMgr)
{
    _timerMgr = timerMgr;
}

void KillMonitorMgr::SetDetectionTimeInterval(Int64 seconds)
{
    _detectionInterval = KERNEL_NS::TimeSlice::FromSeconds(seconds);
}

Int32 KillMonitorMgr::_OnInit()
{
    _isReadyToDie = false;
    _detectionFileNameWithoutDir = KERNEL_NS::DirectoryUtil::GetFileNameInPath(_deadthDetectionFile.c_str());

    if(!_detectionFileNameWithoutDir.empty())
    {
        _enableDetection = true;
        _DelDetectionFile();

        if(LIKELY(_timerMgr))
        {
            _timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer(_timerMgr);
            _timer->SetTimeOutHandler(this, &KillMonitorMgr::_OnDetectionTimerOut);
            _timer->Schedule(_detectionInterval);
        }
    }

    g_Log->Info(LOGFMT_OBJ_TAG("enableDetection:%d _detectionInterval:%lld ms will detect deadth file:%s, name without dir:%s")
        , _enableDetection, _detectionInterval.GetTotalMilliSeconds(), _deadthDetectionFile.c_str(), _detectionFileNameWithoutDir.c_str());

    return Status::Success;
}

void KillMonitorMgr::_OnClose()
{
    _Clear();
}

void KillMonitorMgr::_Clear()
{
    if(_enableDetection)
        _DelDetectionFile();

    if(_timer)
    {
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(_timer);
        _timer = NULL;
    }

    _enableDetection = false;
}

void KillMonitorMgr::_DelDetectionFile() const
{
    if(!_deadthDetectionFile.empty())
        KERNEL_NS::FileUtil::DelFileCStyle(_deadthDetectionFile.c_str());
}

void KillMonitorMgr::_OnDetectionTimerOut(KERNEL_NS::LibTimer *timer)
{
    // 文件检测到说明需要关闭
    if(KERNEL_NS::FileUtil::IsFileExist(_deadthDetectionFile.c_str()))
    {
        _isReadyToDie = true;
        _timer->Cancel();
    }
}

SERVICE_COMMON_END
