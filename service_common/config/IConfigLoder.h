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

#ifndef __CRYSTAL_NET_SERVICE_COMMON_CONFIG_ICONFIG_LOADER_H__
#define __CRYSTAL_NET_SERVICE_COMMON_CONFIG_ICONFIG_LOADER_H__

#pragma once

#include <service_common/common/macro.h>
#include <kernel/comp/CompObject/CompHostObject.h>
#include <kernel/comp/LibString.h>
#include <vector>

SERVICE_COMMON_BEGIN

class IConfigMgr;

class IConfigLoader : public KERNEL_NS::CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, IConfigLoader);

public:
    IConfigLoader(UInt64 objTypeId);
    virtual ~IConfigLoader();

    virtual KERNEL_NS::LibString ToString() const override;

    virtual Int32 Load();
    // reload只支持不改变页签名或新增页签，只变更数据
    virtual Int32 Reload(std::vector<const IConfigMgr *> &changes);
    template<typename ConfigMgrType>
    Int32 ReloadConfigMgr(bool &isChanged);

    // virtual void OnRegisterComps() = 0;
    // virtual void Release() = 0;

   template<typename ConfigMgrType>
   const ConfigMgrType *GetConfigMgr() const;

    // 配置数据文件根路径:设置相对路径或者绝对路径, 相对路径是相对于当前可执行程序的路径
    void SetBasePath(const KERNEL_NS::LibString &basePath);
    const KERNEL_NS::LibString &GetBasePath() const;

protected:
    // 所有配置在此时加载
    virtual Int32 _OnCompsCreated() final;
    virtual Int32 _OnHostInit() override { return Status::Success; }
    virtual Int32 _OnHostStart() override { return Status::Success; }
    virtual void _OnHostClose() override {}

   template<typename ConfigMgrType>
   ConfigMgrType *_GetConfigMgr();
   Int32 _ReloadConfigMgr(IConfigMgr *configMgr, bool &isChanged);

private:
    KERNEL_NS::LibString _basePath;
};

template<typename ConfigMgrType>
ALWAYS_INLINE const ConfigMgrType *IConfigLoader::GetConfigMgr() const
{
    return GetComp<ConfigMgrType>();
}

ALWAYS_INLINE void IConfigLoader::SetBasePath(const KERNEL_NS::LibString &basePath)
{
    _basePath = basePath;
}

ALWAYS_INLINE const KERNEL_NS::LibString &IConfigLoader::GetBasePath() const
{
    return _basePath;
}

template<typename ConfigMgrType>
ALWAYS_INLINE Int32 IConfigLoader::ReloadConfigMgr(bool &isChanged)
{
    return _ReloadConfigMgr(_GetConfigMgr<ConfigMgrType>(), isChanged);
}

template<typename ConfigMgrType>
ALWAYS_INLINE ConfigMgrType *IConfigLoader::_GetConfigMgr()
{
    return GetComp<ConfigMgrType>();
}

SERVICE_COMMON_END

#endif

