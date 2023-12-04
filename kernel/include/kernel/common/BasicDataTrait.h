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
 * Date: 2022-02-21 19:08:36
 * Author: Eric Yonng
 * Description: 基本数据类型识别
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_BASIC_DATA_TRAIT_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_BASIC_DATA_TRAIT_H__

#pragma once

#include <type_traits>
#include <kernel/common/BaseType.h>
#include <kernel/common/BaseMacro.h>

KERNEL_BEGIN

template<class _Ty>
struct is_basic_data : public std::false_type
{};

template<>
struct is_basic_data<bool> : public std::true_type
{};
template<>
struct is_basic_data<bool &> : public std::true_type
{};
template<>
struct is_basic_data<bool &&> : public std::true_type
{};
template<>
struct is_basic_data<const bool> : public std::true_type
{};
template<>
struct is_basic_data<const bool &> : public std::true_type
{};

template<>
struct is_basic_data<U8> : public std::true_type
{};
template<>
struct is_basic_data<U8 &> : public std::true_type
{};
template<>
struct is_basic_data<U8 &&> : public std::true_type
{};
template<>
struct is_basic_data<const U8> : public std::true_type
{};
template<>
struct is_basic_data<const U8 &> : public std::true_type
{};

template<>
struct is_basic_data<Byte8> : public std::true_type
{};
template<>
struct is_basic_data<Byte8 &> : public std::true_type
{};
template<>
struct is_basic_data<Byte8 &&> : public std::true_type
{};
template<>
struct is_basic_data<const Byte8> : public std::true_type
{};
template<>
struct is_basic_data<const Byte8 &> : public std::true_type
{};

template<>
struct is_basic_data<UInt16> : public std::true_type
{};
template<>
struct is_basic_data<UInt16 &> : public std::true_type
{};
template<>
struct is_basic_data<UInt16 &&> : public std::true_type
{};
template<>
struct is_basic_data<const UInt16> : public std::true_type
{};
template<>
struct is_basic_data<const UInt16 &> : public std::true_type
{};

template<>
struct is_basic_data<Int16> : public std::true_type
{};
template<>
struct is_basic_data<Int16 &> : public std::true_type
{};
template<>
struct is_basic_data<Int16 &&> : public std::true_type
{};
template<>
struct is_basic_data<const Int16> : public std::true_type
{};
template<>
struct is_basic_data<const Int16 &> : public std::true_type
{};

template<>
struct is_basic_data<UInt32> : public std::true_type
{};
template<>
struct is_basic_data<UInt32 &> : public std::true_type
{};
template<>
struct is_basic_data<UInt32 &&> : public std::true_type
{};
template<>
struct is_basic_data<const UInt32> : public std::true_type
{};
template<>
struct is_basic_data<const UInt32 &> : public std::true_type
{};

template<>
struct is_basic_data<Int32> : public std::true_type
{};
template<>
struct is_basic_data<Int32 &> : public std::true_type
{};
template<>
struct is_basic_data<Int32 &&> : public std::true_type
{};
template<>
struct is_basic_data<const Int32> : public std::true_type
{};
template<>
struct is_basic_data<const Int32 &> : public std::true_type
{};

template<>
struct is_basic_data<UInt64> : public std::true_type
{};
template<>
struct is_basic_data<UInt64 &> : public std::true_type
{};
template<>
struct is_basic_data<UInt64 &&> : public std::true_type
{};
template<>
struct is_basic_data<const UInt64> : public std::true_type
{};
template<>
struct is_basic_data<const UInt64 &> : public std::true_type
{};

template<>
struct is_basic_data<Int64> : public std::true_type
{};
template<>
struct is_basic_data<Int64 &> : public std::true_type
{};
template<>
struct is_basic_data<Int64 &&> : public std::true_type
{};
template<>
struct is_basic_data<const Int64> : public std::true_type
{};
template<>
struct is_basic_data<const Int64 &> : public std::true_type
{};

template<>
struct is_basic_data<Float> : public std::true_type
{};
template<>
struct is_basic_data<Float &> : public std::true_type
{};
template<>
struct is_basic_data<Float &&> : public std::true_type
{};
template<>
struct is_basic_data<const Float> : public std::true_type
{};
template<>
struct is_basic_data<const Float &> : public std::true_type
{};

template<>
struct is_basic_data<Double> : public std::true_type
{};
template<>
struct is_basic_data<Double &> : public std::true_type
{};
template<>
struct is_basic_data<Double &&> : public std::true_type
{};
template<>
struct is_basic_data<const Double> : public std::true_type
{};
template<>
struct is_basic_data<const Double &> : public std::true_type
{};

KERNEL_END

#endif
