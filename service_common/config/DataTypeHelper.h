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
 * Date: 2023-04-05 18:47:28
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_CONFIG_DATA_TYPE_HELPER_H__
#define __CRYSTAL_NET_SERVICE_COMMON_CONFIG_DATA_TYPE_HELPER_H__

#pragma once

#include <service_common/common/common.h>
#include <kernel/kernel.h>
#include <3rd/3rd.h>

SERVICE_COMMON_BEGIN

class DataTypeHelper
{
public:
    static bool Parse(const KERNEL_NS::LibString &typeStr, KERNEL_NS::LibString &targetType, KERNEL_NS::LibString &errInfo);

    static bool IsSimpleType(const KERNEL_NS::LibString &typeStr);

    static bool IsArray(const KERNEL_NS::LibString &typeStr);
    static bool IsDict(const KERNEL_NS::LibString &typeStr);
    static bool IsString(const KERNEL_NS::LibString &typeStr);
    static bool IsBool(const KERNEL_NS::LibString &typeStr);
    static bool IsInt8(const KERNEL_NS::LibString &typeStr);
    static bool IsUInt8(const KERNEL_NS::LibString &typeStr);
    static bool IsInt16(const KERNEL_NS::LibString &typeStr);
    static bool IsUInt16(const KERNEL_NS::LibString &typeStr);
    static bool IsInt32(const KERNEL_NS::LibString &typeStr);
    static bool IsUInt32(const KERNEL_NS::LibString &typeStr);
    static bool IsInt64(const KERNEL_NS::LibString &typeStr);
    static bool IsUInt64(const KERNEL_NS::LibString &typeStr);
    static bool IsNumber(const KERNEL_NS::LibString &typeStr);

    // 赋值
    static bool Assign(bool &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo);
    static bool Assign(Byte8 &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo);
    static bool Assign(U8 &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo);
    static bool Assign(Int16 &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo);
    static bool Assign(UInt16 &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo);
    static bool Assign(Int32 &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo);
    static bool Assign(UInt32 &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo);
    static bool Assign(Int64 &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo);
    static bool Assign(UInt64 &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo);
    static bool Assign(KERNEL_NS::LibString &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo);
    template<typename ArrayElemType>
    static bool Assign(std::vector<ArrayElemType> &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo);
    template<typename DictKey, typename DictValue>
    static bool Assign(std::unordered_map<DictKey, DictValue> &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo);
    template<typename DictKey, typename DictValue>
    static bool Assign(std::map<DictKey, DictValue> &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo);

    // 序列化
    static void ToString(const bool &field, KERNEL_NS::LibString &dataInfo);
    static void ToString(const Byte8 &field, KERNEL_NS::LibString &dataInfo);
    static void ToString(const U8 &field, KERNEL_NS::LibString &dataInfo);
    static void ToString(const Int16 &field, KERNEL_NS::LibString &dataInfo);
    static void ToString(const UInt16 &field, KERNEL_NS::LibString &dataInfo);
    static void ToString(const Int32 &field, KERNEL_NS::LibString &dataInfo);
    static void ToString(const UInt32 &field, KERNEL_NS::LibString &dataInfo);
    static void ToString(const Int64 &field, KERNEL_NS::LibString &dataInfo);
    static void ToString(const UInt64 &field, KERNEL_NS::LibString &dataInfo);
    static void ToString(const KERNEL_NS::LibString &field, KERNEL_NS::LibString &dataInfo);
    static void ToSimpleTypeString(const KERNEL_NS::LibString &typeStr, const KERNEL_NS::LibString &dataContent, KERNEL_NS::LibString &dataInfo);

    template<typename ArrayElemType>
    static void ToString(const std::vector<ArrayElemType> &field, KERNEL_NS::LibString &dataInfo);
    template<typename DictKey, typename DictValue>
    static void ToString(const std::unordered_map<DictKey, DictValue> &field, KERNEL_NS::LibString &dataInfo);
    template<typename DictKey, typename DictValue>
    static void ToString(const std::map<DictKey, DictValue> &field, KERNEL_NS::LibString &dataInfo);
    
    // 校验数据长度(对于整形:无符号是没有负号的, 数据是有长度限制防止溢出)
    static bool CheckData(const KERNEL_NS::LibString &dataType, const KERNEL_NS::LibString &value, KERNEL_NS::LibString &errInfo);
    static bool CheckSymbolRight(const KERNEL_NS::LibString &signedNumber);

    // 字典的数据适配json
    static bool MakeDataAdaptJson(const KERNEL_NS::LibString &dataType, KERNEL_NS::LibString &value, KERNEL_NS::LibString &errInfo);
    
    // 获取默认值
    static KERNEL_NS::LibString GetTypeDefaultValue(const KERNEL_NS::LibString &typeStr);
};

ALWAYS_INLINE bool DataTypeHelper::Assign(bool &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo)
{
    const auto &jsonString = nlohmann::json::parse(dataInfo.c_str(), NULL, false);
    if(!jsonString.is_boolean())
    {
        errInfo.AppendFormat("parse json fail, assign boolean value fail dataInfo:%s\n", dataInfo.c_str());
        return false;
    }
    
    field = jsonString.get<bool>();
    return true;
}

ALWAYS_INLINE bool DataTypeHelper::Assign(Byte8 &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo)
{
    const auto &jsonString = nlohmann::json::parse(dataInfo.c_str(), NULL, false);
    if(!jsonString.is_number_integer())
    {
        errInfo.AppendFormat("parse json fail, assign byte8 value fail dataInfo:%s\n", dataInfo.c_str());
        return false;
    }

    field = static_cast<Byte8>(KERNEL_NS::StringUtil::StringToInt32(dataInfo.c_str()));
    return true;
}

ALWAYS_INLINE bool DataTypeHelper::Assign(U8 &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo)
{
    const auto &jsonString = nlohmann::json::parse(dataInfo.c_str(), NULL, false);
    if(!jsonString.is_number_unsigned())
    {
        errInfo.AppendFormat("parse json fail, assign u8 value fail dataInfo:%s\n", dataInfo.c_str());
        return false;
    }

    field = jsonString.get<U8>();
    return true;
}

ALWAYS_INLINE bool DataTypeHelper::Assign(Int16 &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo)
{
    const auto &jsonString = nlohmann::json::parse(dataInfo.c_str(), NULL, false);
    if(!jsonString.is_number_integer())
    {
        errInfo.AppendFormat("parse json fail, assign Int16 value fail dataInfo:%s\n", dataInfo.c_str());
        return false;
    }

    field = jsonString.get<Int16>();
    return true;
}

ALWAYS_INLINE bool DataTypeHelper::Assign(UInt16 &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo)
{
    const auto &jsonString = nlohmann::json::parse(dataInfo.c_str(), NULL, false);
    if(!jsonString.is_number_unsigned())
    {
        errInfo.AppendFormat("parse json fail, assign UInt16 value fail dataInfo:%s\n", dataInfo.c_str());
        return false;
    }

    field = jsonString.get<UInt16>();
    return true;
}

ALWAYS_INLINE bool DataTypeHelper::Assign(Int32 &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo)
{
    const auto &jsonString = nlohmann::json::parse(dataInfo.c_str(), NULL, false);
    if(!jsonString.is_number_integer())
    {
        errInfo.AppendFormat("parse json fail, assign Int32 value fail dataInfo:%s\n", dataInfo.c_str());
        return false;
    }

    field = jsonString.get<Int32>();
    return true;
}

ALWAYS_INLINE bool DataTypeHelper::Assign(UInt32 &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo)
{
    const auto &jsonString = nlohmann::json::parse(dataInfo.c_str(), NULL, false);
    if(!jsonString.is_number_unsigned())
    {
        errInfo.AppendFormat("parse json fail, assign UInt32 value fail dataInfo:%s\n", dataInfo.c_str());
        return false;
    }

    field = jsonString.get<UInt32>();
    return true;
}

ALWAYS_INLINE bool DataTypeHelper::Assign(Int64 &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo)
{
    const auto &jsonString = nlohmann::json::parse(dataInfo.c_str(), NULL, false);
    if(!jsonString.is_number_integer())
    {
        errInfo.AppendFormat("parse json fail, assign Int64 value fail dataInfo:%s\n", dataInfo.c_str());
        return false;
    }

    field = jsonString.get<Int64>();
    return true;
}

ALWAYS_INLINE bool DataTypeHelper::Assign(UInt64 &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo)
{
    const auto &jsonString = nlohmann::json::parse(dataInfo.c_str(), NULL, false);
    if(!jsonString.is_number_unsigned())
    {
        errInfo.AppendFormat("parse json fail, assign UInt64 value fail dataInfo:%s\n", dataInfo.c_str());
        return false;
    }

    field = jsonString.get<UInt64>();
    return true;
}

ALWAYS_INLINE bool DataTypeHelper::Assign(KERNEL_NS::LibString &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo)
{
    const auto &jsonString = nlohmann::json::parse(dataInfo.c_str(), NULL, false);
    if(!jsonString.is_string())
    {
        errInfo.AppendFormat("parse json fail, assign LibString value fail dataInfo:%s\n", dataInfo.c_str());
        return false;
    }

    field = jsonString.get<std::string>();
    return true;
}

template<typename ArrayElemType>
ALWAYS_INLINE bool DataTypeHelper::Assign(std::vector<ArrayElemType> &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo)
{
    const auto jsonArray = nlohmann::json::parse(dataInfo.c_str(), NULL, false);
    if(!jsonArray.is_array())
    {
        errInfo.AppendFormat("parse json fail, assign std::vector<%s> value fail dataInfo:%s\n"
                            , KERNEL_NS::RttiUtil::GetByType<ArrayElemType>(), dataInfo.c_str());
        return false;
    }

    for(auto &item : jsonArray.items())
    {
        ArrayElemType elem;
        const KERNEL_NS::LibString &itemDump = item.value().dump();
        if(!Assign(elem, itemDump, errInfo))
        {
            errInfo.AppendFormat("assign array element fail, element type:%s, jsonArray:%s"
                , KERNEL_NS::RttiUtil::GetByType<ArrayElemType>(), dataInfo.c_str());
            return false;
        }

        field.push_back(elem);
    }

    return true;
}

template<typename DictKey, typename DictValue>
ALWAYS_INLINE bool DataTypeHelper::Assign(std::unordered_map<DictKey, DictValue> &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo)
{
    const auto jsonObject = nlohmann::json::parse(dataInfo.c_str(), NULL, false);
    if(!jsonObject.is_object())
    {
        errInfo.AppendFormat("parse json fail, assign std::unordered_map<%s, %s> value fail dataInfo:%s\n"
                            , KERNEL_NS::RttiUtil::GetByType<DictKey>(), KERNEL_NS::RttiUtil::GetByType<DictValue>(), dataInfo.c_str());
        return false;
    }

    for(auto &item : jsonObject.items())
    {
        // key:
        const KERNEL_NS::LibString &keyJson = item.key();
        DictKey keyField;
        if(!Assign(keyField, keyJson, errInfo))
        {
            errInfo.AppendFormat("assign key field fail, std::unordered_map<%s, %s> key type:%s, keyJson:%s"
                , KERNEL_NS::RttiUtil::GetByType<DictKey>(), KERNEL_NS::RttiUtil::GetByType<DictValue>()
                , KERNEL_NS::RttiUtil::GetByType<DictKey>(), keyJson.c_str());
            return false;
        }

        const KERNEL_NS::LibString &valueJson = item.value().dump();
        DictValue valueField;
        if(!Assign(valueField, valueJson, errInfo))
        {
            errInfo.AppendFormat("assign value field fail, std::unordered_map<%s, %s> value type:%s, valueJson:%s"
                        , KERNEL_NS::RttiUtil::GetByType<DictKey>(), KERNEL_NS::RttiUtil::GetByType<DictValue>()
                        , KERNEL_NS::RttiUtil::GetByType<DictKey>(), keyJson.c_str());
            return false;
        }

        if(field.find(keyField) != field.end())
        {
            errInfo.AppendFormat("duplicate key, std::unordered_map<%s, %s> key:%s, value json:%s, old value json:%s"
                        , KERNEL_NS::RttiUtil::GetByType<DictKey>(), KERNEL_NS::RttiUtil::GetByType<DictValue>()
                        , keyJson.c_str(), valueJson.c_str());
            return false;
        }

        field.insert(std::make_pair(keyField, valueField));
    }

    return true;
}


template<typename DictKey, typename DictValue>
ALWAYS_INLINE bool DataTypeHelper::Assign(std::map<DictKey, DictValue> &field, const KERNEL_NS::LibString &dataInfo, KERNEL_NS::LibString &errInfo)
{
    const auto jsonObject = nlohmann::json::parse(dataInfo.c_str(), NULL, false);
    if(!jsonObject.is_object())
    {
        errInfo.AppendFormat("parse json fail, assign std::map<%s, %s> value fail dataInfo:%s\n"
                            , KERNEL_NS::RttiUtil::GetByType<DictKey>(), KERNEL_NS::RttiUtil::GetByType<DictValue>(), dataInfo.c_str());
        return false;
    }

    for(auto &item : jsonObject.items())
    {
        // key:
        const KERNEL_NS::LibString &keyJson = item.key();
        DictKey keyField;
        if(!Assign(keyField, keyJson, errInfo))
        {
            errInfo.AppendFormat("assign key field fail, std::map<%s, %s> key type:%s, keyJson:%s"
                , KERNEL_NS::RttiUtil::GetByType<DictKey>(), KERNEL_NS::RttiUtil::GetByType<DictValue>()
                , KERNEL_NS::RttiUtil::GetByType<DictKey>(), keyJson.c_str());
            return false;
        }

        const KERNEL_NS::LibString &valueJson = item.value().dump();
        DictValue valueField;
        if(!Assign(valueField, valueJson, errInfo))
        {
            errInfo.AppendFormat("assign value field fail, std::map<%s, %s> value type:%s, valueJson:%s"
                        , KERNEL_NS::RttiUtil::GetByType<DictKey>(), KERNEL_NS::RttiUtil::GetByType<DictValue>()
                        , KERNEL_NS::RttiUtil::GetByType<DictValue>(), valueJson.c_str());
            return false;
        }

        if(field.find(keyField) != field.end())
        {
            errInfo.AppendFormat("duplicate key, std::map<%s, %s> key:%s, value json:%s, old value json:%s"
                        , KERNEL_NS::RttiUtil::GetByType<DictKey>(), KERNEL_NS::RttiUtil::GetByType<DictValue>()
                        , keyJson.c_str(), valueJson.c_str());
            return false;
        }

        field.insert(std::make_pair(keyField, valueField));
    }

    return true;
}

ALWAYS_INLINE void DataTypeHelper::ToString(const bool &field, KERNEL_NS::LibString &dataInfo)
{
    nlohmann::json result;
    result = field;
    dataInfo << result.dump();
}

ALWAYS_INLINE void DataTypeHelper::ToString(const Byte8 &field, KERNEL_NS::LibString &dataInfo)
{
    nlohmann::json result;
    result = field;
    dataInfo << result.dump();
}

ALWAYS_INLINE void DataTypeHelper::ToString(const U8 &field, KERNEL_NS::LibString &dataInfo)
{
    nlohmann::json result;
    result = field;
    dataInfo << result.dump();
}

ALWAYS_INLINE void DataTypeHelper::ToString(const Int16 &field, KERNEL_NS::LibString &dataInfo)
{
    nlohmann::json result;
    result = field;
    dataInfo << result.dump();
}

ALWAYS_INLINE void DataTypeHelper::ToString(const UInt16 &field, KERNEL_NS::LibString &dataInfo)
{
    nlohmann::json result;
    result = field;
    dataInfo << result.dump();
}

ALWAYS_INLINE void DataTypeHelper::ToString(const Int32 &field, KERNEL_NS::LibString &dataInfo)
{
    nlohmann::json result;
    result = field;
    dataInfo << result.dump();
}

ALWAYS_INLINE void DataTypeHelper::ToString(const UInt32 &field, KERNEL_NS::LibString &dataInfo)
{
    nlohmann::json result;
    result = field;
    dataInfo << result.dump();
}

ALWAYS_INLINE void DataTypeHelper::ToString(const Int64 &field, KERNEL_NS::LibString &dataInfo)
{
    nlohmann::json result;
    result = field;
    dataInfo << result.dump();
}

ALWAYS_INLINE void DataTypeHelper::ToString(const UInt64 &field, KERNEL_NS::LibString &dataInfo)
{
    nlohmann::json result;
    result = field;
    dataInfo << result.dump();
}

ALWAYS_INLINE void DataTypeHelper::ToString(const KERNEL_NS::LibString &field, KERNEL_NS::LibString &dataInfo)
{
    nlohmann::json result;
    result = field.GetRaw();
    dataInfo << result.dump();
}

template<typename ArrayElemType>
ALWAYS_INLINE void DataTypeHelper::ToString(const std::vector<ArrayElemType> &field, KERNEL_NS::LibString &dataInfo)
{
    auto &&result = nlohmann::json::array();
    for(auto &item : field)
    {
        KERNEL_NS::LibString itemResult;
        DataTypeHelper::ToString(item, itemResult);
        result.push_back(itemResult.GetRaw());
    }

    dataInfo << result.dump();
}

template<typename DictKey, typename DictValue>
ALWAYS_INLINE void DataTypeHelper::ToString(const std::unordered_map<DictKey, DictValue> &field, KERNEL_NS::LibString &dataInfo)
{
    auto &&result = nlohmann::json::object();
    for(auto iter : field)
    {
        auto &key = iter.first;
        auto &value = iter.second;

        KERNEL_NS::LibString keyString;
        DataTypeHelper::ToString(key, keyString);
        KERNEL_NS::LibString valueString;
        DataTypeHelper::ToString(value, valueString);
        result[keyString.GetRaw()] = valueString.GetRaw();
    }
    
    dataInfo << result.dump();
}

template<typename DictKey, typename DictValue>
ALWAYS_INLINE void DataTypeHelper::ToString(const std::map<DictKey, DictValue> &field, KERNEL_NS::LibString &dataInfo)
{
    auto &&result = nlohmann::json::object();
    for(auto iter : field)
    {
        auto &key = iter.first;
        auto &value = iter.second;

        KERNEL_NS::LibString keyString;
        DataTypeHelper::ToString(key, keyString);
        KERNEL_NS::LibString valueString;
        DataTypeHelper::ToString(value, valueString);
        result[keyString.GetRaw()] = valueString.GetRaw();
    }
    
    dataInfo << result.dump();
}

SERVICE_COMMON_END

#endif
