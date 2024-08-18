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
 * Date: 2023-08-06 19:46:39
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/kernel.h>
#include <Comps/config/impl/ConfigLoader.h>
#include <Comps/config/impl/ConfigLoaderFactory.h>
#include <cpp/AllConfigs.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(ConfigLoader);

ConfigLoader::ConfigLoader()
{

}

ConfigLoader::~ConfigLoader()
{

}

void ConfigLoader::Release()
{
    ConfigLoader::DeleteByAdapter_ConfigLoader(ConfigLoaderFactory::_buildType.V, this);
}

void ConfigLoader::OnRegisterComps()
{
    #include <cpp/RegisterAllConfigs.hpp>
}

Int32 ConfigLoader::_OnHostInit()
{
    auto &basePath = GetBasePath();
    if(UNLIKELY(basePath.empty()))
    {
        auto owner = GetOwner();
        g_Log->Error(LOGFMT_OBJ_TAG("ConfigDataPath is empty please check owner:%s"), owner ? owner->GetObjName().c_str():"");
        return Status::ConfigError;
    }

    return Status::Success;
}

Int32 ConfigLoader::_OnHostStart()
{
    return Status::Success;
}

void ConfigLoader::_OnHostClose()
{

}

OBJ_GET_OBJ_TYPEID_IMPL(ConfigLoader)

SERVICE_END
