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
 * Date: 2023-07-23 15:55:11
 * Author: Eric Yonng
 * Description: 只支持单一主键,不支持联合主键
*/

#pragma once

#include <kernel/kernel.h>
#include <service_common/ServiceCommon.h>
#include <service/common/macro.h>
#include <service/common/status.h>
#include <service/common/BaseComps/Storage/Impl/StorageInfoFactory.h>

SERVICE_BEGIN

class IStorageInfo;

// 存储大小定义
class StorageCapacityType
{
public:
    enum ENUMS : UInt64
    {
        Cap16 = 16LLU,     
        Cap32 = 32LLU,     
        Cap64 = 64LLU,     
        Cap128 = 128LLU,   
        Cap256 = 256LLU,   
        Cap512 = 512LLU,   
        Cap1K = 1024LLU,   
        Cap2K = 2048LLU,   
        Cap4K = 4095LLU,   
        Cap8K = 8191LLU,   
        Cap16K = 16383LLU, 
        Cap32K = 32767LLU, 
        Cap48K = 49151LLU, 
        Cap64K = 65535LLU, 

        // 存储大小超过 Cap64K 启服的时候需要打印警告信息, 以便后续优化
        Cap16M = 16777216LLU,     
        Cap4GB = 4294967296LLU,

        // 存储大小超过4GB不可以启服
    };
};

class StorageFlagType
{
private:
    enum POS_ENUMS : UInt64
    {
        MYSQL_STORAGE_POS = 0,
        REDIS_STORAGE_POS,
        AS_FIELD_FLAG_POS,
        KEY_VALUE_SYSTEM_FLAG_POS,
        MULTI_FIELD_SYSTEM_FLAG_POS,
        NEED_STRING_KEY_FLAG_POS,
        NEED_NUMBER_KEY_FLAG_POS,

        // 作为字段的属性
        NUMBER_FIELD_FLAG_POS,
        STRING_FIELD_FLAG_POS,
        BINARY_FIELD_FLAG_POS,
        BLOB_TYPE_BINARY_FIELD_FLAG_POS,
        TEXT_STRING_FIELD_FLAG_POS,

        // 作为数值类型的属性
        UNSIGNED_NUMBER_FIELD_FLAG_POS,
        INT8_NUMBER_FIELD_FLAG_POS,
        INT16_NUMBER_FIELD_FLAG_POS,
        INT32_NUMBER_FIELD_FLAG_POS,
        INT64_NUMBER_FIELD_FLAG_POS,
        FLOAT_NUMBER_FIELD_FLAG_POS,
        DOUBLE_NUMBER_FIELD_FLAG_POS,

        // 作为二进制
        VARBINARY_FIELD_FLAG_POS,
        BLOB_BINARY_FIELD_FLAG_POS,
        MEDIUM_BLOB_BINARY_FIELD_FLAG_POS,
        LONG_BLOB_BINARY_FIELD_FLAG_POS,
        
        // 普通字符串
        NORMAL_STRING_FIELD_FLAG_POS,

        // 作为大字符串
        NORMAL_TEXT_STRING_FIELD_FLAG_POS,
        MEDIUM_TEXT_STRING_FIELD_FLAG_POS,
        LONG_TEXT_STRING_FIELD_FLAG_POS,
        
        // 主键
        PRIMARY_FIELD_FLAG_POS,

        // 某个系统的存储数据
        SYSTEM_DATA_STORAGE_FLAG_POS,

        // 公共系统数据(启动时候加载)
        LOAD_DATA_ON_STARTUP_FLAG_POS,

        // 

    };

public:
    enum MYSQL_DATA_TYPE : Int32
    {
        STORAGE_MYSQL_TYPE_DECIMAL,
        STORAGE_MYSQL_TYPE_TINY,
        STORAGE_MYSQL_TYPE_SHORT,
        STORAGE_MYSQL_TYPE_LONG,
        STORAGE_MYSQL_TYPE_FLOAT,
        STORAGE_MYSQL_TYPE_DOUBLE,
        STORAGE_MYSQL_TYPE_NULL,
        STORAGE_MYSQL_TYPE_TIMESTAMP,
        STORAGE_MYSQL_TYPE_LONGLONG,
        STORAGE_MYSQL_TYPE_INT24,
        STORAGE_MYSQL_TYPE_DATE,
        STORAGE_MYSQL_TYPE_TIME,
        STORAGE_MYSQL_TYPE_DATETIME,
        STORAGE_MYSQL_TYPE_YEAR,
        STORAGE_MYSQL_TYPE_NEWDATE, /**< Internal to MySQL. Not used in protocol */
        STORAGE_MYSQL_TYPE_VARCHAR,
        STORAGE_MYSQL_TYPE_BIT,
        STORAGE_MYSQL_TYPE_TIMESTAMP2,
        STORAGE_MYSQL_TYPE_DATETIME2,   /**< Internal to MySQL. Not used in protocol */
        STORAGE_MYSQL_TYPE_TIME2,       /**< Internal to MySQL. Not used in protocol */
        STORAGE_MYSQL_TYPE_TYPED_ARRAY, /**< Used for replication only */
        STORAGE_MYSQL_TYPE_INVALID = 243,
        STORAGE_MYSQL_TYPE_BOOL = 244, /**< Currently just a placeholder */
        STORAGE_MYSQL_TYPE_JSON = 245,
        STORAGE_MYSQL_TYPE_NEWDECIMAL = 246,
        STORAGE_MYSQL_TYPE_ENUM = 247,
        STORAGE_MYSQL_TYPE_SET = 248,
        STORAGE_MYSQL_TYPE_TINY_BLOB = 249,
        STORAGE_MYSQL_TYPE_MEDIUM_BLOB = 250,
        STORAGE_MYSQL_TYPE_LONG_BLOB = 251,
        STORAGE_MYSQL_TYPE_BLOB = 252,
        STORAGE_MYSQL_TYPE_VAR_STRING = 253,
        STORAGE_MYSQL_TYPE_STRING = 254,
        STORAGE_MYSQL_TYPE_GEOMETRY = 255
    };

    // 值类型
    enum VALUE_TYPE : Int32
    {
        NUMBER_VALUE_TYPE = 0,
        STRING_VALUE_TYPE,
        BINARY_VALUE_TYPE,
    };

    // flag标志位掩码
    enum FLAGS_MASK : UInt64
    {
        NO_FLAGS = 0,
        // 支持mysql存储
        MYSQL_FLAG = 1LLU << MYSQL_STORAGE_POS,
        // 支持redis存储
        REDIS_FLAG = 1LLU << REDIS_STORAGE_POS,

        // 作为其他系统的一个字段
        AS_FIELD_FLAG = 1LLU << AS_FIELD_FLAG_POS,
        // 多字段系统, 包括单字段, 两字段或者以上
        MULTI_FIELD_SYSTEM_FLAG = 1LLU << MULTI_FIELD_SYSTEM_FLAG_POS,
        // key - value 系统 多字段系统的一个特例
        KEY_VALUE_SYSTEM_FLAG = MULTI_FIELD_SYSTEM_FLAG | (1LLU << KEY_VALUE_SYSTEM_FLAG_POS),
        // 需要string key
        NEED_STRING_KEY_FLAG = MULTI_FIELD_SYSTEM_FLAG | (1LLU << NEED_STRING_KEY_FLAG_POS),
        // 需要number key
        NEED_NUMBER_KEY_FLAG = MULTI_FIELD_SYSTEM_FLAG | (1LLU << NEED_NUMBER_KEY_FLAG_POS),

        // 作为字段的一些属性
        NUMBER_FIELD_FLAG = AS_FIELD_FLAG | (1LLU << NUMBER_FIELD_FLAG_POS),
        STRING_FIELD_FLAG = AS_FIELD_FLAG | (1LLU << STRING_FIELD_FLAG_POS),
        BINARY_FIELD_FLAG = AS_FIELD_FLAG | (1LLU << BINARY_FIELD_FLAG_POS),
        BLOB_TYPE_BINARY_FIELD_FLAG = BINARY_FIELD_FLAG | (1LLU << BLOB_TYPE_BINARY_FIELD_FLAG_POS),
        TEXT_STRING_FIELD_FLAG = STRING_FIELD_FLAG | (1LLU << TEXT_STRING_FIELD_FLAG_POS),

        // 作为数值字段的一些属性
        UNSIGNED_NUMBER_FIELD_FLAG = NUMBER_FIELD_FLAG | (1LLU << UNSIGNED_NUMBER_FIELD_FLAG_POS),
        INT8_NUMBER_FIELD_FLAG = NUMBER_FIELD_FLAG | (1LLU << INT8_NUMBER_FIELD_FLAG_POS),
        INT16_NUMBER_FIELD_FLAG = NUMBER_FIELD_FLAG | (1LLU << INT16_NUMBER_FIELD_FLAG_POS),
        INT32_NUMBER_FIELD_FLAG = NUMBER_FIELD_FLAG | (1LLU << INT32_NUMBER_FIELD_FLAG_POS),
        INT64_NUMBER_FIELD_FLAG = NUMBER_FIELD_FLAG | (1LLU << INT64_NUMBER_FIELD_FLAG_POS),
        FLOAT_NUMBER_FIELD_FLAG = NUMBER_FIELD_FLAG | (1LLU << FLOAT_NUMBER_FIELD_FLAG_POS),
        DOUBLE_NUMBER_FIELD_FLAG = NUMBER_FIELD_FLAG | (1LLU << DOUBLE_NUMBER_FIELD_FLAG_POS),

        // 作为二进制字段的属性
        VARBINARY_FIELD_FLAG = BINARY_FIELD_FLAG | (1LLU << VARBINARY_FIELD_FLAG_POS),
        BLOB_BINARY_FIELD_FLAG = BLOB_TYPE_BINARY_FIELD_FLAG | (1LLU << BLOB_BINARY_FIELD_FLAG_POS),
        MEDIUM_BLOB_BINARY_FIELD_FLAG = BLOB_TYPE_BINARY_FIELD_FLAG | (1LLU << MEDIUM_BLOB_BINARY_FIELD_FLAG_POS),
        LONG_BLOB_BINARY_FIELD_FLAG = BLOB_TYPE_BINARY_FIELD_FLAG | (1LLU << LONG_BLOB_BINARY_FIELD_FLAG_POS),

        // 作为普通字符串
        NORMAL_STRING_FIELD_FLAG = STRING_FIELD_FLAG | (1LLU << NORMAL_STRING_FIELD_FLAG_POS),

        // 作为大字符串字段的属性
        NORMAL_TEXT_STRING_FIELD_FLAG = TEXT_STRING_FIELD_FLAG | (1LLU << NORMAL_TEXT_STRING_FIELD_FLAG_POS),
        MEDIUM_TEXT_STRING_FIELD_FLAG = TEXT_STRING_FIELD_FLAG | (1LLU << MEDIUM_TEXT_STRING_FIELD_FLAG_POS),
        LONG_TEXT_STRING_FIELD_FLAG = TEXT_STRING_FIELD_FLAG | (1LLU << LONG_TEXT_STRING_FIELD_FLAG_POS),

        // 是否是某个系统的存储数据
        SYSTEM_DATA_STORAGE_FLAG = 1LLU << SYSTEM_DATA_STORAGE_FLAG_POS,

        // 作为主键
        PRIMARY_FIELD_FLAG = AS_FIELD_FLAG | (1LLU << PRIMARY_FIELD_FLAG_POS),

        // 启动时候加载数据
        LOAD_DATA_ON_STARTUP_FLAG = (1LLU << LOAD_DATA_ON_STARTUP_FLAG_POS),
    };

    // 获取字符串类型的flags
    static bool GetStringFieldFlagsBySize(UInt64 sz, bool onlyText, bool onlyVarchar, UInt64 &flags);
    // 获取二进制类型的flags
    static bool GetBinaryFieldFlagsBySize(UInt64 sz, UInt64 &flags);
    // 生成flags
    static bool GenFlagsByMysqlDataType(const KERNEL_NS::LibString &dataType, UInt64 &flags);
    // 添加flag
    static UInt64 AddFlags(UInt64 origin, UInt64 addFlags);
    static UInt64 ClearFlags(UInt64 origin, UInt64 addFlags);

    // 是否是number
    static bool IsNumber(UInt64 flags);
    static bool IsString(UInt64 flags);
    static bool IsNormalString(UInt64 flags);
    static bool IsTextString(UInt64 flags);
    static bool IsBinary(UInt64 flags);
    static bool IsVarBinary(UInt64 flags);
    static bool IsBlobTypeBinary(UInt64 flags);

    // 比较大小 l > r
    static bool IsMysqlDataTypeNumberBiggerEq(const KERNEL_NS::LibString &l, const KERNEL_NS::LibString &r);
    // 比较大小 l > r
    static bool IsMysqlDataTypeNormalStringBiggerEq(UInt64 lCapacitySize, UInt64 rCapcitySize);
    // 比较大小 l > r
    static bool IsMysqlDataTypeTextBiggerEq(const KERNEL_NS::LibString &l, const KERNEL_NS::LibString &r);
    // 比较大小 l > r
    static bool IsMysqlDataTypeNoamalBinaryBiggerEq(UInt64 lCapacitySize, UInt64 rCapcitySize);
    // 比较大小 l > r
    static bool IsMysqlDataTypeBlobTypeBinaryBiggerEq(const KERNEL_NS::LibString &l, const KERNEL_NS::LibString &r);

    // 更新数值
    static bool UpdateNumberStorageInfo(IStorageInfo *storageInfo, const KERNEL_NS::LibString &dataType);
    static bool UpdateNormalStringStorageInfo(IStorageInfo *storageInfo, UInt64 newCapacitySize);
    static bool UpdateTextStringStorageInfo(IStorageInfo *storageInfo, const KERNEL_NS::LibString &dataType);
    static bool UpdateNormalBinaryStorageInfo(IStorageInfo *storageInfo, UInt64 newCapacitySize);
    static bool UpdateBlobTypeBinaryStorageInfo(IStorageInfo *storageInfo, const KERNEL_NS::LibString &dataType);

    // 检测系统是否支持(只支持mysql部分数据类型)
    static bool CheckCanSupportMysqlDataType(const KERNEL_NS::LibString &dataType);
};

ALWAYS_INLINE UInt64 StorageFlagType::AddFlags(UInt64 origin, UInt64 addFlags)
{
    return origin |= addFlags;
} 

ALWAYS_INLINE UInt64 StorageFlagType::ClearFlags(UInt64 origin, UInt64 addFlags)
{
    return origin &= (~addFlags);
}

ALWAYS_INLINE bool StorageFlagType::IsNumber(UInt64 flags)
{
    return (flags & NUMBER_FIELD_FLAG) == NUMBER_FIELD_FLAG;
}

ALWAYS_INLINE bool StorageFlagType::IsString(UInt64 flags)
{
    return (flags & STRING_FIELD_FLAG) == STRING_FIELD_FLAG;
}

ALWAYS_INLINE bool StorageFlagType::IsNormalString(UInt64 flags)
{
    return (flags & NORMAL_STRING_FIELD_FLAG) == NORMAL_STRING_FIELD_FLAG;
}

ALWAYS_INLINE bool StorageFlagType::IsTextString(UInt64 flags)
{
    return (flags & TEXT_STRING_FIELD_FLAG) == TEXT_STRING_FIELD_FLAG;
}

ALWAYS_INLINE bool StorageFlagType::IsBinary(UInt64 flags)
{
    return (flags & BINARY_FIELD_FLAG) == BINARY_FIELD_FLAG;
}

ALWAYS_INLINE bool StorageFlagType::IsVarBinary(UInt64 flags)
{
    return (flags & VARBINARY_FIELD_FLAG) == VARBINARY_FIELD_FLAG;
}

ALWAYS_INLINE bool StorageFlagType::IsBlobTypeBinary(UInt64 flags)
{
    return (flags & BLOB_TYPE_BINARY_FIELD_FLAG) == BLOB_TYPE_BINARY_FIELD_FLAG;
}

ALWAYS_INLINE bool StorageFlagType::IsMysqlDataTypeNumberBiggerEq(const KERNEL_NS::LibString &l, const KERNEL_NS::LibString &r)
{
    if((l == "TINYINT") && ((r == "SMALLINT") || (r == "INT") || (r == "BIGINT") || (r == "FLOAT") || (r == "DOUBLE")))
        return false;

    if((l == "SMALLINT") && ((r == "INT") || (r == "BIGINT") || (r == "FLOAT") || (r == "DOUBLE")))
        return false;

    if((l == "INT") && ((r == "BIGINT") || (r == "FLOAT") || (r == "DOUBLE")))
        return false;

    if((l == "BIGINT") && ((r == "DOUBLE")))
        return false;

    return true;
}

ALWAYS_INLINE bool StorageFlagType::IsMysqlDataTypeNormalStringBiggerEq(UInt64 lCapacitySize, UInt64 rCapcitySize)
{
    return lCapacitySize > rCapcitySize;
}

ALWAYS_INLINE bool StorageFlagType::IsMysqlDataTypeTextBiggerEq(const KERNEL_NS::LibString &l, const KERNEL_NS::LibString &r)
{
    if((l == "TEXT") && ((r == "MEDIUMTEXT") || (r == "LONGTEXT")))
        return false;

    if((l == "MEDIUMTEXT") && ((r == "LONGTEXT")))
        return false;

    return true;
}

ALWAYS_INLINE bool StorageFlagType::IsMysqlDataTypeNoamalBinaryBiggerEq(UInt64 lCapacitySize, UInt64 rCapcitySize)
{
    return lCapacitySize > rCapcitySize;
}

ALWAYS_INLINE bool StorageFlagType::IsMysqlDataTypeBlobTypeBinaryBiggerEq(const KERNEL_NS::LibString &l, const KERNEL_NS::LibString &r)
{
    if((l == "BLOB") && ((r == "MEDIUMBLOB") || (r == "LONGBLOB")))
        return false;

    if((l == "MEDIUMBLOB") && ((r == "LONGBLOB")))
        return false;

    return true;
}

class IStorageInfo : public KERNEL_NS::CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, IStorageInfo);

public:
    IStorageInfo(const KERNEL_NS::LibString &systemName);
    virtual ~IStorageInfo();

    virtual bool RegisterStorages() { return true; }
    virtual void Release() override;

    template<typename CallbackType>
    ALWAYS_INLINE void SetRelease(CallbackType &&cb)
    {
        auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void);
        if(UNLIKELY(_releaseCb))
            _releaseCb->Release();
        _releaseCb = delg;
    }

    template<typename StorageFactory>
    bool RegisterStorage();

    virtual Int32 ManualStart();
    virtual void ManualClose();

    const std::vector<IStorageInfo *> &GetSubStorageInfos() const;
    std::vector<IStorageInfo *> &GetSubStorageInfos();
    // key:obj name
    const std::unordered_map<KERNEL_NS::LibString, IStorageInfo *> &GetSubStorageDict() const;
    // key:obj name
    std::unordered_map<KERNEL_NS::LibString, IStorageInfo *> &GetSubStorageDict();

    IStorageInfo *GetSubStorageByFieldName(const KERNEL_NS::LibString &fieldName);
    const IStorageInfo *GetSubStorageByFieldName(const KERNEL_NS::LibString &fieldName) const;
    IStorageInfo *GetSubStorageBySysObjName(const KERNEL_NS::LibString &systemObjName);
    const IStorageInfo *GetSubStorageBySysObjName(const KERNEL_NS::LibString &systemObjName) const;

    bool HasSubStorages() const;

    const KERNEL_NS::LibString &GetSystemName() const;

    // 作为表时候用
    void SetTableName(const KERNEL_NS::LibString &tableName);
    const KERNEL_NS::LibString &GetTableName() const;
    const IStorageInfo *GetKeyStorage() const;
    IStorageInfo *GetKeyStorage();

    // 作为字段时候使用
    void SetFieldName(const KERNEL_NS::LibString &name);
    const KERNEL_NS::LibString &GetFieldName() const;
    void SetCapacitySize(UInt64 sz);
    UInt64 GetCapacitySize() const;

    void AddFlags(UInt64 addFlags);
    void SetFlags(UInt64 flags);
    void ClearFlags(UInt64 flags);
    void ClearFlags();
    bool HasFlags(UInt64 flags) const;

    // 存储引擎类型判断
    bool IsUsingMysql() const;
    bool IsUsingRedis() const;

    // 作为字段或者是多字段系统
    bool IsMultiFieldSystem() const;
    // kv系统属性（kv系统会被自动注册key, value两个字段的存储信息）
    bool IsKvSystem() const;
    bool IsUsingNumberKey() const;
    bool IsUsingStringKey() const;

    // key是否需要自动创建
    bool IsNeedStringKey() const;
    bool IsNeedNumberKey() const;

    // 作为字段相关属性
    bool IsAsField() const;
    bool IsNumberField() const;
    bool IsStringField() const;
    bool IsBinaryField() const;
    bool IsTextField() const;

    // 数值字段特性
    bool IsUnsignedField() const;
    bool IsInt8NumberField() const;
    bool IsInt16NumberField() const;
    bool IsInt32NumberField() const;
    bool IsInt64NumberField() const;
    bool IsFloatNumberField() const;
    bool IsDoubleNumberField() const;

    // 二进制字段特性
    bool IsVarBinaryField() const;
    bool IsBlobTypeBinaryField() const;
    bool IsBlobBinaryField() const;
    bool IsMediumBlobBinaryField() const;
    bool IsLongBlobBinaryField() const;

    // 字符串特性
    bool IsNormalStringField() const;
    bool IsNormalTextStringField() const;
    bool IsMediumTextStringField() const;
    bool IsLongTextStringField() const;

    // 是否主键
    bool IsPrimaryField() const;

    // 是否是某个系统的存储数据(如果是,则可以使用SystemName去索引到该系统)
    bool IsSystemDataStorage() const;

    // 是否启动时候加载数据
    bool IsLoadDataOnStartup() const;

    // 注释
    void SetComment(const KERNEL_NS::LibString &comment);
    const KERNEL_NS::LibString &GetComment() const;

    // 设置mysql type类型
    void SetInputMysqlDataType(Int32 type);
    Int32 GetInputMysqlDataType() const;
    void SetOutputMysqlDataType(Int32 type);
    Int32 GetOutputMysqlDataType() const;

    bool AddStorageInfo(IStorageInfo *storageInfo);
    bool AddStorageInfo(const std::vector<IStorageInfo *> &storageInfos);
    bool AddStorageInfo(StorageFactory *factory);
    bool AddStorageInfo(const std::vector<StorageFactory *> &factorys);
    void RemoveAllSubStorage();

    KERNEL_NS::LibString ToString() const override;

protected:
    virtual Int32 _OnInit() override;
    virtual Int32 _OnStart() override;
    virtual void _OnWillClose() override;
    virtual void _OnClose() override;
    
    void _UpadteCapacityAndMysqlDataType();

    void _Release(IStorageInfo *storageInfo);
    void _Release(const std::vector<IStorageInfo *> &storageInfo);

    // 需要存储的系统的类名
    const KERNEL_NS::LibString _systemName;
    KERNEL_NS::LibString _tableName;
    KERNEL_NS::LibString _comment;

    // 属性
    UInt64 _flags;

    // 作为表时候使用
    std::vector<IStorageInfo *> _subStorageInfos;
    std::unordered_map<KERNEL_NS::LibString, IStorageInfo *> _objNameRefStorageInfo;
    std::unordered_map<KERNEL_NS::LibString, IStorageInfo *> _fieldNameRefStorageInfo;
    IStorageInfo *_keyStorage;

    // 作为字段时的名字
    KERNEL_NS::LibString _fieldName;
    UInt64 _capacitySize;
    Int32 _inputMysqlDataType;   // enum_field_types
    Int32 _outputMysqlDataType;   // enum_field_types

    KERNEL_NS::IDelegate<void> *_releaseCb;
};

template<typename StorageFactory>
ALWAYS_INLINE bool IStorageInfo::RegisterStorage()
{
    auto newFactory = StorageFactory::FactoryCreate();
    if(!AddStorageInfo(newFactory))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("AddStorageInfo fail current system :%s"), GetSystemName().c_str());
        return false;
    }

    return true;   
}

ALWAYS_INLINE const std::vector<IStorageInfo *> &IStorageInfo::GetSubStorageInfos() const
{
    return _subStorageInfos;
}

ALWAYS_INLINE std::vector<IStorageInfo *> &IStorageInfo::GetSubStorageInfos()
{
    return _subStorageInfos;
}

ALWAYS_INLINE const std::unordered_map<KERNEL_NS::LibString, IStorageInfo *> &IStorageInfo::GetSubStorageDict() const
{
    return _objNameRefStorageInfo;
}

ALWAYS_INLINE std::unordered_map<KERNEL_NS::LibString, IStorageInfo *> &IStorageInfo::GetSubStorageDict()
{
    return _objNameRefStorageInfo;
}

ALWAYS_INLINE IStorageInfo *IStorageInfo::GetSubStorageByFieldName(const KERNEL_NS::LibString &fieldName)
{
    auto iter = _fieldNameRefStorageInfo.find(fieldName);
    return iter == _fieldNameRefStorageInfo.end() ? NULL : iter->second;
}

ALWAYS_INLINE const IStorageInfo *IStorageInfo::GetSubStorageByFieldName(const KERNEL_NS::LibString &fieldName) const
{
    auto iter = _fieldNameRefStorageInfo.find(fieldName);
    return iter == _fieldNameRefStorageInfo.end() ? NULL : iter->second;
}

ALWAYS_INLINE IStorageInfo *IStorageInfo::GetSubStorageBySysObjName(const KERNEL_NS::LibString &systemObjName)
{
    auto iter = _objNameRefStorageInfo.find(systemObjName);
    return iter == _objNameRefStorageInfo.end() ? NULL : iter->second;
}

ALWAYS_INLINE const IStorageInfo *IStorageInfo::GetSubStorageBySysObjName(const KERNEL_NS::LibString &systemObjName) const
{
    auto iter = _objNameRefStorageInfo.find(systemObjName);
    return iter == _objNameRefStorageInfo.end() ? NULL : iter->second;
}

ALWAYS_INLINE bool IStorageInfo::HasSubStorages() const
{
    return !_subStorageInfos.empty();
}

ALWAYS_INLINE const KERNEL_NS::LibString &IStorageInfo::GetSystemName() const
{
    return _systemName;
}

ALWAYS_INLINE void IStorageInfo::SetTableName(const KERNEL_NS::LibString &tableName)
{
    _tableName = tableName;
}

ALWAYS_INLINE const KERNEL_NS::LibString &IStorageInfo::GetTableName() const
{
    return _tableName;
}

ALWAYS_INLINE const IStorageInfo *IStorageInfo::GetKeyStorage() const
{
    return _keyStorage;
}

ALWAYS_INLINE IStorageInfo *IStorageInfo::GetKeyStorage()
{
    return _keyStorage;
}

ALWAYS_INLINE void IStorageInfo::SetFieldName(const KERNEL_NS::LibString &name)
{
    _fieldName = name;
}

ALWAYS_INLINE const KERNEL_NS::LibString &IStorageInfo::GetFieldName() const
{
    return _fieldName;
}

ALWAYS_INLINE void IStorageInfo::SetCapacitySize(UInt64 sz)
{
    _capacitySize = sz;
}

ALWAYS_INLINE UInt64 IStorageInfo::GetCapacitySize() const
{
    return _capacitySize;
}

ALWAYS_INLINE void IStorageInfo::AddFlags(UInt64 addFlags)
{
    _flags |= addFlags;

    _UpadteCapacityAndMysqlDataType();
}

ALWAYS_INLINE void IStorageInfo::SetFlags(UInt64 flags)
{
    _flags = flags;

    _UpadteCapacityAndMysqlDataType();
}

ALWAYS_INLINE void IStorageInfo::ClearFlags(UInt64 flags)
{
    _flags &= (~flags);
}

ALWAYS_INLINE void IStorageInfo::ClearFlags()
{
    _flags = StorageFlagType::NO_FLAGS;
}

ALWAYS_INLINE bool IStorageInfo::HasFlags(UInt64 flags) const
{
    return  (_flags & flags) == flags;
}

ALWAYS_INLINE bool IStorageInfo::IsUsingMysql() const
{
    return HasFlags(StorageFlagType::MYSQL_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsUsingRedis() const
{
    return HasFlags(StorageFlagType::REDIS_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsMultiFieldSystem() const
{
    return HasFlags(StorageFlagType::MULTI_FIELD_SYSTEM_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsKvSystem() const
{
    return HasFlags(StorageFlagType::KEY_VALUE_SYSTEM_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsUsingNumberKey() const
{
    if(UNLIKELY(!_keyStorage))
        return false;

    return _keyStorage->IsNumberField();
}

ALWAYS_INLINE bool IStorageInfo::IsUsingStringKey() const
{
    if(UNLIKELY(!_keyStorage))
        return false;

    return _keyStorage->IsStringField();
}

ALWAYS_INLINE bool IStorageInfo::IsNeedStringKey() const
{
    return HasFlags(StorageFlagType::NEED_STRING_KEY_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsNeedNumberKey() const
{
    return HasFlags(StorageFlagType::NEED_NUMBER_KEY_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsAsField() const
{
    return HasFlags(StorageFlagType::AS_FIELD_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsNumberField() const
{
    return HasFlags(StorageFlagType::NUMBER_FIELD_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsStringField() const
{
    return HasFlags(StorageFlagType::STRING_FIELD_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsBinaryField() const
{
    return HasFlags(StorageFlagType::BINARY_FIELD_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsTextField() const
{
    return HasFlags(StorageFlagType::TEXT_STRING_FIELD_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsUnsignedField() const
{
    return HasFlags(StorageFlagType::UNSIGNED_NUMBER_FIELD_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsInt8NumberField() const
{
    return HasFlags(StorageFlagType::INT8_NUMBER_FIELD_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsInt16NumberField() const
{
    return HasFlags(StorageFlagType::INT16_NUMBER_FIELD_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsInt32NumberField() const
{
    return HasFlags(StorageFlagType::INT32_NUMBER_FIELD_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsInt64NumberField() const
{
    return HasFlags(StorageFlagType::INT64_NUMBER_FIELD_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsFloatNumberField() const
{
    return HasFlags(StorageFlagType::FLOAT_NUMBER_FIELD_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsDoubleNumberField() const
{
    return HasFlags(StorageFlagType::DOUBLE_NUMBER_FIELD_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsVarBinaryField() const
{
    return HasFlags(StorageFlagType::VARBINARY_FIELD_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsBlobTypeBinaryField() const
{
    return HasFlags(StorageFlagType::BLOB_TYPE_BINARY_FIELD_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsBlobBinaryField() const
{
    return HasFlags(StorageFlagType::BLOB_BINARY_FIELD_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsMediumBlobBinaryField() const
{
    return HasFlags(StorageFlagType::MEDIUM_BLOB_BINARY_FIELD_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsLongBlobBinaryField() const
{
    return HasFlags(StorageFlagType::LONG_BLOB_BINARY_FIELD_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsNormalStringField() const
{
    return HasFlags(StorageFlagType::NORMAL_STRING_FIELD_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsNormalTextStringField() const
{
    return HasFlags(StorageFlagType::NORMAL_TEXT_STRING_FIELD_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsMediumTextStringField() const
{
    return HasFlags(StorageFlagType::MEDIUM_TEXT_STRING_FIELD_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsLongTextStringField() const
{
    return HasFlags(StorageFlagType::LONG_TEXT_STRING_FIELD_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsPrimaryField() const
{
    return HasFlags(StorageFlagType::PRIMARY_FIELD_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsSystemDataStorage() const
{
    return HasFlags(StorageFlagType::SYSTEM_DATA_STORAGE_FLAG);
}

ALWAYS_INLINE bool IStorageInfo::IsLoadDataOnStartup() const
{
    return HasFlags(StorageFlagType::LOAD_DATA_ON_STARTUP_FLAG);
}

ALWAYS_INLINE void IStorageInfo::SetComment(const KERNEL_NS::LibString &comment)
{
    _comment = comment;
}

ALWAYS_INLINE const KERNEL_NS::LibString &IStorageInfo::GetComment() const
{
    return _comment;
}

ALWAYS_INLINE void IStorageInfo::SetInputMysqlDataType(Int32 type)
{
    _inputMysqlDataType = type;
}

ALWAYS_INLINE Int32 IStorageInfo::GetInputMysqlDataType() const
{
    return _inputMysqlDataType;
}

ALWAYS_INLINE void IStorageInfo::SetOutputMysqlDataType(Int32 type)
{
    _outputMysqlDataType = type;
}

ALWAYS_INLINE Int32 IStorageInfo::GetOutputMysqlDataType() const
{
    return _outputMysqlDataType;
}

ALWAYS_INLINE bool IStorageInfo::AddStorageInfo(IStorageInfo *storageInfo)
{
    if(UNLIKELY(storageInfo == this))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("cant add self storage info system name:%s"), storageInfo->GetSystemName().c_str());
        return false;
    }

    auto iter = _objNameRefStorageInfo.find(storageInfo->GetSystemName());
    if(UNLIKELY(iter != _objNameRefStorageInfo.end()))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("storage info already exists system name:%s, current system name:%s"), storageInfo->GetSystemName().c_str(), GetSystemName().c_str());
        return false;
    }

    auto iterFieldName = _fieldNameRefStorageInfo.find(storageInfo->GetFieldName());
    if(UNLIKELY(iterFieldName != _fieldNameRefStorageInfo.end()))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("storage info already exists, current system name:%s, exists system name:%s, will add system name:%s field name:%s")
            , GetSystemName().c_str(), storageInfo->GetSystemName().c_str(), storageInfo->GetFieldName().c_str());
        return false;
    }

    _subStorageInfos.push_back(storageInfo);
    _objNameRefStorageInfo.insert(std::make_pair(storageInfo->GetSystemName(), storageInfo));
    _fieldNameRefStorageInfo.insert(std::make_pair(storageInfo->GetFieldName(), storageInfo));

    AddFlags(StorageFlagType::MULTI_FIELD_SYSTEM_FLAG);

    if(storageInfo->IsPrimaryField())
        _keyStorage = storageInfo;

    return true;
}

ALWAYS_INLINE bool IStorageInfo::AddStorageInfo(const std::vector<IStorageInfo *> &storageInfos)
{
    std::vector<IStorageInfo *> toAdd;
    for(auto storageInfo : storageInfos)
    {
        if(storageInfo == this)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("cant add self system name:%s"), storageInfo->GetSystemName().c_str());
            continue;
        }

        toAdd.push_back(storageInfo);
    }

    const Int32 count = static_cast<Int32>(toAdd.size());
    if(UNLIKELY(count == 0))
        return false;

    for(Int32 idx = count - 1; idx >= 0; --idx)
    {
        auto storageInfo = toAdd[idx];
        if(!AddStorageInfo(storageInfo))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("add storage fail system name:%s, current system name:%s")
                    , storageInfo->GetSystemName().c_str(), GetSystemName().c_str());
            toAdd.erase(toAdd.begin() + idx);
        }
    }
    
    return !toAdd.empty();
}

ALWAYS_INLINE bool IStorageInfo::AddStorageInfo(StorageFactory *factory)
{
    auto storageInfo = factory->Create();
    factory->Release();

    if(!AddStorageInfo(storageInfo))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("add storage info fail:%s, current system name:%s")
                    , storageInfo->GetSystemName().c_str(), GetSystemName().c_str());

        return false;
    }

    return true;
}

ALWAYS_INLINE bool IStorageInfo::AddStorageInfo(const std::vector<StorageFactory *> &factorys)
{
    std::vector<IStorageInfo *> storageInfos;
    for(auto factory : factorys)
    {
        storageInfos.push_back(factory->Create());
        factory->Release();
    }

    return AddStorageInfo(storageInfos);
}

ALWAYS_INLINE void IStorageInfo::RemoveAllSubStorage()
{
    KERNEL_NS::ContainerUtil::DelContainer2(_subStorageInfos);
    _objNameRefStorageInfo.clear();
    _fieldNameRefStorageInfo.clear();
    _keyStorage = NULL;
}



SERVICE_END

