// Generate by ConfigExporter, Dont modify it!!!
// file path:../../service/Client/config/xlsx/Item道具.xlsx
// sheet name:道具|Item

#ifndef __CONFIG_ITEM_CONFIG_H__
#define __CONFIG_ITEM_CONFIG_H__

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

class ItemConfig
{
    POOL_CREATE_OBJ_DEFAULT(ItemConfig);

public:
    ItemConfig();
    ~ItemConfig(){}

    bool Parse(const KERNEL_NS::LibString &lineData);
    void Serialize(KERNEL_NS::LibString &lineData) const;

public:
    Int32 _id;    // id

    Int32 _type;    // 类型:1, 2, 3


};

class ItemConfigMgr : public SERVICE_COMMON_NS::IConfigMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(IConfigMgr, ItemConfigMgr)

public:
    // Empty configs define
    static const std::vector<ItemConfig *> s_empty;

    ItemConfigMgr();
    ~ItemConfigMgr();

    virtual void Release() override;
    virtual void Clear() override;
    virtual KERNEL_NS::LibString ToString() const override;
    virtual Int32 Load() override;
    virtual Int32 Reload() override;
    virtual const KERNEL_NS::LibString & GetConfigDataMd5() const override;
    const std::vector<ItemConfig *> &GetAllConfigs() const;

    // dict configs
    // by Id
    const std::map<Int32, ItemConfig *> &GetAllIdRefConfigs() const;
    const ItemConfig * GetConfigById(const Int32 &key) const;

    // by Type
    const std::map<Int32, std::vector<ItemConfig *>> &GetAllTypeRefConfigs() const;
    const std::vector<ItemConfig *> &GetConfigsByType(const Int32 &key) const;

private:
    virtual void _OnClose() override;
    void _Clear();
    Int64 _ReadConfigData(FILE &fp, KERNEL_NS::LibString &configData, Int32 totalLine, Int32 curLine) const;

private:
    std::vector<ItemConfig *> _configs;
    KERNEL_NS::LibString _dataMd5;
    std::map<Int32, ItemConfig *> _idRefConfig;
    std::map<Int32, std::vector<ItemConfig *>> _typeRefConfigs;

};


ALWAYS_INLINE const std::vector<ItemConfig *> &ItemConfigMgr::GetAllConfigs() const
{
    return _configs;
}

ALWAYS_INLINE const std::map<Int32, ItemConfig *> &ItemConfigMgr::GetAllIdRefConfigs() const
{
    return _idRefConfig;
}

ALWAYS_INLINE const ItemConfig * ItemConfigMgr::GetConfigById(const Int32 &key) const
{
    auto iter = _idRefConfig.find(key);
    return iter == _idRefConfig.end() ? NULL : iter->second;
}

ALWAYS_INLINE const std::map<Int32, std::vector<ItemConfig *>> &ItemConfigMgr::GetAllTypeRefConfigs() const
{
    return _typeRefConfigs;
}

ALWAYS_INLINE const std::vector<ItemConfig *> &ItemConfigMgr::GetConfigsByType(const Int32 &key) const
{
    auto iterConfigs = _typeRefConfigs.find(key);
    return iterConfigs == _typeRefConfigs.end() ? s_empty : iterConfigs->second;
}


class ItemConfigMgrFactory : public KERNEL_NS::CompFactory
{
public:
    static constexpr KERNEL_NS::_Build::MT _buildType{};

    static KERNEL_NS::CompFactory *FactoryCreate()
    {
        return KERNEL_NS::ObjPoolWrap<ItemConfigMgrFactory>::NewByAdapter(_buildType.V);
    }

    virtual void Release() override
    {
        KERNEL_NS::ObjPoolWrap<ItemConfigMgrFactory>::DeleteByAdapter(_buildType.V, this);
    }

    virtual KERNEL_NS::CompObject *Create() const override
    {
        return ItemConfigMgr::NewByAdapter_ItemConfigMgr(_buildType.V);
    }

};

SERVICE_END

#endif // __CONFIG_ITEM_CONFIG_H__
