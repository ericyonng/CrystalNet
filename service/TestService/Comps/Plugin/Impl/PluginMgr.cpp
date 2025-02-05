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
// Date: 2025-02-02 22:02:35
// Author: Eric Yonng
// Description:

#include <pch.h>
#include <kernel/kernel.h>
#include <service_common/ServiceCommon.h>
#include <service/common/common.h>
#include <service/TestService/Common/ServiceCommon.h>

#include <Comps/Plugin/Impl/PluginMgr.h>
#include <Comps/Plugin/Impl/PluginMgrFactory.h>

#include "kernel/comp/NetEngine/Poller/Defs/PollerInnerEvent.h"
#include <TestServicePlugin/TestServicePlugin.h>

SERVICE_BEGIN
    POOL_CREATE_OBJ_DEFAULT_IMPL(IPluginMgr);
POOL_CREATE_OBJ_DEFAULT_IMPL(PluginMgr);

PluginMgr::PluginMgr()
: IPluginMgr(KERNEL_NS::RttiUtil::GetTypeId<PluginMgr>())
{
    
}

PluginMgr::~PluginMgr()
{
    
}

void PluginMgr::Release()
{
    PluginMgr::DeleteByAdapter_PluginMgr(PluginMgrFactory::_buildType.V, this);
}

void PluginMgr::OnRegisterComps()
{
    RegisterComp<KERNEL_NS::ShareLibraryLoaderFactory>();
}

KERNEL_NS::LibString PluginMgr::ToString() const
{
    return KERNEL_NS::LibString().AppendFormat("plugin mgr, hotfix key:%s, %s", _hotfixKey.c_str(), KERNEL_NS::CompHostObject::ToString().c_str());
}

Int32 PluginMgr::_OnGlobalSysCompsCreated()
{
    auto dirPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(GetService()->GetApp()->GetAppPath());

#ifdef _DEBUG
    dirPath.AppendFormat("/libTestServicePlugin_debug.so");
#else
    dirPath.AppendFormat("/libTestServicePlugin.so");
#endif
    g_Log->Info(LOGFMT_OBJ_TAG("plugin so dirPath:%s"), dirPath.c_str());

#if CRYSTAL_TARGET_PLATFORM_LINUX
    auto shareLibrary = GetComp<KERNEL_NS::ShareLibraryLoader>();
    shareLibrary->SetLibraryPath(dirPath);
#endif

    return Status::Success;
}

Int32 PluginMgr::_OnHostStart()
{
    _InitPluginModule();
    return Status::Success;
}

void PluginMgr::_OnHostBeforeCompsWillClose()
{
    _WillClosePlugin();
}

void PluginMgr::_OnHostBeforeCompsClose()
{
   _ClosePlugin();
}

Int32 PluginMgr::_OnGlobalSysInit()
{
    auto ini = GetApp()->GetIni();
    KERNEL_NS::LibString hotfixKey;
    if(UNLIKELY(!ini->ReadStr(GetService()->GetServiceName().c_str(), "PluginHotfixKey", hotfixKey)))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("have no ConfigDataPath please check service:%s"), GetService()->GetServiceName().c_str());
        return Status::ConfigError;
    }

    _hotfixKey = hotfixKey;

    GetService()->GetPoller()->Subscribe(KERNEL_NS::PollerEventType::HotfixShareLibrary, this, &PluginMgr::_OnHotfixPlubin);
    GetService()->GetPoller()->Subscribe(KERNEL_NS::PollerEventType::HotfixShareLibraryComplete, this, &PluginMgr::_OnHotfixPlubinComplete);

    g_Log->Info(LOGFMT_OBJ_TAG("plugin mgr hotfix key:%s"), _hotfixKey.c_str());

    return Status::Success;
}

void PluginMgr::_OnGlobalSysClose()
{
    
}

void PluginMgr::_OnHotfixPlubin(KERNEL_NS::PollerEvent *ev)
{
    auto hotfixEv = ev->CastTo<KERNEL_NS::HotfixShareLibraryEvent>();
    if(!hotfixEv->_hotfixKey.Contain(_hotfixKey))
        return;

    auto shareLoader = GetComp<KERNEL_NS::ShareLibraryLoader>();
    if(UNLIKELY(!shareLoader))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no share library loader."));
        return;
    }

    try
    {
        g_Log->Info(LOGFMT_OBJ_TAG("close old plugin:%s"), shareLoader->ToString().c_str());
        _WillClosePlugin();
        _ClosePlugin();
        
        shareLoader->WillClose();
        shareLoader->Close();
    }
    catch (std::exception &e)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("hotfix remove old sharelibrary fail e:%s"), e.what());
    }
    catch (...)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("hotfix remove old sharelibrary fail unknown error"));
    }

    RemoveComp(shareLoader);

    // 热更组件
    ReplaceComp(hotfixEv->_shareLib.AsSelf());

    auto newComp = GetComp<KERNEL_NS::ShareLibraryLoader>();
    g_Log->Info(LOGFMT_OBJ_TAG("New ShareLibraryLoader:%s"), newComp->ToString().c_str());

    hotfixEv->_shareLib.pop();

    // 组件初始化
    _InitPluginModule();
}

void PluginMgr::_OnHotfixPlubinComplete(KERNEL_NS::PollerEvent *ev)
{
    auto completeEv = ev->CastTo<KERNEL_NS::HotfixShareLibraryCompleteEvent>();
    g_Log->Info(LOGFMT_OBJ_TAG("completeEv:%s"), completeEv->ToString().c_str());
}

void PluginMgr::_InitPluginModule()
{
    auto shareLibrary = GetComp<KERNEL_NS::ShareLibraryLoader>();

    // Windows下
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    SetPluginMgr(this);
    auto ret = InitPlugin();
    
    g_Log->Info(LOGFMT_OBJ_TAG("init plugin ret:%d"), ret);

    ret = StartPlugin();

    g_Log->Info(LOGFMT_OBJ_TAG("start plugin ret:%d"), ret);

#else
    auto initPtr = shareLibrary->LoadSym<InitPluginPtr>(KERNEL_NS::LibString("InitPlugin"));
    auto startPtr = shareLibrary->LoadSym<StartPluginPtr>(KERNEL_NS::LibString("StartPlugin"));
    auto setPluginMgrPtr = shareLibrary->LoadSym<SetPluginMgrPtr>(KERNEL_NS::LibString("SetPluginMgr"));

    if((!initPtr) || (!startPtr) ||(!setPluginMgrPtr))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("load sym fail"));
        return;
    }

    // 设置PluginMgrPtr
    (*setPluginMgrPtr)(this);
    auto pluginRet = (*initPtr)();
    g_Log->Info(LOGFMT_OBJ_TAG("init plugin:%d"), pluginRet);
    pluginRet = (*startPtr)();
    g_Log->Info(LOGFMT_OBJ_TAG("start plugin:%d"), pluginRet);
#endif

    g_Log->Info(LOGFMT_OBJ_TAG("init plugin module success."));
}

void PluginMgr::_WillClosePlugin()
{
    auto shareLibrary = GetComp<KERNEL_NS::ShareLibraryLoader>();

    // Windows下
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    WillClosePlugin();
    g_Log->Info(LOGFMT_OBJ_TAG("will close plugin..."));

#else
    auto willClosePtr = shareLibrary->LoadSym<WillClosePluginPtr>(KERNEL_NS::LibString("WillClosePlugin"));
    if((!willClosePtr))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("load sym fail"));
        return;
    }
    
    (*willClosePtr)();
    g_Log->Info(LOGFMT_OBJ_TAG("will close plugin..."));
#endif
}

void PluginMgr::_ClosePlugin()
{
    auto shareLibrary = GetComp<KERNEL_NS::ShareLibraryLoader>();

    // Windows下
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    ClosePlugin();
    g_Log->Info(LOGFMT_OBJ_TAG("close plugin."));

#else
    auto closePtr = shareLibrary->LoadSym<ClosePluginPtr>(KERNEL_NS::LibString("ClosePlugin"));
    if((!closePtr))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("load sym fail"));
        return;
    }
    
    (*closePtr)();
    g_Log->Info(LOGFMT_OBJ_TAG("close plugin"));
#endif
}

SERVICE_END
