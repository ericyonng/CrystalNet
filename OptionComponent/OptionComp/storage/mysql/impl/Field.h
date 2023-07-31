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
 * Date: 2023-06-22 21:19:00
 * Author: Eric Yonng
 * Description: Mysql一行中的一个字段
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MYSQL_IMPL_FIELD_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MYSQL_IMPL_FIELD_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Delegate/Delegate.h>
#include <kernel/comp/memory/memory.h>

KERNEL_BEGIN

template<typename T>
class LibStream;

class Record;
class LibTime;

class Field
{
    POOL_CREATE_OBJ_DEFAULT(Field);

public:
    enum DATA_FLAGS_POS : UInt64
    {
        UNKNOWN_FLAG_POS = 0,
        NUMBER_FLAG_POS = 1,
        STRING_FLAG_POS = 2,
        BINARY_FLAG_POS = 3,

        // NUMBER特性
        NUMBER_INT8_POS,
        NUMBER_INT16_POS,
        NUMBER_INT32_POS,
        NUMBER_INT64_POS,
        NUMBER_UNSIGNED_POS,
        NUMBER_FLOAT_POS,
        NUMBER_DOUBLE_POS,

        // STRING
        TIME_STRING_POS,
        JSON_STRING_POS,
        NORMAL_STRING_POS,

        // BLOB
        TINY_BINARY_POS,
        NORMAL_BINARY_POS,
        MEDIUM_BINARY_POS,
        LONG_BINARY_POS,
    };

public:
    static constexpr UInt64  UNKNOWN_FLAG = 0;
    static constexpr UInt64  NUMBER_FLAG = (1LLU << Field::NUMBER_FLAG_POS);
    static constexpr UInt64  STRING_FLAG = (1LLU << Field::STRING_FLAG_POS);
    static constexpr UInt64  BINARY_FIELD_FLAG = (1LLU << Field::BINARY_FLAG_POS);

    static constexpr UInt64 NUMBER_INT8 = Field::NUMBER_FLAG | (1LLU << Field::NUMBER_INT8_POS);
    static constexpr UInt64 NUMBER_INT16 = Field::NUMBER_FLAG | (1LLU << Field::NUMBER_INT16_POS);
    static constexpr UInt64 NUMBER_INT32 = Field::NUMBER_FLAG | (1LLU << Field::NUMBER_INT32_POS);
    static constexpr UInt64 NUMBER_INT64 = Field::NUMBER_FLAG | (1LLU << Field::NUMBER_INT64_POS);
    static constexpr UInt64 NUMBER_UNSIGNED = Field::NUMBER_FLAG | (1LLU << Field::NUMBER_UNSIGNED_POS);
    static constexpr UInt64 NUMBER_FLOAT = Field::NUMBER_FLAG | (1LLU << Field::NUMBER_FLOAT_POS);
    static constexpr UInt64 NUMBER_DOUBLE = Field::NUMBER_FLAG | (1LLU << Field::NUMBER_DOUBLE_POS);

        // STRING
    static constexpr UInt64 TIME_STRING = Field::STRING_FLAG | (1LLU << Field::TIME_STRING_POS);
    static constexpr UInt64 JSON_STRING = Field::STRING_FLAG | (1LLU << Field::JSON_STRING_POS);
    static constexpr UInt64 NORMAL_STRING = Field::STRING_FLAG | (1LLU << Field::NORMAL_STRING_POS);

        // blob
    static constexpr UInt64 TINY_BINARY_FLAG = Field::BINARY_FIELD_FLAG | (1LLU << Field::TINY_BINARY_POS);
    static constexpr UInt64 NORMAL_BINARY_FLAG = Field::BINARY_FIELD_FLAG | (1LLU << Field::NORMAL_BINARY_POS);
    static constexpr UInt64 MEDIUM_BINARY_FLAG = Field::BINARY_FIELD_FLAG | (1LLU << Field::MEDIUM_BINARY_POS);
    static constexpr UInt64 LONG_BINARY_FLAG = Field::BINARY_FIELD_FLAG | (1LLU << Field::MEDIUM_BINARY_POS);

public:
    Field(const LibString &tableName, const LibString &fieldName, Int32 dataType, UInt64 flags, Record *owner = NULL);
    ~Field();
    Field(const Field &);
    Field(Field &&);

    template<typename T = _Build::TL>
    static Field *Create(const LibString &tableName, const LibString &fieldName, Int32 dataType, UInt64 flags, Record *owner = NULL);
    void Release();

    void Write(const void *data, Int64 dataSize);

    // owner
    void SetOwner(Record *owner);
    const Record *GetOwner() const;
    Record *GetOwner();

    // 数据大小
    Int64 GetDataSize() const;
    // 字段名
    const LibString &GetName() const;

    // 数据类型
    Int32 GetType() const;
    void SetType(Int32 mysqlFieldType);

    // 表名
    const LibString &GetTableName() const;

    // 字段所在行的索引
    Int32 GetIndexInRecord() const;
    bool HasIndex() const;
    void SetIndexInRecord(Int32 idx);

    // 数据
    const LibStream<_Build::TL> *GetData() const;
    LibStream<_Build::TL> *GetData();
    void SetData(LibStream<_Build::TL> *newData);

    LibString ToString() const;
    LibString Dump() const;
    // value转成文本可读的
    LibString GetValueTextCompatible() const;

    static const Byte8 *DataTypeString(Int32 dataType);
    const Byte8 *GetDataTypeString() const;

    // 数据是否为空
    bool IsNull() const;
    void SetIsNull(bool isNull);

    // 是否无符号
    bool IsUnsigned() const;
    void SetIsUnsigned(bool isUnsigned);

    // 是否自增字段
    bool IsAutoIncField() const;
    void SetAutoIncField(bool isInc);

    // 主键
    bool IsPrimaryKey() const;
    void SetIsPrimaryKey(bool isPrimaryKey);

    // 字段属性信息
    UInt64 GetFlags() const;
    void SetFlags(UInt64 flags);

    // 写数据
    void SetInt8(Byte8 v);
    void SetInt16(Int16 v);
    void SetInt32(Int32 v);
    void SetInt64(Int64 v);
    void SetUInt8(U8 v);
    void SetUInt16(UInt16 v);
    void SetUInt32(UInt32 v);
    void SetUInt64(UInt64 v);
    void SetFloat(Float v);
    void SetDouble(Double v);
    void SetString(const LibString &str);
    void SetString(const void *str, UInt64 strLen);
    void SetDatetime(const LibString &tm);
    void SetDatetime(const LibTime &tm);
    void SetVarBinary(const LibString &b);
    void SetVarBinary(const void *b, UInt64 sz);
    void SetBlob(const LibString &b);
    void SetBlob(const void *p, UInt64 len);
    void SetMediumBlob(const LibString &b);
    void SetMediumBlob(const void *p, UInt64 len);
    void SetLongBlob(const LibString &b);
    void SetLongBlob(const void *p, UInt64 len);

    // 读数据
    Byte8 GetInt8() const;
    Int16 GetInt16() const;
    Int32 GetInt32() const;
    Int64 GetInt64() const;
    U8    GetUInt8() const;
    UInt16 GetUInt16() const;
    UInt32 GetUInt32() const;
    UInt64 GetUInt64() const;
    Float GetFloat() const;
    Double GetDouble() const;
    void GetString(LibString &str) const;
    void GetString(Byte8 *str, UInt64 strSize) const;
    void GetDateTime(LibString &dt) const;
    void GetDateTime(LibTime &tm) const;
    void GetVarBinary(LibString &b) const;
    void GetVarBinary(void *b, UInt64 sz) const;
    void GetBlob(LibString &b) const;
    void GetBlob(void *b, UInt64 sz) const;
    void GetMediumBlob(LibString &b) const;
    void GetMediumBlob(void *b, UInt64 sz) const;
    void GetLongBlob(LibString &b) const;
    void GetLongBlob(void *b, UInt64 sz) const;

    bool IsNumber() const;
    bool IsString() const;
    bool IsBinary() const;

    bool IsInt8() const;
    bool IsUInt8() const;
    bool IsInt16() const;
    bool IsUInt16() const;
    bool IsInt32() const;
    bool IsUInt32() const;
    bool IsInt64() const;
    bool IsUInt64() const;
    bool IsDouble() const;
    bool IsFloat() const;

    bool IsNormalString() const;
    bool IsJsonString() const;
    bool IsTimeString() const;

    bool IsTinyBinary() const;

    // 可能是VARCHAR,也可能是VARBINARY
    bool IsNormalBinary() const;
    bool IsMediumBinary() const;
    bool IsLongBinary() const;

private:
    // 设置释放的回调
    template<typename CallbackType>
    void _SetRelease(CallbackType &&cb);
    void _SetRelease(IDelegate<void, Field *> *cb);

    // 更新flags
    void _UpdateDataFlags();

private:
    Record *_owner;
    Int32 _index;
    LibString _name;
    LibString _tableName;
    LibStream<_Build::TL> *_data;
    Int32 _dataType;        // enum_field_types
    IDelegate<void, Field *> *_release; 
    bool _isNull;
    bool _isUnsigned;
    bool _isAutoIncField;
    bool _isPrimaryKey;
    UInt64 _flags;  // 字段的属性信息

    UInt64 _dataFlags;
};

template<typename T>
ALWAYS_INLINE Field *Field::Create(const LibString &tableName, const LibString &fieldName, Int32 dataType, UInt64 flags, Record *owner)
{
    auto field = Field::NewByAdapter_Field(T::V, tableName, fieldName, dataType, flags, owner);

    // 设置释放
    field->_SetRelease([](Field *ptr){
        Field::DeleteByAdapter_Field(T::V, ptr);
    });

    return field;
}

ALWAYS_INLINE void Field::Release()
{
    if(UNLIKELY(_release))
    {
        _release->Invoke(this);
        return;
    }

    // 默认方式
    Field::DeleteThreadLocal_Field(this);
}

ALWAYS_INLINE void Field::SetOwner(Record *owner)
{
    _owner = owner;
}

ALWAYS_INLINE const Record *Field::GetOwner() const
{
    return _owner;
}

ALWAYS_INLINE Record *Field::GetOwner()
{
    return _owner;
}

ALWAYS_INLINE const LibString &Field::GetName() const
{
    return _name;
}

ALWAYS_INLINE Int32 Field::GetType() const
{
    return _dataType;
}

ALWAYS_INLINE const LibString &Field::GetTableName() const
{
    return _tableName;
}

ALWAYS_INLINE Int32 Field::GetIndexInRecord() const
{
    return _index;
}

ALWAYS_INLINE bool Field::HasIndex() const
{
    return _index >= 0;
}

ALWAYS_INLINE void Field::SetIndexInRecord(Int32 idx)
{
    _index = idx;
}

ALWAYS_INLINE const LibStream<_Build::TL> *Field::GetData() const
{
    return _data;
}

ALWAYS_INLINE LibStream<_Build::TL> *Field::GetData()
{
    return _data;
}

ALWAYS_INLINE LibString Field::ToString() const
{
    LibString info;
    info.AppendFormat("table name:%s, field name:%s, index in record:%d, data size:%lld, data type:%d,%s, is null:%d, is unsigend:%d, is auto inc field:%d, 是否主键:%d, _dataFlags:0x%llx"
    , _tableName.c_str(), _name.c_str(), _index, GetDataSize(), _dataType, DataTypeString(_dataType), _isNull, _isUnsigned, _isAutoIncField, _isPrimaryKey, _dataFlags);
    return info;
}

ALWAYS_INLINE LibString Field::Dump() const
{
    LibString info;
    info.AppendFormat("table name:%s, field name:%s, data type:%d,%s data flags:0x%llx, data size:%lld, data:\n", _tableName.c_str(), _name.c_str(), _dataType, DataTypeString(_dataType), _dataFlags, GetDataSize());
    info.AppendData(GetValueTextCompatible());

    return info;
}

ALWAYS_INLINE const Byte8 *Field::GetDataTypeString() const
{
    return DataTypeString(_dataType);
}

ALWAYS_INLINE bool Field::IsNull() const
{
    return _isNull;
}

ALWAYS_INLINE void Field::SetIsNull(bool isNull)
{
    _isNull = isNull;
}

ALWAYS_INLINE void Field::SetIsUnsigned(bool isUnsigned)
{
    _isUnsigned = isUnsigned;

    _UpdateDataFlags();
}

ALWAYS_INLINE bool Field::IsAutoIncField() const
{
    return _isAutoIncField;
}

ALWAYS_INLINE void Field::SetAutoIncField(bool isInc)
{
    _isAutoIncField = isInc;
}

ALWAYS_INLINE bool Field::IsPrimaryKey() const
{
    return _isPrimaryKey;
}

ALWAYS_INLINE void Field::SetIsPrimaryKey(bool isPrimaryKey)
{
    _isPrimaryKey = isPrimaryKey;
}

ALWAYS_INLINE UInt64 Field::GetFlags() const
{
    return _flags;
}

ALWAYS_INLINE void Field::SetFlags(UInt64 flags)
{
    _flags = flags;
}

ALWAYS_INLINE void Field::SetString(const LibString &str)
{
    SetString(str.data(), static_cast<UInt64>(str.size()));
}

ALWAYS_INLINE void Field::SetVarBinary(const LibString &b)
{
    SetVarBinary(b.data(), static_cast<UInt64>(b.size()));
}

ALWAYS_INLINE void Field::SetBlob(const LibString &b)
{
    SetBlob(b.data(), static_cast<UInt64>(b.size()));
}

ALWAYS_INLINE void Field::SetMediumBlob(const LibString &b)
{
    SetMediumBlob(b.data(), static_cast<UInt64>(b.size()));
}

ALWAYS_INLINE void Field::SetLongBlob(const LibString &b)
{
    SetLongBlob(b.data(), static_cast<UInt64>(b.size()));
}

ALWAYS_INLINE bool Field::IsNumber() const
{
    return (_dataFlags & NUMBER_FLAG) == NUMBER_FLAG;
}

ALWAYS_INLINE bool Field::IsString() const
{
    return (_dataFlags & STRING_FLAG) == STRING_FLAG;
}

ALWAYS_INLINE bool Field::IsBinary() const
{
    return (_dataFlags & BINARY_FIELD_FLAG) == BINARY_FIELD_FLAG;
}

ALWAYS_INLINE bool Field::IsInt8() const
{
    return ((_dataFlags & NUMBER_INT16) == NUMBER_INT16) && (!IsUnsigned());
}

ALWAYS_INLINE bool Field::IsUInt8() const
{
    return ((_dataFlags & NUMBER_INT16) == NUMBER_INT16) && (IsUnsigned());
}

ALWAYS_INLINE bool Field::IsInt16() const
{
    return ((_dataFlags & NUMBER_INT16) == NUMBER_INT16) && (!IsUnsigned());
}

ALWAYS_INLINE bool Field::IsUInt16() const
{
    return ((_dataFlags & NUMBER_INT16) == NUMBER_INT16) && IsUnsigned();
}

ALWAYS_INLINE bool Field::IsInt32() const
{
    return ((_dataFlags & NUMBER_INT32) == NUMBER_INT32) && (!IsUnsigned());
}

ALWAYS_INLINE bool Field::IsUInt32() const
{
    return ((_dataFlags & NUMBER_INT32) == NUMBER_INT32) && IsUnsigned();
}

ALWAYS_INLINE bool Field::IsInt64() const
{
    return ((_dataFlags & NUMBER_INT64) == NUMBER_INT64) && (!IsUnsigned());
}

ALWAYS_INLINE bool Field::IsUInt64() const
{
    return ((_dataFlags & NUMBER_INT64) == NUMBER_INT64) && IsUnsigned();
}

ALWAYS_INLINE bool Field::IsDouble() const
{
    return (_dataFlags & NUMBER_DOUBLE) == NUMBER_DOUBLE;
}

ALWAYS_INLINE bool Field::IsFloat() const
{
    return (_dataFlags & NUMBER_FLOAT) == NUMBER_FLOAT;
}

ALWAYS_INLINE bool Field::IsNormalString() const
{
    return (_dataFlags & NORMAL_STRING) == NORMAL_STRING;
}

ALWAYS_INLINE bool Field::IsJsonString() const
{
    return (_dataFlags & JSON_STRING) == JSON_STRING;
}

ALWAYS_INLINE bool Field::IsTimeString() const
{
    return (_dataFlags & TIME_STRING) == TIME_STRING;
}

ALWAYS_INLINE bool Field::IsTinyBinary() const
{
    return (_dataFlags & TINY_BINARY_FLAG) == TINY_BINARY_FLAG;
}

ALWAYS_INLINE bool Field::IsNormalBinary() const
{
    return (_dataFlags & NORMAL_BINARY_FLAG) == NORMAL_BINARY_FLAG;
}

ALWAYS_INLINE bool Field::IsMediumBinary() const
{
    return (_dataFlags & MEDIUM_BINARY_FLAG) == MEDIUM_BINARY_FLAG;
}

ALWAYS_INLINE bool Field::IsLongBinary() const
{
    return (_dataFlags & LONG_BINARY_FLAG) == LONG_BINARY_FLAG;
}

template<typename CallbackType>
ALWAYS_INLINE void Field::_SetRelease(CallbackType &&cb)
{
    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void, Field *);
    _SetRelease(delg);
}

ALWAYS_INLINE void Field::_SetRelease(IDelegate<void, Field *> *cb)
{
    if(UNLIKELY(_release))
        _release->Release();

    _release = cb;
}


KERNEL_END

#endif
