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
    Int32 InitPlugin()
    {
        auto pluginModuleId = GetPluginModuleId();
        PluginWrap pluginWrap;
        UNUSED(pluginWrap);
        UNUSED(pluginWrap._id);
        pluginWrap._id = 100;

        CLOG_INFO_GLOBAL(PluginWrap, "Plugin init success pluginWrap:%d pluginModuleId:%llu asGlobal:%p.", pluginWrap._id, pluginModuleId, g_PluginGlobal);
        return Status::Success;
    }

    // 启动插件集
    Int32 StartPlugin()
    {
        CLOG_INFO_GLOBAL(PluginWrap, "Plugin start success.");
        g_PluginGlobal->TestHello("StartPlugin");
        
        return Status::Success;
    }

    void StartPluginComplete()
    {
        PluginLogic::OnPluginStartup();

        auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::TEST_PLUGIN_EVENT);
        g_PluginGlobal->GetEventManager()->FireEvent(ev);
        
        CLOG_INFO_GLOBAL(PluginWrap, "plugin start completed");
    }

    // 预关闭插件集
    void WillClosePlugin()
    {
        CLOG_INFO_GLOBAL(PluginWrap, "Plugin will close pluginGlobal:%p...", g_PluginGlobal);
    }

    // 释放插件集
    void ClosePlugin()
    {
        auto moduleId = GetPluginModuleId();
        // plugingglobal 在程序集中关闭和释放
        CLOG_INFO_GLOBAL(PluginWrap, "Plugin close success module id:%llu pluginGlobal:%p.", moduleId, g_PluginGlobal);
    }

    void SetPluginMgr(void *pluginMgr)
    {
        g_PluginMgr = reinterpret_cast<SERVICE_NS::IPluginMgr *>(pluginMgr);
    }

    void SetPluginGlobal(void *pluginGlobal)
    {
        auto oldGlobal = g_PluginGlobal;
        g_PluginGlobal = reinterpret_cast<SERVICE_NS::IPluginGlobal *>(pluginGlobal);
        CLOG_INFO_GLOBAL(PluginWrap, "update plugin global %p => %p, input:%p", oldGlobal, g_PluginGlobal, pluginGlobal);
    }

    UInt64 GetPluginModuleId()
    {
        auto id = KERNEL_NS::GetCrystalModuleId();
// #if _DEBUG
//         if(g_Log)
//         {
//             CLOG_DEBUG_GLOBAL(KERNEL_NS::SystemUtil, "TestPlugin - GetPluginModuleId:%llu", id);
//         }
// #endif
        return id;
    }
}
