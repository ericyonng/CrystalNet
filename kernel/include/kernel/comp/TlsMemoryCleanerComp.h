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
 * Description: tls内存定时清理组件
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP__TLS_MEMORY_CLEANER_COMP_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP__TLS_MEMORY_CLEANER_COMP_H__

#pragma once

#include <kernel/comp/CompObject/CompObject.h>
#include <kernel/comp/CompObject/CompFactory.h>

KERNEL_BEGIN

class TimerMgr;
class LibTimer;
class TlsDefaultObj;

class KERNEL_EXPORT TlsMemoryCleanerComp : public CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, TlsMemoryCleanerComp);

public:
    TlsMemoryCleanerComp();
    ~TlsMemoryCleanerComp();

    virtual void Release() override;

    void SetTimerMgr(TimerMgr *timerMgr);

    // 设置清理时间间隔, 默认1分钟清理一次
    void SetIntervalMs(Int64 intervalMs);

    // 如果不方便将组件加入到宿主中随宿主一起启动(一般随宿主一起启动那么清理的是宿主所在线程的内存), 若不随宿主启动, 那么请设置手动启动, 那么要清理的线程只能挂在那个线程的TimrMgr上）
    void SetManualStart(bool isManualStart = true);

    // 手动模式下调用接口
    Int32 ManualStart();
    void ManualClose();

    OBJ_GET_OBJ_TYPEID_DECLARE();

protected:
    virtual Int32 _OnInit() override;
    virtual Int32 _OnStart() override;
    virtual void _OnWillClose() override;
    void _Clear();

    void _OnCleanTimer(LibTimer *timer);
    Int32 _DoStart();
    void _PurgeTlsMemory();

private:
    TimerMgr *_timerMgr;
    Int64 _intervalMs;         // 定时清理的时间间隔
    LibTimer *_timer;           // 定时器
    TlsDefaultObj *_tlsDefaultObj;
    bool _isManualStart;        // 手动启动
};

ALWAYS_INLINE void TlsMemoryCleanerComp::SetTimerMgr(TimerMgr *timerMgr)
{
    _timerMgr = timerMgr;
}

ALWAYS_INLINE void TlsMemoryCleanerComp::SetManualStart(bool isManualStart)
{
    _isManualStart = isManualStart;
}

// 工厂
class KERNEL_EXPORT TlsMemoryCleanerCompFactory : public KERNEL_NS::CompFactory
{
public:
    static constexpr KERNEL_NS::_Build::MT _buildType{};

    static KERNEL_NS::CompFactory *FactoryCreate();

    virtual void Release() override;
    
    OBJ_GET_OBJ_TYPEID_DECLARE();

public:
    virtual KERNEL_NS::CompObject *Create() const override;
    static KERNEL_NS::CompObject *StaticCreate();
};

KERNEL_END

#endif
