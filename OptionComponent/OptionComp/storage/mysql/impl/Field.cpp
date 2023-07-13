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

Field::Field(const LibString &tableName, const LibString &name, Int32 dataType, Record *owner)
:_owner(owner)
,_index(-1)
,_name(name)
,_tableName(tableName)
,_data(LibStream<_Build::TL>::NewThreadLocal_LibStream())
,_dataType(dataType)
,_release(NULL)
,_isNull(true)
,_isUnsigned(false)
{

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
}

Field::Field(Field &&other)
{
    _owner = other._owner;
    _index = other._index;
    _name = other._name;
    _tableName = other._tableName;
    
    _data = other._data;
    other._data = NULL;

    _dataType = other._dataType;
    
    _release = other._release;
    other._release = NULL;

    _isNull = other._isNull;
    _isUnsigned = other._isUnsigned;
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

Int64 Field::GetDataSize() const
{
    if(UNLIKELY(!_data))
        return 0;

    return _data->GetWriteBytes();
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

// 写数据
void Field::SetInt8(Byte8 v)
{
    _data->Reset();
    _data->WriteInt8(v);
    
    _dataType = MYSQL_TYPE_TINY;
    _isNull = false;
    _isUnsigned = false;
}

void Field::SetInt16(Int16 v)
{
    _data->Reset();
    _data->WriteInt16(v);

    _dataType = MYSQL_TYPE_SHORT;
    _isNull = false;
    _isUnsigned = false;
}

void Field::SetInt32(Int32 v)
{
    _data->Reset();
    _data->WriteInt32(v);

    _dataType = MYSQL_TYPE_LONG;
    _isNull = false;
    _isUnsigned = false;
}

void Field::SetInt64(Int64 v)
{
    _data->Reset();
    _data->WriteInt64(v);

    _dataType = MYSQL_TYPE_LONGLONG;
    _isNull = false;
    _isUnsigned = false;
}

void Field::SetUInt8(U8 v)
{
    _data->Reset();
    _data->Write(&v, static_cast<Int64>(sizeof(v)));

    _dataType = MYSQL_TYPE_TINY;
    _isNull = false;
    _isUnsigned = true;
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
}
void Field::SetUInt64(UInt64 v)
{
    _data->Reset();
    _data->Write(&v, static_cast<Int64>(sizeof(v)));

    _dataType = MYSQL_TYPE_LONGLONG;
    _isNull = false;
    _isUnsigned = true;
}

void Field::SetFloat(Float v)
{
    _data->Reset();
    _data->Write(&v, static_cast<Int64>(sizeof(v)));

    _dataType = MYSQL_TYPE_FLOAT;
    _isNull = false;
    _isUnsigned = false;
}

void Field::SetDouble(Double v)
{
    _data->Reset();
    _data->Write(&v, static_cast<Int64>(sizeof(v)));

    _dataType = MYSQL_TYPE_DOUBLE;
    _isNull = false;
    _isUnsigned = false;
}

void Field::SetString(const LibString &str)
{
    _data->Reset();
    _data->Write(str.data(), static_cast<Int64>(str.size()));

    _dataType = MYSQL_TYPE_VAR_STRING;
    _isNull = false;
    _isUnsigned = false;
}

void Field::SetString(const void *str, UInt64 strLen)
{
    _data->Reset();
    _data->Write(str, static_cast<Int64>(strLen));

    _dataType = MYSQL_TYPE_VAR_STRING;
    _isNull = false;
    _isUnsigned = false;
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
}

void Field::SetBlob(const LibString &b)
{
    _data->Reset();
    _data->Write(b.data(), static_cast<Int64>(b.size()));

    _dataType = MYSQL_TYPE_BLOB;
    _isNull = false;
    _isUnsigned = false;   
}

void Field::SetBlob(const void *p, UInt64 len)
{
    _data->Reset();
    _data->Write(p, static_cast<Int64>(len));

    _dataType = MYSQL_TYPE_BLOB;
    _isNull = false;
    _isUnsigned = false;  
}

void Field::SetMediumBlob(const LibString &b)
{
    _data->Reset();
    _data->Write(b.data(), static_cast<Int64>(b.size()));

    _dataType = MYSQL_TYPE_MEDIUM_BLOB;
    _isNull = false;
    _isUnsigned = false;
}

void Field::SetMediumBlob(const void *p, UInt64 len)
{
    _data->Reset();
    _data->Write(p, static_cast<Int64>(len));

    _dataType = MYSQL_TYPE_MEDIUM_BLOB;
    _isNull = false;
    _isUnsigned = false;
}

void Field::SetLongBlob(const LibString &b)
{
    _data->Reset();
    _data->Write(b.data(), static_cast<Int64>(b.size()));

    _dataType = MYSQL_TYPE_LONG_BLOB;
    _isNull = false;
    _isUnsigned = false;
}

void Field::SetLongBlob(const void *p, UInt64 len)
{
    _data->Reset();
    _data->Write(p, static_cast<Int64>(len));

    _dataType = MYSQL_TYPE_LONG_BLOB;
    _isNull = false;
    _isUnsigned = false;
}

// 读数据
Byte8 Field::GetInt8() const
{
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    auto v = streamCache.ReadInt8();
    return v;
}

Int16 Field::GetInt16() const
{
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    auto v = streamCache.ReadInt16();
    return v;   
}

Int32 Field::GetInt32() const
{
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    auto v = streamCache.ReadInt32();
    return v;   
}

Int64 Field::GetInt64() const
{
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    auto v = streamCache.ReadInt64();
    return v; 
}

U8    Field::GetUInt8() const
{
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    auto v = streamCache.ReadUInt8();
    return v; 
}

UInt16 Field::GetUInt16() const
{
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    auto v = streamCache.ReadUInt16();
    return v; 
}

UInt32 Field::GetUInt32() const
{
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    auto v = streamCache.ReadUInt32();
    return v; 
}

UInt64 Field::GetUInt64() const
{
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    auto v = streamCache.ReadUInt64();
    return v; 
}

Float Field::GetFloat() const
{
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    auto v = streamCache.ReadFloat();
    return v; 
}

Double Field::GetDouble() const
{
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    auto v = streamCache.ReadDouble();
    return v; 
}

void Field::GetString(LibString &str)
{
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    str.AppendData(streamCache.GetReadBegin(), streamCache.GetReadableSize());
}

void Field::GetString(Byte8 *str, UInt64 strSize)
{
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    const auto rBytes = static_cast<UInt64>(streamCache.GetReadableSize());
    ::memcpy(str, streamCache.GetReadBegin(), strSize > rBytes ? rBytes : strSize);
}

void Field::GetDatetime(LibString &dt)
{
    MYSQL_TIME mt;
    ::memset(&mt, 0, sizeof(mt));

    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    const auto rBytes = static_cast<UInt64>(streamCache.GetReadableSize());
    ::memcpy(&mt, streamCache.GetReadBegin(), sizeof(mt) > rBytes ? rBytes : sizeof(mt));

    const auto &t = LibTime::FromTimeMoment(mt.year, mt.month, mt.day, mt.hour, mt.minute, mt.second, mt.second_part);

    dt = t.ToStringOfMillSecondPrecision();
}

void Field::GetBlob(LibString &b)
{
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    b.AppendData(streamCache.GetReadBegin(), streamCache.GetReadableSize());
}

void Field::GetBlob(Byte8 *b, UInt64 sz)
{
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    const auto rBytes = static_cast<UInt64>(streamCache.GetReadableSize());
    ::memcpy(b, streamCache.GetReadBegin(), sz > rBytes ? rBytes : sz);
}

void Field::GetMediumBlob(LibString &b)
{
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    b.AppendData(streamCache.GetReadBegin(), streamCache.GetReadableSize());
}

void Field::GetMediumBlob(Byte8 *b, UInt64 sz)
{
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    const auto rBytes = static_cast<UInt64>(streamCache.GetReadableSize());
    ::memcpy(b, streamCache.GetReadBegin(), sz > rBytes ? rBytes : sz);
}

void Field::GetLongBlob(LibString &b)
{
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    b.AppendData(streamCache.GetReadBegin(), streamCache.GetReadableSize());
}

void Field::GetLongBlob(Byte8 *b, UInt64 sz)
{
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
    streamCache.Attach(*_data);

    const auto rBytes = static_cast<UInt64>(streamCache.GetReadableSize());
    ::memcpy(b, streamCache.GetReadBegin(), sz > rBytes ? rBytes : sz);
}

KERNEL_END