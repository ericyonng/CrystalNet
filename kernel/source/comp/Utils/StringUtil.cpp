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
 * Date: 2020-12-21 02:12:20
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/LibTime.h>
#include <kernel/comp/Utils/StringUtil.h>
#include <3rd/3rdForKernel.h>
#include <kernel/comp/Log/log.h>

// 
// // 3rd
// // openssl
// #ifdef _WIN32
//  // include header
// #ifdef _DEBUG
// #include "3rd/openssl/staticlib/debug/include/openssl/md5.h"
// #include "3rd/openssl/staticlib/debug/include/openssl/aes.h"
// #else
// #include "3rd/openssl/staticlib/release/include/openssl/md5.h"
// #include "3rd/openssl/staticlib/release/include/openssl/aes.h"
// #endif
// 
// // lib static lib
// // #ifdef _DEBUG
// // #pragma comment(lib, "libeay32.lib")
// // #pragma comment(lib, "ssleay32.lib")
// // #else
// // #pragma comment(lib, "libeay32.lib")
// // #pragma comment(lib, "ssleay32.lib")
// // #endif
// 
// #else// linux
// #include "openssl/md5.h"
// #include "openssl/aes.h"
// 
// 
// #endif
// 


KERNEL_BEGIN

LibString StringUtil::I64toA(Int64 value, Int32 radix)
{
    char *p;
    char *firstDigit;
    char temp;
    unsigned int digval;
    char buf[64] = {0};

    p = buf;
    firstDigit = p;

    if(value < 0)
    {
        p[0] = '-';
        firstDigit = ++p;

        value = -value;
    }

    do
    {
        digval = (unsigned int)(value % radix);
        value /= radix;

        if(digval > 9)
            *p++ = (char)(digval - 10 + 'a');
        else
            *p++ = (char)(digval + '0');
    } while(value > 0);

    *p-- = '\0';

    do
    {
        temp = *p;
        *p = *firstDigit;
        *firstDigit = temp;

        --p;
        ++firstDigit;
    } while(firstDigit < p);

    return buf;
}

LibString StringUtil::UI64toA(UInt64 value, Int32 radix)
{
    char *p;
    char *firstDigit;
    char temp;
    unsigned int digval;
    char buf[64] = {0};

    p = buf;
    firstDigit = p;

    do
    {
        digval = (unsigned int)(value % radix);
        value /= radix;

        if(digval > 9)
            *p++ = (char)(digval - 10 + 'a');
        else
            *p++ = (char)(digval + '0');
    } while(value > 0);

    *p-- = '\0';

    do
    {
        temp = *p;
        *p = *firstDigit;
        *firstDigit = temp;

        --p;
        ++firstDigit;
    } while(firstDigit < p);

    return buf;
}

bool StringUtil::IsHex(U8 ch)
{
    if(ch >='0' && ch <= '9')
        return true;
    if(ch >= 'a' && ch <= 'f')
        return true;
    if(ch >= 'A' && ch <= 'F')
        return true;

    return false;
}

bool StringUtil::ToHexString(const LibString &src, LibString &outHexString)
{
//     if(UNLIKELY(!src.GetLength()))
//         return false;

    src.ToHexString(outHexString);

    return true;
}

bool StringUtil::FromHexString(const LibString &hexString, LibString &outBin)
{
    bool ret = outBin.FromHexString(hexString);
    if(!ret)
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(StringUtil, "FromHexString fail hexString:%s"), hexString.c_str());

    return ret;
}

bool StringUtil::ToHexStringView(const Byte8 *buff, Int64 len, LibString &outHexString)
{
//     if (UNLIKELY(!len))
//         return false;

    LibString info;
    info.AppendData(buff, len);
    info.ToHexView(outHexString);

    return true;
}

void StringUtil::PreInstertTime(const LibTime &time, LibString &src)
{
    src << time.ToStringOfMillSecondPrecision();
}

void StringUtil::SplitString(const LibString &str, const LibString &separator, std::vector<LibString> &destStrList, bool justSplitFirst /*= false*/, char escapeChar /*= '\0'*/, bool enableEmptyPart)
{
    if(UNLIKELY(str.empty()))
        return;

    if(UNLIKELY(separator.empty()))
    {
        if(str.empty() && !enableEmptyPart)
            return;

        destStrList.push_back(str);
        return;
    }

    LibString::size_type curPos = 0;
    LibString::size_type prevPos = 0;

    LibString strInternal = str;
    auto &internalRaw = strInternal.GetRaw();
	const auto &sepRaw = separator.GetRaw();
    const UInt64 stepSize = static_cast<UInt64>(sepRaw.size());
    while((curPos = internalRaw.find(sepRaw, curPos)) != std::string::npos)
    {
        if(curPos != 0 && strInternal[curPos - 1] == escapeChar)
        {
            internalRaw.erase(--curPos, 1);
            // curPos += stepSize;
            continue;
        }

        LibString temp = internalRaw.substr(prevPos, curPos - prevPos);
        destStrList.push_back(temp);

        if(justSplitFirst)
        {
            const auto &strPart = internalRaw.substr(curPos + stepSize);
            if(strPart.empty() && !enableEmptyPart)
                return;

            destStrList.push_back(strPart);
            return;
        }

        curPos += stepSize;
        prevPos = curPos;
    }

    LibString temp = internalRaw.substr(prevPos);
    if(!temp.empty() || enableEmptyPart)
        destStrList.push_back(temp);
}

LibString StringUtil::CutString(const LibString &src, const LibString &start, const LibString &end)
{
    // 原文
    auto &raw = src.GetRaw();
    // 找起始位置
    auto startPos = raw.find(start.c_str(), 0);
    if (startPos == std::string::npos)
        return "";

    // 找结尾
    auto endPos = raw.find(end.c_str(), startPos + start.length());
    if (endPos == std::string::npos)
        return "";

    // 截取原文
    return raw.substr(startPos, endPos - startPos + end.length());
}

LibString StringUtil::FilterOutString(const LibString &str, const LibString &filterStr)
{
    std::vector<LibString> strings;
    SplitString(str, filterStr, strings);

    LibString retStr;
    for(size_t i = 0; i < strings.size(); i++)
    {
        retStr += strings[i];
    }

    return retStr;
}

bool StringUtil::CheckDoubleString(const LibString &str)
{// 有.切割之后两块必须是整型,没有.的整个必须是整型
    // 剔除符号
    LibString tripSeps = "-";
    auto tripResult = str.lstrip(tripSeps);

    auto &raw = tripResult.GetRaw();
    if(raw.find(".") == std::string::npos)
        return tripResult.isdigit();
    
    // 数值型必须最多被切割成2块
    const auto &splitParts = tripResult.Split(".");
    if(splitParts.size() > 2)
        return false;

    // 必须都是数值
    Int32 hasNumberNum = 0;
    for(auto &str : splitParts)
    {
        if(str.empty())
            continue;

        if(!str.isdigit())
            return false;

        ++hasNumberNum;
    }

    return hasNumberNum > 0;
}


void StringUtil::ToString(const std::vector<LibString> &contents, const LibString &sep, LibString &target)
{
	const Int32 count = static_cast<Int32>(contents.size());
	for(Int32 idx = 0; idx < count; ++idx)
	{
		auto &content = contents[idx];
		target.AppendData(content);
		if(idx != (count - 1))
			target.AppendData(sep);
	}
}

bool StringUtil::CheckGeneralName(const LibString &name)
{
	if(name.empty())
	{
		return false;
	}

	// 英文,数值,下划线
	const auto len = static_cast<Int32>(name.size());
	for(Int32 idx = 0; idx < len; ++idx)
	{
		const auto ch = name[idx];
		if(!KERNEL_NS::LibString::isalpha(ch) && !KERNEL_NS::LibString::isdigit(ch) && ch != '_')
		{
			return false;
		}
	}

	// 首字母不能是数值
	if(KERNEL_NS::LibString::isdigit(name[0]))
		return false;

	return true;
}

LibString StringUtil::RemoveNameSpace(const LibString &name)
{
    // 命名空间检测
    auto splitNameSpace = name.Split("::", -1, false, true);
    Int32 splitSize = static_cast<Int32>(splitNameSpace.size());

    // 去末尾空
    for(Int32 idx = splitSize - 1; idx >= 0; --idx)
    {
        if(splitNameSpace[idx].empty())
        {
            splitNameSpace.erase(splitNameSpace.begin() + idx);
        }
        else
            break;
    }

	return splitNameSpace.empty() ? "" : splitNameSpace[splitNameSpace.size() - 1];

    // LibString icompName;
    // splitSize = static_cast<Int32>(splitNameSpace.size());
    // for(Int32 idx = 0; idx < splitSize; ++idx)
    // {
    //     if(idx == splitSize -1)
    //         icompName.AppendFormat("%s", ConstantGather::interfacePrefix.c_str());

    //     if(!splitNameSpace[idx].empty())
    //         icompName.AppendFormat("%s", splitNameSpace[idx].c_str());

    //     if(idx != splitSize -1)
    //         icompName.AppendFormat("::");
    // }
}

LibString StringUtil::InterfaceObjName(const LibString &name)
{
	// 命名空间检测
    auto &&splitNameSpace = name.Split("::", -1, false, true);
    Int32 splitSize = static_cast<Int32>(splitNameSpace.size());

    // 去末尾空
    for(Int32 idx = splitSize - 1; idx >= 0; --idx)
    {
        if(splitNameSpace[idx].empty())
        {
            splitNameSpace.erase(splitNameSpace.begin() + idx);
        }
        else
            break;
    }

    LibString icompName;
    splitSize = static_cast<Int32>(splitNameSpace.size());
    for(Int32 idx = 0; idx < splitSize; ++idx)
    {
        if(idx == splitSize -1)
            icompName.AppendFormat("%s", ConstantGather::interfacePrefix.c_str());

        if(!splitNameSpace[idx].empty())
            icompName.AppendFormat("%s", splitNameSpace[idx].c_str());

        if(idx != splitSize -1)
            icompName.AppendFormat("::");
    }

	return icompName;
}
KERNEL_END

