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
 * Date: 2023-03-26 22:07:26
 * Author: Eric Yonng
 * Description: 
*/
#include <pch.h>
#include <service_common/config/IConfigMgr.h>
#include <service_common/config/IConfigLoder.h>

SERVICE_COMMON_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IConfigLoader);

IConfigLoader::IConfigLoader()
{

}

IConfigLoader::~IConfigLoader()
{
}

KERNEL_NS::LibString IConfigLoader::ToString() const
{
    auto &allComps = GetAllComps();
    KERNEL_NS::LibString compsInfo;
    compsInfo.AppendFormat("[ ALL CONFIG MGRS ]\n");
    for(auto &comp : allComps)
        compsInfo.AppendFormat("%s;\n", comp->ToString().c_str());

    compsInfo.AppendFormat("[ CONFIG MGRS END ]");
    return compsInfo;
}

Int32 IConfigLoader::Load()
{
    auto &allComps = GetAllComps();
    for(auto &comp : allComps)
    {
        auto err = comp->CastTo<IConfigMgr>()->Load();
        if(err != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("config mgr:%s, load config fail err:%d"), comp->ToString().c_str(), err);
            return err;
        }
        
        g_Log->Info(LOGFMT_OBJ_TAG("Load %s config success."), comp->GetObjName().c_str());
    }

    g_Log->Info(LOGFMT_OBJ_TAG("config loader loaded configs success config mgr number:%llu."), static_cast<UInt64>(GetAllComps().size()));
    return Status::Success;
}

Int32 IConfigLoader::_OnCompsCreated()
{
    return Load();
}

Int32 IConfigLoader::Reload(std::vector<const IConfigMgr *> &changes)
{
    auto &allComps = GetAllComps();
    for(auto &comp : allComps)
    {
        auto configMgr = comp->CastTo<IConfigMgr>();
        bool isChanged = false;
        const auto configString = configMgr->ToString();
        auto err = _ReloadConfigMgr(configMgr, isChanged);
        if(err != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("reload fail configMgr:%s, err:%d"), configString.c_str(), err);
            return err;
        }

        if(isChanged)
            changes.push_back(configMgr);
    }

    g_Log->Info(LOGFMT_OBJ_TAG("config loader reload configs success config mgr number:%llu, changes number:%llu.")
                , static_cast<UInt64>(GetAllComps().size()), static_cast<UInt64>(changes.size()));
    return Status::Success;
}

Int32 IConfigLoader::_ReloadConfigMgr(IConfigMgr *configMgr, bool &isChanged)
{
    isChanged = false;
    const auto oldMd5s = configMgr->GetConfigDataMd5();
    auto err = configMgr->Reload();
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("config mgr:%s reload fail err:%d"), configMgr->ToString().c_str(), err);
        return err;
    }

    const auto newMd5s = configMgr->GetConfigDataMd5();
    if(oldMd5s != newMd5s)
        isChanged = true;

    return Status::Success;
}



SERVICE_COMMON_END