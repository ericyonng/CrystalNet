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
// Date: 2026-06-21 16:06:18
// Author: Eric Yonng
// Description:


#include <pch.h>
#include <OptionComp/storage/MongoDB/Impl/MongoDataSerialize.h>
#include <OptionComp/storage/MongoDB/Impl/MongoSerializeInfo.h>
#include <bsoncxx/json.hpp>

#include <kernel/comp/Coder/base64.h>
#include <string_view>
#include <cstring>

#include "kernel/comp/Log/log.h"


KERNEL_BEGIN
    bool MongoDataSerialize::AppendSerialize(bsoncxx::builder::basic::document& doc, const KERNEL_NS::LibString &keyName, const MongoSerializeInfo& data)
{
    switch (data.DataType)
    {
    case MongoSerializeInfoType::BOOL:
        {
            // 防御: stream为空或可读长度不足时直接报错, 避免空指针解引用与越界读
            if(UNLIKELY(!data._stream || data._stream->GetReadableSize() < static_cast<Int64>(sizeof(bool))))
            {
                CLOG_ERROR_GLOBAL(MongoDataSerialize, "AppendSerialize bool stream invalid key:%s, readable size:%lld"
                    , keyName.c_str(), data._stream ? data._stream->GetReadableSize() : -1);
                return false;
            }
            // memcpy避免非对齐解引用UB
            bool value = false;
            ::memcpy(&value, data._stream->GetReadBegin(), sizeof(value));
            doc.append(bsoncxx::builder::basic::kvp(keyName.GetRaw(), value));
            break;
        }
    case MongoSerializeInfoType::INT64:
        {
            if(UNLIKELY(!data._stream || data._stream->GetReadableSize() < static_cast<Int64>(sizeof(Int64))))
            {
                CLOG_ERROR_GLOBAL(MongoDataSerialize, "AppendSerialize int64 stream invalid key:%s, readable size:%lld"
                    , keyName.c_str(), data._stream ? data._stream->GetReadableSize() : -1);
                return false;
            }
            Int64 value = 0;
            ::memcpy(&value, data._stream->GetReadBegin(), sizeof(value));
            doc.append(bsoncxx::builder::basic::kvp(keyName.GetRaw(), static_cast<std::int64_t>(value)));
            break;
        }
    case MongoSerializeInfoType::DOUBLE:
        {
            if(UNLIKELY(!data._stream || data._stream->GetReadableSize() < static_cast<Int64>(sizeof(Double))))
            {
                CLOG_ERROR_GLOBAL(MongoDataSerialize, "AppendSerialize double stream invalid key:%s, readable size:%lld"
                    , keyName.c_str(), data._stream ? data._stream->GetReadableSize() : -1);
                return false;
            }
            Double value = 0;
            ::memcpy(&value, data._stream->GetReadBegin(), sizeof(value));
            doc.append(bsoncxx::builder::basic::kvp(keyName.GetRaw(), value));
            break;
        }
    case MongoSerializeInfoType::STRING:
        {
            if(UNLIKELY(!data._stream))
            {
                CLOG_ERROR_GLOBAL(MongoDataSerialize, "AppendSerialize string stream is null key:%s", keyName.c_str());
                return false;
            }
            std::string_view str(data._stream->GetReadBegin(), static_cast<size_t>(data._stream->GetReadableSize()));
            doc.append(bsoncxx::builder::basic::kvp(keyName.GetRaw(), str));
            break;
        }
    case MongoSerializeInfoType::JSON:
        {
            if(UNLIKELY(!data._stream || data._stream->GetReadableSize() <= 0))
            {
                CLOG_ERROR_GLOBAL(MongoDataSerialize, "AppendSerialize json stream invalid key:%s, readable size:%lld"
                    , keyName.c_str(), data._stream ? data._stream->GetReadableSize() : -1);
                return false;
            }
            std::string_view json(data._stream->GetReadBegin(), static_cast<size_t>(data._stream->GetReadableSize()));
            try
            {
                auto &&fromJson = bsoncxx::from_json(json);
                doc.append(bsoncxx::builder::basic::kvp(keyName.GetRaw(), fromJson));
            }
            catch (const std::exception &e)
            {
                // 非法json不允许异常穿透, 走错误返回
                CLOG_ERROR_GLOBAL(MongoDataSerialize, "AppendSerialize json parse fail key:%s, err:%s, json size:%llu"
                    , keyName.c_str(), e.what(), static_cast<UInt64>(json.size()));
                return false;
            }
            break;
        }
    case MongoSerializeInfoType::BINARY:
        {
            if(UNLIKELY(!data._stream))
            {
                CLOG_ERROR_GLOBAL(MongoDataSerialize, "AppendSerialize binary stream is null key:%s", keyName.c_str());
                return false;
            }
            auto binData = bsoncxx::types::b_binary();
            binData.sub_type = bsoncxx::binary_sub_type::k_binary;
            binData.size = static_cast<uint32_t>(data._stream->GetReadableSize());
            binData.bytes = (uint8_t *) data._stream->GetReadBegin();
            doc.append(bsoncxx::builder::basic::kvp(keyName.GetRaw(), binData));
            break;    
        }
        // 空数据不append
    case MongoSerializeInfoType::NULL_DATA:
        {
            auto nullData = bsoncxx::types::b_null();
            doc.append(bsoncxx::builder::basic::kvp(keyName.GetRaw(), nullData));
            break;
        }
    default:
        {
            CLOG_ERROR_GLOBAL(MongoDataSerialize, "unsurpport MongoSerializeInfoType:%d to update data into collection data:%s"
                , data.DataType, KERNEL_NS::LibBase64::Encode(data._stream ? data._stream->GetReadBegin() : nullptr
                    , data._stream ? data._stream->GetReadableSize() : 0).c_str());
            return false;
        }    
    }

    return true;
}

bool MongoDataSerialize::Deserialize(const bsoncxx::types::bson_value::view &bsonValue, MongoSerializeInfo &data)
{
    switch (data.DataType)
    {
    case MongoSerializeInfoType::BOOL:
        {
            if(bsonValue.type() != bsoncxx::v_noabi::type::k_bool)
            {
                CLOG_ERROR_GLOBAL(MongoDataSerialize, "bsonValue type:%d, err, not bool", static_cast<Int32>(bsonValue.type()));
                return false;
            }
            auto &&boolValue = bsonValue.get_bool();
            data._stream->WriteBool(boolValue.value);
            break;
        }
    case MongoSerializeInfoType::INT64:
        {
            if(bsonValue.type() != bsoncxx::v_noabi::type::k_int64)
            {
                CLOG_ERROR_GLOBAL(MongoDataSerialize, "bsonValue type:%d, err, not int64", static_cast<Int32>(bsonValue.type()));
                return false;
            }
            auto &&int64Value = bsonValue.get_int64();
            data._stream->WriteInt64(int64Value.value);
            break;
        }
    case MongoSerializeInfoType::DOUBLE:
        {
            if(bsonValue.type() != bsoncxx::v_noabi::type::k_double)
            {
                CLOG_ERROR_GLOBAL(MongoDataSerialize, "bsonValue type:%d, err, not double", static_cast<Int32>(bsonValue.type()));
                return false;
            }
            auto &&doubleValue = bsonValue.get_double();
            data._stream->WriteDouble(doubleValue.value);
            break;
        }
    case MongoSerializeInfoType::STRING:
        {
            if(bsonValue.type() != bsoncxx::v_noabi::type::k_string && bsonValue.type() != bsoncxx::v_noabi::type::k_oid)
            {
                CLOG_ERROR_GLOBAL(MongoDataSerialize, "bsonValue type:%d, err, not string", static_cast<Int32>(bsonValue.type()));
                return false;
            }
            if(bsonValue.type() == bsoncxx::v_noabi::type::k_oid)
            {
                auto &&stringValue = bsonValue.get_oid().value.to_string();
                data._stream->Write(stringValue.data(), static_cast<Int64>(stringValue.size()));
            }
            else
            {
                auto &&stringValue = bsonValue.get_string();
                data._stream->Write(stringValue.value.data(), static_cast<Int64>(stringValue.value.size()));
            }
            break;
        }
    case MongoSerializeInfoType::JSON:
        {
            if(bsonValue.type() != bsoncxx::v_noabi::type::k_document)
            {
                CLOG_ERROR_GLOBAL(MongoDataSerialize, "bsonValue type:%d, err, not document", static_cast<Int32>(bsonValue.type()));
                return false;
            }
            auto &&docValue = bsonValue.get_document();
            auto &&jsonValue = bsoncxx::to_json(docValue);
            data._stream->Write(jsonValue.data(), static_cast<Int64>(jsonValue.size()));
            break;
        }
    case MongoSerializeInfoType::BINARY:
        {
            if(bsonValue.type() != bsoncxx::v_noabi::type::k_binary)
            {
                CLOG_ERROR_GLOBAL(MongoDataSerialize, "bsonValue type:%d, err, not binary data", static_cast<Int32>(bsonValue.type()));
                return false;
            }
            auto &&binaryData = bsonValue.get_binary();
            data._stream->Write(binaryData.bytes, static_cast<Int64>(binaryData.size));
            break;
        }
    case MongoSerializeInfoType::NULL_DATA:
        {
            CLOG_DEBUG_GLOBAL(MongoDataSerialize, "null data MongoSerializeInfoType:%d", data.DataType);
            break;
        }
    default:
        {
            CLOG_ERROR_GLOBAL(MongoDataSerialize, "unsurpport MongoSerializeInfoType:%d", data.DataType);
            return false;
        }    
    }

    return true;
}

Int32 MongoDataSerialize::GetSuitableSerializeType(const bsoncxx::v_noabi::type &bsonType)
{
    switch (bsonType)
    {
        case bsoncxx::v_noabi::type::k_bool: return MongoSerializeInfoType::BOOL;
        case bsoncxx::v_noabi::type::k_int64: return MongoSerializeInfoType::INT64;
        case bsoncxx::v_noabi::type::k_double: return MongoSerializeInfoType::DOUBLE;
        case bsoncxx::v_noabi::type::k_string: return MongoSerializeInfoType::STRING;
        case bsoncxx::v_noabi::type::k_oid: return MongoSerializeInfoType::STRING;
        case bsoncxx::v_noabi::type::k_document: return MongoSerializeInfoType::JSON;
        case bsoncxx::v_noabi::type::k_binary: return MongoSerializeInfoType::BINARY;
        case bsoncxx::v_noabi::type::k_null: return MongoSerializeInfoType::NULL_DATA;
        default:
            break;
    }

    return MongoSerializeInfoType::UNKNOWN;
}




KERNEL_END