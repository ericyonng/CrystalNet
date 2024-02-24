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
 * Date: 2022-02-21 14:13:45
 * Author: Eric Yonng
 * Description: 检查是否支持可序列化(利用泛型匹配的优先级匹配序列化接口, 返回值constexpr在编译器可以获得值)
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_CHECK_ADAPTER_CHECK_SERIALIZABLE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_CHECK_ADAPTER_CHECK_SERIALIZABLE_H__

#pragma once

#include <kernel/common/DataTypeAdapter.h>

KERNEL_BEGIN

template<typename BuildType>
class LibStream;

#pragma region serializable

// 检查类型可否序列化
template<typename T, typename BuildType, LibDataType::ENUMS>
struct CheckTypeSerializableResult
{
    static constexpr bool value = false;
};

// 简单数据类型是可序列化的
template<typename T, typename BuildType>
struct CheckTypeSerializableResult<T, BuildType, LibDataType::BRIEF_DATA_TYPE>
{
    static constexpr bool value = true;
};

// 指针数据类型是可序列化的
template<typename T, typename BuildType>
struct CheckTypeSerializableResult<T, BuildType, LibDataType::POINTER_TYPE>
{
    static constexpr bool value = true;
};

// 有 bool (T::*)(LibStream<_Build::MT> &) const 签名
template<typename T>
struct CheckTypeSerializableResult<T, _Build::MT, LibDataType::CLASS_TYPE>
{
    template<bool (T::*)(LibStream<_Build::MT> &) const>
    struct CheckRuleParam;

    // 有序列化接口
    template<typename ObjType>
    static constexpr bool CheckRule(ObjType *o, CheckRuleParam<&ObjType::Serialize> *)
    {
        return true;
    }

    // 没有序列化接口
    template<typename ObjType>
    static constexpr bool CheckRule(ObjType *o, ...)
    {
        return false;
    }

    static constexpr bool value = CheckRule((T *)0, 0);
};

// 有 bool (T::*)(LibStream<_Build::TL> &) const 签名
template<typename T>
struct CheckTypeSerializableResult<T, _Build::TL, LibDataType::CLASS_TYPE>
{
    template<bool (T::*)(LibStream<_Build::TL> &) const>
    struct CheckRuleParam;

    // 有序列化接口
    template<typename ObjType>
    static constexpr bool CheckRule(ObjType *o, CheckRuleParam<&ObjType::Serialize> *)
    {
        return true;
    }

    // 没有序列化接口
    template<typename ObjType>
    static constexpr bool CheckRule(ObjType *o, ...)
    {
        return false;
    }

    static constexpr bool value = CheckRule((T *)0, 0);
};

// 简单数据类型, 指针类型, 或者带有bool (T::*)(LibStream<_Build::TL> &) const/bool (T::*)(LibStream<_Build::MT> &) const接口的类
template<typename T, typename BuildType>
struct CheckTypeSerializable
{
    static constexpr bool value = CheckTypeSerializableResult<T, BuildType, LibTraitsDataType<T>::value>::value;
};

#pragma endregion

#pragma region deserializable
// 检查类型可否反序列化
template<typename T, typename BuildType, LibDataType::ENUMS>
struct CheckTypeDeserializableResult
{
    static constexpr bool value = false;
};

// 简单数据类型是可序列化的
template<typename T, typename BuildType>
struct CheckTypeDeserializableResult<T, BuildType, LibDataType::BRIEF_DATA_TYPE>
{
    static constexpr bool value = true;
};

// 指针数据类型是可序列化的
template<typename T, typename BuildType>
struct CheckTypeDeserializableResult<T, BuildType, LibDataType::POINTER_TYPE>
{
    static constexpr bool value = true;
};

// 有 bool (T::*)(bool(T::*)(LibStream<_Build::MT> &) 签名
template<typename T>
struct CheckTypeDeserializableResult<T, _Build::MT, LibDataType::CLASS_TYPE>
{
    template<bool(T::*)(LibStream<_Build::MT> &)>
    struct CheckRuleParam;

    // 有序列化接口
    template<typename ObjType>
    static constexpr bool CheckRule(ObjType *o, CheckRuleParam<&ObjType::DeSerialize> *)
    {
        return true;
    }

    // 没有序列化接口
    template<typename ObjType>
    static constexpr bool CheckRule(ObjType *o, ...)
    {
        return false;
    }

    static constexpr bool value = CheckRule((T *)0, 0);
};

// 有 bool(T::*)(LibStream<_Build::TL> &) 签名
template<typename T>
struct CheckTypeDeserializableResult<T, _Build::TL, LibDataType::CLASS_TYPE>
{
    template<bool(T::*)(LibStream<_Build::TL> &)>
    struct CheckRuleParam;

    // 有序列化接口
    template<typename ObjType>
    static constexpr bool CheckRule(ObjType *o, CheckRuleParam<&ObjType::DeSerialize> *)
    {
        return true;
    }

    // 没有序列化接口
    template<typename ObjType>
    static constexpr bool CheckRule(ObjType *o, ...)
    {
        return false;
    }

    static constexpr bool value = CheckRule((T *)0, 0);
};

// 简单数据类型, 指针类型, 或者带有bool (T::*)(bool(T::*)(LibStream<_Build::MT> &)/bool(T::*)(LibStream<_Build::TL> &)接口的类
template<typename T, typename BuildType>
struct CheckTypeDeserializable
{
    static constexpr bool value = CheckTypeDeserializableResult<T, BuildType, LibTraitsDataType<T>::value>::value;
};

#pragma endregion

#pragma region CheckSerializable

/*
* 检查类型可否序列化: 简单数据类型是可序列化的, 指针类型是可序列化的, 类类型包含了序列化/反序列化接口是可序列化的
*/
template<typename T>
struct CheckSerializable
{
    static constexpr bool value = (CheckTypeSerializable<T, _Build::MT>::value && CheckTypeDeserializable<T, _Build::MT>::value) ||
                                 (CheckTypeSerializable<T, _Build::TL>::value && CheckTypeDeserializable<T, _Build::TL>::value);
};

#pragma endregion

KERNEL_END

#endif
