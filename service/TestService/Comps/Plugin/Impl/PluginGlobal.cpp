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
// Date: 2026-06-05 00:06:42
// Author: Eric Yonng
// Description:


#include <pch.h>
#include <Comps/Plugin/Impl/PluginGlobal.h>
#include <kernel/common/status.h>

#include <Comps/Plugin/Impl/PluginGlobalFactory.h>
#include <kernel/comp/Event/event_inc.h>
#include <kernel/comp/Timer/TimerMgr.h>
#include <kernel/comp/Timer/LibTimer.h>

SERVICE_BEGIN

PluginGlobal::PluginGlobal()
    :IPluginGlobal(KERNEL_NS::RttiUtil::GetTypeId<PluginGlobal>())
    ,_eventManager(NULL)
    ,_timerMgr(KERNEL_NS::TimerMgr::NewThreadLocal_TimerMgr())
    ,_pluginModuleId(0)
{
    _timerMgr->Launch(NULL);
}

PluginGlobal::~PluginGlobal()
{
    _Clear();
}

void PluginGlobal::Release()
{
    PluginGlobal::DeleteByAdapter_PluginGlobal(PluginGlobalFactory::_buildType.V, this);
}

void PluginGlobal::OnRegisterComps()
{
    
}

KERNEL_NS::EventManager *PluginGlobal::GetEventManager()
{
    return _eventManager;
}

KERNEL_NS::TimerMgr *PluginGlobal::GetTimerMgr()
{
    return _timerMgr;
}

KERNEL_NS::LibTimer *PluginGlobal::AddTimer()
{
    auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer(_timerMgr);
    _timerMgr->TakeOverLifeTime(timer, [](KERNEL_NS::LibTimer *t)
    {
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
    });

    return timer;
}


void PluginGlobal::TestHello(const KERNEL_NS::LibString &content) const
{
    CLOG_INFO("PluginGlobal::TestHello content:%s, this:%p", content.c_str(), this);
}

// 设置模块id
void PluginGlobal::SetPluginModuleId(UInt64 moduleId)
{
    _pluginModuleId = moduleId;
}

// 获取模块id
UInt64 PluginGlobal::GetPluginModuleId() const
{
    return _pluginModuleId;
}

void PluginGlobal::OnTick()
{
    if(_timerMgr)
        _timerMgr->Drive();
}

KERNEL_NS::LibString PluginGlobal::ToString() const
{
    KERNEL_NS::LibString info;
    info.AppendFormat("PluginGlobal moduleId:%llu, %s", _pluginModuleId, KERNEL_NS::CompHostObject::ToString().c_str());
    return info;
}


Int32 PluginGlobal::_OnHostInit()
{
    _eventManager = KERNEL_NS::EventManager::NewThreadLocal_EventManager();
    return Status::Success;
}

Int32 PluginGlobal::_OnCompsCreated()
{
    return Status::Success;
}

Int32 PluginGlobal::_OnHostWillStart()
{
    return Status::Success;
}

Int32 PluginGlobal::_OnHostStart()
{
    return Status::Success;
}

void PluginGlobal::_OnHostBeforeCompsWillClose()
{
    
}
void PluginGlobal::_OnHostClose()
{
    CLOG_INFO("plugin global release completed.");
}

void PluginGlobal::_Clear()
{
    if(_eventManager)
    {
        KERNEL_NS::EventManager::DeleteThreadLocal_EventManager(_eventManager);
    }

    if(_timerMgr)
    {
        KERNEL_NS::TimerMgr::DeleteThreadLocal_TimerMgr(_timerMgr);
    }
    _eventManager = NULL;
    _timerMgr = NULL;
}

SERVICE_END