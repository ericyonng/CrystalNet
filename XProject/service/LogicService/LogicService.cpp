/*!
 *  MIT License
 *  
 *  Copyright (c) 2020 ericyonng<120453674@qq.com>
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 * 
 * Date: 2026-09-10 00:08:55
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <service/LogicService/LogicService.h>

#include "LogicServiceFactory.h"
#include "Comps/config/impl/ConfigLoaderProxy.h"
#include "Comps/config/impl/ConfigLoaderProxyFactory.h"
#include "kernel/comp/Coroutines/CoDelay.h"
#include "kernel/comp/Coroutines/Runner.h"
#include "kernel/comp/Event/EventManager.h"
#include "kernel/comp/Event/LibEvent.h"
#include "kernel/comp/NetEngine/Poller/impl/IpRule/IpRuleMgr.h"
#include "OptionComp/storage/MongoDB/MongoDBComp.h"
#include "OptionComp/WinToast/Impl/WinToastMgrFactory.h"
#include "OptionComp/WinToast/Interface/IWinToastMgr.h"
#include "service/common/BaseComps/SessionMgrComp/Impl/SessionMgrFactory.h"
#include "service/common/BaseComps/StubHandle/Impl/StubHandleMgrFactory.h"
#include "service/common/BaseComps/SysLogic/Impl/SysLogicMgrFactory.h"

SERVICE_BEGIN

LogicService::LogicService()
:UnifiedService(KERNEL_NS::RttiUtil::GetTypeId<LogicService>())
,_storageOptions(KERNEL_NS::FileMonitor<StorageOptions, KERNEL_NS::YamlDeserializer>::New_FileMonitor())
{
}

LogicService::~LogicService()
{
    _OnUnifiedServiceClear();
}

void LogicService::_OnUnifiedServiceClear() 
{
    if (_storageOptions)
    {
        KERNEL_NS::FileMonitor<StorageOptions, KERNEL_NS::YamlDeserializer>::Delete_FileMonitor(_storageOptions);
        _storageOptions = NULL;
    }
}

void LogicService::_OnServiceRegisterComps()
{
#ifdef CRYSTAL_STORAGE_ENABLE
    // mongodb
    RegisterComp<KERNEL_NS::MongodbProxyFactory>();
#endif
    
    // 事件转发器 从Service 转发到其他事件管理器
    // RegisterComp<EventRelayGlobalFactory>();
    // 配置表
    RegisterComp<ConfigLoaderProxyFactory>();
    // 会话管理
    RegisterComp<SessionMgrFactory>();
    // 系统逻辑管理
    RegisterComp<SysLogicMgrFactory>();
    // 存根系统
    RegisterComp<StubHandleMgrFactory>();

    // 全球唯一id组件(需要有存储组件)
    // RegisterComp<GlobalUidMgrFactory>();

    // 跨时间组件(需要有GlobalUidMgr与存储组件)
    // RegisterComp<PassTimeGlobalFactory>();

    // 测试组件
    // RegisterComp<MyServiceCompFactory>();

    // 测试
    // RegisterComp<TestMgrFactory>();

    // 用户系统
    // RegisterComp<UserMgrFactory>();

    // 昵称系统
    // RegisterComp<NicknameGlobalFactory>();

    // 图书馆
    // RegisterComp<LibraryGlobalFactory>();

    // 邀请码
    // RegisterComp<InviteCodeGlobalFactory>();

    // 书袋
    // RegisterComp<BookBagGlobalFactory>();

    // 离线处理模块
    // RegisterComp<OfflineGlobalFactory>();

    // 通知模块
    // RegisterComp<NotifyGlobalFactory>();

    // 系统日志模块
    // RegisterComp<SystemLogGlobalFactory>();

    // 插件集
    // RegisterComp<PluginMgrFactory>();
    
    // RegisterComp<KERNEL_NS::WinToastMgrFactory>();
}

Int32 LogicService::_OnUnifiedServiceInit()
{
    auto &serviceName = GetServiceName();
    if (!_storageOptions->Init(GetApp()->GetSourceWrap(), KERNEL_NS::LibString().AppendFormat("%s.StorageOptions", serviceName.c_str())))
    {
        CLOG_ERROR("init storage option fail, service name:%s", serviceName.c_str());
        return Status::ConfigError;
    }
    
    return Status::Success;
}

Int32 LogicService::_OnUnifiedServiceCompsCreated()
{
    // db设置
#if CRYSTAL_STORAGE_ENABLE
    auto mongoProxy = GetComp<KERNEL_NS::IMongodbProxy>();
    // mongodb管理
    mongoProxy->SetMongodbMgr(GetApp()->GetComp<KERNEL_NS::IMongoDbMgr>());
    // 检查模块是否退出
    mongoProxy->SetCheckDependenceQuit([this](const KERNEL_NS::CompHostObject *host)->bool
    {
        return IsServiceModuleQuit(host);
    });
    // 设置Proxy退出
    mongoProxy->SetMongoProxyMaskQuit([this](const KERNEL_NS::CompHostObject *host)->void
    {
        MaskServiceModuleQuitFlag(host);
    });
    // 设置关注，退出的时候需要等待Proxy退出
    mongoProxy->SetRegisterFocus([this](const KERNEL_NS::CompHostObject *host) ->void
    {
        this->RegisterFocusServiceModule(host);
    });
    // 监听关闭事件
    mongoProxy->ListenClose(GetEventMgr(), EventEnums::QUIT_SERVICE_EVENT);
    
#endif
    
    // 配置路径设置
    auto configLoader = GetComp<SERVICE_NS::ConfigLoaderProxy>()->GetConfigLoader();
    auto curConfig = _serviceConfig->Current();
    configLoader->SetBasePath(curConfig->ConfigDataPath);

    // 设置ip rule mgr
    auto &config = GetApp()->GetKernelConfig();
    auto ipRuleMgr = GetComp<KERNEL_NS::IpRuleMgr>();
    auto flags = config.NetConfig.BlackWhiteListMode.ToFlags();
    if(!ipRuleMgr->SetBlackWhiteListFlag(flags))
    {
        CLOG_ERROR("SetBlackWhiteListFlag fail black white list flag:%u", flags);
        if(GetOwner())
            GetOwner()->SetErrCode(this, Status::Failed);
        return Status::Failed;
    }

    return Status::Success;
}

void LogicService::_OnEventLoopStart()
{
  KERNEL_NS::PostCaller([this]()->KERNEL_NS::CoTask<>
  {
      do
      {
          co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(1));
            
          auto service = reinterpret_cast<IService *>(this);
          auto sysLogicMgr = service->GetComp<ISysLogicMgr>();
          if(!sysLogicMgr->IsAllTaskFinish())
          {
              CLOG_WARN("%s not finish task.", sysLogicMgr->GetObjName().c_str());
              continue;
          }

          auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::SERVICE_WILL_STARTUP);
          GetEventMgr()->FireEvent(ev);

          ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::SERVICE_STARTUP);
          GetEventMgr()->FireEvent(ev);
          
          GetApp()->GetComp<KERNEL_NS::IWinToastMgr>()->Notify(KERNEL_NS::LibString().AppendFormat("%s started.", GetServiceName().c_str()));
          
          break;
      }
      while (true);

      CLOG_INFO("service start up, service:%s", ToString().c_str());
  });
}


void LogicService::Release()
{
    LogicService::DeleteByAdapter_LogicService(LogicServiceFactory::_buildType.V, this);
}

KERNEL_NS::SmartPtr<StorageOptions, KERNEL_NS::AutoDelMethods::Release> LogicService::GetStorageOption() const
{
    return _storageOptions->Current();
}


SERVICE_END
