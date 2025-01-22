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

void TestLoadShareLibrary::Run()
{
    KERNEL_NS::SmartPtr<KERNEL_NS::ShareLibraryLoader, KERNEL_NS::AutoDelMethods::Release> loader = KERNEL_NS::ShareLibraryLoaderFactory().Create()->CastTo<KERNEL_NS::ShareLibraryLoader>();

    auto libraryPath = KERNEL_NS::SystemUtil::GetCurProgRootPath();
    libraryPath.AppendFormat("/libTestServicePlugin_debug.so");
    loader->SetLibraryPath(libraryPath);
    loader->Init();
    loader->Start();

    // Windows下
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    auto ret = InitPlugin();
    
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLoadShareLibrary, "init plugin ret:%d"), ret);

    ret = StartPlugin();

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLoadShareLibrary, "start plugin ret:%d"), ret);

    WillClosePlugin();

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLoadShareLibrary, "will close plugin..."));

    ClosePlugin();

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLoadShareLibrary, "close plugin."));

#else

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
#endif
    
    KERNEL_NS::SystemUtil::ThreadSleep(5000);

    loader->WillClose();
    loader->Close();

    KERNEL_NS::SystemUtil::ThreadSleep(5000);
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLoadShareLibrary, "loader finish"));
}
