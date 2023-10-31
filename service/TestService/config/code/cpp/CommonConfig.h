// Generate by ConfigExporter, Dont modify it!!!
// file path:../../service/TestService/config/xlsx/公共参数.xlsx
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

        USER_LOGIN_EXPIRE_TIME,    // id

        USER_HEARTBEAT_EXPIRE_TIME,    // id

        USER_LOGIN_KEY_CHAR_COUNT_MAX_LEN,    // id

        NAME_MAX_LEN,    // id

        BORROW_COUNT_LIMIT,    // id

        ORDER_SAVE_DAYS,    // id

        MAX_BORROW_DAYS,    // id

        CONTENT_LIMIT,    // id

        FIRST_DAY_OF_WEEK,    // id

        CREATE_LIBRARY_NEED_INVITE_CODE,    // id

        MAX_IMAGE_SIZE,    // id

        BOOK_CONTENT_LIMIT,    // id

        BOOK_SNAP_SHOT_MAX_LIMIT,    // id

        KEYWORDS_LIMIT,    // id

        NOTIFY_MAX_LIMIT,    // id

        OUTSTORE_WAIT_GOT_DAYS,    // id

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

    Int64 _int64Value;

    KERNEL_NS::LibString _stringValue;


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

private:
    virtual void _OnClose() override;
    void _Clear();
    Int64 _ReadConfigData(FILE &fp, KERNEL_NS::LibString &configData, Int32 totalLine, Int32 curLine) const;

private:
    std::vector<CommonConfig *> _configs;
    KERNEL_NS::LibString _dataMd5;
    std::map<Int32, CommonConfig *> _idRefConfig;

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
