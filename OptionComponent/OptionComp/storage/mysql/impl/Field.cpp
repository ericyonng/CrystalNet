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

#include <pch.h>
#include <OptionComp/storage/mysql/impl/Record.h>
#include <OptionComp/storage/mysql/impl/Field.h>
#include <kernel/comp/LibStream.h>
#include <mysql.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(Field);

Field::Field(const LibString &tableName, const LibString &name, Int32 dataType, UInt64 flags, Record *owner)
:_owner(owner)
,_index(-1)
,_name(name)
,_tableName(tableName)
,_data(LibStream<_Build::TL>::NewThreadLocal_LibStream())
,_dataType(dataType)
,_release(NULL)
,_isNull(true)
,_isUnsigned(false)
,_isAutoIncField(false)
,_isPrimaryKey(false)
,_flags(flags)
,_dataFlags(0)
{
    _UpdateDataFlags();
}

Field::~Field()
{
    if(_data)
    {
        LibStream<_Build::TL>::DeleteThreadLocal_LibStream(_data);
        _data = NULL;
    }

    if(_release)
    {
        _release->Release();
        _release = NULL;
    }
}

Field::Field(const Field &other)
{
    _owner = other._owner;
    _index = other._index;
    _name = other._name;
    _tableName = other._tableName;
    _data = LibStream<_Build::TL>::NewThreadLocal_LibStream();
    *_data = *(other._data);
    _dataType = other._dataType;
    _release = other._release->CreateNewCopy();
    _isNull = other._isNull;
    _isUnsigned = other._isUnsigned;
    _isAutoIncField = other._isAutoIncField;
    _isPrimaryKey = other._isPrimaryKey;
    _flags = other._flags;
    _dataFlags = other._dataFlags;
}

Field::Field(Field &&other)
{
    _owner = other._owner;
    other._owner = NULL;

    _index = other._index;
    other._index = -1;

    _name.Swap(other._name);
    _tableName.Swap(other._tableName);
    
    _data = other._data;
    other._data = NULL;

    _dataType = other._dataType;
    other._dataType = MYSQL_TYPE_NULL;
    
    _release = other._release;
    other._release = NULL;

    _isNull = other._isNull;
    other._isNull = true;

    _isUnsigned = other._isUnsigned;
    other._isUnsigned = false;

    _isAutoIncField = other._isAutoIncField;
    other._isAutoIncField = false;

    _isPrimaryKey = other._isPrimaryKey;
    other._isPrimaryKey = false;

    _flags = other._flags;
    other._flags = 0;

    _dataFlags = other._dataFlags;
    other._dataFlags = 0;
}

void Field::Write(const void *data, Int64 dataSize)
{
    if(UNLIKELY(!data || dataSize <= 0))
        return;

    if(UNLIKELY(!_data))
    {
        // 默认4个字节空间
        _data = LibStream<_Build::TL>::NewThreadLocal_LibStream();
        _data->Init(static_cast<Int64>(sizeof(Int32)));
    }

    _data->Write(data, dataSize);

    SetIsNull(false);
}

LibString Field::GetValueTextCompatible() const
{
    if(UNLIKELY(!_data || (_dataType == MYSQL_TYPE_NULL)))
        return "";

    LibStream<_Build::TL> attach; 
    attach.Attach(*_data);

    LibString data;
    switch (_dataType)
    {
    case MYSQL_TYPE_TINY:
    {// signed char
        if(IsUnsigned())
        {
            auto value = attach.ReadUInt8();
            data.AppendFormat("%u", value);
        }
        else
        {
            auto value = attach.ReadInt8();
            data.AppendFormat("%d", value);
        }
    }
    break;
    case MYSQL_TYPE_SHORT:
    {
        if(IsUnsigned())
        {
            auto value = attach.ReadUInt16();
            data.AppendFormat("%hu", value);
        }
        else
        {
            auto value = attach.ReadInt16();
            data.AppendFormat("%hd", value);
        }
    }break;
    case MYSQL_TYPE_LONG:
    {
        if(IsUnsigned())
        {
            auto value = attach.ReadUInt32();
            data.AppendFormat("%u", value);
        }
        else
        {
            auto value = attach.ReadInt32();
            data.AppendFormat("%d", value);
        }
    }break;
    case MYSQL_TYPE_FLOAT:
    {
        auto value = attach.ReadFloat();
        data.AppendFormat("%f", value);
    }break;
    case MYSQL_TYPE_DOUBLE:
    {
        auto value = attach.ReadDouble();
        data.AppendFormat("%lf", value);
    }break;
    case MYSQL_TYPE_LONGLONG:
    {
        if(IsUnsigned())
        {
            auto value = attach.ReadUInt64();
            data.AppendFormat("%llu", value);
        }
        else
        {
            auto value = attach.ReadInt64();
            data.AppendFormat("%lld", value);
        }
    }break;
    case MYSQL_TYPE_INT24:
    {
        if(IsUnsigned())
        {
            auto value = attach.ReadUInt32();
            data.AppendFormat("%u", value);
        }
        else
        {
            auto value = attach.ReadInt32();
            data.AppendFormat("%d", value);
        }
    }break;
    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_DATETIME:
    {
        GetDateTime(data);

    auto buffer = reinterpret_cast<Byte8 *>(KERNEL_ALLOC_MEMORY_TL((data.size() * 2 + 1)));
        ::memset(buffer, 0, static_cast<size_t>(data.size() * 2 + 1));
        mysql_escape_string(buffer, data.data(), static_cast<ULong>(data.size()));
        data.clear();
        data.AppendData(buffer, ::strlen(buffer));
        KERNEL_FREE_MEMORY_TL(buffer);
    }break;
    case MYSQL_TYPE_JSON:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_STRING:
    {
        auto buffer = reinterpret_cast<Byte8 *>(KERNEL_ALLOC_MEMORY_TL((attach.GetReadableSize() * 2 + 1)));
        ::memset(buffer, 0, static_cast<size_t>(attach.GetReadableSize() * 2 + 1));
        mysql_escape_string(buffer, attach.GetReadBegin(), static_cast<ULong>(attach.GetReadableSize()));
        data.clear();
        data.AppendData(buffer, ::strlen(buffer));
        KERNEL_FREE_MEMORY_TL(buffer);
    }break;
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
    {// 二进制的打印成hex格式
        data.AppendFormat("[HEX DATA]:\n");
        StringUtil::ToHexStringView(attach.GetReadBegin(), attach.GetReadableSize(), data);
    }break;
    default:
    {
        data.AppendFormat("[UNKNOWN DATA TYPE:%d %s HEX DATA]:", _dataType, DataTypeString(_dataType));
        StringUtil::ToHexStringView(attach.GetReadBegin(), attach.GetReadableSize(), data);
    }break;
    }

    return data;
}

bool Field::IsUnsigned() const
{
    return _isUnsigned || ((_flags & UNSIGNED_FLAG) == UNSIGNED_FLAG);
}

Int64 Field::GetDataSize() const
{
    if(UNLIKELY(!_data))
        return 0;

    return _data->GetWriteBytes();
}

void Field::SetType(Int32 mysqlFieldType)
{
    _dataType = mysqlFieldType;
    _UpdateDataFlags();
}

const Byte8 *Field::DataTypeString(Int32 dataType)
{
    switch (dataType)
    {
    case MYSQL_TYPE_DECIMAL: return "MYSQL_TYPE_DECIMAL";
    case MYSQL_TYPE_TINY: return "MYSQL_TYPE_TINY";
    case MYSQL_TYPE_SHORT: return "MYSQL_TYPE_SHORT";
    case MYSQL_TYPE_LONG: return "MYSQL_TYPE_LONG";
    case MYSQL_TYPE_FLOAT: return "MYSQL_TYPE_FLOAT";
    case MYSQL_TYPE_DOUBLE: return "MYSQL_TYPE_DOUBLE";
    case MYSQL_TYPE_NULL: return "MYSQL_TYPE_NULL";
    case MYSQL_TYPE_TIMESTAMP: return "MYSQL_TYPE_TIMESTAMP";
    case MYSQL_TYPE_LONGLONG: return "MYSQL_TYPE_LONGLONG";
    case MYSQL_TYPE_INT24: return "MYSQL_TYPE_INT24";
    case MYSQL_TYPE_DATE: return "MYSQL_TYPE_DATE";
    case MYSQL_TYPE_TIME: return "MYSQL_TYPE_TIME";
    case MYSQL_TYPE_DATETIME: return "MYSQL_TYPE_DATETIME";
    case MYSQL_TYPE_YEAR: return "MYSQL_TYPE_YEAR";
    case MYSQL_TYPE_NEWDATE: return "MYSQL_TYPE_NEWDATE";
    case MYSQL_TYPE_VARCHAR: return "MYSQL_TYPE_VARCHAR";
    case MYSQL_TYPE_BIT: return "MYSQL_TYPE_BIT";
    case MYSQL_TYPE_TIMESTAMP2: return "MYSQL_TYPE_TIMESTAMP2";
    case MYSQL_TYPE_DATETIME2: return "MYSQL_TYPE_DATETIME2";
    case MYSQL_TYPE_TIME2: return "MYSQL_TYPE_TIME2";
    case MYSQL_TYPE_TYPED_ARRAY: return "MYSQL_TYPE_TYPED_ARRAY";
    case MYSQL_TYPE_INVALID: return "MYSQL_TYPE_INVALID";
    case MYSQL_TYPE_BOOL: return "MYSQL_TYPE_BOOL";
    case MYSQL_TYPE_JSON: return "MYSQL_TYPE_JSON";
    case MYSQL_TYPE_NEWDECIMAL: return "MYSQL_TYPE_NEWDECIMAL";
    case MYSQL_TYPE_ENUM: return "MYSQL_TYPE_ENUM";
    case MYSQL_TYPE_SET: return "MYSQL_TYPE_SET";
    case MYSQL_TYPE_TINY_BLOB: return "MYSQL_TYPE_TINY_BLOB";
    case MYSQL_TYPE_MEDIUM_BLOB: return "MYSQL_TYPE_MEDIUM_BLOB";
    case MYSQL_TYPE_LONG_BLOB: return "MYSQL_TYPE_LONG_BLOB";
    case MYSQL_TYPE_BLOB: return "MYSQL_TYPE_BLOB";
    case MYSQL_TYPE_VAR_STRING: return "MYSQL_TYPE_VAR_STRING";
    case MYSQL_TYPE_STRING: return "MYSQL_TYPE_STRING";
    case MYSQL_TYPE_GEOMETRY: return "MYSQL_TYPE_GEOMETRY";
    default:
        break;
    }

    return "UNKNOWN_DATA_TYPE";
}

void Field::SetData(LibStream<_Build::TL> *newData)
{
    if(LIKELY(_data))
        LibStream<_Build::TL>::DeleteThreadLocal_LibStream(_data);

    _data = newData;
}


// 写数据
void Field::SetInt8(Byte8 v)
{
    _data->Reset();
    _data->WriteInt8(v);
    
    _dataType = MYSQL_TYPE_TINY;
    _isNull = false;
    _isUnsigned = false;

    _UpdateDataFlags();
}

void Field::SetInt16(Int16 v)
{
    _data->Reset();
    _data->WriteInt16(v);

    _dataType = MYSQL_TYPE_SHORT;
    _isNull = false;
    _isUnsigned = false;

    _UpdateDataFlags();
}

void Field::SetInt32(Int32 v)
{
    _data->Reset();
    _data->WriteInt32(v);

    _dataType = MYSQL_TYPE_LONG;
    _isNull = false;
    _isUnsigned = false;

    _UpdateDataFlags();
}

void Field::SetInt64(Int64 v)
{
    _data->Reset();
    _data->WriteInt64(v);

    _dataType = MYSQL_TYPE_LONGLONG;
    _isNull = false;
    _isUnsigned = false;

    _UpdateDataFlags();
}

void Field::SetUInt8(U8 v)
{
    _data->Reset();
    _data->Write(&v, static_cast<Int64>(sizeof(v)));

    _dataType = MYSQL_TYPE_TINY;
    _isNull = false;
    _isUnsigned = true;

    _UpdateDataFlags();
}

void Field::SetUInt16(UInt16 v)
{
    _data->Reset();
    _data->Write(&v, static_cast<Int64>(sizeof(v)));

    _dataType = MYSQL_TYPE_SHORT;
    _isNull = false;
    _isUnsigned = true;
}

void Field::SetUInt32(UInt32 v)
{
    _data->Reset();
    _data->Write(&v, static_cast<Int64>(sizeof(v)));

    _dataType = MYSQL_TYPE_LONG;
    _isNull = false;
    _isUnsigned = true;

    _UpdateDataFlags();
}

void Field::SetUInt64(UInt64 v)
{
    _data->Reset();
    _data->Write(&v, static_cast<Int64>(sizeof(v)));

    _dataType = MYSQL_TYPE_LONGLONG;
    _isNull = false;
    _isUnsigned = true;

    _UpdateDataFlags();
}

void Field::SetFloat(Float v)
{
    _data->Reset();
    _data->Write(&v, static_cast<Int64>(sizeof(v)));

    _dataType = MYSQL_TYPE_FLOAT;
    _isNull = false;
    _isUnsigned = false;

    _UpdateDataFlags();
}

void Field::SetDouble(Double v)
{
    _data->Reset();
    _data->Write(&v, static_cast<Int64>(sizeof(v)));

    _dataType = MYSQL_TYPE_DOUBLE;
    _isNull = false;
    _isUnsigned = false;

    _UpdateDataFlags();
}

void Field::SetString(const void *str, UInt64 strLen)
{
    _data->Reset();
    _data->Write(str, static_cast<Int64>(strLen));

    _dataType = MYSQL_TYPE_VAR_STRING;
    _isNull = false;
    _isUnsigned = false;

    _UpdateDataFlags();
}

void Field::SetDatetime(const LibString &tm)
{
    KERNEL_NS::LibTime t = KERNEL_NS::LibTime::FromFmtString(tm);
    SetDatetime(t);
}

void Field::SetDatetime(const LibTime &t)
{
    MYSQL_TIME mt;
    mt.year = static_cast<decltype(mt.year)>(t.GetLocalYear());
    mt.month = static_cast<decltype(mt.month)>(t.GetLocalMonth());
    mt.day = static_cast<decltype(mt.day)>(t.GetLocalDay());
    mt.hour = static_cast<decltype(mt.hour)>(t.GetLocalHour());
    mt.minute = static_cast<decltype(mt.minute)>(t.GetLocalMinute());
    mt.second = static_cast<decltype(mt.minute)>(t.GetLocalSecond());
    mt.second_part = static_cast<decltype(mt.second_part)>(t.GetLocalMilliSecond());
    mt.time_type = MYSQL_TIMESTAMP_DATETIME;
    mt.time_zone_displacement = static_cast<decltype(mt.time_zone_displacement)>(KERNEL_NS::TimeUtil::GetTimeZone());

    _data->Reset();
    _data->Write(&mt, static_cast<Int64>(sizeof(mt)));

    _dataType = MYSQL_TYPE_DATETIME;
    _isNull = false;
    _isUnsigned = true;

    _UpdateDataFlags();
}

void Field::SetVarBinary(const void *b, UInt64 sz)
{
    _data->Reset();
    _data->Write(b, static_cast<Int64>(sz));

    _dataType = MYSQL_TYPE_BLOB;
    _isNull = false;
    _isUnsigned = false;   

    _UpdateDataFlags();
}

void Field::SetBlob(const void *p, UInt64 len)
{
    _data->Reset();
    _data->Write(p, static_cast<Int64>(len));

    _dataType = MYSQL_TYPE_BLOB;
    _isNull = false;
    _isUnsigned = false;  

    _UpdateDataFlags();
}

void Field::SetMediumBlob(const void *p, UInt64 len)
{
    _data->Reset();
    _data->Write(p, static_cast<Int64>(len));

    _dataType = MYSQL_TYPE_MEDIUM_BLOB;
    _isNull = false;
    _isUnsigned = false;

    _UpdateDataFlags();
}

void Field::SetLongBlob(const void *p, UInt64 len)
{
    _data->Reset();
    _data->Write(p, static_cast<Int64>(len));

    _dataType = MYSQL_TYPE_LONG_BLOB;
    _isNull = false;
    _isUnsigned = false;

    _UpdateDataFlags();
}

// 读数据
Byte8 Field::GetInt8() const
{
    // 必须是数值型
    if(UNLIKELY(!IsNumber()))
        return 0;

    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    if(LIKELY(streamCache.CanRead(1)))
        return streamCache.ReadInt8();

    return 0;
}

Int16 Field::GetInt16() const
{
    // 必须是数值型
    if(UNLIKELY(!IsNumber()))
        return 0;

    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    Int64 bytes = static_cast<Int64>(sizeof(Int16));
    bytes = bytes >= streamCache.GetReadableSize() ? streamCache.GetReadableSize() : bytes;

    if(UNLIKELY(bytes == 0))
        return 0;

    Int16 v = 0;
    streamCache.Read(&v, bytes);
    return v;   
}

Int32 Field::GetInt32() const
{
    // 必须是数值型
    if(UNLIKELY(!IsNumber()))
        return 0;

    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    Int64 bytes = static_cast<Int64>(sizeof(Int32));
    bytes = bytes >= streamCache.GetReadableSize() ? streamCache.GetReadableSize() : bytes;

    if(UNLIKELY(bytes == 0))
        return 0;

    Int32 v = 0;
    streamCache.Read(&v, bytes);
    return v;   
}

Int64 Field::GetInt64() const
{
    // 必须是数值型
    if(UNLIKELY(!IsNumber()))
        return 0;
        
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    Int64 bytes = static_cast<Int64>(sizeof(Int64));
    bytes = bytes >= streamCache.GetReadableSize() ? streamCache.GetReadableSize() : bytes;

    if(UNLIKELY(bytes == 0))
        return 0;

    Int64 v = 0;
    streamCache.Read(&v, bytes);
    return v;
}

U8    Field::GetUInt8() const
{
    // 必须是数值型
    if(UNLIKELY(!IsNumber()))
        return 0;

    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    if(LIKELY(streamCache.CanRead(1)))
        return streamCache.ReadUInt8();

    return 0;
}

UInt16 Field::GetUInt16() const
{
    // 必须是数值型
    if(UNLIKELY(!IsNumber()))
        return 0;

    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    Int64 bytes = static_cast<Int64>(sizeof(UInt16));
    bytes = bytes >= streamCache.GetReadableSize() ? streamCache.GetReadableSize() : bytes;

    if(UNLIKELY(bytes == 0))
        return 0;

    UInt16 v = 0;
    streamCache.Read(&v, bytes);
    return v;   
}

UInt32 Field::GetUInt32() const
{
    // 必须是数值型
    if(UNLIKELY(!IsNumber()))
        return 0;

    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    Int64 bytes = static_cast<Int64>(sizeof(UInt32));
    bytes = bytes >= streamCache.GetReadableSize() ? streamCache.GetReadableSize() : bytes;

    if(UNLIKELY(bytes == 0))
        return 0;

    UInt32 v = 0;
    streamCache.Read(&v, bytes);
    return v;   
}

UInt64 Field::GetUInt64() const
{
    // 必须是数值型
    if(UNLIKELY(!IsNumber()))
        return 0;
        
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    Int64 bytes = static_cast<Int64>(sizeof(UInt64));
    bytes = bytes >= streamCache.GetReadableSize() ? streamCache.GetReadableSize() : bytes;

    if(UNLIKELY(bytes == 0))
        return 0;

    UInt64 v = 0;
    streamCache.Read(&v, bytes);
    return v;
}

Float Field::GetFloat() const
{
    // 必须是数值型
    if(UNLIKELY(!IsNumber()))
        return 0;
        
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    Int64 bytes = static_cast<Int64>(sizeof(Float));
    bytes = bytes >= streamCache.GetReadableSize() ? streamCache.GetReadableSize() : bytes;

    if(UNLIKELY(bytes == 0))
        return 0;

    Float v = 0;
    streamCache.Read(&v, bytes);
    return v;
}

Double Field::GetDouble() const
{
    // 必须是数值型
    if(UNLIKELY(!IsNumber()))
        return 0;
        
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    Int64 bytes = static_cast<Int64>(sizeof(Double));
    bytes = bytes >= streamCache.GetReadableSize() ? streamCache.GetReadableSize() : bytes;

    if(UNLIKELY(bytes == 0))
        return 0;

    Double v = 0;
    streamCache.Read(&v, bytes);
    return v;
}

void Field::GetString(LibString &str) const
{
    if(UNLIKELY(_data->GetReadableSize() <= 0))
        return;

    str.AppendData(_data->GetReadBegin(), _data->GetReadableSize());
}

void Field::GetString(Byte8 *str, UInt64 strSize) const
{
    if(UNLIKELY(_data->GetReadableSize() <= 0))
        return;

    const auto rBytes = static_cast<UInt64>(_data->GetReadableSize());
    ::memcpy(str, _data->GetReadBegin(), strSize > rBytes ? rBytes : strSize);
}

void Field::GetDateTime(LibString &dt) const
{
    // 必须是字符串类型
    if(UNLIKELY(!IsString()))
        return;

    if(UNLIKELY(_data->GetReadableSize() <= 0))
        return;

    MYSQL_TIME mt;
    ::memset(&mt, 0, sizeof(mt));

    const auto rBytes = static_cast<UInt64>(_data->GetReadableSize());
    ::memcpy(&mt, _data->GetReadBegin(), sizeof(mt) > rBytes ? rBytes : sizeof(mt));

    const auto &t = LibTime::FromTimeMoment(mt.year, mt.month, mt.day, mt.hour, mt.minute, mt.second, mt.second_part);

    dt = t.ToStringOfMillSecondPrecision();
}

void Field::GetVarBinary(LibString &b) const
{
    if(UNLIKELY(_data->GetReadableSize() <= 0))
        return;
    
    b.AppendData(_data->GetReadBegin(), _data->GetReadableSize());
}

void Field::GetVarBinary(void *b, UInt64 sz) const
{
    if(UNLIKELY(_data->GetReadableSize() <= 0))
        return;

    const auto rBytes = static_cast<UInt64>(_data->GetReadableSize());
    ::memcpy(b, _data->GetReadBegin(), sz > rBytes ? rBytes : sz);
}

void Field::GetBlob(LibString &b) const
{
    if(UNLIKELY(_data->GetReadableSize() <= 0))
        return;

    b.AppendData(_data->GetReadBegin(), _data->GetReadableSize());
}

void Field::GetBlob(void *b, UInt64 sz) const
{
    if(UNLIKELY(_data->GetReadableSize() <= 0))
        return;

    const auto rBytes = static_cast<UInt64>(_data->GetReadableSize());
    ::memcpy(b, _data->GetReadBegin(), sz > rBytes ? rBytes : sz);
}

void Field::GetMediumBlob(LibString &b) const
{
    if(UNLIKELY(_data->GetReadableSize() <= 0))
        return;

    b.AppendData(_data->GetReadBegin(), _data->GetReadableSize());
}

void Field::GetMediumBlob(void *b, UInt64 sz) const
{
    if(UNLIKELY(_data->GetReadableSize() <= 0))
        return;

    const auto rBytes = static_cast<UInt64>(_data->GetReadableSize());
    ::memcpy(b, _data->GetReadBegin(), sz > rBytes ? rBytes : sz);
}

void Field::GetLongBlob(LibString &b) const
{
    if(UNLIKELY(_data->GetReadableSize() <= 0))
        return;

    b.AppendData(_data->GetReadBegin(), _data->GetReadableSize());
}

void Field::GetLongBlob(void *b, UInt64 sz) const
{
    if(UNLIKELY(_data->GetReadableSize() <= 0))
        return;

    const auto rBytes = static_cast<UInt64>(_data->GetReadableSize());
    ::memcpy(b, _data->GetReadBegin(), sz > rBytes ? rBytes : sz);
}

void Field::_UpdateDataFlags()
{
   // 数据类型标记
    switch (_dataType)
    {
        case MYSQL_TYPE_TINY:
        {
            if(_isUnsigned)
            {
                _dataFlags = NUMBER_FLAG | NUMBER_INT8 | UNSIGNED_FLAG;

            }
            else
            {
                _dataFlags = NUMBER_FLAG | NUMBER_INT8;
            }
        }break;
        case MYSQL_TYPE_SHORT:
        {
            if(_isUnsigned)
            {
                _dataFlags = NUMBER_FLAG | NUMBER_INT16 | UNSIGNED_FLAG;
            }
            else
            {
                _dataFlags = NUMBER_FLAG | NUMBER_INT16;
            }
        }break;
        case MYSQL_TYPE_INT24:
        case MYSQL_TYPE_LONG:
        {
            if(_isUnsigned)
            {
                _dataFlags = NUMBER_FLAG | NUMBER_INT32 | UNSIGNED_FLAG;

            }
            else
            {
                _dataFlags = NUMBER_FLAG | NUMBER_INT32;
            }
        }break;
        case MYSQL_TYPE_FLOAT:
        {
            _dataFlags = NUMBER_FLAG | NUMBER_FLOAT;
        }break;
        case MYSQL_TYPE_DOUBLE:
        {
            _dataFlags = NUMBER_FLAG | NUMBER_DOUBLE;
        }break;
        case MYSQL_TYPE_LONGLONG:
        {
            if(_isUnsigned)
            {
                _dataFlags = NUMBER_FLAG | NUMBER_INT64 | UNSIGNED_FLAG;
            }
            else
            {
                _dataFlags = NUMBER_FLAG | NUMBER_INT64;
            }

        }break;
        case MYSQL_TYPE_TIMESTAMP:
        case MYSQL_TYPE_DATE:
        case MYSQL_TYPE_TIME:
        case MYSQL_TYPE_DATETIME:
        {
            _dataFlags = TIME_STRING;
        }break;
        case MYSQL_TYPE_JSON:
        {
            _dataFlags = JSON_STRING;
        }break;
        case MYSQL_TYPE_VAR_STRING:
        {
            _dataFlags = NORMAL_BINARY_FLAG;
        }break;
        case MYSQL_TYPE_TINY_BLOB:
        {
            _dataFlags = TINY_BINARY_FLAG;
        }break;
        case MYSQL_TYPE_MEDIUM_BLOB:
        {
            _dataFlags = MEDIUM_BINARY_FLAG;
        }break;
        case MYSQL_TYPE_LONG_BLOB:
        {
            _dataFlags = LONG_BINARY_FLAG;
        }break;
        case MYSQL_TYPE_BLOB:
        {
            _dataFlags = NORMAL_BINARY_FLAG;
        }break;
        default:
            break;
    }
}



KERNEL_END