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
 * Date: 2023-04-05 18:47:34
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <service_common/config/DataTypeHelper.h>

SERVICE_COMMON_BEGIN

bool DataTypeHelper::Parse(const KERNEL_NS::LibString &typeStr, KERNEL_NS::LibString &targetType, KERNEL_NS::LibString &errInfo)
{
    // 校验
    auto copyTypeStr = typeStr.strip();
    if(IsBool(copyTypeStr))
    {
        targetType.AppendFormat("bool");
        return true;
    }

    if(IsInt8(copyTypeStr))
    {
        targetType.AppendFormat("Byte8");
        return true;
    }

    if(IsUInt8(copyTypeStr))
    {
        targetType.AppendFormat("U8");
        return true;
    }

    if(IsInt16(copyTypeStr))
    {
        targetType.AppendFormat("Int16");
        return true;
    }

    if(IsUInt16(copyTypeStr))
    {
        targetType.AppendFormat("UInt16");
        return true;
    }

    if(IsInt32(copyTypeStr))
    {
        targetType.AppendFormat("Int32");
        return true;
    }

    if(IsUInt32(copyTypeStr))
    {
        targetType.AppendFormat("UInt32");
        return true;
    }

    if(IsInt64(copyTypeStr))
    {
        targetType.AppendFormat("Int64");
        return true;
    }

    if(IsUInt64(copyTypeStr))
    {
        targetType.AppendFormat("UInt64");
        return true;
    }

    if(IsString(copyTypeStr))
    {
        targetType.AppendFormat("KERNEL_NS::LibString");
        return true;
    }

    if(IsEnum(copyTypeStr))
    {
        targetType.AppendFormat("Int32");
        return true;
    }

    // 解析array
    if(IsArray(copyTypeStr))
    {
        KERNEL_NS::LibString leftType = copyTypeStr.GetRaw().substr(5);
        leftType.strip();
        if(leftType.empty())
        {
            if(!targetType.empty())
                targetType.clear();
            errInfo.AppendFormat("have no any after key array, copyTypeStr:%s\n", copyTypeStr.c_str());
            return false;
        }

        if(leftType[0] != '[')
        {
            if(!targetType.empty())
                targetType.clear();

            errInfo.AppendFormat("have no array left flag [:%s, copyTypeStr:%s\n", leftType.c_str(), copyTypeStr.c_str());
            return false;
        }

        if(leftType[leftType.size() - 1] != ']')
        {
            if(!targetType.empty())
                targetType.clear();

            errInfo.AppendFormat("have no array right flag ]:%s, copyTypeStr:%s\n", leftType.c_str(), copyTypeStr.c_str());
            return false;
        }

        targetType.AppendFormat("std::vector<");
        auto arrayElementType = leftType.sub("[", "]");
        if(arrayElementType.empty())
        {
            if(!targetType.empty())
                targetType.clear();

            errInfo.AppendFormat("have no anry array element type leftType:%s, copyTypeStr:%s\n", leftType.c_str(), copyTypeStr.c_str());
            return false;
        }

        if(!DataTypeHelper::Parse(arrayElementType, targetType, errInfo))
        {
            errInfo.AppendFormat("parse array element fail element:%s\n", arrayElementType.c_str());

            if(!targetType.empty())
                targetType.clear();

            return false;
        }

        targetType.AppendFormat(">");

        return true;
    }

    // 解析dict
    if(IsDict(copyTypeStr))
    {
        KERNEL_NS::LibString leftTypeStr = copyTypeStr.GetRaw().substr(4);
        leftTypeStr.strip();
        if(leftTypeStr.empty())
        {
            if(!targetType.empty())
                targetType.clear();

            errInfo.AppendFormat("have no any after key dict copyTypeStr:%s\n", copyTypeStr.c_str());
            return false;
        }

        if(leftTypeStr[0] != '<')
        {
            if(!targetType.empty())
                targetType.clear();

            errInfo.AppendFormat("have no any type after left flag < copyTypeStr:%s, leftTypeStr:%s\n"
                        , copyTypeStr.c_str(), leftTypeStr.c_str());
            return false;
        }

        if(leftTypeStr[leftTypeStr.size() - 1] != '>')
        {
            if(!targetType.empty())
                targetType.clear();

            errInfo.AppendFormat("have no any type before right flag > copyTypeStr:%s, leftTypeStr:%s\n"
                        , copyTypeStr.c_str(), leftTypeStr.c_str());
            return false;
        }

        targetType.AppendFormat("std::map<");
        auto dictKvTypeStr = leftTypeStr.sub("<", ">");
        if(dictKvTypeStr.empty())
        {
            if(!targetType.empty())
                targetType.clear();

            errInfo.AppendFormat("have no anry dict element type leftTypeStr:%s, copyTypeStr:%s\n", leftTypeStr.c_str(), copyTypeStr.c_str());
            return false;
        }

        if(!IsSimpleType(dictKvTypeStr))
        {
            if(!targetType.empty())
                targetType.clear();

            errInfo.AppendFormat("dict key is not simple data type dictKvTypeStr:%s, copyTypeStr:%s\n"
                        , dictKvTypeStr.c_str(),  copyTypeStr.c_str());

            return false;
        }

        // 分离key, value
        auto sepPos = std::string::npos;
        do
        {
            auto offSetCount = std::string::npos;
            if(IsBool(dictKvTypeStr) || 
                IsInt8(dictKvTypeStr))
            {
                offSetCount = 4;
            }
            else if(IsUInt8(dictKvTypeStr) || 
            IsInt16(dictKvTypeStr) || 
            IsInt32(dictKvTypeStr) || 
            IsInt64(dictKvTypeStr)
            )
            {
                offSetCount = 5;
            }
            else if(IsUInt32(dictKvTypeStr) ||
            IsUInt16(dictKvTypeStr) || 
            IsUInt64(dictKvTypeStr) || 
            IsString(dictKvTypeStr))
            {
                offSetCount = 6;
            }

            if(offSetCount == std::string::npos)
                break;

            KERNEL_NS::LibString leftPart = dictKvTypeStr.GetRaw().substr(offSetCount);
            leftPart.strip();
            if(leftPart.empty())
                break;

            if(leftPart[0] == ',')
                sepPos = dictKvTypeStr.GetRaw().find_first_of(',');

        } while (false);

        if(sepPos == std::string::npos)
        {
            if(!targetType.empty())
                targetType.clear();

            errInfo.AppendFormat("dict key value error dictKvTypeStr:%s, copyTypeStr:%s\n"
                        , dictKvTypeStr.c_str(),  copyTypeStr.c_str());

            return false;
        }

        if(dictKvTypeStr.size() == (sepPos + 1))
        {
            if(!targetType.empty())
                targetType.clear();

            errInfo.AppendFormat("dict key value error have no value type dictKvTypeStr:%s, copyTypeStr:%s\n"
                        , dictKvTypeStr.c_str(),  copyTypeStr.c_str());
            return false;
        }

        KERNEL_NS::LibString keyString = dictKvTypeStr.GetRaw().substr(0, sepPos);
        KERNEL_NS::LibString valueString = dictKvTypeStr.GetRaw().substr(sepPos + 1);

        keyString.strip();
        valueString.strip();
        if(keyString.empty())
        {
            if(!targetType.empty())
                targetType.clear();

            errInfo.AppendFormat("dict key value error have no key type dictKvTypeStr:%s, copyTypeStr:%s\n"
                        , dictKvTypeStr.c_str(),  copyTypeStr.c_str());

            return false;
        }

        if(valueString.empty())
        {
            if(!targetType.empty())
                targetType.clear();

            errInfo.AppendFormat("dict key value error have no value type dictKvTypeStr:%s, copyTypeStr:%s\n"
                        , dictKvTypeStr.c_str(),  copyTypeStr.c_str());

            return false;
        }
        
        // 解析key type
        if(!DataTypeHelper::Parse(keyString, targetType, errInfo))
        {
            if(!targetType.empty())
                targetType.clear();

            errInfo.AppendFormat("dict key value error parse key fail dictKvTypeStr:%s, copyTypeStr:%s\n"
                        , dictKvTypeStr.c_str(),  copyTypeStr.c_str());
            return false;
        }

        targetType.AppendFormat(", ");

        // 解析value type
        if(!DataTypeHelper::Parse(valueString, targetType, errInfo))
        {
            if(!targetType.empty())
                targetType.clear();

            errInfo.AppendFormat("dict key value error parse value fail dictKvTypeStr:%s, copyTypeStr:%s\n"
                        , dictKvTypeStr.c_str(),  copyTypeStr.c_str());
            return false;
        }

        targetType.AppendFormat(">");

        return true;
    }

    if(!targetType.empty())
        targetType.clear();

    errInfo.AppendFormat("data type not surpport copyTypeStr:%s\n", copyTypeStr.c_str());

    return false;
}

bool DataTypeHelper::IsSimpleType(const KERNEL_NS::LibString &typeStr)
{
    if(IsBool(typeStr) ||
        IsInt8(typeStr) ||
        IsUInt8(typeStr) ||
        IsInt16(typeStr) ||
        IsUInt16(typeStr) ||
        IsInt32(typeStr) ||
        IsUInt32(typeStr) ||
        IsInt64(typeStr) ||
        IsUInt64(typeStr) ||
        IsString(typeStr) ||
        IsEnum(typeStr)
     )
     {
         return true;
     }
     
    return false;
}

bool DataTypeHelper::IsArray(const KERNEL_NS::LibString &typeStr)
{
    return typeStr.GetRaw().substr(0, 5) == "array";
}

bool DataTypeHelper::IsDict(const KERNEL_NS::LibString &typeStr)
{
    return typeStr.GetRaw().substr(0, 4) == "dict";
}

bool DataTypeHelper::IsString(const KERNEL_NS::LibString &typeStr)
{
    if(typeStr.GetRaw().substr(0, 6) == "string")
        return true;

    return false;
}

bool DataTypeHelper::IsBool(const KERNEL_NS::LibString &typeStr)
{
    if(typeStr.GetRaw().substr(0, 4) == "bool")
        return true;

    return false;
}

bool DataTypeHelper::IsInt8(const KERNEL_NS::LibString &typeStr)
{
    if(typeStr.GetRaw().substr(0, 4) == "int8")
        return true;

    return false;
}

bool DataTypeHelper::IsUInt8(const KERNEL_NS::LibString &typeStr)
{
    if(typeStr.GetRaw().substr(0, 5) == "uint8")
        return true;

    return false;
}

bool DataTypeHelper::IsInt16(const KERNEL_NS::LibString &typeStr)
{
    if(typeStr.GetRaw().substr(0, 5) == "int16")
        return true;

    return false;
}

bool DataTypeHelper::IsUInt16(const KERNEL_NS::LibString &typeStr)
{
    if(typeStr.GetRaw().substr(0, 6) == "uint16")
        return true;

    return false;
}

bool DataTypeHelper::IsInt32(const KERNEL_NS::LibString &typeStr)
{
    if(typeStr.GetRaw().substr(0, 5) == "int32")
        return true;

    return false;
}

bool DataTypeHelper::IsUInt32(const KERNEL_NS::LibString &typeStr)
{
    if(typeStr.GetRaw().substr(0, 6) == "uint32")
        return true;

    return false;
}

bool DataTypeHelper::IsInt64(const KERNEL_NS::LibString &typeStr)
{
    if(typeStr.GetRaw().substr(0, 5) == "int64")
        return true;

    return false;
}

bool DataTypeHelper::IsUInt64(const KERNEL_NS::LibString &typeStr)
{
    if(typeStr.GetRaw().substr(0, 6) == "uint64")
        return true;

    return false;
}

bool DataTypeHelper::IsNumber(const KERNEL_NS::LibString &typeStr)
{
    return IsInt8(typeStr) || IsUInt8(typeStr) || IsInt16(typeStr) || IsUInt16(typeStr) || IsInt32(typeStr) || IsUInt32(typeStr) || IsInt64(typeStr) || IsUInt64(typeStr); 
}

bool DataTypeHelper::IsEnum(const KERNEL_NS::LibString &typeStr)
{
    if((typeStr.GetRaw().substr(0, 4) == "enum") || 
        (typeStr.GetRaw().substr(0, 4) == "ENUM") )
        return true;

    return false;
}


void DataTypeHelper::ToSimpleTypeString(const KERNEL_NS::LibString &typeStr, const KERNEL_NS::LibString &dataContent, KERNEL_NS::LibString &dataInfo)
{
    if(IsBool(typeStr))
    {
        bool value = false;
        if(dataContent.isdigit())
        {
            value = KERNEL_NS::StringUtil::StringToInt64(dataContent.c_str()) != 0;
        }
        else
        {
            value = dataContent.tolower() == "true";
        }

        DataTypeHelper::ToString(value, dataInfo);
        return;
    }

    if(IsInt8(typeStr))
    {
        Byte8 value = static_cast<Byte8>(KERNEL_NS::StringUtil::StringToInt32(dataContent.c_str()));
        DataTypeHelper::ToString(value, dataInfo);
        return;
    }

    if(IsUInt8(typeStr))
    {
        U8 value = static_cast<U8>(KERNEL_NS::StringUtil::StringToUInt32(dataContent.c_str()));
        DataTypeHelper::ToString(value, dataInfo);
        return;
    }

    if(IsInt16(typeStr))
    {
        Int16 value = KERNEL_NS::StringUtil::StringToInt16(dataContent.c_str());
        DataTypeHelper::ToString(value, dataInfo);
        return;
    }

    if(IsUInt16(typeStr))
    {
        UInt16 value = KERNEL_NS::StringUtil::StringToUInt16(dataContent.c_str());
        DataTypeHelper::ToString(value, dataInfo);
        return;
    }

    if(IsInt32(typeStr) || IsEnum(typeStr))
    {
        Int32 value = KERNEL_NS::StringUtil::StringToInt32(dataContent.c_str());
        DataTypeHelper::ToString(value, dataInfo);
        return;
    }

    if(IsUInt32(typeStr))
    {
        UInt32 value = KERNEL_NS::StringUtil::StringToUInt32(dataContent.c_str());
        DataTypeHelper::ToString(value, dataInfo);
        return;
    }

    if(IsInt64(typeStr))
    {
        Int64 value = KERNEL_NS::StringUtil::StringToInt64(dataContent.c_str());
        DataTypeHelper::ToString(value, dataInfo);
        return;
    }

    if(IsUInt64(typeStr))
    {
        UInt64 value = KERNEL_NS::StringUtil::StringToUInt64(dataContent.c_str());
        DataTypeHelper::ToString(value, dataInfo);
        return;
    }

    if(IsString(typeStr))
    {
        DataTypeHelper::ToString(dataContent, dataInfo);
        return;
    }

    g_Log->Error(LOGFMT_NON_OBJ_TAG(DataTypeHelper, "not simple data type:%s, content:%s"), typeStr.c_str(), dataContent.c_str());
}

bool DataTypeHelper::CheckData(const KERNEL_NS::LibString &dataType, const KERNEL_NS::LibString &value, KERNEL_NS::LibString &errInfo)
{
    if(IsBool(dataType))
    {
        if(value.isdigit())
            return true;

        if(value.isalpha())
        {
            const auto lower = value.tolower();
            if(lower == "true" || lower == "false")
                return true;
        }

        errInfo.AppendFormat("data not bool type value, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
        return false;
    }

    if(IsInt8(dataType))
    {
        // 长度顶多4个字符(负号一个字符数值3个字符)
        if(value.length() > 4)
        {
            errInfo.AppendFormat("data over int8 limit, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        // 符号位准确性
        if(!CheckSymbolRight(value))
        {
            errInfo.AppendFormat("data format error, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        // 值范围:-127 - 127
        const Int32 temp = KERNEL_NS::StringUtil::StringToInt32(value.c_str());
        if(temp > 127 || temp < -127)
        {
            errInfo.AppendFormat("data overflow, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        return true;
    }

    if(IsUInt8(dataType))
    {
        // 长度顶多3个字符 256
        if(value.length() > 3)
        {
            errInfo.AppendFormat("data over uint8 limit, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        // 必须都是数值
        if(!value.isdigit())
        {
            errInfo.AppendFormat("data format error, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        // 值范围:0 - 255
        const UInt32 temp = KERNEL_NS::StringUtil::StringToUInt32(value.c_str());
        if(temp > 255)
        {
            errInfo.AppendFormat("data overflow, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        return true;
    }

    if(IsInt16(dataType))
    {
        // 长度顶多6个字符 -32767 - 32767
        if(value.length() > 6)
        {
            errInfo.AppendFormat("data over int16 limit, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        // 符号位
        if(!CheckSymbolRight(value))
        {
            errInfo.AppendFormat("data format error, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        // 值范围:-32767 - 32767
        const Int32 temp = KERNEL_NS::StringUtil::StringToInt32(value.c_str());
        if(temp > 32767 || temp < -32767)
        {
            errInfo.AppendFormat("data overflow, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        return true;
    }

    if(IsUInt16(dataType))
    {
        // 长度顶多5个字符 0 - 65535
        if(value.length() > 5)
        {
            errInfo.AppendFormat("data over uint16 limit, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        // 符号位
        if(!value.isdigit())
        {
            errInfo.AppendFormat("data format error, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        // 值范围:0 - 65535
        const UInt32 temp = KERNEL_NS::StringUtil::StringToUInt32(value.c_str());
        if(temp > 65535)
        {
            errInfo.AppendFormat("data overflow, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        return true;
    }

    if(IsInt32(dataType))
    {
        // 长度顶多11个字符 -2147483647 - 2147483647
        if(value.length() > 11)
        {
            errInfo.AppendFormat("data over int32 limit, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        // 符号位
        if(!CheckSymbolRight(value))
        {
            errInfo.AppendFormat("data format error, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        // 值范围:-2147483647 - 2147483647
        const Int64 temp = KERNEL_NS::StringUtil::StringToInt64(value.c_str());
        if(temp > 2147483647 || temp < -2147483647)
        {
            errInfo.AppendFormat("data overflow, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        return true;
    }

    if(IsUInt32(dataType))
    {
        // 长度顶多10个字符 0 - 4294967294
        if(value.length() > 10)
        {
            errInfo.AppendFormat("data over uint16 limit, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        if(!value.isdigit())
        {
            errInfo.AppendFormat("data format error, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        // 值范围:0 - 4294967294
        const UInt64 temp = KERNEL_NS::StringUtil::StringToUInt64(value.c_str());
        if(temp > 4294967294)
        {
            errInfo.AppendFormat("data overflow, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        return true;
    }

    if(IsInt64(dataType))
    {
        // 长度顶多20个字符 -9223372036854775807 - 922337203 6854775807
        if(value.length() > 20)
        {
            errInfo.AppendFormat("data over int64 limit, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        // 符号位
        if(!CheckSymbolRight(value))
        {
            errInfo.AppendFormat("data format error, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        // 值范围:-9223372036854775807 - 9223372036854775807
        // 把数值剔除符号后, 拆成高10个整数, 低10个整数, 高10个整数不能超过922337203, 当高10个整数是922337203时, 低10个整数不能超过6854775807
        if(value.length() <= 10)
            return true;

        auto copy = value;
        copy.EraseAnyOf("-");

        const KERNEL_NS::LibString hi = copy.GetRaw().substr(0, 10);
        const KERNEL_NS::LibString lo = copy.GetRaw().substr(10, value.length() - 10);
        const Int64 hiValue = KERNEL_NS::StringUtil::StringToInt64(hi.c_str());
        const Int64 loValue = KERNEL_NS::StringUtil::StringToInt64(lo.c_str());
        if(hiValue > 922337203)
        {
            errInfo.AppendFormat("data overflow(hi ten number over 922337203)(erase - symbol), value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        if((hiValue == 922337203) && (loValue > 6854775807))
        {
            errInfo.AppendFormat("data overflow(hi ten number is 922337203 and lo ten number over 6854775807)(erase - symbol), value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        return true;
    }

    if(IsUInt64(dataType))
    {
        // 长度顶多20个字符 0 - 1844674407 3709551615
        if(value.length() > 20)
        {
            errInfo.AppendFormat("data over uint16 limit, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        if(!value.isdigit())
        {
            errInfo.AppendFormat("data format error, value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        // 值范围:0 - 1844674407 3709551615
        // 把数值剔除符号后, 拆成高10个整数, 低10个整数, 高10个整数不能超过 1844674407, 当高10个整数是 1844674407 时, 低10个整数不能超过 3709551615
        if(value.length() <= 10)
            return true;

        const KERNEL_NS::LibString hi = value.GetRaw().substr(0, 10);
        const KERNEL_NS::LibString lo = value.GetRaw().substr(10, value.length() - 10);
        const Int64 hiValue = KERNEL_NS::StringUtil::StringToInt64(hi.c_str());
        const Int64 loValue = KERNEL_NS::StringUtil::StringToInt64(lo.c_str());
        if(hiValue > 1844674407)
        {
            errInfo.AppendFormat("data overflow(hi ten number over 1844674407), value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        if((hiValue == 1844674407) && (loValue > 3709551615))
        {
            errInfo.AppendFormat("data overflow(hi ten number is 1844674407 and lo ten number over 3709551615), value:%s, data type:%s\n", value.c_str(), dataType.c_str());
            return false;
        }

        return true;
    }

    if(IsArray(dataType))
    {
        // value必须是array
        const auto jsonArray = nlohmann::json::parse(value.c_str(), NULL, false);
        if(!jsonArray.is_array())
        {
            errInfo.AppendFormat("value is not array dataType:%s, value:%s\n", dataType.c_str(), value.c_str());
            return false;
        }
        
        KERNEL_NS::LibString leftType = dataType.GetRaw().substr(5);
        leftType.strip();
        if(leftType.empty())
        {
            errInfo.AppendFormat("have no any after key array, dataType:%s\n", dataType.c_str());
            return false;
        }

        if(leftType[0] != '[')
        {
            errInfo.AppendFormat("have no array left flag [:%s, dataType:%s\n", leftType.c_str(), dataType.c_str());
            return false;
        }

        if(leftType[leftType.size() - 1] != ']')
        {
            errInfo.AppendFormat("have no array right flag ]:%s, dataType:%s\n", leftType.c_str(), dataType.c_str());
            return false;
        }

        auto arrayElementType = leftType.sub("[", "]");
        if(arrayElementType.empty())
        {
            errInfo.AppendFormat("have no anry array element type leftType:%s, dataType:%s\n", leftType.c_str(), dataType.c_str());
            return false;
        }

        // 校验元素
        const Int32 elemCount = static_cast<Int32>(jsonArray.size());
        for(Int32 idx = 0; idx < elemCount; ++idx)
        {
            const KERNEL_NS::LibString elem = jsonArray[idx].dump();

            if(!DataTypeHelper::CheckData(arrayElementType, elem, errInfo))
            {
                errInfo.AppendFormat("check data array element data fail element type:%s, element data:%s, array data:%s, index:%d\n"
                                , arrayElementType.c_str(), elem.c_str(), value.c_str(), idx);
                return false;
            }
        }

        return true;
    }

    if(IsDict(dataType))
    {
        // 校验数据是不是字典类型的
        const auto jsonObject = nlohmann::json::parse(value.c_str(), NULL, false);
        if(!jsonObject.is_object())
        {
            errInfo.AppendFormat("parse json fail, check data data type:%s fail of data value:%s not a dict type\n"
                                , dataType.c_str(), value.c_str());
            return false;
        }

        KERNEL_NS::LibString leftTypeStr = dataType.GetRaw().substr(4);
        leftTypeStr.strip();
        if(leftTypeStr.empty())
        {
            errInfo.AppendFormat("have no any after key dict dataType:%s\n", dataType.c_str());
            return false;
        }

        if(leftTypeStr[0] != '<')
        {
            errInfo.AppendFormat("have no any type after left flag < copyTypeStr:%s, leftTypeStr:%s\n"
                        , dataType.c_str(), leftTypeStr.c_str());
            return false;
        }

        if(leftTypeStr[leftTypeStr.size() - 1] != '>')
        {
            errInfo.AppendFormat("have no any type before right flag > copyTypeStr:%s, leftTypeStr:%s\n"
                        , dataType.c_str(), leftTypeStr.c_str());
            return false;
        }

        auto dictKvTypeStr = leftTypeStr.sub("<", ">");
        if(dictKvTypeStr.empty())
        {
            errInfo.AppendFormat("have no anry dict element type leftTypeStr:%s, copyTypeStr:%s\n", leftTypeStr.c_str(), dataType.c_str());
            return false;
        }

        if(!IsSimpleType(dictKvTypeStr))
        {
            errInfo.AppendFormat("dict key is not simple data type dictKvTypeStr:%s, copyTypeStr:%s\n"
                        , dictKvTypeStr.c_str(),  dataType.c_str());

            return false;
        }

        // 分离key, value
        auto sepPos = std::string::npos;
        do
        {
            auto offSetCount = std::string::npos;
            if(IsBool(dictKvTypeStr) || 
                IsInt8(dictKvTypeStr))
            {
                offSetCount = 4;
            }
            else if(IsUInt8(dictKvTypeStr) || 
            IsInt16(dictKvTypeStr) || 
            IsInt32(dictKvTypeStr) || 
            IsInt64(dictKvTypeStr)
            )
            {
                offSetCount = 5;
            }
            else if(IsUInt32(dictKvTypeStr) ||
            IsUInt16(dictKvTypeStr) || 
            IsUInt64(dictKvTypeStr) || 
            IsString(dictKvTypeStr))
            {
                offSetCount = 6;
            }

            if(offSetCount == std::string::npos)
                break;

            KERNEL_NS::LibString leftPart = dictKvTypeStr.GetRaw().substr(offSetCount);
            leftPart.strip();
            if(leftPart.empty())
                break;

            if(leftPart[0] == ',')
                sepPos = dictKvTypeStr.GetRaw().find_first_of(',');

        } while (false);

        if(sepPos == std::string::npos)
        {
            errInfo.AppendFormat("dict key value error dictKvTypeStr:%s, dataType:%s\n"
                        , dictKvTypeStr.c_str(), dataType.c_str());

            return false;
        }

        if(dictKvTypeStr.size() == (sepPos + 1))
        {
            errInfo.AppendFormat("dict key value error have no value type dictKvTypeStr:%s, dataType:%s\n"
                        , dictKvTypeStr.c_str(), dataType.c_str());
            return false;
        }

        KERNEL_NS::LibString keyString = dictKvTypeStr.GetRaw().substr(0, sepPos);
        KERNEL_NS::LibString valueString = dictKvTypeStr.GetRaw().substr(sepPos + 1);

        keyString.strip();
        valueString.strip();
        if(keyString.empty())
        {
            errInfo.AppendFormat("dict key value error have no key type dictKvTypeStr:%s, dataType:%s\n"
                        , dictKvTypeStr.c_str(),  dataType.c_str());

            return false;
        }

        if(valueString.empty())
        {
            errInfo.AppendFormat("dict key value error have no value type dictKvTypeStr:%s, dataType:%s\n"
                        , dictKvTypeStr.c_str(),  dataType.c_str());

            return false;
        }
        
        for(auto &item : jsonObject.items())
        {
            // key:
            const KERNEL_NS::LibString &keyJson = item.key();
            const KERNEL_NS::LibString &valueJson = item.value().dump();

            if(!CheckData(keyString, keyJson, errInfo))
            {
                errInfo.AppendFormat("check key data fail:key data type:%s, key value:%s when check dict type:%s, value:%s\n"
                            , keyString.c_str(), keyJson.c_str(), dataType.c_str(), value.c_str());

                return false;
            }

            if(!CheckData(valueString, valueJson, errInfo))
            {
                errInfo.AppendFormat("check value data fail:key data type:%s, key value:%s when check dict type:%s, value:%s\n"
                            , keyString.c_str(), keyJson.c_str(), dataType.c_str(), value.c_str());

                return false;
            }
        }

        return true;
    }

    if(IsString(dataType))
    {
        // 数据必须是字符串
        const auto jsonObject = nlohmann::json::parse(value.c_str(), NULL, false);
        if(!jsonObject.is_string())
        {
            errInfo.AppendFormat("check a string but data not a string data dataType:%s, value:%s\n", dataType.c_str(), value.c_str());
            return false;
        }
        
        return true;
    }

    if(IsEnum(dataType))
    {
        if(value.empty())
        {
            errInfo.AppendFormat("enum type must have a string, data type:%s, value:%s\n", dataType.c_str(), value.c_str());
            return false;
        }

        // 首字符必须是英文
        if(!value.isalpha(value[0]))
        {
            errInfo.AppendFormat("enum fist char must be alpha, data type:%s, value:%s\n", dataType.c_str(), value.c_str());
            return false;
        }

        // 其他只能是字符数值和下划线
        const Int32 count = static_cast<Int32>(value.size());
        for(Int32 idx = 0; idx < count; ++idx)
        {
            if((!value.isalpha(value[idx])) && 
            (!value.isdigit(value[idx])) && 
            (value[idx] != '_'))
            {
                errInfo.AppendFormat("have invalid char in enum name, data type:%s, enum name:%s\n", dataType.c_str(), value.c_str());
                return false;
            }
        }

        return true;
    }

    // 未知类型
    errInfo.AppendFormat("unknown data type:%s, value:%s\n", dataType.c_str(), value.c_str());
    return false;
}


KERNEL_NS::LibString DataTypeHelper::GetTypeDefaultValue(const KERNEL_NS::LibString &typeStr)
{
    if(IsBool(typeStr))
    {
        return "false";
    }

    if(IsInt8(typeStr) ||
        IsUInt8(typeStr) ||
        IsInt16(typeStr) ||
        IsUInt16(typeStr) ||
        IsInt32(typeStr) ||
        IsUInt32(typeStr) ||
        IsInt64(typeStr) ||
        IsUInt64(typeStr) ||
        IsEnum(typeStr)
    )
    {
        return "0";
    }

    if(IsString(typeStr))
        return "";

    if(IsArray(typeStr))
        return "[]";

    if(IsDict(typeStr))
        return "{}";

    return "";
}

bool DataTypeHelper::CheckSymbolRight(const KERNEL_NS::LibString &signedNumber)
{
    auto signedSymbolPos = signedNumber.GetRaw().find_first_of('-');
    if(signedSymbolPos == std::string::npos)
        return signedNumber.isdigit();

    // 如果有符号那么符号是第0位
    if(signedSymbolPos != 0)
        return false;

    // 移除符号位后必须都是数值
    auto copy = signedNumber;
    return copy.EraseAnyOf("-").isdigit();
}

bool DataTypeHelper::MakeDataAdaptJson(const KERNEL_NS::LibString &dataType, KERNEL_NS::LibString &value, KERNEL_NS::LibString &errInfo)
{
    if(IsBool(dataType) || IsNumber(dataType) || IsString(dataType))
        return true;

    if(IsArray(dataType))
    {
        // value必须是array
        const auto jsonArray = nlohmann::json::parse(value.c_str(), NULL, false);
        if(!jsonArray.is_array())
        {
            errInfo.AppendFormat("value is not array dataType:%s, value:%s\n", dataType.c_str(), value.c_str());
            return false;
        }
        
        KERNEL_NS::LibString leftType = dataType.GetRaw().substr(5);
        leftType.strip();
        if(leftType.empty())
        {
            errInfo.AppendFormat("have no any after key array, dataType:%s\n", dataType.c_str());
            return false;
        }

        if(leftType[0] != '[')
        {
            errInfo.AppendFormat("have no array left flag [:%s, dataType:%s\n", leftType.c_str(), dataType.c_str());
            return false;
        }

        if(leftType[leftType.size() - 1] != ']')
        {
            errInfo.AppendFormat("have no array right flag ]:%s, dataType:%s\n", leftType.c_str(), dataType.c_str());
            return false;
        }

        auto arrayElementType = leftType.sub("[", "]");
        if(arrayElementType.empty())
        {
            errInfo.AppendFormat("have no anry array element type leftType:%s, dataType:%s\n", leftType.c_str(), dataType.c_str());
            return false;
        }

        // 校验元素
        value.clear();
        const Int32 elemCount = static_cast<Int32>(jsonArray.size());
        value.AppendFormat("[");
        for(Int32 idx = 0; idx < elemCount; ++idx)
        {
            KERNEL_NS::LibString elem = jsonArray[idx].dump();
            if(!DataTypeHelper::MakeDataAdaptJson(arrayElementType, elem, errInfo))
            {
                errInfo.AppendFormat("make data adpapt json data array element data fail element type:%s, element data:%s, array data:%s, index:%d\n"
                                , arrayElementType.c_str(), elem.c_str(), value.c_str(), idx);
                return false;
            }

            value += elem;
            if(idx != elemCount - 1)
                value += ",";
        }

        value.AppendFormat("]");

        return true;
    }

    if(IsDict(dataType))
    {
        // 校验数据是不是字典类型的
        const auto jsonObject = nlohmann::json::parse(value.c_str(), NULL, false);
        if(!jsonObject.is_object())
        {
            errInfo.AppendFormat("parse json fail when MakeDataAdaptJson, check data data type:%s fail of data value:%s not a dict type\n"
                                , dataType.c_str(), value.c_str());
            return false;
        }

        KERNEL_NS::LibString leftTypeStr = dataType.GetRaw().substr(4);
        leftTypeStr.strip();
        if(leftTypeStr.empty())
        {
            errInfo.AppendFormat("MakeDataAdaptJson have no any after key dict dataType:%s\n", dataType.c_str());
            return false;
        }

        if(leftTypeStr[0] != '<')
        {
            errInfo.AppendFormat("MakeDataAdaptJson have no any type after left flag < copyTypeStr:%s, leftTypeStr:%s\n"
                        , dataType.c_str(), leftTypeStr.c_str());
            return false;
        }

        if(leftTypeStr[leftTypeStr.size() - 1] != '>')
        {
            errInfo.AppendFormat("MakeDataAdaptJson have no any type before right flag > copyTypeStr:%s, leftTypeStr:%s\n"
                        , dataType.c_str(), leftTypeStr.c_str());
            return false;
        }

        auto dictKvTypeStr = leftTypeStr.sub("<", ">");
        if(dictKvTypeStr.empty())
        {
            errInfo.AppendFormat("MakeDataAdaptJson have no anry dict element type leftTypeStr:%s, copyTypeStr:%s\n", leftTypeStr.c_str(), dataType.c_str());
            return false;
        }

        if(!IsSimpleType(dictKvTypeStr))
        {
            errInfo.AppendFormat("MakeDataAdaptJson dict key is not simple data type dictKvTypeStr:%s, copyTypeStr:%s\n"
                        , dictKvTypeStr.c_str(),  dataType.c_str());

            return false;
        }

        // 分离key, value
        auto sepPos = std::string::npos;
        do
        {
            auto offSetCount = std::string::npos;
            if(IsBool(dictKvTypeStr) || 
                IsInt8(dictKvTypeStr))
            {
                offSetCount = 4;
            }
            else if(IsUInt8(dictKvTypeStr) || 
            IsInt16(dictKvTypeStr) || 
            IsInt32(dictKvTypeStr) || 
            IsInt64(dictKvTypeStr)
            )
            {
                offSetCount = 5;
            }
            else if(IsUInt32(dictKvTypeStr) ||
            IsUInt16(dictKvTypeStr) || 
            IsUInt64(dictKvTypeStr) || 
            IsString(dictKvTypeStr))
            {
                offSetCount = 6;
            }

            if(offSetCount == std::string::npos)
                break;

            KERNEL_NS::LibString leftPart = dictKvTypeStr.GetRaw().substr(offSetCount);
            leftPart.strip();
            if(leftPart.empty())
                break;

            if(leftPart[0] == ',')
                sepPos = dictKvTypeStr.GetRaw().find_first_of(',');

        } while (false);

        if(sepPos == std::string::npos)
        {
            errInfo.AppendFormat("MakeDataAdaptJson dict key value error dictKvTypeStr:%s, dataType:%s\n"
                        , dictKvTypeStr.c_str(), dataType.c_str());

            return false;
        }

        if(dictKvTypeStr.size() == (sepPos + 1))
        {
            errInfo.AppendFormat("dict key value error have no value type dictKvTypeStr:%s, dataType:%s\n"
                        , dictKvTypeStr.c_str(), dataType.c_str());
            return false;
        }

        KERNEL_NS::LibString keyString = dictKvTypeStr.GetRaw().substr(0, sepPos);
        KERNEL_NS::LibString valueString = dictKvTypeStr.GetRaw().substr(sepPos + 1);

        keyString.strip();
        valueString.strip();
        if(keyString.empty())
        {
            errInfo.AppendFormat("MakeDataAdaptJson dict key value error have no key type dictKvTypeStr:%s, dataType:%s\n"
                        , dictKvTypeStr.c_str(),  dataType.c_str());

            return false;
        }

        if(valueString.empty())
        {
            errInfo.AppendFormat("MakeDataAdaptJson dict key value error have no value type dictKvTypeStr:%s, dataType:%s\n"
                        , dictKvTypeStr.c_str(),  dataType.c_str());

            return false;
        }
        
        value.clear();
        const auto count = static_cast<Int32>(jsonObject.size());
        Int32 loop = 0;
        value.AppendFormat("{");
        for(auto &item : jsonObject.items())
        {
            KERNEL_NS::LibString keyJson = item.key();
            KERNEL_NS::LibString valueJson = item.value().dump();

            if(!MakeDataAdaptJson(keyString, keyJson, errInfo))
            {
                errInfo.AppendFormat("check key data fail:key data type:%s, key value:%s when check dict type:%s, value:%s\n"
                            , keyString.c_str(), keyJson.c_str(), dataType.c_str(), value.c_str());

                return false;
            }

            if(!MakeDataAdaptJson(valueString, valueJson, errInfo))
            {
                errInfo.AppendFormat("check value data fail:key data type:%s, key value:%s when check dict type:%s, value:%s\n"
                            , keyString.c_str(), keyJson.c_str(), dataType.c_str(), value.c_str());

                return false;
            }

            // key:
            KERNEL_NS::LibString realKey;
            // 数值或者bool做key需要带上双引号
            if(IsNumber(keyString) || IsBool(keyString))
            {
                if(!keyJson.IsStartsWith("\""))
                    realKey.AppendFormat("\"");
            }

            realKey += keyJson;

            if(IsNumber(keyString) || IsBool(keyString))
            {
                if(!keyJson.IsEndsWith("\""))
                    realKey.AppendFormat("\"");
            }

            KERNEL_NS::LibString realValue;

            value += realKey;
            value += ":";
            value += valueJson;
            ++loop;
            if(loop != count)
                value += ", ";
        }

        value.AppendFormat("}");

        return true;
    }

    // 未知类型
    errInfo.AppendFormat("unknown data type:%s, value:%s\n", dataType.c_str(), value.c_str());
    return false;
}

SERVICE_COMMON_END