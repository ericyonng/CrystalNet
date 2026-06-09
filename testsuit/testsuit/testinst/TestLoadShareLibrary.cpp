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
// Date: 2025-01-23 00:01:37
// Author: Eric Yonng
// Description:

#include <pch.h>
#include <testsuit/testinst/TestLoadShareLibrary.h>

#include "kernel/comp/ShareLibraryLoader/ShareLibraryLoaderFactory.h"
#include <TestServicePlugin/TestServicePlugin.h>
#include <iostream>
#include <OptionComp/Command/Command.h>

// 监控dll/so热更
class HotfixMonitor 
{
public:
    static void DoHotfix(const KERNEL_NS::LibString &path)
    {
        KERNEL_NS::SmartPtr<KERNEL_NS::ShareLibraryLoader, KERNEL_NS::AutoDelMethods::Release> loader = KERNEL_NS::ShareLibraryLoaderFactory().Create()->CastTo<KERNEL_NS::ShareLibraryLoader>();
        loader->SetLibraryPath(path);
        loader->Init();
        loader->Start();

        auto initPtr = loader->LoadSym<InitPluginPtr>(KERNEL_NS::LibString("InitPlugin"));
        auto startPtr = loader->LoadSym<StartPluginPtr>(KERNEL_NS::LibString("StartPlugin"));
        auto willClosePtr = loader->LoadSym<WillClosePluginPtr>(KERNEL_NS::LibString("WillClosePlugin"));
        auto closePtr = loader->LoadSym<ClosePluginPtr>(KERNEL_NS::LibString("ClosePlugin"));
        if((!initPtr) || (!startPtr) || (!willClosePtr) || (!closePtr))
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestLoadShareLibrary, "load sym fail"));
            return;
        }
    
        auto pluginRet = (*initPtr)();
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLoadShareLibrary, "init plugin:%d"), pluginRet);
        pluginRet = (*startPtr)();
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLoadShareLibrary, "start plugin:%d"), pluginRet);
        (*willClosePtr)();
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLoadShareLibrary, "will close plugin..."));
        (*closePtr)();
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLoadShareLibrary, "close plugin"));
    
        loader->WillClose();
        loader->Close();
    }
};

void TestLoadShareLibrary::Run()
{
    auto libraryPath = KERNEL_NS::SystemUtil::GetCurProgRootPath();
    
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    libraryPath.AppendFormat("/libTestServicePlugin_debug.dll");

#else

    libraryPath.AppendFormat("/libTestServicePlugin_debug.so");

#endif

    auto poller = KERNEL_NS::TlsUtil::GetPoller();
    KERNEL_NS::SmartPtr<KERNEL_NS::ICommandMgr, KERNEL_NS::AutoDelMethods::Release> commandMgr = KERNEL_NS::CommandMgrFactory().Create()->CastTo<KERNEL_NS::ICommandMgr>();

    commandMgr->AddCommand("fix", [libraryPath]()
    {
        HotfixMonitor::DoHotfix(libraryPath);
    });
    commandMgr->AddCommand("quit", [libraryPath, poller]()
    {
        poller->QuitLoop();
    });

    commandMgr->Init();
    commandMgr->Start();
    
    poller->PrepareLoop();
    CLOG_DEBUG_GLOBAL(TestLoadShareLibrary, "Start eventloop");
    poller->EventLoop();
    CLOG_DEBUG_GLOBAL(TestLoadShareLibrary, "finish eventloop");
    poller->OnLoopEnd();

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLoadShareLibrary, "will quit test:%s"), libraryPath.c_str());

    commandMgr->WillClose();
    commandMgr->Close();
    
    CLOG_DEBUG_GLOBAL(TestLoadShareLibrary, "finishe test share library");
}
