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
 * Date: 2024-08-02 11:10:36
 * Author: Eric Yonng
 * Description: 由高64位, 低64位的UInt64组成
 * 需要实现四则运算, 加减乘除,不需要实现乘除, 过于复杂，只实现加减
 * 位运算, 与或非异或
 * 逻辑运算，AND, OR, !运算
 * 比较运算符,等等, 不等, 大于, 大等于, 小于, 小等于
 * 构造: 数值构造
 * 转字符串
 * 转十六进制字符串
 * 
 * 可以生成全球唯一id, 以便大规模机器使用
 * 
 * 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_BIG_NUM_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_BIG_NUM_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>
#include <kernel/common/DataTypeAdapter.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class BigNum;

template<typename T>
concept NumericTypeTraits = IsLibBriefDataType<T>::value;

struct KERNEL_EXPORT BigNumRaw
{
    UInt64 _high;
    UInt64 _low;
};

class KERNEL_EXPORT BigNum
{
    POOL_CREATE_OBJ_DEFAULT(BigNum);

public:
    // 64位最大值
    static constexpr UInt64 MAX64_NUM = 0xFFFFFFFFFFFFFFFFLLU;
    
    // 0
    static constexpr BigNumRaw ZeroBigNum = {0, 0};

public:
    ALWAYS_INLINE BigNum(UInt64 high, UInt64 low)
    {
        _raw._high = high;
        _raw._low = low;
    }

    ALWAYS_INLINE BigNum(const BigNumRaw &other)
    :_raw(other)
    {

    }

    template<NumericTypeTraits T>
    ALWAYS_INLINE BigNum(T number)
    :_raw{0, (UInt64)number}
    {
        
    }

    template<NumericTypeTraits T>
    ALWAYS_INLINE BigNum operator +(T other) const
    {
        // TODO:可以使用SIMD指令来加速运算
        // 大数运算规则, 将UInt64拆分成两个32位整数运算
        // UInt64 num = (UInt64)high1 << 32 + (UInt64)low1;
        // UInt64 num2 = (UInt64)high2 << 32 + (UInt64)low2;
        UInt64 turn64 = (UInt64)other;
        
        // 判断是否会溢出
        BigNumRaw newRaw = {_raw._high, 0};
        if(UNLIKELY(MAX64_NUM - _raw._low < turn64))
        {
            // low + turn64 = MAX_NUM + 1 + x
            // x = low - MAX_NUM - 1 + turn64
            newRaw._low = _raw._low - MAX64_NUM - 1 + turn64;

            // 进位, 两个最大max64最多也就进1位置 max64 + max64 = 2 * (max64+1-1) = 2 * (max64 + 1) - 2 < 2 * (max64 + 1)
            // max64 + 1相当于进位1，2 * (max64 + 1)相当于进2位，但是有 - 2所以不可能进两位
            // 只管低位进位, 不管高位溢出
            newRaw._high += 1;
        }

        // 没有溢出
        else
        {
            newRaw._low = _raw._low + turn64;
        }
        
        // 编译器会进行优化
        return BigNum(newRaw);
    }

    template<NumericTypeTraits T>
    ALWAYS_INLINE BigNum &operator +=(T other)
    {
        // TODO:可以使用SIMD指令来加速运算
        // 大数运算规则, 将UInt64拆分成两个32位整数运算
        // UInt64 num = (UInt64)high1 << 32 + (UInt64)low1;
        // UInt64 num2 = (UInt64)high2 << 32 + (UInt64)low2;
        UInt64 turn64 = (UInt64)other;
        
        // 判断是否会溢出
        if(UNLIKELY(MAX64_NUM - _raw._low < turn64))
        {
            _raw._low += (turn64 - MAX64_NUM - 1);

            // 进位, 两个最大max64最多也就进1位置 max64 + max64 = 2 * (max64+1-1) = 2 * (max64 + 1) - 2 < 2 * (max64 + 1)
            // max64 + 1相当于进位1，2 * (max64 + 1)相当于进2位，但是有 - 2所以不可能进两位
            // 只管低位进位, 不管高位溢出
            _raw._high += 1;
        }

        // 没有溢出
        else
        {
            _raw._low += turn64;
        }
        
        return *this;
    }

    ALWAYS_INLINE BigNum operator +(const BigNum &bigNum) const
    {
        // 先求低位, 再求高位
        BigNumRaw newRaw = {_raw._high, _raw._low};

        // 低位溢出进位
        auto &otherRaw = bigNum._raw;
        if(UNLIKELY(MAX64_NUM - _raw._low < otherRaw._low))
        {
            newRaw._low += (otherRaw._low - MAX64_NUM - 1);

            newRaw._high += 1;
        }
        else
        {
            newRaw._low += otherRaw._low;
        }

        // 不管溢出
        newRaw._high += (_raw._high + otherRaw._high);

        return BigNum(newRaw);
    }

    ALWAYS_INLINE BigNum &operator +=(const BigNum &bigNum)
    {
        // 低位溢出进位
        auto &otherRaw = bigNum._raw;
        if(UNLIKELY(MAX64_NUM - _raw._low < otherRaw._low))
        {
            _raw._low += ((otherRaw._low - MAX64_NUM - 1));

            _raw._high += 1;
        }
        else
        {
            _raw._low += otherRaw._low;
        }

        // 不管溢出
        _raw._high += otherRaw._high;

        return *this;
    }

    template<NumericTypeTraits T>
    ALWAYS_INLINE BigNum operator -(T other) const
    {
        // TODO:可以使用SIMD指令来加速运算
        // 大数运算规则, 将UInt64拆分成两个32位整数运算
        // UInt64 num = (UInt64)high1 << 32 + (UInt64)low1;
        // UInt64 num2 = (UInt64)high2 << 32 + (UInt64)low2;
        UInt64 turn64 = (UInt64)other;
        
        // 判断是否会溢出
        BigNumRaw newRaw = {_raw._high, _raw._low};
        if(_raw._low < turn64)
        {
            // 高位借位
            newRaw._low += (MAX64_NUM - turn64 + 1);

            // 不管高位溢出
            newRaw._high -= 1;
        }

        // 没有溢出
        else
        {
            newRaw._low -= turn64;
        }
        
        // 编译器会进行优化
        return BigNum(newRaw);
    }

    template<NumericTypeTraits T>
    ALWAYS_INLINE BigNum &operator -=(T other)
    {
        // TODO:可以使用SIMD指令来加速运算
        // 大数运算规则, 将UInt64拆分成两个32位整数运算
        // UInt64 num = (UInt64)high1 << 32 + (UInt64)low1;
        // UInt64 num2 = (UInt64)high2 << 32 + (UInt64)low2;
        UInt64 turn64 = (UInt64)other;
        
        // 判断是否会溢出
        if(_raw._low < turn64)
        {
            // 高位借位
            _raw._low += (MAX64_NUM - turn64 + 1);

            // 不管高位溢出
            _raw._high -= 1;
        }

        // 没有溢出
        else
        {
            _raw._low -= turn64;
        }
        
        return *this;
    }

    ALWAYS_INLINE BigNum operator -(const BigNum &bigNum) const
    {
        // 先求低位, 再求高位
        BigNumRaw newRaw = {_raw._high, _raw._low};

        // 低位溢出借位
        auto &otherRaw = bigNum._raw;
        if(_raw._low < otherRaw._low)
        {
            newRaw._low += (MAX64_NUM - otherRaw._low + 1);

            newRaw._high -= 1;
        }
        else
        {
            newRaw._low -= otherRaw._low;
        }

        // 不管溢出
        newRaw._high -= otherRaw._high;

        return BigNum(newRaw);
    }

    ALWAYS_INLINE BigNum &operator -=(const BigNum &bigNum)
    {
        // 低位溢出进位
        auto &otherRaw = bigNum._raw;
        if(UNLIKELY((MAX64_NUM - _raw._low) < otherRaw._low))
        {
            _raw._low += (MAX64_NUM - otherRaw._low + 1);

            _raw._high -= 1;
        }
        else
        {
            _raw._low -= otherRaw._low;
        }

        // 不管溢出
        _raw._high -= otherRaw._high;

        return *this;
    }

    template<NumericTypeTraits T>
    ALWAYS_INLINE BigNum operator &(T number) const
    {
        return BigNum(0, _raw._low & (UInt64)number);
    }

    ALWAYS_INLINE BigNum operator &(const BigNum &number) const
    {
        auto &otherRaw = number._raw;
        return BigNum(_raw._high & otherRaw._high, _raw._low & otherRaw._low);
    }

    template<NumericTypeTraits T>
    ALWAYS_INLINE BigNum &operator &=(T number)
    {
        _raw._low &= (UInt64)number;
        _raw._high = 0;

        return *this; 
    }

    ALWAYS_INLINE BigNum &operator &=(const BigNum &number)
    {
        auto &otherRaw = number._raw;
        _raw._low &= otherRaw._low;
        _raw._high &= otherRaw._high;

        return *this;
    }

//////////////////////

    ALWAYS_INLINE BigNum operator <<(Int32 number) const
    {
        if(number >= 64)
        {
            return BigNum(_raw._low << (number - 64), 0);
        }

        return BigNum((_raw._high << number) | (_raw._low >> (64 - number)), _raw._low << number);
    }

    ALWAYS_INLINE BigNum operator >>(Int32 number) const
    {
        if(number >= 64)
        {
            return BigNum(0, _raw._high >> (number - 64));
        }
        // 低位:高位先通过保留低number位,并左移到64 - number位 | 低位 >> number即结果所得
        return BigNum((_raw._high >> number), ((_raw._high & ~(1LLU << number)) << (64 - number)) | _raw._low >> number);
    }

    ALWAYS_INLINE BigNum &operator <<=(Int32 number)
    {
        if(number >= 64)
        {
            _raw._high = _raw._low << (number - 64);
            _raw._low = 0;

            return *this;
        }

        _raw._high = (_raw._high << number) | (_raw._low >> (64 - number));
        _raw._low = _raw._low << number;

        return *this;
    }

    ALWAYS_INLINE BigNum &operator >>=(Int32 number)
    {
        if(number >= 64)
        {
            _raw._low = _raw._high >> (number - 64);
            _raw._high = 0;
            return *this;
        }

        // 低位:高位先通过保留低number位,并左移到64 - number位 | 低位 >> number即结果所得
        auto newHigh = _raw._high >> number;
        _raw._low = ((_raw._high & ~(1LLU << number)) << (64 - number)) | _raw._low >> number;
        _raw._high = newHigh;
        return *this;
    }

    template<NumericTypeTraits T>
    ALWAYS_INLINE BigNum operator |(T number) const
    {
        return BigNum(_raw._high, _raw._low | (UInt64)number);
    }

    ALWAYS_INLINE BigNum operator |(const BigNum &number) const
    {
        auto &otherRaw = number._raw;

        return BigNum(_raw._high | otherRaw._high, _raw._low | otherRaw._low);
    }

    
    template<NumericTypeTraits T>
    ALWAYS_INLINE BigNum &operator |=(T number)
    {
        _raw._low |= (UInt64)number;

        return *this; 
    }

    ALWAYS_INLINE BigNum &operator |=(const BigNum &number)
    {
        auto &otherRaw = number._raw;

        _raw._low |= otherRaw._low;
        _raw._high |= otherRaw._high;

        return *this;
    }

    template<NumericTypeTraits T>
    ALWAYS_INLINE BigNum operator ^(T number) const
    {
        return BigNum(_raw._high ^ 0, _raw._low ^ (UInt64)number);
    }

    ALWAYS_INLINE BigNum operator ^(const BigNum &number) const
    {
        auto &otherRaw = number._raw;

        return BigNum(_raw._high ^ otherRaw._high, _raw._low ^ otherRaw._low);
    }

    template<NumericTypeTraits T>
    ALWAYS_INLINE BigNum &operator ^=(T number)
    {
        _raw._low ^= (UInt64)number;

        return *this; 
    }

    ALWAYS_INLINE BigNum &operator ^=(const BigNum &number)
    {
        auto &otherRaw = number._raw;

        _raw._low ^= otherRaw._low;
        _raw._high ^= otherRaw._high;

        return *this;
    }

    ALWAYS_INLINE BigNum operator ~() const
    {
        return BigNum(~_raw._high, ~_raw._low);
    }

    // 有operator bool 可以不重载逻辑运算符, 因为会隐式转换成bool
    // ALWAYS_INLINE operator bool() const
    // {
    //     return _raw._low != 0 || _raw._high != 0;
    // }

    ALWAYS_INLINE bool operator ==(const BigNum &number) const
    {
        auto &otherRaw = number._raw;

        return (_raw._low == otherRaw._low) && (_raw._high == otherRaw._high);
    }

    ALWAYS_INLINE bool operator !=(const BigNum &number) const
    {
        return !operator==(number);
    }

    ALWAYS_INLINE bool operator <(const BigNum &number) const
    {
        auto &otherRaw = number._raw;

        return (_raw._high < otherRaw._high) || ((_raw._high == otherRaw._high) && (_raw._low < otherRaw._low)); 
    }

    ALWAYS_INLINE bool operator <=(const BigNum &number) const
    {
        return operator==(number) || operator<(number);
    }

    ALWAYS_INLINE bool operator >(const BigNum &number) const
    {
        return !operator<=(number);
    }

    ALWAYS_INLINE bool operator >=(const BigNum &number) const
    {
        return !operator<(number);
    }

    template<NumericTypeTraits T>
    ALWAYS_INLINE BigNum &operator =(T number)
    {
        _raw._low = (UInt64)number;
        _raw._high = 0;
        return *this;
    }

    LibString ToString() const;

    LibString ToHexString() const;

    ALWAYS_INLINE const BigNumRaw &GetRaw() const
    {
        return _raw;
    }

    template<typename T>
    ALWAYS_INLINE bool operator && (const T &other) const
    {
        return IsNotZero() && other;
    }

    ALWAYS_INLINE bool operator && (const BigNum &other) const
    {
        return IsNotZero() && other.IsNotZero();
    }
    
    template<typename T>
    ALWAYS_INLINE bool operator || (const T &other) const
    {
        return IsNotZero() || other;
    }

    ALWAYS_INLINE bool operator || (const BigNum &other) const
    {
        return IsNotZero() || other.IsNotZero();
    }

    ALWAYS_INLINE bool operator! () const
    {
        return IsNotZero();
    }

    ALWAYS_INLINE bool IsZero() const
    {
        return (_raw._low == 0) && (_raw._high == 0);
    }

    ALWAYS_INLINE bool IsNotZero() const
    {
        return (_raw._low != 0) || (_raw._high != 0);
    }

private:
    BigNumRaw _raw;
};

ALWAYS_INLINE LibString BigNum::ToString() const
{
    return LibString().AppendFormat("%llu%llu", _raw._high, _raw._low);
}

ALWAYS_INLINE LibString BigNum::ToHexString() const
{
    return LibString().AppendFormat("%llx%llx", _raw._high, _raw._low);
}

KERNEL_END

template<KERNEL_NS::NumericTypeTraits T>
extern ALWAYS_INLINE bool operator && (T number, const KERNEL_NS::BigNum &other)
{
    return number && other.IsNotZero();
}

template<KERNEL_NS::NumericTypeTraits T>
extern ALWAYS_INLINE bool operator || (T number, const KERNEL_NS::BigNum &other)
{
    return number || other.IsNotZero();
}

template<KERNEL_NS::NumericTypeTraits T>
extern ALWAYS_INLINE bool operator == (T number, const KERNEL_NS::BigNum &other)
{
    return other == number;
}

template<KERNEL_NS::NumericTypeTraits T>
extern ALWAYS_INLINE bool operator != (T number, const KERNEL_NS::BigNum &other)
{
    return other != number;
}

template<KERNEL_NS::NumericTypeTraits T>
extern ALWAYS_INLINE bool operator < (T number, const KERNEL_NS::BigNum &other)
{
    return other >= number;
}

template<KERNEL_NS::NumericTypeTraits T>
extern ALWAYS_INLINE bool operator <= (T number, const KERNEL_NS::BigNum &other)
{
    return other > number;
}

template<KERNEL_NS::NumericTypeTraits T>
extern ALWAYS_INLINE bool operator > (T number, const KERNEL_NS::BigNum &other)
{
    return other <= number;
}

template<KERNEL_NS::NumericTypeTraits T>
extern ALWAYS_INLINE bool operator >= (T number, const KERNEL_NS::BigNum &other)
{
    return other < number;
}
#endif
