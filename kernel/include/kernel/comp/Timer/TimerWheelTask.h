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
 * Date: 2023-12-31 20:04:11
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TIME_WHEEL_TASK_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TIME_WHEEL_TASK_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class TimeWheelTimer;

// 能被TimerWhell销毁的条件:_attachTimer为空
// 能被attachTimer销毁的条件:_isInTick为false
class KERNEL_EXPORT TimerWheelTask
{
    POOL_CREATE_OBJ_DEFAULT(TimerWheelTask);

public:
    explicit TimerWheelTask();
    ~TimerWheelTask(){
        _head = NULL;
        _pre = NULL;
        _next = NULL;
        _attachTimer = NULL;
        _isInTicking = false;
    }

    void UpdateTimerInfo();

    LibString ToString() const;
    
public:
    // 链表头
    TimerWheelTask **_head;
    TimerWheelTask *_pre;
    TimerWheelTask *_next;

    // 当前过期时间 ns
    Int64 _expiredTime;

    // 定时周期 ns
    Int64 _period;

    // 是否需要调度
    bool _isScheduling;

    // 定时器
    TimeWheelTimer *_attachTimer;

    // 是否正在tick
    bool _isInTicking;

    // timer 的信息
    LibString _timerInfo;
};

KERNEL_END


#endif

