// Generate by ConfigExporter, Dont modify it!!!
// file path:../../service/CenterService/config/xlsx/example.xlsx
// file path:../../service/CenterService/config/xlsx/example.xlsx
// sheet name:示例part1|Example;示例part2|Example

#ifndef __CONFIG_EXAMPLE_CONFIG_H__
#define __CONFIG_EXAMPLE_CONFIG_H__

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

class ExampleConfig
{
    POOL_CREATE_OBJ_DEFAULT(ExampleConfig);

public:
    ExampleConfig();
    ~ExampleConfig(){}

    bool Parse(const KERNEL_NS::LibString &lineData);
    void Serialize(KERNEL_NS::LibString &lineData) const;

public:
    Int32 _id;    // id

    Int32 _type;    // 类型:1, 2, 3

    KERNEL_NS::LibString _title;    // 称号，文字id

    std::vector<Int32> _goal;    // 目标id列表

    std::map<Int32, KERNEL_NS::LibString> _testDict;    // 测试字典

    std::map<Int32, Int32> _roleBuff;    // 角色的buff列表

    bool _isLucky;    // 是否幸运者

    std::map<Int32, std::vector<Int32>> _awards;

    std::map<Int32, std::map<Int32, std::vector<Int32>>> _achieve;


};

class ExampleConfigMgr : public SERVICE_COMMON_NS::IConfigMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(IConfigMgr, ExampleConfigMgr)

public:
    // Empty configs define
    static const std::vector<ExampleConfig *> s_empty;

    ExampleConfigMgr();
    ~ExampleConfigMgr();

    virtual void Release() override;
    virtual UInt64 GetObjTypeId() const override;
    virtual void Clear() override;
    virtual KERNEL_NS::LibString ToString() const override;
    virtual Int32 Load() override;
    virtual Int32 Reload() override;
    virtual const KERNEL_NS::LibString & GetConfigDataMd5() const override;
    const std::vector<ExampleConfig *> &GetAllConfigs() const;

    // dict configs
    // by Id
    const std::map<Int32, ExampleConfig *> &GetAllIdRefConfigs() const;
    const ExampleConfig * GetConfigById(const Int32 &key) const;

    // by Type
    const std::map<Int32, std::vector<ExampleConfig *>> &GetAllTypeRefConfigs() const;
    const std::vector<ExampleConfig *> &GetConfigsByType(const Int32 &key) const;

private:
    virtual void _OnClose() override;
    void _Clear();
    Int64 _ReadConfigData(FILE &fp, KERNEL_NS::LibString &configData, Int32 totalLine, Int32 curLine) const;

private:
    std::vector<ExampleConfig *> _configs;
    KERNEL_NS::LibString _dataMd5;
    std::map<Int32, ExampleConfig *> _idRefConfig;
    std::map<Int32, std::vector<ExampleConfig *>> _typeRefConfigs;

};


ALWAYS_INLINE const std::vector<ExampleConfig *> &ExampleConfigMgr::GetAllConfigs() const
{
    return _configs;
}

ALWAYS_INLINE const std::map<Int32, ExampleConfig *> &ExampleConfigMgr::GetAllIdRefConfigs() const
{
    return _idRefConfig;
}

ALWAYS_INLINE const ExampleConfig * ExampleConfigMgr::GetConfigById(const Int32 &key) const
{
    auto iter = _idRefConfig.find(key);
    return iter == _idRefConfig.end() ? NULL : iter->second;
}

ALWAYS_INLINE const std::map<Int32, std::vector<ExampleConfig *>> &ExampleConfigMgr::GetAllTypeRefConfigs() const
{
    return _typeRefConfigs;
}

ALWAYS_INLINE const std::vector<ExampleConfig *> &ExampleConfigMgr::GetConfigsByType(const Int32 &key) const
{
    auto iterConfigs = _typeRefConfigs.find(key);
    return iterConfigs == _typeRefConfigs.end() ? s_empty : iterConfigs->second;
}


class ExampleConfigMgrFactory : public KERNEL_NS::CompFactory
{
public:
    static constexpr KERNEL_NS::_Build::MT _buildType{};

    static KERNEL_NS::CompFactory *FactoryCreate()
    {
        return KERNEL_NS::ObjPoolWrap<ExampleConfigMgrFactory>::NewByAdapter(_buildType.V);
    }

    virtual void Release() override
    {
        KERNEL_NS::ObjPoolWrap<ExampleConfigMgrFactory>::DeleteByAdapter(_buildType.V, this);
    }

    virtual KERNEL_NS::CompObject *Create() const override
    {
        return ExampleConfigMgr::NewByAdapter_ExampleConfigMgr(_buildType.V);
    }

    virtual UInt64 GetObjTypeId() const override
    {
        return KERNEL_NS::RttiUtil::GetTypeId<ExampleConfigMgrFactory>();
    }

};

SERVICE_END

#endif // __CONFIG_EXAMPLE_CONFIG_H__
