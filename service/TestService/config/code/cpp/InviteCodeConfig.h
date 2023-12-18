// Generate by ConfigExporter, Dont modify it!!!
// file path:../../service/TestService/config/xlsx/邀请码.xlsx
// sheet name:邀请码|InviteCode

#ifndef __CONFIG_INVITECODE_CONFIG_H__
#define __CONFIG_INVITECODE_CONFIG_H__

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

class InviteCodeConfig
{
    POOL_CREATE_OBJ_DEFAULT(InviteCodeConfig);

public:
    InviteCodeConfig();
    ~InviteCodeConfig(){}

    bool Parse(const KERNEL_NS::LibString &lineData);
    void Serialize(KERNEL_NS::LibString &lineData) const;

public:
    Int32 _id;    // id

    KERNEL_NS::LibString _inviteCode;    // 邀请码(不超过64个英文字符),且邀请码不重复


};

class InviteCodeConfigMgr : public SERVICE_COMMON_NS::IConfigMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(IConfigMgr, InviteCodeConfigMgr)

public:
    // Empty configs define
    static const std::vector<InviteCodeConfig *> s_empty;

    InviteCodeConfigMgr();
    ~InviteCodeConfigMgr();

    virtual void Release() override;
    virtual void Clear() override;
    virtual KERNEL_NS::LibString ToString() const override;
    virtual Int32 Load() override;
    virtual Int32 Reload() override;
    virtual const KERNEL_NS::LibString & GetConfigDataMd5() const override;
    const std::vector<InviteCodeConfig *> &GetAllConfigs() const;

    // dict configs
    // by Id
    const std::map<Int32, InviteCodeConfig *> &GetAllIdRefConfigs() const;
    const InviteCodeConfig * GetConfigById(const Int32 &key) const;

    // by InviteCode
    const std::map<KERNEL_NS::LibString, InviteCodeConfig *> &GetAllInviteCodeRefConfigs() const;
    const InviteCodeConfig * GetConfigByInviteCode(const KERNEL_NS::LibString &key) const;

private:
    virtual void _OnClose() override;
    void _Clear();
    Int64 _ReadConfigData(FILE &fp, KERNEL_NS::LibString &configData, Int32 totalLine, Int32 curLine) const;

private:
    std::vector<InviteCodeConfig *> _configs;
    KERNEL_NS::LibString _dataMd5;
    std::map<Int32, InviteCodeConfig *> _idRefConfig;
    std::map<KERNEL_NS::LibString, InviteCodeConfig *> _inviteCodeRefConfig;

};


ALWAYS_INLINE const std::vector<InviteCodeConfig *> &InviteCodeConfigMgr::GetAllConfigs() const
{
    return _configs;
}

ALWAYS_INLINE const std::map<Int32, InviteCodeConfig *> &InviteCodeConfigMgr::GetAllIdRefConfigs() const
{
    return _idRefConfig;
}

ALWAYS_INLINE const InviteCodeConfig * InviteCodeConfigMgr::GetConfigById(const Int32 &key) const
{
    auto iter = _idRefConfig.find(key);
    return iter == _idRefConfig.end() ? NULL : iter->second;
}

ALWAYS_INLINE const std::map<KERNEL_NS::LibString, InviteCodeConfig *> &InviteCodeConfigMgr::GetAllInviteCodeRefConfigs() const
{
    return _inviteCodeRefConfig;
}

ALWAYS_INLINE const InviteCodeConfig * InviteCodeConfigMgr::GetConfigByInviteCode(const KERNEL_NS::LibString &key) const
{
    auto iter = _inviteCodeRefConfig.find(key);
    return iter == _inviteCodeRefConfig.end() ? NULL : iter->second;
}


class InviteCodeConfigMgrFactory : public KERNEL_NS::CompFactory
{
public:
    static constexpr KERNEL_NS::_Build::MT _buildType{};

    static KERNEL_NS::CompFactory *FactoryCreate()
    {
        return KERNEL_NS::ObjPoolWrap<InviteCodeConfigMgrFactory>::NewByAdapter(_buildType.V);
    }

    virtual void Release() override
    {
        KERNEL_NS::ObjPoolWrap<InviteCodeConfigMgrFactory>::DeleteByAdapter(_buildType.V, this);
    }

    virtual KERNEL_NS::CompObject *Create() const override
    {
        return InviteCodeConfigMgr::NewByAdapter_InviteCodeConfigMgr(_buildType.V);
    }
};

SERVICE_END

#endif // __CONFIG_INVITECODE_CONFIG_H__
