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
 * Date: 2023-02-19 22:12:07
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <service/ConfigExporter/Comps/Exporter/Impl/ExporterMgr.h>
#include <service/ConfigExporter/Comps/Exporter/Impl/ExporterMgrFactory.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IExporterMgr);

POOL_CREATE_OBJ_DEFAULT_IMPL(ExporterMgr);

ExporterMgr::ExporterMgr()
{

}

ExporterMgr::~ExporterMgr()
{
    _Clear();
}

void ExporterMgr::Release()
{
    ExporterMgr::DeleteByAdapter_ExporterMgr(ExporterMgrFactory::_buildType.V, this);
}

KERNEL_NS::LibString ExporterMgr::ToString() const
{
    return IExporterMgr::ToString();
}

Int32 ExporterMgr::_OnGlobalSysInit() 
{
    _RegisterEvents();

    auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    timer->SetTimeOutHandler(this, &ExporterMgr::_OnExporter);
    timer->Schedule(0);

    // 1.读取所有配置表数据
    // 2.读取表头
    // 3.生成配置

    return Status::Success;
}

void ExporterMgr::_OnGlobalSysClose()
{
    _Clear();
}

void ExporterMgr::_OnExporter(KERNEL_NS::LibTimer *t)
{
    bool genSuc = false;
    auto app = GetApp();
    const auto &appArgs = app->GetAppArgs();
    
    // ConfigExporter --lang=S:cpp|C:C#,lua --source_dir=/xxx/ --target_dir=/xxx/ --data=/xx/

    // target字典
    KERNEL_NS::LibString sourceDir;
    KERNEL_NS::LibString targetDir;
    KERNEL_NS::LibString dataDir;
    std::unordered_map<KERNEL_NS::LibString, std::unordered_set<KERNEL_NS::LibString>> configTypeRefLangTypes;

    // 1.传入的参数
    const Int32 argCount = static_cast<Int32>(appArgs.size());
    for(Int32 idx = 0; idx < argCount; ++idx)
    {
        const auto &arg = appArgs[idx];
        auto kv = arg.Split("=");
        if(kv.empty())
            continue;

        if(kv.size() < 2)
            continue;

        auto &k = kv[0];
        k.strip();
        KERNEL_NS::LibString &v = kv[1];
        if(k.empty())
            continue;

        v.strip();

        // 生成的语言版本
        if(k == "--lang")
        {
            auto configTypeRefLangsArr = v.Split("|");
            if(!configTypeRefLangsArr.empty())
            {
                for(auto &configTypeRefLangs : configTypeRefLangsArr)
                {
                    configTypeRefLangs.strip();
                    if(configTypeRefLangs.empty())
                        continue;
                    
                    auto configTypeLangPieces = configTypeRefLangs.Split(":");
                    if(configTypeLangPieces.empty())
                        continue;

                    if(configTypeLangPieces.size() < 2)
                        continue;

                    auto &configType = configTypeLangPieces[0];
                    auto &langsStr = configTypeLangPieces[1];
                    configType.strip();
                    langsStr.strip();
                    if(configType.empty() || langsStr.empty())
                        continue;

                    auto langsPieces = langsStr.Split(",");
                    if(langsPieces.empty())
                        continue;

                    auto iter = configTypeRefLangTypes.find(configType);
                    if(iter == configTypeRefLangTypes.end())
                        iter = configTypeRefLangTypes.insert(std::make_pair(configType, std::unordered_set<KERNEL_NS::LibString>())).first;
                    auto &langsSet = iter->second; 

                    for(auto &lang : langsPieces)
                        langsSet.insert(lang);    
                }
            }
        }
        else if(k == "--source_dir")
        {
            sourceDir = v;
        }
        else if(k == "target_dir")
        {
            targetDir = v;
        }
        else if(k == "--data")
        {
            dataDir = v;
        }
        else
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("unknown param:k:%s, v:%s"), k.c_str(), v.c_str());
        }
    }

    // 2.加载xlsx文件
    
    do
    {
        g_Log->Custom("[CONFIG GEN] START.");

        // 1.读取所有配置数据
        
        g_Log->Custom("[CONFIG GEN] END.");

    }while (false);

    // 4.关闭app
    Int32 err = genSuc ? Status::Success : Status::Failed;
    GetServiceProxy()->CloseApp(err);
    
    KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
}

void ExporterMgr::_Clear()
{
    _UnRegisterEvents();
}

void ExporterMgr::_RegisterEvents()
{

}

void ExporterMgr::_UnRegisterEvents()
{

}


SERVICE_END