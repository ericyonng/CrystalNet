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
 * Date: 2022-06-21 12:42:14
 * Author: Eric Yonng
 * Description: 
*/


#ifndef __CRYSTAL_NET_SERVICE_COMMON_APPLICATION_APPLICATION_H__
#define __CRYSTAL_NET_SERVICE_COMMON_APPLICATION_APPLICATION_H__

#pragma once

#include <service_common/common/macro.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Lock/Impl/SpinLock.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerConfig.h>
#include <kernel/comp/App/app.h>
#include <service_common/application/KernelConfig.h>
#include <service_common/application/ApplicationConfig.h>

KERNEL_BEGIN

class LibIniFile;
class LibThread;
class LibTimer;

KERNEL_END

SERVICE_COMMON_BEGIN

struct StatisticsInfo;

class Application : public KERNEL_NS::IApplication
{
    POOL_CREATE_OBJ_DEFAULT_P1(IApplication, Application);

public:
    Application();
    ~Application();
    void Release() override;

public:
    void SetIniFile(const KERNEL_NS::LibString &ini);
    void SetMemoryIniContent(const KERNEL_NS::LibString &content);
    const KernelConfig &GetKernelConfig() const;
    const KERNEL_NS::LibString &GetProjectMainServiceName() const; // 获取项目功能名, 如：Gate, Login等
    const KERNEL_NS::LibString &GetMachineApplyId() const;
    UInt32 GetMachineId() const; 
    void SetMachineId(UInt32 machineId);
    const KERNEL_NS::LibIniFile *GetIni() const;
    KERNEL_NS::LibIniFile *GetIni();
    virtual const KERNEL_NS::LibString &GetAppAliasName() const override;

    void Clear() override;
    virtual KERNEL_NS::LibString ToString() const override;
    virtual void OnRegisterComps() override; 

    void PushResponceNs(UInt64 costNs); 

    // 设置事件最大类型
    void SetMaxEventType(Int32 maxEventType);
    Int32 GetMaxEventType() const;

    void SinalFinish(Int32 err = Status::Success) override;

protected:
    // 在组件初始化前
    virtual Int32 _OnHostInit() override;
    // 所有组件创建完成
    virtual Int32 _OnCompsCreated() override;
    // 在组件启动之前
    virtual Int32 _OnHostWillStart() override;
    // 组件启动之后
    virtual Int32 _OnHostStart() override;
    // 在组件willclose之前
    virtual void _OnHostBeforeCompsWillClose() override;
    // 在组件willclose之后
    virtual void _OnHostWillClose() override;
    // 在组件Close之后
    virtual void _OnHostClose() override;

protected:
    virtual void _OnMonitorThreadFrame();
    
    void _OnQuitApplicationEvent(KERNEL_NS::PollerEvent *ev);

    void _DoCloseApp();

    // 每帧打印
private:
    void _Clear();

    // 读基本配置
    Int32 _ReadBaseConfigs();

    // 生成机器id的申请id
    void _GenerateMachineApplyId();

    // 监控
    void _OnMonitor(KERNEL_NS::LibThread *t);

    // 检测程序退出
    void _OnKillMonitorTimeOut(KERNEL_NS::LibTimer *timer);

private:
    KERNEL_NS::LibString _ini;                              // 配置表路径
    KERNEL_NS::LibString _memoryIni;                        // 内存配置表内容
    KERNEL_NS::LibIniFile *_configIni;                      // 配置表
    KernelConfig _kernelConfig;                             // 内核配置
    ApplicationConfig _appConfig;                           // app配置

    // 监控线程
    KERNEL_NS::LibThread *_monitor;

    // 退出程序监控定时
    KERNEL_NS::LibTimer *_killMonitorTimer;

    // 响应时间
    KERNEL_NS::SpinLock _guard;
    StatisticsInfo *_statisticsInfo;
    StatisticsInfo *_statisticsInfoCache;

    Int32 _maxEventType;
};
    
ALWAYS_INLINE void Application::SetIniFile(const KERNEL_NS::LibString &ini)
{
    _ini = ini;
}

ALWAYS_INLINE void Application::SetMemoryIniContent(const KERNEL_NS::LibString &content)
{
    _memoryIni = content;
}

ALWAYS_INLINE const KernelConfig &Application::GetKernelConfig() const
{
    return _kernelConfig;
}

ALWAYS_INLINE const KERNEL_NS::LibString &Application::GetProjectMainServiceName() const
{
    return _appConfig._projectMainServiceName;
}

ALWAYS_INLINE const KERNEL_NS::LibString &Application::GetMachineApplyId() const
{
    return _appConfig._machineApplyId;
}

ALWAYS_INLINE UInt32 Application::GetMachineId() const
{
    return _appConfig._machineId;
}

ALWAYS_INLINE const KERNEL_NS::LibIniFile *Application::GetIni() const
{
    return _configIni;
}

ALWAYS_INLINE KERNEL_NS::LibIniFile *Application::GetIni()
{
    return _configIni;
}

ALWAYS_INLINE void Application::SetMaxEventType(Int32 maxEventType)
{
    if(maxEventType >= _maxEventType)
        _maxEventType = maxEventType;
}

ALWAYS_INLINE Int32 Application::GetMaxEventType() const
{
    return _maxEventType;
}

SERVICE_COMMON_END


#endif
