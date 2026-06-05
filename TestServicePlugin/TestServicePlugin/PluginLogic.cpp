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
// Date: 2026-06-06 00:06:09
// Author: Eric Yonng
// Description:

#include <pch.h>
#include <TestServicePlugin/PluginLogic.h>
#include <kernel/comp/Log/log.h>
#include <TestService/Common/ServiceCommon.h>
#include <TestServicePlugin/ExternPluginMgr.h>
#include <kernel/comp/Event/event_inc.h>


void PluginLogic::OnPluginStartup()
{
    CLOG_INFO_GLOBAL(PluginLogic, "OnPluginStartup");

    // 监听事件
    g_PluginGlobal->GetEventManager()->AddListener(EventEnums::TEST_PLUGIN_EVENT, &PluginLogic::OnPluginTestEvent);
}

void PluginLogic::OnPluginTestEvent(KERNEL_NS::LibEvent *ev)
{
    CLOG_INFO_GLOBAL(PluginLogic, "OnPluginTestEvent");

    g_PluginGlobal->GetEventManager()->AddListener(EventEnums::TEST_PLUGIN_EVENT2, &PluginLogic::OnPluginTestEvent2);
    auto ev2 = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::TEST_PLUGIN_EVENT2);
    g_PluginGlobal->GetEventManager()->FireEvent(ev2);
}

void PluginLogic::OnPluginTestEvent2(KERNEL_NS::LibEvent *ev)
{
    CLOG_INFO_GLOBAL(PluginLogic, "OnPluginTestEvent2");

    g_PluginGlobal->GetEventManager()->AddListener(EventEnums::TEST_PLUGIN_EVENT3, &PluginLogic::OnPluginTestEvent3);
    auto ev3 = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::TEST_PLUGIN_EVENT3);
    g_PluginGlobal->GetEventManager()->FireEvent(ev3);
}

void PluginLogic::OnPluginTestEvent3(KERNEL_NS::LibEvent *ev)
{
    CLOG_INFO_GLOBAL(PluginLogic, "OnPluginTestEvent3");
}



