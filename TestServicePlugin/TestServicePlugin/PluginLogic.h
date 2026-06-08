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
// Date: 2026-06-06 00:06:29
// Author: Eric Yonng
// Description:

#pragma once

#include <service/common/macro.h>

KERNEL_BEGIN
    class LibEvent;
class LibTimer;
KERNEL_END

SERVICE_BEGIN
class IPluginGlobal;

SERVICE_END

#include <kernel/comp/Coroutines/CoTask.h>

class PluginLogic
{
public:
    // 比较重的任务不允许放在插件集启动, 应该放在程序集(因为如果比较久, 会卡住线程影响业务)
    static void OnPluginStartup(SERVICE_NS::IPluginGlobal *pluginGlobal);
    static void OnPluginTestEvent(KERNEL_NS::LibEvent *ev);
    static void OnPluginTestEvent2(KERNEL_NS::LibEvent *ev);
    static void OnPluginTestEvent3(KERNEL_NS::LibEvent *ev);

    static KERNEL_NS::CoTask<Int32> GetRandInt();

    static void OnPluginTestTimer(KERNEL_NS::LibTimer *t);

    static KERNEL_NS::CoTask<> TestAddTask();

    static bool &IsTestAddTas();
};
