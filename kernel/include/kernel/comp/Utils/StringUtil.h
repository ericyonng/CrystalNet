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
 * Date: 2020-12-21 00:49:19
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_STRING_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_STRING_UTIL_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/common/BaseType.h>
#include <kernel/comp/LibString.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <list>
#include <concepts>
#include <ranges>

#include "kernel/comp/LibList.h"

KERNEL_BEGIN
    class LibTime;

class KERNEL_EXPORT StringUtil
{
public:
	static Int32 StringToInt32(const char *str);
    static UInt32 StringToUInt32(const char *str);
    static Int16 StringToInt16(const char *str);
    static UInt16 StringToUInt16(const char *str);
    static Long StringToLong(const char *str);
    static ULong StringToULong(const char *str);
    static Int64 StringToInt64(const char *str);
    static UInt64 StringToUInt64(const char *str);
    static Double StringToDouble(const char *str);
    static LibString ItoA(Int32 value, Int32 radix);
    static LibString UItoA(UInt32 value, Int32 radix);
    static LibString I64toA(Int64 value, Int32 radix);
    static LibString UI64toA(UInt64 value, Int32 radix);
	static bool IsHex(U8 ch);

    template <typename ObjType>
    static LibString Num2Str(ObjType val, Int32 radix = 10);
    static bool ToHexString(const LibString &src, LibString &outHexString);
    static bool FromHexString(const LibString &hexString, LibString &outBin);
	// 加了换行 以便显示
    static bool ToHexStringView(const Byte8 *buff, Int64 len, LibString &outHexString);
    static void PreInstertTime(const LibTime &time, LibString &src);

	/**
	 * Split string using specific separator.
	 * @param[in]  str            - the source string.
	 * @param[in]  separator      - separator string.
	 * @param[out] destStrList    - sestination string list.
	 * @param[in]  justSplitFirst - split first flag, if true, when split one time, will stop.
	 * @param[in]  escapeChar     - escape character, default is '\0' 成功匹配 separator后剔除 separator之前的escapeChar字符.
	 * @param[in]  enableEmptyPart - 切割后是否允许存在空字符串
	 */
    static void SplitString(const LibString &str,
                     const LibString &separator,
                     std::vector<LibString> &destStrList,
                     bool justSplitFirst = false,
                     char escapeChar = '\0', bool enableEmptyPart = true);
    // src原文,start:第一个字符串,end第一个结尾
    static LibString CutString(const LibString &src, const LibString &start, const LibString &end);
    static LibString FilterOutString(const LibString &str, const LibString &filterStr);
	static bool CheckDoubleString(const LibString &str);

	static UInt64 CalcUtf8CharBytes(U8 ctrlChar);
	static bool IsUtf8String(const LibString &str);

	static void MergerMultiLine(const std::vector<LibString> &lines, LibString &target);
	static void SepMultiLine(const LibString &multiLine, std::vector<LibString> &lines);

	static LibString ToString(const std::vector<LibString> &contents, const LibString &sep);
	static void ToString(const std::vector<LibString> &contents, const LibString &sep, LibString &target);
	template<typename T>
	static LibString ToString(const std::vector<T> &contents, const LibString &sep);
	template<typename T>
	static LibString ToString(const std::vector<T *> &contents, const LibString &sep);
	template<typename T>
	static LibString ToString(const std::vector<const T *> &contents, const LibString &sep);
	template<typename T>
	static LibString ToString(const std::set<T> &contents, const LibString &sep);
	template<typename T>
	static LibString ToString(const std::set<T *> &contents, const LibString &sep);
	template<typename T>
	static LibString ToString(const std::set<const T *> &contents, const LibString &sep);
	template<typename K, typename V>
	static LibString ToString(const std::map<K, V> &contents, const LibString &sep);
	template<typename K, typename V>
	static LibString ToString(const std::unordered_map<K, V> &contents, const LibString &sep);
	template<typename V>
	static LibString ToString(const std::list<V> &contents, const LibString &sep);

	// T具有范围, 且嵌套value_type类型, CallbackType形参得是value_type, 返回值LibString
	template<typename T, typename CallbackType>
	// T具有范围
	requires std::ranges::range<T> 
	// T必须有 value_type 类型
	&& requires
	{
		typename T::value_type;
	}
	// callback 是一个形参是 T::value_type, 返回值是LibString
	&& requires(typename T::value_type value,  CallbackType cb)
	{
		{ cb(value)	} -> std::same_as<KERNEL_NS::LibString>; 
	}
	static LibString ToStringBy(const T &contentsContainer, const LibString &sep, CallbackType &&cb);

    // callback 需要传key,value进去, 返回string
    template<typename K, typename V, typename CallbackType>
    requires requires(K key, V value,  CallbackType cb)
    {
        { cb(key, value) } -> std::same_as<KERNEL_NS::LibString>; 
    }
    static LibString ToStringBy(const std::map<K, V> &dict, const LibString &sep, CallbackType &&cb);
	
	// 校验标准名字:英文, 数字, 下划线, 且首字母非数字, name 长度为0也是非法
	static bool CheckGeneralName(const LibString &name);

	// 移除命名空间
	static LibString RemoveNameSpace(const LibString &name);
	// 转成接口类
	static LibString InterfaceObjName(const LibString &name);

	// 正则表达式匹配
	static bool IsMatch(const LibString &content, const LibString &matchStr);
};

ALWAYS_INLINE Int32 StringUtil::StringToInt32(const char *str)
{
    return ::atoi(str);
}

ALWAYS_INLINE UInt32 StringUtil::StringToUInt32(const char *str)
{
    return static_cast<UInt32>(StringToInt32(str));
}

ALWAYS_INLINE Int16 StringUtil::StringToInt16(const char *str)
{
    return static_cast<Int16>(StringToInt32(str));
}

ALWAYS_INLINE UInt16 StringUtil::StringToUInt16(const char *str)
{
    return static_cast<UInt16>(StringToInt32(str));
}

ALWAYS_INLINE Long StringUtil::StringToLong(const char *str)
{
    return ::atol(str);
}

ALWAYS_INLINE ULong StringUtil::StringToULong(const char *str)
{
    return static_cast<ULong>(StringToLong(str));
}

ALWAYS_INLINE Int64 StringUtil::StringToInt64(const char *str)
{
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    return ::atoll(str);
#else
    return ::_atoi64(str);
#endif
}

ALWAYS_INLINE UInt64 StringUtil::StringToUInt64(const char *str)
{
    return static_cast<UInt64>(StringToInt64(str));
}

ALWAYS_INLINE Double StringUtil::StringToDouble(const char *str)
{
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    return ::atof(str);
#else
    return ::atof(str);
#endif
}

ALWAYS_INLINE LibString StringUtil::ItoA(Int32 value, Int32 radix)
{
    return I64toA(value, radix);
}

ALWAYS_INLINE LibString StringUtil::UItoA(UInt32 value, Int32 radix)
{
    return UI64toA(value, radix);
}


template <>
ALWAYS_INLINE LibString StringUtil::Num2Str(Int64 val, Int32 radix)
{
    return I64toA(val, radix);
}

template <>
ALWAYS_INLINE LibString StringUtil::Num2Str(UInt64 val, Int32 radix)
{
    return UI64toA(val, radix);
}

template <>
ALWAYS_INLINE LibString StringUtil::Num2Str(Int32 val, Int32 radix)
{
    return Num2Str<Int64>(val, radix);
}

template <>
ALWAYS_INLINE LibString StringUtil::Num2Str(UInt32 val, Int32 radix)
{
    return Num2Str<UInt64>(val, radix);
}

template <>
ALWAYS_INLINE LibString StringUtil::Num2Str(Int16 val, Int32 radix)
{
    return Num2Str<Int64>(val, radix);
}

template <>
ALWAYS_INLINE LibString StringUtil::Num2Str(UInt16 val, Int32 radix)
{
    return Num2Str<UInt64>(val, radix);
}

template <>
ALWAYS_INLINE LibString StringUtil::Num2Str(Byte8 val, Int32 radix)
{
    return Num2Str<Int64>(val, radix);
}

template <>
ALWAYS_INLINE LibString StringUtil::Num2Str(U8 val, Int32 radix)
{
    return Num2Str<UInt64>(val, radix);
}

template <>
ALWAYS_INLINE LibString StringUtil::Num2Str(Long val, Int32 radix)
{
    return Num2Str<Int64>(val, radix);
}

template <>
ALWAYS_INLINE LibString StringUtil::Num2Str(ULong val, Int32 radix)
{
    return Num2Str<UInt64>(val, radix);
}

template <>
ALWAYS_INLINE LibString StringUtil::Num2Str(Double val, Int32 radix)
{
    char buf[64] = {0};

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    sprintf(buf, "%f", val);
#else // LLBC_TARGET_PLATFORM_WIN32
    sprintf_s(buf, sizeof(buf), "%f", val);
#endif // LLBC_TARGET_PLATFORM_NON_WIN32
    return buf;
}

template <>
ALWAYS_INLINE LibString StringUtil::Num2Str(Float val, Int32 radix)
{
    return Num2Str<Double>(val, radix);
}

template <typename ObjType>
ALWAYS_INLINE LibString StringUtil::Num2Str(ObjType val, Int32 radix)
{
    if(radix != 10 && radix != 16)
        radix = 10;

    LibString str;
    if(radix == 16)
        str.GetRaw() += "0x";

    UInt64 ptrVal = 0;
	auto objsz = sizeof(ObjType);
	auto u64sz = sizeof(UInt64);
    ::memcpy(&ptrVal, &val, objsz > u64sz ? u64sz : objsz);
    return (str + Num2Str<UInt64>(ptrVal, radix));
}

ALWAYS_INLINE bool StringUtil::IsUtf8String(const LibString &str)
{
	return str.IsUtf8();
}

ALWAYS_INLINE UInt64 StringUtil::CalcUtf8CharBytes(U8 ctrlChar)
{
	if ((ctrlChar & (U8)0x80) == 0x00)
	{
		return 1;
	}
    // 110x xxxx
    // Encoding len: 2 bytes.
    else if ((ctrlChar & (U8)0xe0) == 0xc0)
	{
        return 2;
	}
    // 1110 xxxx
    // Encoding len: 3 bytes.
    else if ((ctrlChar & (U8)0xf0) == 0xe0)
	{
        return 3;
	}
    // 1111 0xxx
    // Encoding len: 4 bytes.
    else if ((ctrlChar & (U8)0xf8) == 0xf0)
	{
        return 4;
	}
    // 1111 10xx
    // Encoding len: 5 bytes.
    else if ((ctrlChar & (U8)0xfc) == 0xf8)
	{
        return 5;
	}
    // 1111 110x
    // Encoding len: 6 bytes.
    else if ((ctrlChar & (U8)0xfe) == 0xfc)
	{
        return 6;
	}

	return 0;
}

ALWAYS_INLINE void StringUtil::MergerMultiLine(const std::vector<LibString> &lines, LibString &target)
{
	const Int32 maxLine = static_cast<Int32>(lines.size());
	for(Int32 idx = 0; idx < maxLine; ++idx)
	{
		auto &lineData = lines[idx];
		target.AppendFormat("%s", lineData.c_str());
		if(idx != (maxLine - 1))
			target.AppendFormat("\n");
	}
}

ALWAYS_INLINE void StringUtil::SepMultiLine(const LibString &multiLine, std::vector<LibString> &lines)
{
	lines = multiLine.Split("\n");
}

ALWAYS_INLINE LibString StringUtil::ToString(const std::vector<LibString> &contents, const LibString &sep)
{
	LibString target;
	ToString(contents, sep, target);
	return target;
}

template<typename T>
ALWAYS_INLINE LibString StringUtil::ToString(const std::vector<T> &contents, const LibString &sep)
{
	std::vector<LibString> strs;
	for(auto &elem : contents)
		strs.push_back(KERNEL_NS::LibString() << elem);
	
	return StringUtil::ToString(strs, sep);
}

template<typename T>
ALWAYS_INLINE LibString StringUtil::ToString(const std::vector<T *> &contents, const LibString &sep)
{
	std::vector<LibString> strs;
	for(auto &elem : contents)
		strs.push_back(KERNEL_NS::LibString() << *elem);
	
	return StringUtil::ToString(strs, sep);
}

template<typename T>
ALWAYS_INLINE LibString StringUtil::ToString(const std::vector<const T *> &contents, const LibString &sep)
{
	std::vector<LibString> strs;
	for(auto &elem : contents)
		strs.push_back(KERNEL_NS::LibString() << *elem);
	
	return StringUtil::ToString(strs, sep);
}

template<typename T>
ALWAYS_INLINE LibString StringUtil::ToString(const std::set<T> &contents, const LibString &sep)
{
	std::vector<LibString> strs;
	for(auto &elem : contents)
		strs.push_back(KERNEL_NS::LibString() << elem);
	
	return StringUtil::ToString(strs, sep);
}

template<typename T>
ALWAYS_INLINE LibString StringUtil::ToString(const std::set<T *> &contents, const LibString &sep)
{
	std::vector<LibString> strs;
	for(auto &elem : contents)
		strs.push_back(KERNEL_NS::LibString() << *elem);
	
	return StringUtil::ToString(strs, sep);
}

template<typename T>
ALWAYS_INLINE LibString StringUtil::ToString(const std::set<const T *> &contents, const LibString &sep)
{
	std::vector<LibString> strs;
	for(auto &elem : contents)
		strs.push_back(KERNEL_NS::LibString() << *elem);
	
	return StringUtil::ToString(strs, sep);
}

template<typename K, typename V>
ALWAYS_INLINE LibString StringUtil::ToString(const std::map<K, V> &contents, const LibString &sep)
{
	std::vector<LibString> strs;
	for(auto &it : contents)
		strs.push_back(KERNEL_NS::LibString() << it.first << ":" << it.second);
	
	return StringUtil::ToString(strs, sep);
}

template<typename K, typename V>
ALWAYS_INLINE LibString StringUtil::ToString(const std::unordered_map<K, V> &contents, const LibString &sep)
{
	std::vector<LibString> strs;
	for(auto &it : contents)
		strs.push_back(KERNEL_NS::LibString() << it.first << ":" << it.second);
	
	return StringUtil::ToString(strs, sep);
}

template<typename V>
ALWAYS_INLINE LibString StringUtil::ToString(const std::list<V> &contents, const LibString &sep)
{
	std::vector<LibString> strs;
	for(auto &it : contents)
		strs.push_back(KERNEL_NS::LibString() << it);
	
	return StringUtil::ToString(strs, sep);
}

template<typename T, typename CallbackType>
// T具有范围
requires std::ranges::range<T> 
// T必须有 value_type 类型
&& requires
{
	typename T::value_type;
}
// callback 是一个形参是 T::value_type, 返回值是LibString
&& requires(typename T::value_type value,  CallbackType cb)
{
	{ cb(value)	} -> std::same_as<KERNEL_NS::LibString>; 
}
ALWAYS_INLINE LibString StringUtil::ToStringBy(const T &contentsContainer, const LibString &sep, CallbackType &&cb)
{
	std::vector<LibString> strs;
	for(auto &elem : contentsContainer)
		strs.emplace_back(cb(elem));

	return StringUtil::ToString(strs, sep);
}

// callback 需要传key,value进去, 返回string
template<typename K, typename V, typename CallbackType>
requires requires(K key, V value,  CallbackType cb)
{
    { cb(key, value) } -> std::same_as<KERNEL_NS::LibString>; 
}
ALWAYS_INLINE LibString StringUtil::ToStringBy(const std::map<K, V> &dict, const LibString &sep, CallbackType &&cb)
{
    std::vector<LibString> strs;
    for(auto &iter : dict)
        strs.emplace_back(cb(iter.first, iter.second));

    return StringUtil::ToString(strs, sep);
}

KERNEL_END

#endif
