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
    typedef Int32 (*InitPluginPtr)();
    // 插件集启动 比较重的任务不允许放在插件集启动, 应该放在程序集(因为如果比较久, 会卡住线程影响业务)
    typedef Int32 (*StartPluginPtr)();
    // 启动完成 比较重的任务不允许放在插件集启动, 应该放在程序集(因为如果比较久, 会卡住线程影响业务)
    typedef void (*StartPluginCompletePtr)();
    // 通知关闭, 此时插件集相关定时等, 需要终止
    typedef void (*NotifyQuitPluginPtr)();
    // 插件集即将关闭 比较重的任务不允许放在插件集结束, 应该放在程序集(因为如果比较久, 会卡住线程影响业务)
    typedef void (*WillClosePluginPtr)();
    // 插件集关闭 比较重的任务不允许放在插件集结束, 应该放在程序集(因为如果比较久, 会卡住线程影响业务)
    typedef void (*ClosePluginPtr)();
    // 设置插件集对象
    typedef void (*SetPluginMgrPtr)(void *);
    // 设置动态库使用的全局对象
    typedef void (*SetPluginGlobalPtr)(void *);
    // 获取插件集的模块id
    typedef UInt64 (*GetPluginModuleIdPtr)();
    
    // 初始化插件集
    extern TEST_PLUGIN_EXPORT Int32 InitPlugin();

    // 启动插件集
    extern TEST_PLUGIN_EXPORT Int32 StartPlugin();

    // 插件集启动完成
    extern TEST_PLUGIN_EXPORT void StartPluginComplete();

    // 通知即将关闭 => 预关闭 => 释放
    extern TEST_PLUGIN_EXPORT void NotifyQuitPlugin();
    // 预关闭插件集
    extern TEST_PLUGIN_EXPORT void WillClosePlugin();

    // 释放插件集
    extern TEST_PLUGIN_EXPORT void ClosePlugin();

    // 设置对象
    extern TEST_PLUGIN_EXPORT void SetPluginMgr(void *pluginMgr);

    // 设置对象
    extern TEST_PLUGIN_EXPORT void SetPluginGlobal(void *pluginGlobal);
    
    // 模块id
    extern TEST_PLUGIN_EXPORT UInt64 GetPluginModuleId();
}

#endif

