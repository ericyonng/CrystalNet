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

#include <service_common/common/common.h>
#include <kernel/kernel.h>
#include <service_common/application/KernelConfig.h>
#include <service_common/application/ApplicationConfig.h>
#include <service_common/application/ResponseInfo.h>

SERVICE_COMMON_BEGIN

class Application : public KERNEL_NS::IApplication
{
    POOL_CREATE_OBJ_DEFAULT_P1(IApplication, Application);

public:
    Application();
    ~Application();
    void Release();

public:
    void SetIniFile(const KERNEL_NS::LibString &ini);
    void SetMemoryIniContent(const KERNEL_NS::LibString &content);
    const KernelConfig &GetKernelConfig() const;
    const KERNEL_NS::PollerConfig &GetPollerConfig() const;
    const KERNEL_NS::LibString &GetProjectMainServiceName() const; // ?????????????????????, ??????Gate, Login???
    const KERNEL_NS::LibString &GetMachineApplyId() const;
    UInt16 GetMachineId() const; 
    const KERNEL_NS::LibIniFile *GetIni() const;
    KERNEL_NS::LibIniFile *GetIni();
    virtual const KERNEL_NS::LibString &GetAppAliasName() const;

    void Clear() override;
    virtual KERNEL_NS::LibString ToString() const override;
    virtual void OnRegisterComps() final; 

    void PushResponceNs(UInt64 costNs); 

protected:
    // ?????????????????????
    virtual Int32 _OnHostInit() final;
    // ????????????????????????
    virtual Int32 _OnCompsCreated() final;
    // ?????????????????????
    virtual Int32 _OnHostWillStart() final;
    // ??????????????????
    virtual Int32 _OnHostStart() final;
    // ?????????willclose??????
    virtual void _OnHostBeforeCompsWillClose() final;
    // ?????????willclose??????
    virtual void _OnHostWillClose() final;
    // ?????????Close??????
    virtual void _OnHostClose() final;

protected:
    virtual void _OnMonitorThreadFrame();

    // ????????????
private:
    void _Clear();

    // ???????????????
    Int32 _ReadBaseConfigs();

    // ????????????id?????????id
    void _GenerateMachineApplyId();

    // ??????
    void _OnMonitor(KERNEL_NS::LibThread *t);

private:
    KERNEL_NS::LibString _ini;                              // ???????????????
    KERNEL_NS::LibString _memoryIni;                        // ?????????????????????
    KERNEL_NS::LibIniFile *_configIni;                      // ?????????
    KERNEL_NS::PollerConfig _pollerConfig;                  // poller??????
    KernelConfig _kernelConfig;                             // ????????????
    ApplicationConfig _appConfig;                           // app??????

    // ????????????
    KERNEL_NS::LibThread *_monitor;

    // ????????????
    KERNEL_NS::SpinLock _lck;
    StatisticsInfo *_statisticsInfo;
    StatisticsInfo *_statisticsInfoCache;
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

ALWAYS_INLINE const KERNEL_NS::PollerConfig &Application::GetPollerConfig() const
{
    return _pollerConfig;
}

ALWAYS_INLINE const KERNEL_NS::LibString &Application::GetProjectMainServiceName() const
{
    return _appConfig._projectMainServiceName;
}

ALWAYS_INLINE const KERNEL_NS::LibString &Application::GetMachineApplyId() const
{
    return _appConfig._machineApplyId;
}

ALWAYS_INLINE UInt16 Application::GetMachineId() const
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

ALWAYS_INLINE void Application::PushResponceNs(UInt64 costNs)
{
    _lck.Lock();
    if(_statisticsInfo->_minResNs == 0)
        _statisticsInfo->_minResNs = costNs;
    if(_statisticsInfo->_maxResNs < costNs)
        _statisticsInfo->_maxResNs = costNs;

    ++_statisticsInfo->_resCount;
    _statisticsInfo->_resTotalNs += costNs;
    _lck.Unlock();
}

SERVICE_COMMON_END


#endif
