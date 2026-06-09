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
#include <kernel/comp/Event/event_inc.h>
#include <service/common/BaseComps/SessionMgrComp/SessionMgr.h>
#include <TestServicePlugin/PluginEntry.h>
#include <kernel/comp/Delegate/LibDelegate.h>
#include <Comps/Plugin/Plugin.h>

#include "ExternPluginMgr.h"


void PluginLogic::OnPluginStartup()
{
    CLOG_INFO_GLOBAL(PluginLogic, "OnPluginStartup");

    // 监听事件
    g_PluginGlobal->GetEventManager()->AddListener(EventEnums::TEST_PLUGIN_EVENT, &PluginLogic::OnPluginTestEvent);

    // tick
    auto timer = g_PluginGlobal->AddTimer();
    timer->SetTimeOutHandler(&PluginLogic::OnPluginTestTimer);
    timer->Schedule(KERNEL_NS::TimeSlice::FromSeconds(5));

    KERNEL_NS::PostCaller([]()->KERNEL_NS::CoTask<>
    {
        KERNEL_NS::SmartPtr<KERNEL_NS::TaskParamRefWrapper, KERNEL_NS::AutoDelMethods::Release> param = KERNEL_NS::TaskParamRefWrapper::NewThreadLocal_TaskParamRefWrapper();
        auto randNum = co_await GetRandInt().GetParam(param);
        auto moduleId = KERNEL_NS::GetCrystalModuleId();
        auto &set = KERNEL_NS::GetCoroutineThreadLocalSet(moduleId);
        auto setCount = static_cast<Int32>(set.size());

        CLOG_INFO_GLOBAL(PluginLogic, "test fix plugin wait coroutine completed coroutine count:%d", setCount);

        co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(1));
        
        CLOG_INFO_GLOBAL(PluginLogic, "startup moduleId:%llu, param moduleId:%llu get rand:%d, setCount:%d"
            , moduleId, param->_params->_moduleId, randNum, setCount);
    });
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
    CLOG_INFO_GLOBAL(PluginLogic, "OnPluginTestEvent2");
}

void PluginLogic::OnPluginTestEvent3(KERNEL_NS::LibEvent *ev)
{
    CLOG_INFO_GLOBAL(PluginLogic, "OnPluginTestEvent3");
}

KERNEL_NS::CoTask<Int32> PluginLogic::GetRandInt()
{
    co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(1));

    KERNEL_NS::LibRandom<Int32, KERNEL_NS::_Build::TL> rand;
    rand.ResetSeed();
    auto randNum = rand.Gen();
    CLOG_INFO_GLOBAL(PluginLogic, "get rand num:%d", randNum);
    co_return randNum;
}

void PluginLogic::OnPluginTestTimer(KERNEL_NS::LibTimer *t)
{
    KERNEL_NS::LibString info;
    info.AppendFormat("=> plugin test timer hello world service:%s", g_PluginMgr->GetService()->ToString().c_str());
    g_PluginGlobal->TestHello(info);

    if(!IsTestAddTas())
    {
        IsTestAddTas() = true;

        KERNEL_NS::PostCaller([]()->KERNEL_NS::CoTask<>
        {
            co_await PluginLogic::TestAddTask();
        });
    }
}

bool &PluginLogic::IsTestAddTas()
{
    static DEF_THREAD_LOCAL_DECLEAR bool isTestAddTas = false;

    return isTestAddTas;
}

KERNEL_NS::CoTask<> PluginLogic::TestAddTask()
{
    // 当前连接数
    KERNEL_NS::PostCaller([]()->KERNEL_NS::CoTask<>
    {
        co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(120));

        CLOG_INFO_GLOBAL(PluginLogic, "plugin module:%llu, test session new test count", GetPluginModuleId());
        
        auto service = g_PluginMgr->GetService();
        auto sessionMgr = service->GetComp<SERVICE_NS::ISessionMgr>();
        if(sessionMgr)
        {
            auto count = sessionMgr->GetSessionAmount();
            CLOG_INFO_GLOBAL(PluginLogic, "service session count:%llu", count);
        }
        else
        {
            CLOG_WARN_GLOBAL(PluginLogic, "session mgr not found");
        }
    });

    co_return;
}







