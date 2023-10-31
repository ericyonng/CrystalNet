// Generate by ConfigExporter, Dont modify it!!!
// file path:../../service/TestService/config/xlsx/文字id.xlsx
// sheet name:文字id|WordId

#ifndef __CONFIG_WORDID_CONFIG_H__
#define __CONFIG_WORDID_CONFIG_H__

#pragma once

#include <kernel/kernel.h>
#include <service_common/config/config.h>
#include <service/common/common.h>

SERVICE_BEGIN

class WordIdConfig
{
    POOL_CREATE_OBJ_DEFAULT(WordIdConfig);

public:
    WordIdConfig();
    ~WordIdConfig(){}

    bool Parse(const KERNEL_NS::LibString &lineData);
    void Serialize(KERNEL_NS::LibString &lineData) const;

public:
    KERNEL_NS::LibString _id;    // id

    KERNEL_NS::LibString _stringValue;


};

class WordIdConfigMgr : public SERVICE_COMMON_NS::IConfigMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(IConfigMgr, WordIdConfigMgr)

public:
    // Empty configs define
    static const std::vector<WordIdConfig *> s_empty;

    WordIdConfigMgr();
    ~WordIdConfigMgr();

    virtual void Release() override;
    virtual void Clear() override;
    virtual KERNEL_NS::LibString ToString() const override;
    virtual Int32 Load() override;
    virtual Int32 Reload() override;
    virtual const KERNEL_NS::LibString & GetConfigDataMd5() const override;
    const std::vector<WordIdConfig *> &GetAllConfigs() const;

    // dict configs
    // by Id
    const std::map<KERNEL_NS::LibString, WordIdConfig *> &GetAllIdRefConfigs() const;
    const WordIdConfig * GetConfigById(const KERNEL_NS::LibString &key) const;

private:
    virtual void _OnClose() override;
    void _Clear();
    Int64 _ReadConfigData(FILE &fp, KERNEL_NS::LibString &configData, Int32 totalLine, Int32 curLine) const;

private:
    std::vector<WordIdConfig *> _configs;
    KERNEL_NS::LibString _dataMd5;
    std::map<KERNEL_NS::LibString, WordIdConfig *> _idRefConfig;

};


ALWAYS_INLINE const std::vector<WordIdConfig *> &WordIdConfigMgr::GetAllConfigs() const
{
    return _configs;
}

ALWAYS_INLINE const std::map<KERNEL_NS::LibString, WordIdConfig *> &WordIdConfigMgr::GetAllIdRefConfigs() const
{
    return _idRefConfig;
}

ALWAYS_INLINE const WordIdConfig * WordIdConfigMgr::GetConfigById(const KERNEL_NS::LibString &key) const
{
    auto iter = _idRefConfig.find(key);
    return iter == _idRefConfig.end() ? NULL : iter->second;
}


class WordIdConfigMgrFactory : public KERNEL_NS::CompFactory
{
public:
    static constexpr KERNEL_NS::_Build::MT _buildType{};

    static KERNEL_NS::CompFactory *FactoryCreate()
    {
        return KERNEL_NS::ObjPoolWrap<WordIdConfigMgrFactory>::NewByAdapter(_buildType.V);
    }

    virtual void Release()
    {
        KERNEL_NS::ObjPoolWrap<WordIdConfigMgrFactory>::DeleteByAdapter(_buildType.V, this);
    }

    virtual KERNEL_NS::CompObject *Create() const
    {
        return WordIdConfigMgr::NewByAdapter_WordIdConfigMgr(_buildType.V);
    }
};

SERVICE_END

#endif // __CONFIG_WORDID_CONFIG_H__
