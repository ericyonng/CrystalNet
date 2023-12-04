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
 * Author: Eric Yonng
 * Date: 2021-03-16 15:27:30
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_VARIANT_VARIANT_ARITHMETIC_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_VARIANT_VARIANT_ARITHMETIC_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>

KERNEL_BEGIN

class Variant;

class KERNEL_EXPORT VariantArithmetic
{
public:
    /**
     * Operations type enumeration.
     */
    enum
    {
        VT_ARITHMETIC_ADD = 0,
        VT_ARITHMETIC_SUB = 1,
        VT_ARITHMETIC_MUL = 2,
        VT_ARITHMETIC_DIV = 3,

        VT_ARITHMETIC_END
    };

    /**
     * Performs arithmetic.
     * @param[in/out] left - left object.
     * @param[in] right    - right object.
     * @param[in] type     - operation type, see enumerations value.
     */
    static void Performs(Variant &left, const Variant &right, int type);

private:
    /**
     * Implementation functions.
     */
    static void _Performs_bool_any(Variant &left, const Variant &right, int type);
    static void _Performs_byte8_any(Variant &left, const Variant &right, int type);
    static void _Performs_uint8_any(Variant &left, const Variant &right, int type);
    static void _Performs_int16_any(Variant &left, const Variant &right, int type);
    static void _Performs_uint16_any(Variant &left, const Variant &right, int type);
    static void _Performs_int32_any(Variant &left, const Variant &right, int type);
    static void _Performs_uint32_any(Variant &left, const Variant &right, int type);
    static void _Performs_long_any(Variant &left, const Variant &right, int type);
    static void _Performs_ulong_any(Variant &left, const Variant &right, int type);
    static void _Performs_ptr_any(Variant &left, const Variant &right, int type);
    static void _Performs_int64_any(Variant &left, const Variant &right, int type);
    static void _Performs_uint64_any(Variant &left, const Variant &right, int type);
    static void _Performs_float_any(Variant &left, const Variant &right, int type);
    static void _Performs_double_any(Variant &left, const Variant &right, int type);

    /**
     * Implementation function.
     */
    template <typename RawType>
    static RawType _Performs_raw_operation(RawType left, RawType right, int type);
};


template <>
ALWAYS_INLINE bool VariantArithmetic::_Performs_raw_operation(bool left, bool right, int type)
{
    switch(type)
    {
        case VT_ARITHMETIC_ADD:
            return left || right;

        case VT_ARITHMETIC_SUB:
            if(left) // true - true = false, true - false = true.
                return !right;
            else // false - true = false, false - false = false.
                return false;

        case VT_ARITHMETIC_MUL:
        case VT_ARITHMETIC_DIV:
            return left && right; // left == true and right == true return true, otherwise return false.

        default:
            break;
    }

    return bool();
}

template <>
ALWAYS_INLINE void *VariantArithmetic::_Performs_raw_operation(void *left, void *right, int type)
{
    Int64 leftVal = 0, rightVal = 0;
    ::memcpy(&leftVal, &left, sizeof(void *));
    ::memcpy(&rightVal, &right, sizeof(void *));

    switch(type)
    {
        case VT_ARITHMETIC_ADD:
            leftVal += rightVal;
            break;

        case VT_ARITHMETIC_SUB:
            leftVal -= rightVal;
            break;

        case VT_ARITHMETIC_MUL:
            leftVal *= rightVal;
            break;

        case VT_ARITHMETIC_DIV:
            leftVal /= rightVal;
            break;

        default:
            leftVal = 0;
            break;
    }

    void *finalPtr = NULL;
    ::memcpy(&finalPtr, &leftVal, sizeof(void *));

    return finalPtr;
}

template <typename RawType>
ALWAYS_INLINE RawType VariantArithmetic::_Performs_raw_operation(RawType left, RawType right, int type)
{
    switch(type)
    {
        case VT_ARITHMETIC_ADD:
            return left + right;

        case VT_ARITHMETIC_SUB:
            return left - right;

        case VT_ARITHMETIC_MUL:
            return left * right;

        case VT_ARITHMETIC_DIV:
            return left / right;

        default:
            break;
    }

    return RawType();
}


KERNEL_END

#endif
