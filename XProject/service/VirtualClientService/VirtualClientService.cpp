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
#include <service/VirtualClientService/VirtualClientService.h>
#include "VirtualClientServiceFactory.h"
#include "Comps/config/impl/ConfigLoaderProxy.h"
#include "Comps/config/impl/ConfigLoaderProxyFactory.h"
#include "Comps/Test/Impl/TestMgrFactory.h"
#include "kernel/comp/Coroutines/CoDelay.h"
#include "kernel/comp/Coroutines/Runner.h"
#include "kernel/comp/Event/EventManager.h"
#include "kernel/comp/Event/LibEvent.h"
#include "kernel/comp/NetEngine/Poller/impl/IpRule/IpRuleMgr.h"
#include "OptionComp/WinToast/Impl/WinToastMgrFactory.h"
#include "OptionComp/WinToast/Interface/IWinToastMgr.h"
#include "service/common/BaseComps/SessionMgrComp/Impl/SessionMgrFactory.h"
#include "service/common/BaseComps/StubHandle/Impl/StubHandleMgrFactory.h"
#include "service/common/BaseComps/SysLogic/Impl/SysLogicMgrFactory.h"

SERVICE_BEGIN

VirtualClientService::VirtualClientService()
:UnifiedService(KERNEL_NS::RttiUtil::GetTypeId<VirtualClientService>())
{
}

VirtualClientService::~VirtualClientService()
{
    _OnUnifiedServiceClear();
}

void VirtualClientService::_OnUnifiedServiceClear() 
{
}

void VirtualClientService::_OnServiceRegisterComps()
{
    // 配置表
    RegisterComp<ConfigLoaderProxyFactory>();
    // 会话管理
    RegisterComp<SessionMgrFactory>();
    // 系统逻辑管理
    RegisterComp<SysLogicMgrFactory>();
    // 存根系统
    RegisterComp<StubHandleMgrFactory>();
    
    // 测试
    RegisterComp<TestMgrFactory>();
}

Int32 VirtualClientService::_OnUnifiedServiceInit()
{
    return Status::Success;
}

Int32 VirtualClientService::_OnUnifiedServiceCompsCreated()
{
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

void VirtualClientService::_OnEventLoopStart()
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
          
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
          auto span = static_cast<double>((KERNEL_NS::LibTime::Now() - GetApp()->GetAppStartTime()).GetTotalMilliSeconds()) / 1000;
          GetApp()->GetComp<KERNEL_NS::IWinToastMgr>()->Notify(KERNEL_NS::LibString().AppendFormat("%s started cost time:(%lf)seconds.", GetServiceName().c_str(), span));
#endif
          break;
      }
      while (true);

      CLOG_INFO("service start up, service:%s", ToString().c_str());
  });
}


void VirtualClientService::Release()
{
    VirtualClientService::DeleteByAdapter_VirtualClientService(VirtualClientServiceFactory::_buildType.V, this);
}

SERVICE_END
