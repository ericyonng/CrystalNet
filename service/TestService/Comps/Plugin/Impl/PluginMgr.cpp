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
#include <OptionComp/Command/Command.h>
#include <Comps/Plugin/Impl/PluginGlobalFactory.h>

#include "MyTestService.h"
#include "Comps/Plugin/Interface/IPluginGlobal.h"
#include <TestServicePlugin/TestServicePlugin.h>

SERVICE_BEGIN
    PluginMgr::PluginMgr()
: IPluginMgr(KERNEL_NS::RttiUtil::GetTypeId<PluginMgr>())
,_options(KERNEL_NS::FileMonitor<PluginOptions, KERNEL_NS::YamlDeserializer>::New_FileMonitor())
,_pluginGlobal(NULL)
,_delayRemovePluginTimer(NULL)
,_tick(NULL)
,_willQuitService(false)
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

IPluginGlobal *PluginMgr::GetCurPluginGlobal()
{
    return _pluginGlobal;
}

const IPluginGlobal *PluginMgr::GetCurPluginGlobal() const
{
    return _pluginGlobal;
}

Int32 PluginMgr::_OnGlobalSysCompsCreated()
{
    auto &&path = _GetPluginLibraryFinalPath();
    auto shareLibrary = GetComp<KERNEL_NS::ShareLibraryLoader>();
    shareLibrary->SetLibraryPath(path);

    GetEventMgr()->AddListener(0, this, &PluginMgr::_OnAnyEvent);
    
    return Status::Success;
}

Int32 PluginMgr::_OnHostStart()
{
    auto shareLibrary = GetComp<KERNEL_NS::ShareLibraryLoader>();
    _pluginGlobal = NULL;
    IPluginGlobal *newGlobal = NULL;
    auto ret = _InitPluginModule(shareLibrary, newGlobal);
    if(!ret)
    {
        CLOG_ERROR("plugin init fail");
        return Status::Failed;
    }

    _UpdatePluginGlobal(newGlobal);

    _CompletePlugin(shareLibrary);
    
    CLOG_INFO("plugin init module success service:%s.", GetService()->ToString().c_str());
    return Status::Success;
}

void PluginMgr::_OnHostBeforeCompsWillClose()
{
}

void PluginMgr::_OnHostBeforeCompsClose()
{
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

    _delayRemovePluginTimer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    _delayRemovePluginTimer->SetTimeOutHandler(this, &PluginMgr::_OnDelayRemovePluginTimer);

    // 帧
    _tick = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    _tick->SetTimeOutHandler(this, &PluginMgr::_OnTick);
    _tick->Schedule(KERNEL_NS::TimeSlice::FromMilliSeconds(20));
    
    // 注册监听热更(windows下生效, 其他平台命令行不生效)
    auto app = GetService()->GetApp();
    auto hotfixFilePath = _hotfixFilePath;
    auto hotfixKey = _hotfixKey;
    app->GetComp<KERNEL_NS::ICommandMgr>()->AddRegularCommand("fix.*", [hotfixFilePath, hotfixKey, serviceId, app](const KERNEL_NS::LibString &input) mutable
    {
        KERNEL_NS::LibString path = hotfixFilePath;
        auto &&content =  input.strip();
        auto parts = content.Split(' ');
        auto &&dirPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(path);
        auto &&fileNamePart = KERNEL_NS::DirectoryUtil::GetFileNameInPath(path);
        auto &&withoutExtension = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(fileNamePart);
        auto &&withoutExtensionMatch = withoutExtension + ".*";
        CLOG_INFO("regular command fix.* input:%s", input.c_str());
        
        // 匹配时间戳最大的那个
        if(content == "fix")
        {
            UInt64 maxTimestampSuffix = 0;
            KERNEL_NS::DirectoryUtil::TraverseDirRecursively(dirPath, [&withoutExtensionMatch, &maxTimestampSuffix, &fileNamePart](const KERNEL_NS::FindFileInfo &fileInfo, bool &isContinue)->bool
            {
                if(KERNEL_NS::FileUtil::IsDir(fileInfo))
                    return true;

                if(KERNEL_NS::FileUtil::IsFile(fileInfo))
                {
                    // .dll
                    if(fileInfo._extension != ".dll")
                        return true;
                    
                    if(KERNEL_NS::StringUtil::IsMatch(fileInfo._fileName, withoutExtensionMatch))
                    {
                        // 不带时间戳
                        if(fileInfo._fileName == fileNamePart)
                        {
                            return true;
                        }

                        auto &&removeDll = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(fileInfo._fileName);
                        auto &&afterRemove = KERNEL_NS::FileUtil::ExtractFileExtension(removeDll);
                        auto &&getTimestampPart = afterRemove.lsub(".");
                        getTimestampPart.strip();
                        if(getTimestampPart.isdigit())
                        {
                            auto timePart = KERNEL_NS::StringUtil::StringToUInt64(getTimestampPart.c_str());
                            if(maxTimestampSuffix == 0)
                            {
                                maxTimestampSuffix = timePart;
                            }
                            else if(maxTimestampSuffix < timePart)
                            {
                                maxTimestampSuffix = timePart;
                            }
                        }
                    }
                }

                return true;
            }, 1);

            // 没有时间戳后缀, 用原始文件名
            if(maxTimestampSuffix != 0)
            {
                path = dirPath + withoutExtension + KERNEL_NS::LibString().AppendFormat(".%llu.dll", maxTimestampSuffix);
            }
        }
        // 指定匹配某个文件
        else
        {
            if(parts.size() <= 1)
            {
                CLOG_WARN("input command err, input:%s", input.c_str());
                return;
            }
            auto &timePartStr = parts[1];
            if(!timePartStr.isdigit())
            {
                CLOG_WARN("input command err, input:%s", input.c_str());
                return;
            }

            path = dirPath + withoutExtension + KERNEL_NS::LibString().AppendFormat(".%s.dll", timePartStr.c_str());
        }
        
        // 1. 判断文件存在否
        if(!KERNEL_NS::FileUtil::IsFileExist(path.c_str()))
        {
            CLOG_WARN("file not exist %s when hotfix", path.c_str());
            return;
        }

        // windows下会对dll进行文件锁, 而且判断一个dll是不是同一个dll, 凭借是完整的文件路径, 如果路径一样, 只会在旧的已打开的dll上引用计数+1，多线程情况下dll可能引用计数还没归零, 还得等待其他线程卸载完
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

    if(_willQuitService)
    {
        CLOG_WARN("will quit service cant hotfix, service:%s", GetService()->ToString().c_str());
        return;
    }

    auto shareLoader = GetComp<KERNEL_NS::ShareLibraryLoader>();
    if(shareLoader && shareLoader->GetLibrary() == hotfixEv->_shareLib->GetLibrary())
    {
        CLOG_WARN("cant reload same library : %p", shareLoader->GetLibrary());
        return;
    }

    // 先初始化新的插件集, 有问题则停止热更
    IPluginGlobal *newPluginGlobal = NULL;
    auto ret = _InitPluginModule(hotfixEv->_shareLib.AsSelf(), newPluginGlobal);
    if(!ret)
    {
        CLOG_ERROR("plugin hotfix fail service:%s", GetService()->ToString().c_str());
        return;
    }

    CLOG_INFO("will hotfix old sharlib:%p => new share lib:%p, newPluginGlobal:%p", shareLoader, hotfixEv->_shareLib.AsSelf(), newPluginGlobal);

    if(UNLIKELY(!shareLoader))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no share library loader."));
    }
    else
    {
        // 弹出
        PopComp(shareLoader);
    }

    // 热更组件
    ReplaceComp(hotfixEv->_shareLib.AsSelf());

    auto newShareLib = GetComp<KERNEL_NS::ShareLibraryLoader>();
    CLOG_INFO("will hotfix after replace comp, old sharlib:%p => new share lib:%p, newShareLib:%p, newPluginGlobal:%p", shareLoader, hotfixEv->_shareLib.AsSelf(), newShareLib, newPluginGlobal);

    // 移除信息
    auto removeInfo = PluginDelayRemoveInfo::NewThreadLocal_PluginDelayRemoveInfo();
    removeInfo->_pluginGlobal = _pluginGlobal;
    removeInfo->_shareLibraryLoader = shareLoader;

    // 插件集协程是否都完成了
    auto oldPluginGlobal = removeInfo->_pluginGlobal;
    if(oldPluginGlobal)
    {
        _delayRemoveInfos.push_back(removeInfo);

        auto oldModuleId = oldPluginGlobal->GetPluginModuleId();
        auto &coroutineSet = KERNEL_NS::GetCoroutineThreadLocalSet(oldModuleId);
        if(coroutineSet.empty())
        {
            CLOG_INFO("plugin execute remove when coroutines are all completed service:%s module id:%llu, library name:%s"
                , GetService()->ToString().c_str(), oldModuleId, removeInfo->_shareLibraryLoader->GetLibraryPath().c_str());

            _delayRemoveInfos.pop_back();
            PluginDelayRemoveInfo::DeleteThreadLocal_PluginDelayRemoveInfo(removeInfo);
        }
        else
        {
            CLOG_WARN("plugin execute remove but plugin coroutine not completed, and will wait coroutine completed coroutine count:%d, oldModuleId:%llu service:%s", static_cast<Int32>(coroutineSet.size()), oldModuleId, GetService()->ToString().c_str());
            _StartDelayRemovePluginTimer();
        }
    }
    else
    {
        PluginDelayRemoveInfo::DeleteThreadLocal_PluginDelayRemoveInfo(removeInfo);
    }

    // 必要信息打印
    _UpdatePluginGlobal(newPluginGlobal);
    auto newShareLibrary = GetComp<KERNEL_NS::ShareLibraryLoader>();
    g_Log->Info(LOGFMT_OBJ_TAG("New ShareLibraryLoader:%s, module id:%llu, _pluginGlobal:%p"), newShareLibrary->ToString().c_str(), _pluginGlobal->GetPluginModuleId(), _pluginGlobal);

    hotfixEv->_shareLib.pop();
    
    // 热更完成
    _CompletePlugin(newShareLibrary);
}

void PluginMgr::_OnHotfixPlubinComplete(KERNEL_NS::PollerEvent *ev)
{
    auto completeEv = ev->CastTo<KERNEL_NS::HotfixShareLibraryCompleteEvent>();
    g_Log->Info(LOGFMT_OBJ_TAG("completeEv:%s"), completeEv->ToString().c_str());
}

bool PluginMgr::_InitPluginModule(KERNEL_NS::ShareLibraryLoader *shareLibrary, IPluginGlobal *&newPluginGlobal)
{
    try
    {
        auto initPtr = shareLibrary->LoadSym<InitPluginPtr>(KERNEL_NS::LibString("InitPlugin"));
        auto startPtr = shareLibrary->LoadSym<StartPluginPtr>(KERNEL_NS::LibString("StartPlugin"));
        auto startCompletePtr = shareLibrary->LoadSym<StartPluginCompletePtr>(KERNEL_NS::LibString("StartPluginComplete"));
        auto willClosePtr = shareLibrary->LoadSym<WillClosePluginPtr>(KERNEL_NS::LibString("WillClosePlugin"));
        auto closePluginPtr = shareLibrary->LoadSym<ClosePluginPtr>(KERNEL_NS::LibString("ClosePlugin"));
        auto setPluginMgrPtr = shareLibrary->LoadSym<SetPluginMgrPtr>(KERNEL_NS::LibString("SetPluginMgr"));
        auto getPluginModuleIdPtr = shareLibrary->LoadSym<GetPluginModuleIdPtr>(KERNEL_NS::LibString("GetPluginModuleId"));

        if((!initPtr) || (!startPtr) || (!startCompletePtr) || (!willClosePtr) || (!closePluginPtr) ||(!setPluginMgrPtr) || (!getPluginModuleIdPtr))
        {
            CLOG_ERROR("load sym fail");
            return false;
        }

        KERNEL_NS::SmartPtr<SERVICE_NS::IPluginGlobal, KERNEL_NS::AutoDelMethods::Release> pluginGlobal = PluginGlobalFactory().Create()->CastTo<IPluginGlobal>();
        // 设置模块id
        auto moduleId = (*getPluginModuleIdPtr)();
        pluginGlobal->SetPluginModuleId(moduleId);
    
        // 设置PluginMgrPtr
        (*setPluginMgrPtr)(this);
    
        auto pluginRet = (*initPtr)(pluginGlobal.AsSelf());
        if(pluginRet != Status::Success)
        {
            CLOG_ERROR("init fail st:%d", pluginRet);
            _WillClosePlugin(shareLibrary, pluginGlobal.AsSelf());
            _ClosePlugin(shareLibrary, pluginGlobal.AsSelf());
            return false;
        }
    
        pluginRet = (*startPtr)(pluginGlobal.AsSelf());
        if(pluginRet != Status::Success)
        {
            CLOG_ERROR("start fail st:%d", pluginRet);
            _WillClosePlugin(shareLibrary, pluginGlobal.AsSelf());
            _ClosePlugin(shareLibrary, pluginGlobal.AsSelf());
            return false;
        }

        newPluginGlobal = pluginGlobal.pop();
        CLOG_INFO("start plugin success service:%s moduleId:%llu library load at:%p newPluginGlobal:%p.", GetService()->ToString().c_str(), moduleId, shareLibrary->GetLibrary(), newPluginGlobal);

        return true;
    }
    catch (const std::exception &e)
    {
        CLOG_ERROR("plugin init err service:%s, exception:%s", GetService()->ToString().c_str(), e.what());
    }
    catch (...)
    {
        CLOG_ERROR("plugin init err service:%s, unknown exception", GetService()->ToString().c_str());
    }

    return false;
}

void PluginMgr::_WillClosePlugin(KERNEL_NS::ShareLibraryLoader *shareLibrary, IPluginGlobal *pluginGlobal)
{
    try
    {
        auto willClosePtr = shareLibrary->LoadSym<WillClosePluginPtr>(KERNEL_NS::LibString("WillClosePlugin"));
        if((!willClosePtr))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("load sym fail"));
            return;
        }
    
        (*willClosePtr)(pluginGlobal);
        g_Log->Info(LOGFMT_OBJ_TAG("will close plugin..."));
    }
    catch (const std::exception &e)
    {
        CLOG_ERROR("will close plugin fail exception:%s", e.what());
    }
    catch (...)
    {
        CLOG_ERROR("will close plugin unknown exception");
    }

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

void PluginMgr::_CompletePlugin(KERNEL_NS::ShareLibraryLoader *shareLibrary)
{
    // 热更完成
    auto startCompletePtr = shareLibrary->LoadSym<StartPluginCompletePtr>(KERNEL_NS::LibString("StartPluginComplete"));
    if(LIKELY(startCompletePtr))
    {
        try
        {
            (*startCompletePtr)(_pluginGlobal);
            CLOG_INFO("plugin hotfix complete service:%s", GetService()->ToString().c_str());
        }
        catch (const std::exception &e)
        {
            CLOG_ERROR("plugin start complete err service:%s, e:%s", GetService()->ToString().c_str(), e.what());
        }
        catch (...)
        {
            CLOG_ERROR("plugin start complete unknown err service:%s", GetService()->ToString().c_str());
        }
    }
}


void PluginMgr::_ClosePlugin(KERNEL_NS::ShareLibraryLoader *shareLibrary, IPluginGlobal *pluginGlobal)
{
    try
    {
        auto closePtr = shareLibrary->LoadSym<ClosePluginPtr>(KERNEL_NS::LibString("ClosePlugin"));
        if((!closePtr))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("load sym fail"));
            return;
        }
    
        (*closePtr)(pluginGlobal);
        g_Log->Info(LOGFMT_OBJ_TAG("close plugin pluginGlobal:%p"), pluginGlobal);
    }
    catch (const std::exception &e)
    {
        CLOG_ERROR("will close plugin fail exception:%s", e.what());
    }
    catch (...)
    {
        CLOG_ERROR("will close plugin unknown exception");
    }
}

void PluginMgr::_Clear()
{
    if(_options)
    {
        KERNEL_NS::FileMonitor<PluginOptions, KERNEL_NS::YamlDeserializer>::Delete_FileMonitor(_options);
        _options = NULL;
    }

    _UpdatePluginGlobal(NULL);
    
    if(_delayRemovePluginTimer)
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(_delayRemovePluginTimer);
    _delayRemovePluginTimer = NULL;

    if(_tick)
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(_tick);
    _tick = NULL;

    KERNEL_NS::ContainerUtil::DelContainer2(_delayRemoveInfos);
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

void PluginMgr::_OnAnyEvent(KERNEL_NS::LibEvent *ev)
{
    // 事件中转到插件集
    if(LIKELY(_pluginGlobal))
        _pluginGlobal->GetEventManager()->FireEvent(ev);
}

void PluginMgr::_OnDelayRemovePluginTimer(KERNEL_NS::LibTimer *t)
{
    auto curSharLibrary = GetComp<KERNEL_NS::ShareLibraryLoader>();

    if(_delayRemoveInfos.empty())
    {
        // 服务即将退出, 则标记卸载插件集完成
        if(_willQuitService)
        {
            CLOG_INFO("delay remove info empty and will quit service");
            
            _UpdatePluginGlobal(NULL);
            // 先移除组件
            if(curSharLibrary)
                PopComp(curSharLibrary);

            GetService()->MaskServiceModuleQuitFlag(this);
        }

        t->Cancel();
        return;
    }

    // TODO:验证协程全部完成后是否正确移除动态库, 以及多个service情况下是否正确卸载动态库(通过在动态库中的全局变量的释放来观察动态库卸载, 动态库的卸载会根据引用计数)
    const Int32 sz = static_cast<Int32>(_delayRemoveInfos.size());
    for(Int32 idx = (sz - 1); idx >= 0; --idx)
    {
        auto info = _delayRemoveInfos[idx];
        auto moduleId = info->_pluginGlobal->GetPluginModuleId();
        auto &tlsSet = KERNEL_NS::GetCoroutineThreadLocalSet(moduleId);
        if(!tlsSet.empty())
            continue;

        CLOG_INFO("plugin execute remove after coroutines are all completed in delay timer service:%s module id:%llu, library name:%s, library loaded at:%p, plugin global:%p, _shareLibraryLoader:%p, curSharLibrary:%p"
            , GetService()->ToString().c_str(), moduleId, info->_shareLibraryLoader->GetLibraryPath().c_str(), info->_shareLibraryLoader->GetLibrary(), info->_pluginGlobal, info->_shareLibraryLoader, curSharLibrary);

        // 卸载了当前的PluginGlobal
        if (_pluginGlobal == info->_pluginGlobal)
        {
            CLOG_INFO("delay removed pluginglobal:%p, _shareLibraryLoader:%p", _pluginGlobal, info->_shareLibraryLoader);
            _UpdatePluginGlobal(NULL);
        }
        
        // 当前的插件集卸载了
        if(curSharLibrary == info->_shareLibraryLoader)
        {
            CLOG_INFO("delay remove cur share library and will set null to plugin global shareLib:%p, ", info->_shareLibraryLoader);
            // 先移除组件
            _UpdatePluginGlobal(NULL);
            if(curSharLibrary)
                PopComp(curSharLibrary);

            curSharLibrary = NULL;
        }
        PluginDelayRemoveInfo::DeleteThreadLocal_PluginDelayRemoveInfo(info);
        _delayRemoveInfos.erase(_delayRemoveInfos.begin() + idx);
    }

    if(_delayRemoveInfos.empty())
    {
        // 服务即将退出, 则标记卸载插件集完成
        if(_willQuitService)
        {
            CLOG_INFO("delay remove info empty and will quit service and update plugin global to null");
            // 先移除组件
            _UpdatePluginGlobal(NULL);
            if(curSharLibrary)
                PopComp(curSharLibrary);

            GetService()->MaskServiceModuleQuitFlag(this);
        }
        
        t->Cancel();
    }
}

void PluginMgr::_StartDelayRemovePluginTimer()
{
    if(_delayRemoveInfos.empty())
        return;

    if(_delayRemovePluginTimer->IsScheduling())
        return;
    
    _delayRemovePluginTimer->Schedule(KERNEL_NS::TimeSlice::FromSeconds(5));
}

KERNEL_NS::LibString PluginMgr::_GetPluginLibraryFinalPath()
{
    _InitPath();
    
    KERNEL_NS::LibString path = _hotfixFilePath;

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    auto &&dirPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(_hotfixFilePath);
    auto &&fileNamePart = KERNEL_NS::DirectoryUtil::GetFileNameInPath(_hotfixFilePath);
    auto &&withoutExtension = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(fileNamePart);
    auto &&withoutExtensionMatch = withoutExtension + ".*";
    
    UInt64 maxTimestampSuffix = 0;
    KERNEL_NS::DirectoryUtil::TraverseDirRecursively(dirPath, [&withoutExtensionMatch, &withoutExtension, &maxTimestampSuffix, &fileNamePart](const KERNEL_NS::FindFileInfo &fileInfo, bool &isContinue)->bool
    {
        if(KERNEL_NS::FileUtil::IsDir(fileInfo))
            return true;

        if(KERNEL_NS::FileUtil::IsFile(fileInfo))
        {
            // .dll
            if(fileInfo._extension != ".dll")
                return true;
            
            if(KERNEL_NS::StringUtil::IsMatch(fileInfo._fileName, withoutExtensionMatch))
            {
                // 不带时间戳
                if(fileInfo._fileName == fileNamePart)
                {
                    return true;
                }

                auto &&removeDll = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(fileInfo._fileName);
                auto &&afterRemove = KERNEL_NS::FileUtil::ExtractFileExtension(removeDll);
                auto &&getTimestampPart = afterRemove.lsub(".");
                getTimestampPart.strip();
                if(getTimestampPart.isdigit())
                {
                    auto timePart = KERNEL_NS::StringUtil::StringToUInt64(getTimestampPart.c_str());
                    if(maxTimestampSuffix == 0)
                    {
                        maxTimestampSuffix = timePart;
                    }
                    else if(maxTimestampSuffix < timePart)
                    {
                        maxTimestampSuffix = timePart;
                    }
                }
            }
        }

        return true;
    }, 1);

    // 没有时间戳后缀, 用原始文件名
    if(maxTimestampSuffix != 0)
    {
        path = dirPath + withoutExtension + KERNEL_NS::LibString().AppendFormat(".%llu.dll", maxTimestampSuffix);
    }
#endif
    
    // 1. 判断文件存在否
    if(!KERNEL_NS::FileUtil::IsFileExist(path.c_str()))
    {
        CLOG_WARN("file not exist %s", path.c_str());
    }

    return path;
}

void PluginMgr::_OnTick(KERNEL_NS::LibTimer *t)
{
    if(_pluginGlobal)
        _pluginGlobal->OnTick();
}

void PluginMgr::_OnQuitServiceEventDefault(KERNEL_NS::LibEvent *ev)
{
    CLOG_INFO("plugin quit service");

    _willQuitService = true;
    auto shareLoader = GetComp<KERNEL_NS::ShareLibraryLoader>();
    if(UNLIKELY(!shareLoader))
    {
        CLOG_WARN("share library comp not found service:%s", GetService()->ToString().c_str());
        return;
    }
    
    // 移除信息
    CLOG_INFO("will quit service and will let plugin global and cur share library remove...");
    auto removeInfo = PluginDelayRemoveInfo::NewThreadLocal_PluginDelayRemoveInfo();
    removeInfo->_pluginGlobal = _pluginGlobal;
    removeInfo->_shareLibraryLoader = shareLoader;

    // 插件集协程是否都完成了
    auto oldPluginGlobal = removeInfo->_pluginGlobal;
    if(oldPluginGlobal)
    {
        _delayRemoveInfos.push_back(removeInfo);

        auto oldModuleId = oldPluginGlobal->GetPluginModuleId();
        auto &coroutineSet = KERNEL_NS::GetCoroutineThreadLocalSet(oldModuleId);
        if(coroutineSet.empty())
        {
            CLOG_INFO("plugin execute remove when coroutines are all completed service:%s module id:%llu, library name:%s quit service, sharelib:%p, load at:%p, plugin global:%p"
                , GetService()->ToString().c_str(), oldModuleId, removeInfo->_shareLibraryLoader->GetLibraryPath().c_str(), removeInfo->_shareLibraryLoader, removeInfo->_shareLibraryLoader->GetLibrary(), removeInfo->_pluginGlobal);

            _delayRemoveInfos.pop_back();

            // 先移除组件
            _UpdatePluginGlobal(NULL);
            PopComp(shareLoader);
            PluginDelayRemoveInfo::DeleteThreadLocal_PluginDelayRemoveInfo(removeInfo);

            GetService()->MaskServiceModuleQuitFlag(this);
        }
        else
        {
            CLOG_WARN("plugin execute remove but plugin coroutine not completed, and will wait coroutine completed coroutine count:%d, oldModuleId:%llu service:%s quit service sharelib:%p, load at:%p, plugin global:%p"
                , static_cast<Int32>(coroutineSet.size()), oldModuleId, GetService()->ToString().c_str(), removeInfo->_shareLibraryLoader, removeInfo->_shareLibraryLoader->GetLibrary(), removeInfo->_pluginGlobal);
            _StartDelayRemovePluginTimer();
        }
    }
    else
    {
        // 先移除组件
        _UpdatePluginGlobal(NULL);
        PopComp(shareLoader);
        
        CLOG_INFO("plugin global not exists...");
        PluginDelayRemoveInfo::DeleteThreadLocal_PluginDelayRemoveInfo(removeInfo);

        GetService()->MaskServiceModuleQuitFlag(this);
    }
}

void PluginMgr::_UpdatePluginGlobal(IPluginGlobal *pluginGlobal)
{
    auto oldGlobal = _pluginGlobal;
    _pluginGlobal = pluginGlobal;
    CLOG_DEBUG("_UpdatePluginGlobal %p => %p", oldGlobal, _pluginGlobal);
}


SERVICE_END
