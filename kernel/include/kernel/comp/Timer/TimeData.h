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
 * Date: 2021-03-17 10:31:46
 * Description: 只支持thread local 创建和释放
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TIMER_TIMER_DATA_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TIMER_TIMER_DATA_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class TimeData;
class Variant;

class KERNEL_EXPORT TimeDataComp
{
public:
    bool operator()(const TimeData *l, const TimeData *r) const;
};

class LibTimer;
template<typename T>
class LibStream;

class AsynTimeData;

class KERNEL_EXPORT TimeData
{
    POOL_CREATE_OBJ_DEFAULT(TimeData);

public:
    TimeData(Int64 id, LibTimer *timer);
    ~TimeData();
    void Release();
    void MoveParams(Variant *params);

    LibString ToString() const;

    const Int64 _id;               // 定时器id
    Int64 _expiredTime;             // 当前过期时间 微妙
    Int64 _period;                  // 定时周期 微妙
    LibTimer *_owner;               // 定时器对象

    bool _isScheduing;              // 处于定时阶段
    AsynTimeData *_asynData;        // 异步数据 这个数据由TimerMgr统一创建与释放，这里只引用
    Variant *_params;             // 参数
};

ALWAYS_INLINE bool TimeDataComp::operator()(const TimeData *l, const TimeData *r) const
{
    if(!l || !r)
        return l < r;

    if(l == r)
        return false;
    
    if(l->_expiredTime == r->_expiredTime)
        return l->_id < r->_id;

    return l->_expiredTime < r->_expiredTime;
}

ALWAYS_INLINE void TimeData::Release()
{
    TimeData::DeleteThreadLocal_TimeData(this);
}


KERNEL_END

#endif
