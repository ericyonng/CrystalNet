// Generate by ConfigExporter, Dont modify it!!!
// file path:../../service/CenterService/config/xlsx/公共参数.xlsx
// sheet name:公共参数|Common

#ifndef __CONFIG_COMMON_CONFIG_H__
#define __CONFIG_COMMON_CONFIG_H__

#pragma once

#include <kernel/kernel.h>
#include <service_common/config/config.h>
#include <service/common/common.h>

SERVICE_BEGIN

class CommonConfigIdEnums
{
public:
    enum ENUMS
    {
        __UNKNOWN_ENUM = 0,

        USER_LRU_CAPACITY_LIMIT,    // id

        USER_LOGIN_KEY_EXPIRE_TIME,    // id

        USER_LOGIN_KEY_CHAR_COUNT,    // id

        __ENUM_MAX,

    };
};

class CommonConfig
{
    POOL_CREATE_OBJ_DEFAULT(CommonConfig);

public:
    CommonConfig();
    ~CommonConfig(){}

    bool Parse(const KERNEL_NS::LibString &lineData);
    void Serialize(KERNEL_NS::LibString &lineData) const;

public:
    Int32 _id;    // id

    Int32 _value;


};

class CommonConfigMgr : public SERVICE_COMMON_NS::IConfigMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(IConfigMgr, CommonConfigMgr)

public:
    // Empty configs define
    static const std::vector<CommonConfig *> s_empty;

    CommonConfigMgr();
    ~CommonConfigMgr();

    virtual void Release() override;
    virtual void Clear() override;
    virtual KERNEL_NS::LibString ToString() const override;
    virtual Int32 Load() override;
    virtual Int32 Reload() override;
    virtual const KERNEL_NS::LibString & GetConfigDataMd5() const override;
    const std::vector<CommonConfig *> &GetAllConfigs() const;

    // dict configs
    // by Id
    const std::map<Int32, CommonConfig *> &GetAllIdRefConfigs() const;
    const CommonConfig * GetConfigById(const Int32 &key) const;

    // by Value
    const std::map<Int32, std::vector<CommonConfig *>> &GetAllValueRefConfigs() const;
    const std::vector<CommonConfig *> &GetConfigsByValue(const Int32 &key) const;

private:
    virtual void _OnClose() override;
    void _Clear();
    Int64 _ReadConfigData(FILE &fp, KERNEL_NS::LibString &configData, Int32 totalLine, Int32 curLine) const;

private:
    std::vector<CommonConfig *> _configs;
    KERNEL_NS::LibString _dataMd5;
    std::map<Int32, CommonConfig *> _idRefConfig;
    std::map<Int32, std::vector<CommonConfig *>> _valueRefConfigs;

};


ALWAYS_INLINE const std::vector<CommonConfig *> &CommonConfigMgr::GetAllConfigs() const
{
    return _configs;
}

ALWAYS_INLINE const std::map<Int32, CommonConfig *> &CommonConfigMgr::GetAllIdRefConfigs() const
{
    return _idRefConfig;
}

ALWAYS_INLINE const CommonConfig * CommonConfigMgr::GetConfigById(const Int32 &key) const
{
    auto iter = _idRefConfig.find(key);
    return iter == _idRefConfig.end() ? NULL : iter->second;
}

ALWAYS_INLINE const std::map<Int32, std::vector<CommonConfig *>> &CommonConfigMgr::GetAllValueRefConfigs() const
{
    return _valueRefConfigs;
}

ALWAYS_INLINE const std::vector<CommonConfig *> &CommonConfigMgr::GetConfigsByValue(const Int32 &key) const
{
    auto iterConfigs = _valueRefConfigs.find(key);
    return iterConfigs == _valueRefConfigs.end() ? s_empty : iterConfigs->second;
}


class CommonConfigMgrFactory : public KERNEL_NS::CompFactory
{
public:
    static constexpr KERNEL_NS::_Build::MT _buildType{};

    static KERNEL_NS::CompFactory *FactoryCreate()
    {
        return KERNEL_NS::ObjPoolWrap<CommonConfigMgrFactory>::NewByAdapter(_buildType.V);
    }

    virtual void Release()
    {
        KERNEL_NS::ObjPoolWrap<CommonConfigMgrFactory>::DeleteByAdapter(_buildType.V, this);
    }

    virtual KERNEL_NS::CompObject *Create() const
    {
        return CommonConfigMgr::NewByAdapter_CommonConfigMgr(_buildType.V);
    }
};

SERVICE_END

#endif // __CONFIG_COMMON_CONFIG_H__
