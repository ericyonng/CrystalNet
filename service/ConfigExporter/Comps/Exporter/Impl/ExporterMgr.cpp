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

    auto nextFrame = [this](KERNEL_NS::LibTimer *t)
    {

    };

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
    
    // ConfigExporter --source_path=xxx.xlsx --target_XX_path= --target_XX_path=

    // target字典
    std::unordered_map<KERNEL_NS::LibString, KERNEL_NS::LibString> targetTypeRefPath;
    KERNEL_NS::LibString sourcePath;

    // const Int32 argCount = static_cast<Int32>(appArgs.size());
    // for(Int32 idx = 0; idx < argCount; ++idx)
    // {
    //     const auto &arg = appArgs[idx];
    //     auto kv = arg.Split("=");
    //     if(kv.empty())
    //         continue;

    //     if(kv.size() < 2)
    //         continue;

    //     auto &k = kv[0];
    //     k.strip();
    //     auto &v = kv[0];
    //     if(k.empty())
    //         continue;

    //     v.strip();

    //     auto &raw = k.GetRaw();
    //     if(raw.find("--source_path") != std::string::npos)
    //         sourcePath = v;
    //     else if(raw.find("--target_") != std::string::npos)
    //     {
    //         raw.find("")
    //     }
    // }
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