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
 * Date: 2021-03-22 00:21:06
 * Author: Eric Yonng
 * Description: 
 *              1.文件读写能力
 *              2.网络引擎
 *              3.数据库能力 支持MySql,MongoDB,Redis,数据结构：IRecord
 *              4.辅助组件功能
 *              5.监控信息更新
 *              6.......
 * 
 *  1.可选重写的接口:OnRegisterComps/ToString/Clear/OnUpdate/
 * 2.对象初始化启动相关重写:_OnInit/_OnWillStart/_OnStart/_OnWillClose/_OnClose
 * 3.使用规则: 在OnekeyInit之前请设置app配置等，之后：OnekeyInit/OnekeyStart/OnekeyFinish
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_APP_IAPPLICATION_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_APP_IAPPLICATION_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/App/BaseAppOption.h>
#include <kernel/comp/CompObject/CompObjectInc.h>
#include <kernel/comp/Lock/Lock.h>
#include <kernel/comp/LibTime.h>

KERNEL_BEGIN

class LibThread;
class PollerMgr;
class IServiceProxy;
class ILogFactory;
class IServiceProxyFactory;
struct PollerEvent;
class Poller;

class KERNEL_EXPORT IApplication : public CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, IApplication);
public:
    IApplication();
    virtual ~IApplication();
    // 多线程组件需要重写该接口
    virtual void DefaultMaskReady(bool isReady) {}

public:
    virtual void WaitFinish(Int32 &err);
    virtual void SinalFinish(Int32 err = Status::Success) = 0;

    virtual const LibString &GetAppName() const;
    virtual const LibString &GetAppAliasName() const = 0;
    const LibString &GetAppPath() const;
    Int32 GetProcessId() const;
    UInt64 GetThreadId() const;
    const LibTime &GetAppStartTime() const;
    Int32 GetCpuCoreAmount() const;

    void SetAppArgs(const std::vector<KERNEL_NS::LibString> &args);
    const std::vector<KERNEL_NS::LibString> &GetAppArgs() const;

    // poller 的时间片 默认1秒
    void SetMaxPieceTimeInMicroseconds(Int64 maxPieceTimeInMicroseconds);
    // 最大优先级 默认0
    void SetMaxPriorityLevel(Int32 maxPriorityLevel);
    // 扫描时间间隔 默认1秒
    void SetMaxSleepMilliseconds(UInt64 maxSleepMilliseconds);

public:
    void Clear() override;
    virtual LibString ToString() const override;
    virtual LibString IntroduceStr() const;

protected:
    virtual void OnRegisterComps() override;  
    // 在组件初始化前
    virtual Int32 _OnHostInit() override;
    // 优先级组件创建完成
    virtual Int32 _OnPriorityLevelCompsCreated() override;
    // 所有组件创建完成
    virtual Int32 _OnCompsCreated() override;
    // 在组件启动之前
    virtual Int32 _OnHostWillStart() override;
    // 组件启动之后
    virtual Int32 _OnHostStart() override;
    // 在组件willclose之后
    virtual void _OnHostWillClose() override;
    // 在组件Close之后
    virtual void _OnHostClose() override;
    // 在组件都更新之后
    virtual void _OnHostUpdate() override;

private:
    void _Clear();

protected:
    LibString _appName;         // 程序名
    LibString _path;            // 程序路径
    Int32 _processId;           // 进程id
    UInt64 _threadId;            // app所在线程id
    LibTime _appStartTime;      // 程序启动时间
    std::atomic<Int32> _runErr; // 运行时错误码

    std::vector<KERNEL_NS::LibString> _appArguments;    // 程序传入的参数 

    Int32 _cpuCoreAmount;       // cpu核心数量
    LibString _cpuVendor;       // cpu厂商
    LibString _cpuBrand;        // cpu商标

    Poller *_poller;
    Int64 _maxPieceTimeInMicroseconds;  // poller的时间片
    Int32 _maxPriorityLevel;            // poller的最大优先级别, 默认是0, 也就是一个优先级队列
    UInt64 _maxSleepMilliseconds;       // poller 的扫描时间间隔

    LockWrap<_Build::MT, LockParticleType::Heavy> _lck;
};

ALWAYS_INLINE const LibString &IApplication::GetAppName() const
{
    return _appName;
}

ALWAYS_INLINE const LibString &IApplication::GetAppPath() const
{
    return _path;
}

ALWAYS_INLINE Int32 IApplication::GetProcessId() const
{
    return _processId;
}

ALWAYS_INLINE UInt64 IApplication::GetThreadId() const
{
    return _threadId;
}

ALWAYS_INLINE const LibTime &IApplication::GetAppStartTime() const
{
    return _appStartTime;
}

ALWAYS_INLINE Int32 IApplication::GetCpuCoreAmount() const
{
    return _cpuCoreAmount;
}

ALWAYS_INLINE void IApplication::SetAppArgs(const std::vector<KERNEL_NS::LibString> &args)
{
    _appArguments = args;
}

ALWAYS_INLINE const std::vector<KERNEL_NS::LibString> &IApplication::GetAppArgs() const
{
    return _appArguments;
}

ALWAYS_INLINE void IApplication::SetMaxPieceTimeInMicroseconds(Int64 maxPieceTimeInMicroseconds)
{
    _maxPieceTimeInMicroseconds = maxPieceTimeInMicroseconds;
}

ALWAYS_INLINE void IApplication::SetMaxPriorityLevel(Int32 maxPriorityLevel)
{
    _maxPriorityLevel = maxPriorityLevel;
}

ALWAYS_INLINE void IApplication::SetMaxSleepMilliseconds(UInt64 maxSleepMilliseconds)
{
    _maxSleepMilliseconds = maxSleepMilliseconds;
}

KERNEL_END

KERNEL_EXPORT extern KERNEL_NS::IApplication *g_Application;

#endif
