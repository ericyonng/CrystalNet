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
#include <service/ConfigExporter/Comps/Exporter/Interface/IXlsxExporterMgr.h>

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
    timer->GetMgr()->TakeOverLifeTime(timer, [](KERNEL_NS::LibTimer *t){
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
    });
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
    auto app = GetApp();
    const auto &appArgs = app->GetAppArgs();
    
    // ConfigExporter --config=xlsx --lang=S:cpp|C:C#,lua --source_dir=/xxx/ --target_dir=/xxx/ --data=/xx/ --meta=/xxx/

    // 1.解析程序参数
    std::map<KERNEL_NS::LibString, KERNEL_NS::LibString> kv;
    SERVICE_COMMON_NS::ParamsHandler::GetParams(appArgs, [this, &kv](const KERNEL_NS::LibString &key, const KERNEL_NS::LibString &value)->bool{

        auto iter = kv.find(key);
        if(iter != kv.end())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("repeated kv:%s=>%s, old kv:%s=>%s"), key.c_str(), value.c_str(), iter->first.c_str(), iter->second.c_str());
            return false;
        }

        kv.insert(std::make_pair(key, value));
        return true;
    });

    // 2.dispatch
    Int32 err = Status::Success;
    do
    {
        // 需要--config参数
        auto iterConfig = kv.find("--config");
        if(iterConfig == kv.end())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("have no config set please select a config to export."));
            err = Status::ParamError;
            break;
        }

        if(iterConfig->second == "xlsx")
        {// 解析xlsx
            auto xlsxExporter = GetGlobalSys<IXlsxExporterMgr>();
            err = xlsxExporter->ExportConfigs(kv);
            if(err != Status::Success)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("xlsx export config fail err:%d"), err);
                break;
            }
        }
        else
        {
            err = Status::ParamError;
            g_Log->Warn(LOGFMT_OBJ_TAG("cant surport current config:%s export!!!"), iterConfig->second.c_str());
            break;
        }

        g_Log->Custom("config:%s, export success.", iterConfig->second.c_str());
    }while(false);

    // 4.关闭app
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