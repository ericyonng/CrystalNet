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
// Date: 2026-06-05 00:06:46
// Author: Eric Yonng
// Description: 用于插件集的全局对象, 由插件集在Init的时候初始化, Close的时候销毁


#pragma once

#include <kernel/comp/CompObject/CompHostObject.h>
#include <service/common/macro.h>

KERNEL_BEGIN

class EventManager;
class TimerMgr;
class LibTimer;

KERNEL_END

SERVICE_BEGIN

class IPluginGlobal : public KERNEL_NS::CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, IPluginGlobal);

public:
    IPluginGlobal(UInt64 objTypeId) : CompHostObject(objTypeId) {}

    // 插件集事件管理器
    virtual KERNEL_NS::EventManager *GetEventManager() = 0;
    // 插件集事件管理器
    virtual KERNEL_NS::TimerMgr *GetTimerMgr() = 0;
    // 插件集的定时器只能通过global来管理,避免泄露对象
    virtual KERNEL_NS::LibTimer *AddTimer() = 0;
    // 测试
    virtual void TestHello(const KERNEL_NS::LibString &content) const = 0;
    // 设置模块id
    virtual void SetPluginModuleId(UInt64 moduleId) = 0;
    // 获取模块id
    virtual UInt64 GetPluginModuleId() const = 0;
    // tick
    virtual void OnTick() = 0;
};

SERVICE_END