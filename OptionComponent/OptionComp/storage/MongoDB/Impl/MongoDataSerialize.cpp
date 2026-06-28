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


KERNEL_BEGIN
    bool MongoDataSerialize::AppendSerialize(bsoncxx::builder::basic::document& doc, const KERNEL_NS::LibString &keyName, const MongoSerializeInfo& data)
{
    switch (data.DataType)
    {
    case MongoSerializeInfoType::BOOL:
        {
            bool *value = reinterpret_cast<bool *>(data._stream->GetReadBegin());
            doc.append(bsoncxx::builder::basic::kvp(keyName.GetRaw(), *value));
            break;
        }
    case MongoSerializeInfoType::INT64:
        {
            Int64 *value = reinterpret_cast<Int64 *>(data._stream->GetReadBegin());
            doc.append(bsoncxx::builder::basic::kvp(keyName.GetRaw(), static_cast<std::int64_t>(*value)));
            break;
        }
    case MongoSerializeInfoType::DOUBLE:
        {
            Double *value = reinterpret_cast<Double *>(data._stream->GetReadBegin());
            doc.append(bsoncxx::builder::basic::kvp(keyName.GetRaw(), *value));
            break;
        }
    case MongoSerializeInfoType::STRING:
        {
            const UInt64 len = data._stream->ReadUInt64();
            std::string_view str(data._stream->GetReadBegin(), static_cast<size_t>(len));
            doc.append(bsoncxx::builder::basic::kvp(keyName.GetRaw(), str));
            break;
        }
    case MongoSerializeInfoType::JSON:
        {
            std::string_view json(data._stream->GetReadBegin(), data._stream->GetReadableSize());
            auto &&fromJson = bsoncxx::from_json(json);
            doc.append(bsoncxx::builder::basic::kvp(keyName.GetRaw(), fromJson));
            break;
        }
    case MongoSerializeInfoType::BINARY:
        {
            auto binData = bsoncxx::types::b_binary();
            binData.sub_type = bsoncxx::binary_sub_type::k_binary;
            binData.size = static_cast<uint32_t>(data._stream->GetReadableSize());
            binData.bytes = (uint8_t *) data._stream->GetReadBegin();
            doc.append(bsoncxx::builder::basic::kvp(keyName.GetRaw(), binData));
            break;    
        }
    default:
        {
            CLOG_ERROR_GLOBAL(MongoDataSerialize, "unsurpport MongoSerializeInfoType:%d to update data into collection data:%s"
                , data.DataType, KERNEL_NS::LibBase64::Encode(data._stream->GetReadBegin(), data._stream->GetReadableSize()).c_str());
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
                data._stream->Write(stringValue);
            }
            else
            {
                auto &&stringValue = bsonValue.get_string();
                data._stream->Write(stringValue.value);
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
            data._stream->Write(jsonValue);
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
        default:
            break;
    }

    return MongoSerializeInfoType::UNKNOWN;
}




KERNEL_END