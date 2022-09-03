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
 * Date: 2022-08-21 19:36:42
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Variant/Variant.h>

#include <kernel/comp/Variant/VariantArithmetic.h>

KERNEL_BEGIN

void VariantArithmetic::_Performs_bool_any(
    Variant &left, const Variant &right, int type)
{
    switch(right.GetType())
    {
        case VariantRtti::VT_BRIEF_BOOL:
            left = _Performs_raw_operation(left.AsBool(), right.AsBool(), type);
            break;

        case VariantRtti::VT_BRIEF_BYTE8:
            left = _Performs_raw_operation(left.AsByte8(), right.AsByte8(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT8:
            left = _Performs_raw_operation(left.AsUInt8(), right.AsUInt8(), type);
            break;

        case VariantRtti::VT_BRIEF_INT16:
            left = _Performs_raw_operation(left.AsInt16(), right.AsInt16(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT16:
            left = _Performs_raw_operation(left.AsUInt16(), right.AsUInt16(), type);
            break;

        case VariantRtti::VT_BRIEF_INT32:
            left = _Performs_raw_operation(left.AsInt32(), right.AsInt32(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT32:
            left = _Performs_raw_operation(left.AsUInt32(), right.AsUInt32(), type);
            break;

        case VariantRtti::VT_BRIEF_LONG:
            left = _Performs_raw_operation(left.AsLong(), right.AsLong(), type);
            break;

        case VariantRtti::VT_BRIEF_ULONG:
            left = _Performs_raw_operation(left.AsULong(), right.AsULong(), type);
            break;

        case VariantRtti::VT_BRIEF_PTR:
            left = _Performs_raw_operation(left.AsPtr<void>(), right.AsPtr<void>(), type);
            break;

        case VariantRtti::VT_BRIEF_INT64:
            left = _Performs_raw_operation(left.AsInt64(), right.AsInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT64:
            left = _Performs_raw_operation(left.AsUInt64(), right.AsUInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_FLOAT:
        case VariantRtti::VT_BRIEF_DOUBLE:
            left = _Performs_raw_operation(left.AsDouble(), right.AsDouble(), type);
            break;

        default:
            break;
    }
}

void VariantArithmetic::_Performs_byte8_any(
    Variant &left, const Variant &right, int type)
{
    switch(right.GetType())
    {
        case VariantRtti::VT_BRIEF_BOOL:
        case VariantRtti::VT_BRIEF_BYTE8:
            left = _Performs_raw_operation(left.AsByte8(), right.AsByte8(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT8:
            left = _Performs_raw_operation(left.AsUInt8(), right.AsUInt8(), type);
            break;

        case VariantRtti::VT_BRIEF_INT16:
            left = _Performs_raw_operation(left.AsInt16(), right.AsInt16(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT16:
            left = _Performs_raw_operation(left.AsUInt16(), right.AsUInt16(), type);
            break;

        case VariantRtti::VT_BRIEF_INT32:
            left = _Performs_raw_operation(left.AsInt32(), right.AsInt32(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT32:
            left = _Performs_raw_operation(left.AsUInt32(), right.AsUInt32(), type);
            break;

        case VariantRtti::VT_BRIEF_LONG:
            left = _Performs_raw_operation(left.AsLong(), right.AsLong(), type);
            break;

        case VariantRtti::VT_BRIEF_ULONG:
            left = _Performs_raw_operation(left.AsULong(), right.AsULong(), type);
            break;

        case VariantRtti::VT_BRIEF_PTR:
            left = _Performs_raw_operation(left.AsPtr<void>(), right.AsPtr<void>(), type);
            break;

        case VariantRtti::VT_BRIEF_INT64:
            left = _Performs_raw_operation(left.AsInt64(), right.AsInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT64:
            left = _Performs_raw_operation(left.AsUInt64(), right.AsUInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_FLOAT:
        case VariantRtti::VT_BRIEF_DOUBLE:
            left = _Performs_raw_operation(left.AsDouble(), right.AsDouble(), type);
            break;

        default:
            break;
    }
}

void VariantArithmetic::_Performs_uint8_any(
    Variant &left, const Variant &right, int type)
{
    switch(right.GetType())
    {
        case VariantRtti::VT_BRIEF_BOOL:
        case VariantRtti::VT_BRIEF_BYTE8:
        case VariantRtti::VT_BRIEF_UINT8:
            left = _Performs_raw_operation(left.AsUInt8(), right.AsUInt8(), type);
            break;

        case VariantRtti::VT_BRIEF_INT16:
            left = _Performs_raw_operation(left.AsInt16(), right.AsInt16(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT16:
            left = _Performs_raw_operation(left.AsUInt16(), right.AsUInt16(), type);
            break;

        case VariantRtti::VT_BRIEF_INT32:
            left = _Performs_raw_operation(left.AsInt32(), right.AsInt32(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT32:
            left = _Performs_raw_operation(left.AsUInt32(), right.AsUInt32(), type);
            break;

        case VariantRtti::VT_BRIEF_LONG:
            left = _Performs_raw_operation(left.AsLong(), right.AsLong(), type);
            break;

        case VariantRtti::VT_BRIEF_ULONG:
            left = _Performs_raw_operation(left.AsULong(), right.AsULong(), type);
            break;

        case VariantRtti::VT_BRIEF_PTR:
            left = _Performs_raw_operation(left.AsPtr<void>(), right.AsPtr<void>(), type);
            break;

        case VariantRtti::VT_BRIEF_INT64:
            left = _Performs_raw_operation(left.AsInt64(), right.AsInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT64:
            left = _Performs_raw_operation(left.AsUInt64(), right.AsUInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_FLOAT:
        case VariantRtti::VT_BRIEF_DOUBLE:
            left = _Performs_raw_operation(left.AsDouble(), right.AsDouble(), type);
            break;

        default:
            break;
    }
}

void VariantArithmetic::_Performs_int16_any(
    Variant &left, const Variant &right, int type)
{
    switch(right.GetType())
    {
        case VariantRtti::VT_BRIEF_BOOL:
        case VariantRtti::VT_BRIEF_BYTE8:
        case VariantRtti::VT_BRIEF_UINT8:
        case VariantRtti::VT_BRIEF_INT16:
            left = _Performs_raw_operation(left.AsInt16(), right.AsInt16(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT16:
            left = _Performs_raw_operation(left.AsUInt16(), right.AsUInt16(), type);
            break;

        case VariantRtti::VT_BRIEF_INT32:
            left = _Performs_raw_operation(left.AsInt32(), right.AsInt32(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT32:
            left = _Performs_raw_operation(left.AsUInt32(), right.AsUInt32(), type);
            break;

        case VariantRtti::VT_BRIEF_LONG:
            left = _Performs_raw_operation(left.AsLong(), right.AsLong(), type);
            break;

        case VariantRtti::VT_BRIEF_ULONG:
            left = _Performs_raw_operation(left.AsULong(), right.AsULong(), type);
            break;

        case VariantRtti::VT_BRIEF_PTR:
            left = _Performs_raw_operation(left.AsPtr<void>(), right.AsPtr<void>(), type);
            break;

        case VariantRtti::VT_BRIEF_INT64:
            left = _Performs_raw_operation(left.AsInt64(), right.AsInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT64:
            left = _Performs_raw_operation(left.AsUInt64(), right.AsUInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_FLOAT:
        case VariantRtti::VT_BRIEF_DOUBLE:
            left = _Performs_raw_operation(left.AsDouble(), right.AsDouble(), type);
            break;

        default:
            break;
    }
}

void VariantArithmetic::_Performs_uint16_any(
    Variant &left, const Variant &right, int type)
{
    switch(right.GetType())
    {
        case VariantRtti::VT_BRIEF_BOOL:
        case VariantRtti::VT_BRIEF_BYTE8:
        case VariantRtti::VT_BRIEF_UINT8:
        case VariantRtti::VT_BRIEF_INT16:
        case VariantRtti::VT_BRIEF_UINT16:
            left = _Performs_raw_operation(left.AsUInt16(), right.AsUInt16(), type);
            break;

        case VariantRtti::VT_BRIEF_INT32:
            left = _Performs_raw_operation(left.AsInt32(), right.AsInt32(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT32:
            left = _Performs_raw_operation(left.AsUInt32(), right.AsUInt32(), type);
            break;

        case VariantRtti::VT_BRIEF_LONG:
            left = _Performs_raw_operation(left.AsLong(), right.AsLong(), type);
            break;

        case VariantRtti::VT_BRIEF_ULONG:
            left = _Performs_raw_operation(left.AsULong(), right.AsULong(), type);
            break;

        case VariantRtti::VT_BRIEF_PTR:
            left = _Performs_raw_operation(left.AsPtr<void>(), right.AsPtr<void>(), type);
            break;

        case VariantRtti::VT_BRIEF_INT64:
            left = _Performs_raw_operation(left.AsInt64(), right.AsInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT64:
            left = _Performs_raw_operation(left.AsUInt64(), right.AsUInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_FLOAT:
        case VariantRtti::VT_BRIEF_DOUBLE:
            left = _Performs_raw_operation(left.AsDouble(), right.AsDouble(), type);
            break;

        default:
            break;
    }
}

void VariantArithmetic::_Performs_int32_any(
    Variant &left, const Variant &right, int type)
{
    switch(right.GetType())
    {
        case VariantRtti::VT_BRIEF_BOOL:
        case VariantRtti::VT_BRIEF_BYTE8:
        case VariantRtti::VT_BRIEF_UINT8:
        case VariantRtti::VT_BRIEF_INT16:
        case VariantRtti::VT_BRIEF_UINT16:
        case VariantRtti::VT_BRIEF_INT32:
            left = _Performs_raw_operation(left.AsInt32(), right.AsInt32(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT32:
            left = _Performs_raw_operation(left.AsUInt32(), right.AsUInt32(), type);
            break;

        case VariantRtti::VT_BRIEF_LONG:
            left = _Performs_raw_operation(left.AsLong(), right.AsLong(), type);
            break;

        case VariantRtti::VT_BRIEF_ULONG:
            left = _Performs_raw_operation(left.AsULong(), right.AsULong(), type);
            break;

        case VariantRtti::VT_BRIEF_PTR:
            left = _Performs_raw_operation(left.AsPtr<void>(), right.AsPtr<void>(), type);
            break;

        case VariantRtti::VT_BRIEF_INT64:
            left = _Performs_raw_operation(left.AsInt64(), right.AsInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT64:
            left = _Performs_raw_operation(left.AsUInt64(), right.AsUInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_FLOAT:
        case VariantRtti::VT_BRIEF_DOUBLE:
            left = _Performs_raw_operation(left.AsDouble(), right.AsDouble(), type);
            break;

        default:
            break;
    }
}

void VariantArithmetic::_Performs_uint32_any(
    Variant &left, const Variant &right, int type)
{
    switch(right.GetType())
    {
        case VariantRtti::VT_BRIEF_BOOL:
        case VariantRtti::VT_BRIEF_BYTE8:
        case VariantRtti::VT_BRIEF_UINT8:
        case VariantRtti::VT_BRIEF_INT16:
        case VariantRtti::VT_BRIEF_UINT16:
        case VariantRtti::VT_BRIEF_INT32:
        case VariantRtti::VT_BRIEF_UINT32:
            left = _Performs_raw_operation(left.AsUInt32(), right.AsUInt32(), type);
            break;

        case VariantRtti::VT_BRIEF_LONG:
            left = _Performs_raw_operation(left.AsLong(), right.AsLong(), type);
            break;

        case VariantRtti::VT_BRIEF_ULONG:
            left = _Performs_raw_operation(left.AsULong(), right.AsULong(), type);
            break;

        case VariantRtti::VT_BRIEF_PTR:
            left = _Performs_raw_operation(left.AsPtr<void>(), right.AsPtr<void>(), type);
            break;

        case VariantRtti::VT_BRIEF_INT64:
            left = _Performs_raw_operation(left.AsInt64(), right.AsInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT64:
            left = _Performs_raw_operation(left.AsUInt64(), right.AsUInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_FLOAT:
        case VariantRtti::VT_BRIEF_DOUBLE:
            left = _Performs_raw_operation(left.AsDouble(), right.AsDouble(), type);
            break;

        default:
            break;
    }
}

void VariantArithmetic::_Performs_long_any(
    Variant &left, const Variant &right, int type)
{
    switch(right.GetType())
    {
        case VariantRtti::VT_BRIEF_BOOL:
        case VariantRtti::VT_BRIEF_BYTE8:
        case VariantRtti::VT_BRIEF_UINT8:
        case VariantRtti::VT_BRIEF_INT16:
        case VariantRtti::VT_BRIEF_UINT16:
        case VariantRtti::VT_BRIEF_INT32:
        case VariantRtti::VT_BRIEF_UINT32:
        case VariantRtti::VT_BRIEF_LONG:
            left = _Performs_raw_operation(left.AsLong(), right.AsLong(), type);
            break;

        case VariantRtti::VT_BRIEF_ULONG:
            left = _Performs_raw_operation(left.AsULong(), right.AsULong(), type);
            break;

        case VariantRtti::VT_BRIEF_PTR:
            left = _Performs_raw_operation(left.AsPtr<void>(), right.AsPtr<void>(), type);
            break;

        case VariantRtti::VT_BRIEF_INT64:
            left = _Performs_raw_operation(left.AsInt64(), right.AsInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT64:
            left = _Performs_raw_operation(left.AsUInt64(), right.AsUInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_FLOAT:
        case VariantRtti::VT_BRIEF_DOUBLE:
            left = _Performs_raw_operation(left.AsDouble(), right.AsDouble(), type);
            break;

        default:
            break;
    }
}

void VariantArithmetic::_Performs_ulong_any(
    Variant &left, const Variant &right, int type)
{
    switch(right.GetType())
    {
        case VariantRtti::VT_BRIEF_BOOL:
        case VariantRtti::VT_BRIEF_BYTE8:
        case VariantRtti::VT_BRIEF_UINT8:
        case VariantRtti::VT_BRIEF_INT16:
        case VariantRtti::VT_BRIEF_UINT16:
        case VariantRtti::VT_BRIEF_INT32:
        case VariantRtti::VT_BRIEF_UINT32:
        case VariantRtti::VT_BRIEF_LONG:
        case VariantRtti::VT_BRIEF_ULONG:
            left = _Performs_raw_operation(left.AsULong(), right.AsULong(), type);
            break;

        case VariantRtti::VT_BRIEF_PTR:
            left = _Performs_raw_operation(left.AsPtr<void>(), right.AsPtr<void>(), type);
            break;

        case VariantRtti::VT_BRIEF_INT64:
            left = _Performs_raw_operation(left.AsInt64(), right.AsInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT64:
            left = _Performs_raw_operation(left.AsUInt64(), right.AsUInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_FLOAT:
        case VariantRtti::VT_BRIEF_DOUBLE:
            left = _Performs_raw_operation(left.AsDouble(), right.AsDouble(), type);
            break;

        default:
            break;
    }
}

void VariantArithmetic::_Performs_ptr_any(
    Variant &left, const Variant &right, int type)
{
    switch(right.GetType())
    {
        case VariantRtti::VT_BRIEF_BOOL:
        case VariantRtti::VT_BRIEF_BYTE8:
        case VariantRtti::VT_BRIEF_UINT8:
        case VariantRtti::VT_BRIEF_INT16:
        case VariantRtti::VT_BRIEF_UINT16:
        case VariantRtti::VT_BRIEF_INT32:
        case VariantRtti::VT_BRIEF_UINT32:
        case VariantRtti::VT_BRIEF_LONG:
        case VariantRtti::VT_BRIEF_ULONG:
        case VariantRtti::VT_BRIEF_PTR:
            left = _Performs_raw_operation(left.AsPtr<void>(), right.AsPtr<void>(), type);
            break;

        case VariantRtti::VT_BRIEF_INT64:
            left = _Performs_raw_operation(left.AsInt64(), right.AsInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT64:
            left = _Performs_raw_operation(left.AsUInt64(), right.AsUInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_FLOAT:
        case VariantRtti::VT_BRIEF_DOUBLE:
            left = _Performs_raw_operation(left.AsDouble(), right.AsDouble(), type);
            break;

        default:
            break;
    }
}

void VariantArithmetic::_Performs_int64_any(
    Variant &left, const Variant &right, int type)
{
    switch(right.GetType())
    {
        case VariantRtti::VT_BRIEF_BOOL:
        case VariantRtti::VT_BRIEF_BYTE8:
        case VariantRtti::VT_BRIEF_UINT8:
        case VariantRtti::VT_BRIEF_INT16:
        case VariantRtti::VT_BRIEF_UINT16:
        case VariantRtti::VT_BRIEF_INT32:
        case VariantRtti::VT_BRIEF_UINT32:
        case VariantRtti::VT_BRIEF_LONG:
        case VariantRtti::VT_BRIEF_ULONG:
        case VariantRtti::VT_BRIEF_PTR:
        case VariantRtti::VT_BRIEF_INT64:
            left = _Performs_raw_operation(left.AsInt64(), right.AsInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_UINT64:
            left = _Performs_raw_operation(left.AsUInt64(), right.AsUInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_FLOAT:
        case VariantRtti::VT_BRIEF_DOUBLE:
            left = _Performs_raw_operation(left.AsDouble(), right.AsDouble(), type);
            break;

        default:
            break;
    }
}

void VariantArithmetic::_Performs_uint64_any(
    Variant &left, const Variant &right, int type)
{
    switch(right.GetType())
    {
        case VariantRtti::VT_BRIEF_BOOL:
        case VariantRtti::VT_BRIEF_BYTE8:
        case VariantRtti::VT_BRIEF_UINT8:
        case VariantRtti::VT_BRIEF_INT16:
        case VariantRtti::VT_BRIEF_UINT16:
        case VariantRtti::VT_BRIEF_INT32:
        case VariantRtti::VT_BRIEF_UINT32:
        case VariantRtti::VT_BRIEF_LONG:
        case VariantRtti::VT_BRIEF_ULONG:
        case VariantRtti::VT_BRIEF_PTR:
        case VariantRtti::VT_BRIEF_INT64:
        case VariantRtti::VT_BRIEF_UINT64:
            left = _Performs_raw_operation(left.AsUInt64(), right.AsUInt64(), type);
            break;

        case VariantRtti::VT_BRIEF_FLOAT:
        case VariantRtti::VT_BRIEF_DOUBLE:
            left = _Performs_raw_operation(left.AsDouble(), right.AsDouble(), type);
            break;

        default:
            break;
    }
}

void VariantArithmetic::_Performs_float_any(
    Variant &left, const Variant &right, int type)
{
    left = static_cast<float>(_Performs_raw_operation(left.AsDouble(), right.AsDouble(), type));
}

void VariantArithmetic::_Performs_double_any(
    Variant &left, const Variant &right, int type)
{
    left = _Performs_raw_operation(left.AsDouble(), right.AsDouble(), type);
}

KERNEL_END
