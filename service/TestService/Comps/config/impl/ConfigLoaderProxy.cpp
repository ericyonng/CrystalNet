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
// Date: 2026-08-23 15:08:53
// Author: Eric Yonng
// Description:
#include <pch.h>
#include <Comps/config/impl/ConfigLoaderProxy.h>
#include <Comps/config/impl/ConfigLoaderFactory.h>
#include <Comps/config/impl/ConfigLoader.h>
#include <Comps/config/impl/ConfigLoaderProxyFactory.h>

#include "MyTestService.h"
#include "OptionComp/Command/Interface/ICommandMgr.h"

SERVICE_BEGIN

ConfigLoaderProxy::ConfigLoaderProxy()
:KERNEL_NS::CompHostObject(KERNEL_NS::RttiUtil::GetTypeId<ConfigLoaderProxy>())
{
    _configLoader = ConfigLoaderFactory().Create()->CastTo<ConfigLoader>();
}

ConfigLoaderProxy::~ConfigLoaderProxy()
{

}

void ConfigLoaderProxy::Release()
{
    ConfigLoaderProxy::DeleteByAdapter_ConfigLoaderProxy(ConfigLoaderProxyFactory::_buildType.V, this);
}

void ConfigLoaderProxy::OnRegisterComps()
{
    
}

KERNEL_NS::SmartPtr<ConfigLoader, KERNEL_NS::AutoDelMethods::Release> ConfigLoaderProxy::GetConfigLoader() const
{
    return _configLoader;
}

Int32 ConfigLoaderProxy::_OnHostInit()
{
    if(UNLIKELY(!_configLoader))
    {
        auto owner = GetOwner();
        g_Log->Error(LOGFMT_OBJ_TAG("create ConfigLoader fail, owner:%s"), owner ? owner->GetObjName().c_str():"");
        return Status::ConfigError;
    }

    auto st = _configLoader->Init();
    if(st!= Status::Success)
    {
        CLOG_ERROR("config loader init fail st:%d", st);
        return st;
    }

    auto service = GetOwner()->CastTo<MyTestService>();
    KERNEL_NS::LibString cmd;

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    cmd.AppendFormat("reload");
#else
    const auto pid = KERNEL_NS::SystemUtil::GetCurProcessId();
    cmd.AppendFormat("reload_%d\\.cmd.*", pid);
#endif
    
    // 配置重加载
    GetOwner()->CastTo<MyTestService>()->GetApp()->GetComp<KERNEL_NS::ICommandMgr>()->AddRegularCommand(cmd, [service](const KERNEL_NS::LibString &cmd)
    {
        CLOG_INFO_GLOBAL(ConfigLoaderProxy, "will start command cmd:%s", cmd.c_str());

        // 非windows删文件
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        // 删除文件
        auto &&projPath = KERNEL_NS::SystemUtil::GetCurProgRootPath();
        auto &&fullFilePath = projPath + "/" + cmd;
        KERNEL_NS::FileUtil::DelFileCStyle(fullFilePath.c_str());
#endif
        
        g_EventLoopHeavyTaskThreadPool->Send([service]()
        {
            auto curConfig = service->GetServiceConfig();
            KERNEL_NS::SmartPtr<SERVICE_NS::ConfigLoader, KERNEL_NS::AutoDelMethods::Release> newConfigLoader = ConfigLoaderFactory().Create()->CastTo<ConfigLoader>();
            newConfigLoader->SetBasePath(curConfig->ConfigDataPath);

            auto st = newConfigLoader->Init();
            if(st != Status::Success)
            {
                CLOG_ERROR_GLOBAL(ConfigLoaderProxy, "reload fail: init new config loader fail, st:%d", st);
                return;
            }

            st = newConfigLoader->Start();
            if(st != Status::Success)
            {
                CLOG_ERROR_GLOBAL(ConfigLoaderProxy, "reload fail: start new config loader fail, st:%d", st);
                return;
            }

            service->GetPoller()->Push([service, newConfigLoader]()
            {
                auto oldConfigLoader = service->GetComp<ConfigLoaderProxy>()->_configLoader;
                service->GetComp<ConfigLoaderProxy>()->_configLoader = newConfigLoader;

                CLOG_INFO_GLOBAL(ConfigLoaderProxy, "reload success old config loader:%p, new config loader:%p", oldConfigLoader.AsSelf(), newConfigLoader.AsSelf());
            });
        });
    });

    return Status::Success;
}

Int32 ConfigLoaderProxy::_OnHostStart()
{
    auto st = _configLoader->Start();
    if(st!= Status::Success)
    {
        CLOG_ERROR("config loader start fail st:%d", st);
        return st;
    }
    return Status::Success;
}

void ConfigLoaderProxy::_OnHostWillClose()
{

}

void ConfigLoaderProxy::_OnHostClose()
{
}

SERVICE_END
