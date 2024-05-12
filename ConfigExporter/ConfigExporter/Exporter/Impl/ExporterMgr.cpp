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
#include <ConfigExporter/Exporter/Impl/ExporterMgr.h>
#include <ConfigExporter/Exporter/Impl/ExporterMgrFactory.h>
#include <ConfigExporter/Exporter/Interface/IXlsxExporterMgr.h>

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

Int32 ExporterMgr::_OnInit() 
{
    return Status::Success;
}

Int32 ExporterMgr::_OnStart()
{
    _ExportConfigs();
    return Status::Success;
}

void ExporterMgr::_OnWillClose()
{
    _Clear();
}

void ExporterMgr::_ExportConfigs()
{
    // 设置传入的参数
    auto owner = GetOwner();
    const auto &args = owner->GetArgs();

    // 1.解析程序参数
    std::map<KERNEL_NS::LibString, KERNEL_NS::LibString> kv;
    KERNEL_NS::ParamsHandler::GetParams(args, [&kv](const KERNEL_NS::LibString &key, const KERNEL_NS::LibString &value)->bool{

        auto iter = kv.find(key);
        if(iter != kv.end())
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(ExporterMgr, "repeated kv:%s=>%s, old kv:%s=>%s"), key.c_str(), value.c_str(), iter->first.c_str(), iter->second.c_str());
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
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(ExporterMgr, "have no config set please select a config to export."));
            err = Status::ParamError;
            break;
        }

        if(iterConfig->second == "xlsx")
        {// 解析xlsx
            auto xlsxExporter = GetOwner()->CastTo<KERNEL_NS::CompHostObject>()->GetComp<IXlsxExporterMgr>();
            err = xlsxExporter->ExportConfigs(kv);
            if(err != Status::Success)
            {
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(ExporterMgr, "xlsx export config fail err:%d"), err);
                break;
            }
        }
        else
        {
            err = Status::ParamError;
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(ExporterMgr, "cant surport current config:%s export!!!"), iterConfig->second.c_str());
            break;
        }

        g_Log->Custom("config:%s, export success.", iterConfig->second.c_str());
    }while(false);
}

void ExporterMgr::_Clear()
{
}
