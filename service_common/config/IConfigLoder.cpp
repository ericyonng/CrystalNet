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
    }

    g_Log->Info(LOGFMT_OBJ_TAG("config loader loaded configs success config mgr number:%llu."), static_cast<UInt64>(GetAllComps().size()));
    return Status::Success;
}

Int32 IConfigLoader::Reload(std::vector<const IConfigMgr *> &changes)
{
    auto &allComps = GetAllComps();
    for(auto &comp : allComps)
    {
        auto configMgr = comp->CastTo<IConfigMgr>();
        const auto &oldMd5s = configMgr->GetAllConfigFileMd5();
        auto err = configMgr->Reload();
        if(err != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("config mgr:%s reload fail err:%d"), configMgr->ToString().c_str(), err);
            return err;
        }

        const auto &newMd5s = configMgr->GetAllConfigFileMd5();
        if(oldMd5s.size() != newMd5s.size())
        {
            changes.push_back(configMgr);
        }
        else
        {
            const Int32 md5Count = static_cast<Int32>(oldMd5s.size());
            for(Int32 idx = 0; idx < md5Count; ++idx)
            {
                auto &oldMd5 = oldMd5s[idx];
                auto &newMd5 = newMd5s[idx];
                if(oldMd5 != newMd5)
                {
                    changes.push_back(configMgr);
                    break;
                }
            }
        }
    }

    g_Log->Info(LOGFMT_OBJ_TAG("config loader reload configs success config mgr number:%llu, changes number:%llu.")
                , static_cast<UInt64>(GetAllComps().size()), static_cast<UInt64>(changes.size()));
    return Status::Success;
}


SERVICE_COMMON_END