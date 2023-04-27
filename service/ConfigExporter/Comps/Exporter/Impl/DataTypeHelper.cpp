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
#include <service/ConfigExporter/Comps/Exporter/Impl/DataTypeHelper.h>

SERVICE_BEGIN

bool DataTypeHelper::Parse(const KERNEL_NS::LibString &typeStr, KERNEL_NS::LibString &targetType, KERNEL_NS::LibString &errInfo, bool toNormalMapIfExist)
{
    // 校验
    auto copyTypeStr = typeStr.strip();
    if(copyTypeStr.GetRaw().substr(0, 4) == "bool")
    {
        targetType.AppendFormat("bool");
        return true;
    }

    if(copyTypeStr.GetRaw().substr(0, 4) == "int8")
    {
        targetType.AppendFormat("Byte8");
        return true;
    }

    if(copyTypeStr.GetRaw().substr(0, 5) == "uint8")
    {
        targetType.AppendFormat("U8");
        return true;
    }

    if(copyTypeStr.GetRaw().substr(0, 5) == "int16")
    {
        targetType.AppendFormat("Int16");
        return true;
    }

    if(copyTypeStr.GetRaw().substr(0, 6) == "uint16")
    {
        targetType.AppendFormat("UInt16");
        return true;
    }

    if(copyTypeStr.GetRaw().substr(0, 5) == "int32")
    {
        targetType.AppendFormat("Int32");
        return true;
    }

    if(copyTypeStr.GetRaw().substr(0, 6) == "uint32")
    {
        targetType.AppendFormat("UInt32");
        return true;
    }

    if(copyTypeStr.GetRaw().substr(0, 5) == "int64")
    {
        targetType.AppendFormat("Int64");
        return true;
    }

    if(copyTypeStr.GetRaw().substr(0, 6) == "uint64")
    {
        targetType.AppendFormat("UInt64");
        return true;
    }

    if(copyTypeStr.GetRaw().substr(0, 6) == "string")
    {
        targetType.AppendFormat("KERNEL_NS::LibString");
        return true;
    }

    // 解析array
    if(copyTypeStr.GetRaw().substr(0, 5) == "array")
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

        if(!DataTypeHelper::Parse(arrayElementType, targetType, errInfo, toNormalMapIfExist))
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
    if(copyTypeStr.GetRaw().substr(0, 4) == "dict")
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

        if(toNormalMapIfExist)
        {
            targetType.AppendFormat("std::map<");
        }
        else
        {
            targetType.AppendFormat("std::unordered_map<");
        }

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
        Int32 sepPos = std::string::npos;
        do
        {
            auto offSetCount = std::string::npos;
            if((dictKvTypeStr.GetRaw().substr(0, 4) == "bool") || 
                (typeStr.GetRaw().substr(0, 4) == "int8"))
            {
                offSetCount = 4;
            }
            else if((typeStr.GetRaw().substr(0, 5) == "uint8") || 
            (typeStr.GetRaw().substr(0, 5) == "int16") || 
            (typeStr.GetRaw().substr(0, 5) == "int32") || 
            (typeStr.GetRaw().substr(0, 5) == "int64")
            )
            {
                offSetCount = 5;
            }
            else if((typeStr.GetRaw().substr(0, 6) == "uint32") ||
            (typeStr.GetRaw().substr(0, 6) == "uint64") || 
            (typeStr.GetRaw().substr(0, 6) == "string"))
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

            /* code */
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
        if(!DataTypeHelper::Parse(keyString, targetType, errInfo, toNormalMapIfExist))
        {
            if(!targetType.empty())
                targetType.clear();

            errInfo.AppendFormat("dict key value error parse key fail dictKvTypeStr:%s, copyTypeStr:%s\n"
                        , dictKvTypeStr.c_str(),  copyTypeStr.c_str());
            return false;
        }

        targetType.AppendFormat(", ");

        // 解析value type
        if(!DataTypeHelper::Parse(valueString, targetType, errInfo, toNormalMapIfExist))
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

    errInfo.AppendFormat("data type not surpport copyTypeStr:%s", copyTypeStr.c_str());

    return false;
}

bool DataTypeHelper::IsSimpleType(const KERNEL_NS::LibString &typeStr)
{
    if(typeStr.GetRaw().substr(0, 4) == "bool")
        return true;

    if(typeStr.GetRaw().substr(0, 4) == "int8")
        return true;

    if(typeStr.GetRaw().substr(0, 5) == "uint8")
        return true;

    if(typeStr.GetRaw().substr(0, 5) == "int16")
        return true;

    if(typeStr.GetRaw().substr(0, 5) == "int32")
        return true;

    if(typeStr.GetRaw().substr(0, 6) == "uint32")
        return true;

    if(typeStr.GetRaw().substr(0, 5) == "int64")
        return true;

    if(typeStr.GetRaw().substr(0, 6) == "uint64")
        return true;

    if(typeStr.GetRaw().substr(0, 6) == "string")
        return true;

    return false;
}

bool DataTypeHelper::IsArray(const KERNEL_NS::LibString &typeStr)
{
    return typeStr.GetRaw().substr(0, 5) == "array";
}

bool DataTypeHelper::IsDict(const KERNEL_NS::LibString &typeStr)
{
    return typeStr.GetRaw().substr(0, 5) == "dict";
}

bool DataTypeHelper::IsString(const KERNEL_NS::LibString &typeStr)
{
    if(typeStr.GetRaw().substr(0, 6) == "string")
        return true;

    return false;
}

KERNEL_NS::LibString DataTypeHelper::GetTypeDefaultValue(const KERNEL_NS::LibString &typeStr)
{
   if(typeStr.GetRaw().substr(0, 4) == "bool")
   {
        return "false";
   }

    if(typeStr.GetRaw().substr(0, 4) == "int8")
    {
        return "0";
    }

    if(typeStr.GetRaw().substr(0, 5) == "uint8")
    {
        return "0";
    }

    if(typeStr.GetRaw().substr(0, 5) == "int16")
    {
        return "0";
    }

    if(typeStr.GetRaw().substr(0, 5) == "int32")
    {
        return "0";
    }

    if(typeStr.GetRaw().substr(0, 6) == "uint32")
    {
        return "0";
    }

    if(typeStr.GetRaw().substr(0, 5) == "int64")
    {
        return "0";
    }
    if(typeStr.GetRaw().substr(0, 6) == "uint64")
    {
        return "0";
    }

    return "";
}

SERVICE_END