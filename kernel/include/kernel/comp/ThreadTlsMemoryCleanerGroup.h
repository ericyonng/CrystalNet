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
 * Date: 2023-07-02 21:08:0
 * Author: Eric Yonng
 * Description: tls内存定时清理组件 组合(timermgr, tlsmemorycleaner)
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_THREAD_TLS_MEMORY_CLEANER_GROUP_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_THREAD_TLS_MEMORY_CLEANER_GROUP_H__

#pragma once

#include <kernel/comp/CompObject/CompHostObject.h>

KERNEL_BEGIN

class TimerMgr;

class KERNEL_EXPORT ThreadTlsMemoryCleanerGroup : public CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, ThreadTlsMemoryCleanerGroup);

public:
    ThreadTlsMemoryCleanerGroup();
    ~ThreadTlsMemoryCleanerGroup();

    virtual void Release() override;

public:
    virtual void OnRegisterComps() override;

    // 设置清理时间间隔, 默认1分钟清理一次
    void SetIntervalMs(Int64 intervalMs);

    // 如果不方便将组件加入到宿主中随宿主一起启动(一般随宿主一起启动那么清理的是宿主所在线程的内存), 若不随宿主启动, 那么请设置手动启动, 那么要清理的线程只能挂在那个线程的TimrMgr上）
    void SetManualStart(bool isManualStart = true);

    // 已经有了Timer则不可Attach
    bool AttachTimerMgr(TimerMgr *timerMgr);

    // 时钟驱动
    void Drive();

    OBJ_GET_OBJ_TYPEID_DECLARE();

protected:
    virtual Int32 _OnHostInit() override;

    virtual Int32 _OnCompsCreated() override;
    virtual Int32 _OnHostStart() override;
    virtual void _OnHostClose() override;
    void _Clear();

private:
    TimerMgr *_timerMgr;
    bool _isTimerAttach;
    Int64 _intervalMs;
    bool _isManualStart;
};

ALWAYS_INLINE void ThreadTlsMemoryCleanerGroup::SetIntervalMs(Int64 intervalMs)
{
    _intervalMs = intervalMs;
}

ALWAYS_INLINE void ThreadTlsMemoryCleanerGroup::SetManualStart(bool isManualStart)
{
    _isManualStart = isManualStart;
}

ALWAYS_INLINE bool ThreadTlsMemoryCleanerGroup::AttachTimerMgr(TimerMgr *timerMgr)
{
    if(_timerMgr)
        return false;

    _timerMgr = timerMgr;
    _isTimerAttach = true;

    return true;
}

// 工厂
class KERNEL_EXPORT ThreadTlsMemoryCleanerGroupFactory : public KERNEL_NS::CompFactory
{
public:
    static constexpr KERNEL_NS::_Build::MT _buildType{};

    static KERNEL_NS::CompFactory *FactoryCreate();

    virtual void Release() override;
    
    OBJ_GET_OBJ_TYPEID_DECLARE();

public:
    virtual KERNEL_NS::CompObject *Create() const override;
    static KERNEL_NS::CompObject *StaticCreate();
    static ThreadTlsMemoryCleanerGroup *StaticCreateAs();
};

KERNEL_END

#endif
