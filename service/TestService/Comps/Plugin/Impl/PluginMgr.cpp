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
#include <OptionComp/Command/Command.h>

#include "MyTestService.h"

SERVICE_BEGIN
    PluginMgr::PluginMgr()
: IPluginMgr(KERNEL_NS::RttiUtil::GetTypeId<PluginMgr>())
,_options(KERNEL_NS::FileMonitor<PluginOptions, KERNEL_NS::YamlDeserializer>::New_FileMonitor())
{
    
}

PluginMgr::~PluginMgr()
{
    _Clear();
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
    _InitPath();
    auto shareLibrary = GetComp<KERNEL_NS::ShareLibraryLoader>();
    shareLibrary->SetLibraryPath(_hotfixFilePath);
    
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
    if(!_options->Init(GetApp()->GetSourceWrap(), KERNEL_NS::LibString().AppendFormat("%s.PluginOptions", GetService()->GetServiceName().c_str())))
    {
        CLOG_ERROR("plugin options init fail service:%s", GetService()->GetServiceName().c_str());
        return Status::ConfigError;
    }

    _InitPath();
    
    _hotfixKey = _options->Current()->PluginHotfixKey;

    auto serviceId = GetService()->GetServiceId();
    GetService()->GetPoller()->Subscribe(KERNEL_NS::PollerEventType::HotfixShareLibrary, this, &PluginMgr::_OnHotfixPlubin);
    GetService()->GetPoller()->Subscribe(KERNEL_NS::PollerEventType::HotfixShareLibraryComplete, this, &PluginMgr::_OnHotfixPlubinComplete);

    // 注册监听热更(windows下生效, 其他平台命令行不生效)
    auto app = GetService()->GetApp();
    auto path = _hotfixFilePath;
    auto hotfixKey = _hotfixKey;
    app->GetComp<KERNEL_NS::ICommandMgr>()->AddCommand("fix", [path, hotfixKey, serviceId, app]()
    {
        // 1. 判断文件存在否
        if(!KERNEL_NS::FileUtil::IsFileExist(path.c_str()))
        {
            CLOG_WARN("file not exist %s when hotfix", path.c_str());
            return;
        }

        KERNEL_NS::SmartPtr<KERNEL_NS::ShareLibraryLoader, KERNEL_NS::AutoDelMethods::Release> newShareLibrary = KERNEL_NS::ShareLibraryLoaderFactory().Create()->CastTo<KERNEL_NS::ShareLibraryLoader>();
        newShareLibrary->SetLibraryPath(path);
        auto ret = newShareLibrary->Init();
        if(ret != Status::Success)
        {
            CLOG_ERROR("plugin init new share library fail path:%s when hotfix ret:%d, serviceId:%llu"
                , path.c_str(), ret, serviceId);
            return;
        }

        ret = newShareLibrary->Start();
        if(ret != Status::Success)
        {
            CLOG_ERROR("plugin start new share library fail path:%s when hotfix ret:%d, serviceId:%llu", path.c_str(), ret, serviceId);
            return;
        }

        auto ev = KERNEL_NS::HotfixShareLibraryEvent::New_HotfixShareLibraryEvent();
        ev->_hotfixKey = hotfixKey;
        ev->_shareLib = std::move(newShareLibrary);
        app->GetComp<KERNEL_NS::IServiceProxy>()->PostMsg(serviceId, ev);
    });

    CLOG_INFO("plugin mgr hotfix key:%s", _hotfixKey.c_str());

    return Status::Success;
}

void PluginMgr::_OnGlobalSysClose()
{
    _Clear();
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
    
// #if CRYSTAL_TARGET_PLATFORM_WINDOWS
//     SetPluginMgr(this);
//     auto ret = InitPlugin();
//     
//     g_Log->Info(LOGFMT_OBJ_TAG("init plugin ret:%d"), ret);
//
//     ret = StartPlugin();
//
//     g_Log->Info(LOGFMT_OBJ_TAG("start plugin ret:%d"), ret);
//
// #else
//
// #endif

    g_Log->Info(LOGFMT_OBJ_TAG("init plugin module success."));
}

void PluginMgr::_WillClosePlugin()
{
    auto shareLibrary = GetComp<KERNEL_NS::ShareLibraryLoader>();

    auto willClosePtr = shareLibrary->LoadSym<WillClosePluginPtr>(KERNEL_NS::LibString("WillClosePlugin"));
    if((!willClosePtr))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("load sym fail"));
        return;
    }
    
    (*willClosePtr)();
    g_Log->Info(LOGFMT_OBJ_TAG("will close plugin..."));
//     
//     // Windows下
// #if CRYSTAL_TARGET_PLATFORM_WINDOWS
//     WillClosePlugin();
//     g_Log->Info(LOGFMT_OBJ_TAG("will close plugin..."));
//
// #else
//
// #endif
}

void PluginMgr::_ClosePlugin()
{
    auto shareLibrary = GetComp<KERNEL_NS::ShareLibraryLoader>();
    auto closePtr = shareLibrary->LoadSym<ClosePluginPtr>(KERNEL_NS::LibString("ClosePlugin"));
    if((!closePtr))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("load sym fail"));
        return;
    }
    
    (*closePtr)();
    g_Log->Info(LOGFMT_OBJ_TAG("close plugin"));
//     
//     // Windows下
// #if CRYSTAL_TARGET_PLATFORM_WINDOWS
//     ClosePlugin();
//     g_Log->Info(LOGFMT_OBJ_TAG("close plugin."));
//
// #else
//
// #endif
}

void PluginMgr::_Clear()
{
    if(_options)
    {
        KERNEL_NS::FileMonitor<PluginOptions, KERNEL_NS::YamlDeserializer>::Delete_FileMonitor(_options);
        _options = NULL;
    }
}

void PluginMgr::_InitPath()
{
    if(!_hotfixFilePath.empty())
        return;
    
    auto dirPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(GetService()->GetApp()->GetAppPath());

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
#ifdef _DEBUG
    dirPath.AppendFormat("/libTestServicePlugin_debug.dll");
#else
    dirPath.AppendFormat("/libTestServicePlugin.dll");
#endif
#else
#ifdef _DEBUG
    dirPath.AppendFormat("/libTestServicePlugin_debug.so");
#else
    dirPath.AppendFormat("/libTestServicePlugin.so");
#endif
#endif
    

    _hotfixFilePath = dirPath;
    g_Log->Info(LOGFMT_OBJ_TAG("plugin so dirPath:%s"), _hotfixFilePath.c_str());
}



SERVICE_END
