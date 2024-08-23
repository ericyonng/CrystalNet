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
 * Date: 2023-07-01 13:41:16
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_KILL_MONITOR_INTERFACE_IKILL_MONITOR_H__
#define __CRYSTAL_NET_SERVICE_COMMON_KILL_MONITOR_INTERFACE_IKILL_MONITOR_H__

#pragma once

#include <service_common/common/macro.h>
#include <kernel/comp/CompObject/CompObject.h>

KERNEL_BEGIN

class LibString;
class TimerMgr;

KERNEL_END

SERVICE_COMMON_BEGIN

class IKillMonitorMgr : public KERNEL_NS::CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, IKillMonitorMgr);

public:
    IKillMonitorMgr(UInt64 objTypeId) : KERNEL_NS::CompObject(objTypeId) {}
    
    virtual bool IsReadyToDie() const = 0;

    // 存在该文件说明是关闭程序的操作, 这个检测是每隔10秒检测一次， detectionFile需要带绝对路径
    virtual void SetDeadthDetectionFile(const KERNEL_NS::LibString &detectionFile) = 0;

    // 设置timerMgr
    virtual void SetTimerMgr(KERNEL_NS::TimerMgr *timerMgr) = 0;

    // 设置检测时间间隔, 没有设置默认10秒
    virtual void SetDetectionTimeInterval(Int64 seconds) = 0;
};

SERVICE_COMMON_END

#endif
