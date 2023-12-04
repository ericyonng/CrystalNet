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
 * Date: 2022-03-10 13:15:40
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_BIT_WIDTH_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_BIT_WIDTH_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>

KERNEL_BEGIN

template<typename ValueType, typename T>
class BitWidth
{
public:
};

template<typename ValueType>
class BitWidth<ValueType, UInt64>
{
public:
    static constexpr ValueType _value = ValueType(64);
};

template<typename ValueType>
class BitWidth<ValueType, Int64>
{
public:
    static constexpr ValueType _value = ValueType(64);
};

template<typename ValueType>
class BitWidth<ValueType, Double>
{
public:
    static constexpr ValueType _value = ValueType(64);
};

template<typename ValueType>
class BitWidth<ValueType, Float>
{
public:
    static constexpr ValueType _value = ValueType(32);
};

template<typename ValueType>
class BitWidth<ValueType, ULong>
{
public:
    static constexpr ValueType _value = ValueType(32);
};

template<typename ValueType>
class BitWidth<ValueType, Long>
{
public:
    static constexpr ValueType _value = ValueType(32);
};

template<typename ValueType>
class BitWidth<ValueType, UInt32>
{
public:
    static constexpr ValueType _value = ValueType(32);
};

template<typename ValueType>
class BitWidth<ValueType, Int32>
{
public:
    static constexpr ValueType _value = ValueType(32);
};

template<typename ValueType>
class BitWidth<ValueType, UInt16>
{
public:
    static constexpr ValueType _value = ValueType(16);
};

template<typename ValueType>
class BitWidth<ValueType, Int16>
{
public:
    static constexpr ValueType _value = ValueType(16);
};

template<typename ValueType>
class BitWidth<ValueType, U8>
{
public:
    static constexpr ValueType _value = ValueType(8);
};

template<typename ValueType>
class BitWidth<ValueType, Byte8>
{
public:
    static constexpr ValueType _value = ValueType(8);
};

KERNEL_END

#endif
