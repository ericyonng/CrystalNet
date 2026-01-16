// Generate by ConfigExporter, Dont modify it!!!
// file path:../../service/TestService/config/xlsx/权限表.xlsx
// sheet name:权限表|RoleAuth

#ifndef __CONFIG_ROLEAUTH_CONFIG_H__
#define __CONFIG_ROLEAUTH_CONFIG_H__

#pragma once

#include <kernel/common/common.h>
#include <kernel/comp/CompObject/CompObjectInc.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/memory/ObjPoolWrap.h>
#include <service_common/config/IConfigMgr.h>
#include <service/common/macro.h>
#include <service/common/status.h>

#include <map>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>

SERVICE_BEGIN

class RoleAuthConfig
{
    POOL_CREATE_OBJ_DEFAULT(RoleAuthConfig);

public:
    RoleAuthConfig();
    ~RoleAuthConfig(){}

    bool Parse(const KERNEL_NS::LibString &lineData);
    void Serialize(KERNEL_NS::LibString &lineData) const;

public:
    KERNEL_NS::LibString _id;    // id

    Int32 _accountType;    // 0:普通1:接受广播消息

    KERNEL_NS::LibString _pwd;    // pwd


};

class RoleAuthConfigMgr : public SERVICE_COMMON_NS::IConfigMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(IConfigMgr, RoleAuthConfigMgr)

public:
    // Empty configs define
    static const std::vector<RoleAuthConfig *> s_empty;

    RoleAuthConfigMgr();
    ~RoleAuthConfigMgr();

    virtual void Release() override;
    virtual void Clear() override;
    virtual KERNEL_NS::LibString ToString() const override;
    virtual Int32 Load() override;
    virtual Int32 Reload() override;
    virtual const KERNEL_NS::LibString & GetConfigDataMd5() const override;
    const std::vector<RoleAuthConfig *> &GetAllConfigs() const;

    // dict configs
    // by Id
    const std::map<KERNEL_NS::LibString, RoleAuthConfig *> &GetAllIdRefConfigs() const;
    const RoleAuthConfig * GetConfigById(const KERNEL_NS::LibString &key) const;

private:
    virtual void _OnClose() override;
    void _Clear();
    Int64 _ReadConfigData(FILE &fp, KERNEL_NS::LibString &configData, Int32 totalLine, Int32 curLine) const;

private:
    std::vector<RoleAuthConfig *> _configs;
    KERNEL_NS::LibString _dataMd5;
    std::map<KERNEL_NS::LibString, RoleAuthConfig *> _idRefConfig;

};


ALWAYS_INLINE const std::vector<RoleAuthConfig *> &RoleAuthConfigMgr::GetAllConfigs() const
{
    return _configs;
}

ALWAYS_INLINE const std::map<KERNEL_NS::LibString, RoleAuthConfig *> &RoleAuthConfigMgr::GetAllIdRefConfigs() const
{
    return _idRefConfig;
}

ALWAYS_INLINE const RoleAuthConfig * RoleAuthConfigMgr::GetConfigById(const KERNEL_NS::LibString &key) const
{
    auto iter = _idRefConfig.find(key);
    return iter == _idRefConfig.end() ? NULL : iter->second;
}


class RoleAuthConfigMgrFactory : public KERNEL_NS::CompFactory
{
public:
    static constexpr KERNEL_NS::_Build::MT _buildType{};

    static KERNEL_NS::CompFactory *FactoryCreate()
    {
        return KERNEL_NS::ObjPoolWrap<RoleAuthConfigMgrFactory>::NewByAdapter(_buildType.V);
    }

    virtual void Release() override
    {
        KERNEL_NS::ObjPoolWrap<RoleAuthConfigMgrFactory>::DeleteByAdapter(_buildType.V, this);
    }

    virtual KERNEL_NS::CompObject *Create() const override
    {
        return RoleAuthConfigMgr::NewByAdapter_RoleAuthConfigMgr(_buildType.V);
    }

};

SERVICE_END

#endif // __CONFIG_ROLEAUTH_CONFIG_H__
