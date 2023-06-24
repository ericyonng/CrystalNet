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
,_data(NULL)
,_dataType(dataType)
,_release(NULL)
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

KERNEL_END