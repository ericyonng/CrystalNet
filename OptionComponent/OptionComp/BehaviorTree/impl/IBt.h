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
 * Date: 2023-09-27 13:18:00
 * Author: Eric Yonng
 * Description:
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_BEHAVIOR_TREE_IMPL_IBT_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_BEHAVIOR_TREE_IMPL_IBT_H__

#pragma once

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/LibTime.h>
#include <kernel/comp/TimeSlice.h>
#include <OptionComp/BehaviorTree/impl/BtNodeData.h>
#include <kernel/comp/Delegate/LibDelegate.h>

KERNEL_BEGIN

class LibTimer;
class IBtNode;
class TimerMgr;

class IBt
{
    POOL_CREATE_OBJ_DEFAULT(IBt);

public:
    IBt(UInt64 id);
    virtual ~IBt();
    virtual void Release() = 0;

    virtual Int32 Init() final;
    virtual void Close() final;

    // tick 时间间隔
    void SetIntervalMs(Int64 ms);

    // 根节点
    void SetRootNode(IBtNode *root);
    IBtNode *GetRoot();
    const IBtNode *GetRoot() const;

    // 时钟调度
    void SetTimerMgr(TimerMgr *timerMgr);

    const LibTime &GetNow() const;

    virtual void SetState(Int32 state);
    Int32 GetState() const;

    UInt64 GetBtId() const;

    virtual KERNEL_NS::LibString ToString() const;

    bool IsStarted() const;
    bool IsRunning() const;
    bool IsFinished() const;
    bool IsClosed() const;
    bool IsSuccess() const;
    bool IsFailure() const;

protected:
    virtual void _OnTick(LibTimer *t);

    virtual Int32 _OnInit() { return Status::Success; }
    virtual void _OnFinished() {}
    
    void _BeforeClose() {}
    void _OnBtClose() {}

private:
    void _Clear();

protected:
    bool _isInited;
    bool _isClosed;

    Int32 _state;  // BtState
    bool _isSuccess;
    bool _isFailure;
    const UInt64 _id;

    IBtNode *_root;
    LibTime _nowTime;
    LibTimer *_timer;
    TimeSlice _interval;
    TimerMgr *_timerMgr;
};

ALWAYS_INLINE void IBt::SetIntervalMs(Int64 ms)
{
    _interval = TimeSlice::FromMilliSeconds(ms);

    if(UNLIKELY(_timer && _timer->IsScheduling()))
        _timer->Schedule(_interval);
}

ALWAYS_INLINE IBtNode *IBt::GetRoot()
{
    return _root;
}

ALWAYS_INLINE const IBtNode *IBt::GetRoot() const
{
    return _root;
}

ALWAYS_INLINE void IBt::SetTimerMgr(TimerMgr *timerMgr)
{
    _timerMgr = timerMgr;
}

ALWAYS_INLINE const LibTime &IBt::GetNow() const
{
    return _nowTime;
}

ALWAYS_INLINE Int32 IBt::GetState() const
{
    return _state;
}

ALWAYS_INLINE UInt64 IBt::GetBtId() const
{
    return _id;
}

ALWAYS_INLINE bool IBt::IsStarted() const
{
    return _state > BtState::INITED;
}

ALWAYS_INLINE bool IBt::IsRunning() const
{
    return _state == BtState::RUNNING;
}

ALWAYS_INLINE bool IBt::IsFinished() const
{
    return _state == BtState::FINISHED;
}

ALWAYS_INLINE bool IBt::IsClosed() const
{
    return _isClosed;
}

ALWAYS_INLINE bool IBt::IsSuccess() const
{
    return _isSuccess;
}

ALWAYS_INLINE bool IBt::IsFailure() const
{
    return _isFailure;
}


KERNEL_END

#endif
