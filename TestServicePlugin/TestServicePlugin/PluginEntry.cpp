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
// Date: 2025-01-23 00:01:58
// Author: Eric Yonng
// Description:

#include <pch.h>
#include <TestServicePlugin/PluginEntry.h>
#include <TestServicePlugin/ExternPluginMgr.h>
#include <TestServicePlugin/PluginLogic.h>
#include <kernel/comp/Event/event_inc.h>

#include <TestService/Common/ServiceCommon.h>

extern "C"
{
    struct PluginWrap
    {
        Int32 _id = 0;
    };
    
    // 初始化插件集
    Int32 InitPlugin(void *pluginGlobal)
    {
        auto asGlobal = KERNEL_NS::KernelCastTo<SERVICE_NS::IPluginGlobal>(pluginGlobal);
        auto pluginModuleId = GetPluginModuleId();
        auto st = asGlobal->Init();
        if(st != Status::Success)
        {
            CLOG_ERROR_GLOBAL(SERVICE_NS::IPluginGlobal, "plugin global init fail st:%d", st);
            return st;
        }
        
        PluginWrap pluginWrap;
        UNUSED(pluginWrap);
        UNUSED(pluginWrap._id);
        pluginWrap._id = 100;

        CLOG_INFO_GLOBAL(PluginWrap, "Plugin init success pluginWrap:%d pluginModuleId:%llu asGlobal:%p.", pluginWrap._id, pluginModuleId, asGlobal);
        return Status::Success;
    }

    // 启动插件集
    Int32 StartPlugin(void *pluginGlobal)
    {
        auto asGlobal = KERNEL_NS::KernelCastTo<SERVICE_NS::IPluginGlobal>(pluginGlobal);

        auto st = asGlobal->Start();
        if(st != Status::Success)
        {
            CLOG_ERROR_GLOBAL(SERVICE_NS::IPluginGlobal, "plugin global start fail st:%d", st);
            return st;
        }

        CLOG_INFO_GLOBAL(PluginWrap, "Plugin start success.");
        asGlobal->TestHello("StartPlugin");
        
        return Status::Success;
    }

    void StartPluginComplete(void *pluginGlobal)
    {
        auto asGlobal = KERNEL_NS::KernelCastTo<SERVICE_NS::IPluginGlobal>(pluginGlobal);

        PluginLogic::OnPluginStartup(asGlobal);

        auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::TEST_PLUGIN_EVENT);
        asGlobal->GetEventManager()->FireEvent(ev);
        
        CLOG_INFO_GLOBAL(PluginWrap, "plugin start completed");
    }

    // 预关闭插件集
    void WillClosePlugin(void *pluginGlobal)
    {
        CLOG_INFO_GLOBAL(PluginWrap, "Plugin will close pluginGlobal:%p...", pluginGlobal);
    }

    // 释放插件集
    void ClosePlugin(void *pluginGlobal)
    {
        auto moduleId = GetPluginModuleId();
        // plugingglobal 在程序集中关闭和释放
        CLOG_INFO_GLOBAL(PluginWrap, "Plugin close success module id:%llu pluginGlobal:%p.", moduleId, pluginGlobal);
    }

    void SetPluginMgr(void *pluginMgr)
    {
        g_PluginMgr = reinterpret_cast<SERVICE_NS::IPluginMgr *>(pluginMgr);
    }

    UInt64 GetPluginModuleId()
    {
        auto id = KERNEL_NS::GetCrystalModuleId();
#if _DEBUG
        if(g_Log)
        {
            CLOG_DEBUG_GLOBAL(KERNEL_NS::SystemUtil, "TestPlugin - GetPluginModuleId:%llu", id);
        }
#endif
        return id;
    }

    UInt64 UpdateModuleId()
    {
        KERNEL_NS::GetCrystalModuleId() = (KERNEL_NS::GetGlobalIdSrc().fetch_add(1, std::memory_order_release) + 1);

        auto newId = KERNEL_NS::GetCrystalModuleId();
        CLOG_INFO_GLOBAL(PluginWrap, "new module id:%llu", newId);
        return newId;
    }


}
