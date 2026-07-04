// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2026-06-28 18:06:13
// Author: Eric Yonng
// Description:

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_IMONGODB_STORAGE_INFO_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_IMONGODB_STORAGE_INFO_H__

#pragma once

#include <kernel/comp/LibString.h>
#include <kernel/comp/CompObject/CompHostObject.h>
#include <OptionComp/storage/MongoDB/Impl/MongoStorageFlags.h>
#include <OptionComp/storage/MongoDB/Impl/ShardKeyInfo.h>
#include <OptionComp/storage/MongoDB/Impl/MongoIndexInfo.h>

KERNEL_BEGIN

class MongodbIndexFieldValue
{
public:
    enum ENUMS
    {
        // 升序
        ASC = 1,
        // 降序
        DESC = -1,
        // hash索引
        HASHED = -2,
    };
};

// 有需要持久化的, 需要注册信息, 并设置OnSave回调SetOnSaveCb User等多子系统时可以通过注册组件, 来构建子字段, 注意以IMongodbStorageInfo为基类的组件, 那么InterfaceTypeId会都一样, 用基类GetComp需要注意以下, 建议使用最终的类类型GetComp
class IMongodbStorageInfo : public KERNEL_NS::CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, IMongodbStorageInfo);

public:
    // fieldName: 需要使用RttiUtil获取系统的objName, 内部自动移除命名空间
    IMongodbStorageInfo(UInt64 objTypeId, const KERNEL_NS::LibString &rttiObjName);
    ~IMongodbStorageInfo() override;

    const KERNEL_NS::LibString &GetDbName() const;
    const KERNEL_NS::LibString &GetSystemName() const;
    // 指定常用的唯一索引
    const std::vector<std::pair<KERNEL_NS::LibString, Int32>> &GetUniqueIndexFields() const;
    // 索引
    const std::map<KERNEL_NS::LibString, KERNEL_NS::MongoIndexInfo> &GetNormalIndexDict() const;
    const std::map<KERNEL_NS::LibString, Int32> &GetFieldNameRefStorageType() const;
    // MongoSerializeInfoType
    Int32 GetStorageType() const;

    UInt64 GetFlags() const;

    void AsKvSystem(const KERNEL_NS::LibString &keyField, const KERNEL_NS::LibString &valueField);
    void AsMultiSystem();
    // MongoSerializeInfoType
    void AsFieldSystem(Int32 storageType);

    // 是否kv系统
    bool IsKvSystem() const;
    // 是否多字段系统
    bool IsMultiSystem() const;
    // 是否作为某个系统的一个字段
    bool IsFieldSystem() const;

    // kv system
    const KERNEL_NS::LibString &GetKeyFieldName() const;
    const KERNEL_NS::LibString &GetValueFieldName() const;

    // 分片键
    const KERNEL_NS::ShardKeyInfoGroup &GetShardKeyInfoGroup() const;

    // 设置持久化回调
    template<typename T>
    void SetOnSaveCb(Int32 (T::*Cb)(Int64 key, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb) const)
    {
        auto lambCb = [Cb](const KERNEL_NS::CompHostObject *host, Int64 key, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb) -> Int32
        {
            return  (host->CastTo<T>()->*Cb)(key, fieldRefdb);
        };
        auto deleg = KERNEL_CREATE_CLOSURE_DELEGATE(lambCb, Int32, const KERNEL_NS::CompHostObject *, Int64, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &);
        CRYSTAL_RELEASE_SAFE(_numberSaveCb);
        _numberSaveCb = deleg;
    }
    
    template<typename T>
    void SetOnSaveCb(Int32 (T::*Cb)(Int64 key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &data) const)
    {
        auto lambCb = [Cb](const KERNEL_NS::CompHostObject *host, Int64 key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &data) -> Int32
        {
            return  (host->CastTo<T>()->*Cb)(key, data);
        };
        auto deleg = KERNEL_CREATE_CLOSURE_DELEGATE(lambCb, Int32, const KERNEL_NS::CompHostObject *, Int64, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &);
        CRYSTAL_RELEASE_SAFE(_numberSteamSaveCb);
        _numberSteamSaveCb = deleg;
    }
    
    // 设置持久化回调
    template<typename T>
    void SetOnSaveCb(Int32 (T::*Cb)(const KERNEL_NS::LibString &key, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb) const)
    {
        auto lambCb = [Cb](const KERNEL_NS::CompHostObject *host, const KERNEL_NS::LibString &key, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb) -> Int32
        {
            return  (host->CastTo<T>()->*Cb)(key, fieldRefdb);
        };
        auto deleg = KERNEL_CREATE_CLOSURE_DELEGATE(lambCb, Int32, const KERNEL_NS::CompHostObject *, const KERNEL_NS::LibString &, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &);
        CRYSTAL_RELEASE_SAFE(_stringSaveCb);
        _stringSaveCb = deleg;
    }
    
    // 设置持久化回调
    template<typename T>
    void SetOnSaveCb(Int32 (T::*Cb)(const KERNEL_NS::LibString &key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &data) const)
    {
        auto lambCb = [Cb](const KERNEL_NS::CompHostObject *host, const KERNEL_NS::LibString &key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &data) -> Int32
        {
            return  (host->CastTo<T>()->*Cb)(key, data);
        };
        auto deleg = KERNEL_CREATE_CLOSURE_DELEGATE(lambCb, Int32, const KERNEL_NS::CompHostObject *, const KERNEL_NS::LibString &, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &);
        CRYSTAL_RELEASE_SAFE(_stringSteamSaveCb);
        _stringSteamSaveCb = deleg;
    }

    // 设置持久化回调
    template<typename T>
    void SetOnSaveCb(Int32 (T::*Cb)(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &data) const)
    {
        auto lambCb = [Cb](const KERNEL_NS::CompHostObject *host, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &data) -> Int32
        {
            return  (host->CastTo<T>()->*Cb)(data);
        };
        auto deleg = KERNEL_CREATE_CLOSURE_DELEGATE(lambCb, Int32, const KERNEL_NS::CompHostObject *, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &);
        CRYSTAL_RELEASE_SAFE(_asFieldSystemSaveCb);
        _asFieldSystemSaveCb = deleg;
    }

    // 检查设置
    virtual Int32 _OnHostStart() final;

    // 持久化回调
    Int32 OnSave(const KERNEL_NS::CompHostObject *host, Int64 key, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb) const;
    Int32 OnSave(const KERNEL_NS::CompHostObject *host, Int64 key,  KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &data) const;
    Int32 OnSave(const KERNEL_NS::CompHostObject *host, const KERNEL_NS::LibString &key, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb) const;
    Int32 OnSave(const KERNEL_NS::CompHostObject *host, const KERNEL_NS::LibString &key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &data) const;
    Int32 OnSave(const KERNEL_NS::CompHostObject *host, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &data) const;

protected:
    /** 当系统作为表时需要填写索引, 必要的简单字段名存储类型，db名等 **/
    // db名 必须指定否则启动失败
    KERNEL_NS::LibString _dbName;
    // 系统名(表名)如果为空就使用所在宿主的类名, 如果系统作为字段, 那么collectionName是字段名
    const KERNEL_NS::LibString _systemName;
    // 索引信息:key,Int32:1:升序, -1降序,-2:hashed MongodbIndexFieldValue 必须指定否则启动失败 唯一索引
    std::vector<std::pair<KERNEL_NS::LibString, Int32>> _uniqueIndexFields;
    // 普通索引
    std::map<KERNEL_NS::LibString, KERNEL_NS::MongoIndexInfo> _indexNameRefInfo;
    // 系统含有多字段, 给每个字段做存储类型定义 必须指定 _fieldNameRefStorageType 和 _storageType之一 MongoSerializeInfoType
    std::map<KERNEL_NS::LibString, Int32> _fieldNameRefStorageType;

    /** 当系统只作为其他系统的一个字段, 只需要填写storrageType即可 MongoSerializeInfoType **/
    // 系统整体作为一个字段, 存储类型定义(子系统默认只作为母系统的一个字段整体存储) 当系统作为其他系统的组件时使用
    Int32 _storageType;

    // MongoStorageFlags 设置特性
    UInt64 _flags;

    // kv系统时key, value 的字段名
    KERNEL_NS::LibString _keyFieldName;
    KERNEL_NS::LibString _valueFieldName;

    // 设置分片键:基数大, 定向性好, 随机性好(避免热写)
    KERNEL_NS::ShardKeyInfoGroup _shardKeyInfos;

    // 数值key持久化回调
    KERNEL_NS::IDelegate<Int32, const KERNEL_NS::CompHostObject *, Int64, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &> *_numberSaveCb;
    KERNEL_NS::IDelegate<Int32, const KERNEL_NS::CompHostObject *, Int64, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &> *_numberSteamSaveCb;

    // stringkey持久化回调
    KERNEL_NS::IDelegate<Int32, const KERNEL_NS::CompHostObject *, const KERNEL_NS::LibString &, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &> *_stringSaveCb;
    KERNEL_NS::IDelegate<Int32, const KERNEL_NS::CompHostObject *, const KERNEL_NS::LibString &, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &> *_stringSteamSaveCb;

    // 作为其他存储系统的字段持久化回调
    KERNEL_NS::IDelegate<Int32, const KERNEL_NS::CompHostObject *, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &> *_asFieldSystemSaveCb;
};


ALWAYS_INLINE const KERNEL_NS::LibString &IMongodbStorageInfo::GetDbName() const
{
    return _dbName;
}

ALWAYS_INLINE const KERNEL_NS::LibString &IMongodbStorageInfo::GetSystemName() const
{
    return _systemName;
}

ALWAYS_INLINE const std::vector<std::pair<KERNEL_NS::LibString, Int32>> &IMongodbStorageInfo::GetUniqueIndexFields() const
{
    return _uniqueIndexFields;
}

ALWAYS_INLINE const std::map<KERNEL_NS::LibString, KERNEL_NS::MongoIndexInfo> &IMongodbStorageInfo::GetNormalIndexDict() const
{
    return _indexNameRefInfo;
}


ALWAYS_INLINE const std::map<KERNEL_NS::LibString, Int32> &IMongodbStorageInfo::GetFieldNameRefStorageType() const
{
    return _fieldNameRefStorageType;

}

ALWAYS_INLINE Int32 IMongodbStorageInfo::GetStorageType() const
{
    return _storageType;
}

ALWAYS_INLINE UInt64 IMongodbStorageInfo::GetFlags() const
{
    return _flags;
}

ALWAYS_INLINE void IMongodbStorageInfo::AsKvSystem(const KERNEL_NS::LibString &keyField, const KERNEL_NS::LibString &valueField)
{
    _flags = MongoStorageFlags::AsKvSystem(_flags);
    _keyFieldName = keyField;
    _valueFieldName = valueField;
}

ALWAYS_INLINE void IMongodbStorageInfo::AsMultiSystem()
{
    _flags = MongoStorageFlags::AsMultiSystem(_flags);
}

ALWAYS_INLINE void IMongodbStorageInfo::AsFieldSystem(Int32 storageType)
{
    _flags = MongoStorageFlags::AsFieldSystem(_flags);
    _storageType = storageType;
}


ALWAYS_INLINE bool IMongodbStorageInfo::IsKvSystem() const
{
    return MongoStorageFlags::IsKvSystem(_flags);
}

ALWAYS_INLINE bool IMongodbStorageInfo::IsMultiSystem() const
{
    return MongoStorageFlags::IsMultiSystem(_flags);
}
ALWAYS_INLINE bool IMongodbStorageInfo::IsFieldSystem() const
{
    return MongoStorageFlags::IsFieldSystem(_flags);
}

ALWAYS_INLINE const KERNEL_NS::LibString &IMongodbStorageInfo::GetKeyFieldName() const
{
    return _keyFieldName;    
}

ALWAYS_INLINE const KERNEL_NS::LibString &IMongodbStorageInfo::GetValueFieldName() const
{
    return _valueFieldName;
}

ALWAYS_INLINE const KERNEL_NS::ShardKeyInfoGroup &IMongodbStorageInfo::GetShardKeyInfoGroup() const
{
    return _shardKeyInfos;
}

ALWAYS_INLINE Int32 IMongodbStorageInfo::OnSave(const KERNEL_NS::CompHostObject *host, Int64 key, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb) const
{
    return _numberSaveCb->Invoke(host, key, fieldRefdb);
}

ALWAYS_INLINE Int32 IMongodbStorageInfo::OnSave(const KERNEL_NS::CompHostObject *host, Int64 key,  KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &data) const
{
    return _numberSteamSaveCb->Invoke(host, key, data);
}


ALWAYS_INLINE Int32 IMongodbStorageInfo::OnSave(const KERNEL_NS::CompHostObject *host, const KERNEL_NS::LibString &key, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb) const
{
    return _stringSaveCb->Invoke(host, key, fieldRefdb);
}

ALWAYS_INLINE Int32 IMongodbStorageInfo::OnSave(const KERNEL_NS::CompHostObject *host, const KERNEL_NS::LibString &key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &data) const
{
    return _stringSteamSaveCb->Invoke(host, key, data);
}

ALWAYS_INLINE Int32 IMongodbStorageInfo::OnSave(const KERNEL_NS::CompHostObject *host, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &data) const
{
    return _asFieldSystemSaveCb->Invoke(host, data);
}

KERNEL_END

#endif

