// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2026-06-05 00:06:00
// Author: Eric Yonng
// Description: 所有Global的接口只能间接的被系统调用, 避免做出闭包注册到系统, 以免在热更的时候, 如果暂时卸载不掉, 会造成Global注册到系统的被触发, 造成重复的调用（新的Global也被创建出来, 也注册了回调）
// 插件集中的所有接口的注册回调只能注册给PluginGlobal, 以便统一释放, 避免注册到程序集的其他对象, 造成卸载后遗漏反注册

#pragma once

#include <Comps/Plugin/Interface/IPluginGlobal.h>

SERVICE_BEGIN

class PluginGlobal : public IPluginGlobal
{
    POOL_CREATE_OBJ_DEFAULT_P1(IPluginGlobal, PluginGlobal);
    
public:
    PluginGlobal();
    ~PluginGlobal() override;

    void Release() override;
    void OnRegisterComps() override;

    KERNEL_NS::EventManager *GetEventManager() override;
    virtual KERNEL_NS::TimerMgr *GetTimerMgr() override;
    virtual KERNEL_NS::LibTimer *AddTimer() override;

    virtual void TestHello(const KERNEL_NS::LibString &content) const override;
    // 设置模块id
    virtual void SetPluginModuleId(UInt64 moduleId) override;
    // 获取模块id
    virtual UInt64 GetPluginModuleId() const override;
    virtual void OnTick() override;

    virtual KERNEL_NS::LibString ToString() const override;
private:
    virtual Int32 _OnHostInit() override;
    virtual Int32 _OnCompsCreated() override;
    virtual Int32 _OnHostWillStart() override;

    virtual Int32 _OnHostStart() override;
    virtual void _OnHostBeforeCompsWillClose() override;
    virtual void _OnHostClose() override;
    void _Clear();

private:
    KERNEL_NS::EventManager *_eventManager;
    KERNEL_NS::TimerMgr *_timerMgr;
    UInt64 _pluginModuleId;
};

SERVICE_END