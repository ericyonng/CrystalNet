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
// Date: 2025-01-23 00:01:27
// Author: Eric Yonng
// Description:

#ifndef __CRYSTAL_NET_TEST_SERVICE_PLUGIN_TEST_SERVICE_PLUGIN_PLUGIN_ENTRY_H__
#define __CRYSTAL_NET_TEST_SERVICE_PLUGIN_TEST_SERVICE_PLUGIN_PLUGIN_ENTRY_H__

#pragma once

#include <TestServicePlugin/test_plugin_export.h>

class IPluginMgr;

extern "C"
{
    // 入口方法 比较重的任务不允许放在插件集启动, 应该放在程序集(因为如果比较久, 会卡住线程影响业务)
    typedef Int32 (*InitPluginPtr)(void *);
    // 插件集启动 比较重的任务不允许放在插件集启动, 应该放在程序集(因为如果比较久, 会卡住线程影响业务)
    typedef Int32 (*StartPluginPtr)(void *);
    // 启动完成 比较重的任务不允许放在插件集启动, 应该放在程序集(因为如果比较久, 会卡住线程影响业务)
    typedef void (*StartPluginCompletePtr)(void *);
    // 插件集即将关闭 比较重的任务不允许放在插件集结束, 应该放在程序集(因为如果比较久, 会卡住线程影响业务)
    typedef void (*WillClosePluginPtr)(void *);
    // 插件集关闭 比较重的任务不允许放在插件集结束, 应该放在程序集(因为如果比较久, 会卡住线程影响业务)
    typedef void (*ClosePluginPtr)(void *);
    // 设置插件集对象
    typedef void (*SetPluginMgrPtr)(void *);
    // 获取插件集的模块id
    typedef UInt64 (*GetPluginModuleIdPtr)();
    // 获取插件集的模块id
    typedef UInt64 (*UpdateModuleIdPtr)();
    
    // 初始化插件集
    extern TEST_PLUGIN_EXPORT Int32 InitPlugin(void *pluginGlobal);

    // 启动插件集
    extern TEST_PLUGIN_EXPORT Int32 StartPlugin(void *pluginGlobal);

    // 插件集启动完成
    extern TEST_PLUGIN_EXPORT void StartPluginComplete(void *pluginGlobal);

    // 预关闭插件集
    extern TEST_PLUGIN_EXPORT void WillClosePlugin(void *pluginGlobal);

    // 释放插件集
    extern TEST_PLUGIN_EXPORT void ClosePlugin(void *pluginGlobal);

    // 设置对象
    extern TEST_PLUGIN_EXPORT void SetPluginMgr(void *pluginMgr);

    // 模块id
    extern TEST_PLUGIN_EXPORT UInt64 GetPluginModuleId();

    // 更新模块id
    extern TEST_PLUGIN_EXPORT UInt64 UpdateModuleId();
}

#endif

