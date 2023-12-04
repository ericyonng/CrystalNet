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
 * Date: 2022-02-22 03:50:52
 * Author: Eric Yonng
 * Description: 字符串流适配
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LIBSTRING_OUT_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LIBSTRING_OUT_H__

#pragma once

#include <kernel/common/DataTypeAdapter.h>

KERNEL_BEGIN
// 
// ALWAYS_INLINE KERNEL_EXPORT void AppendFormatString(std::string &o, const Byte8 *fmt, ...)
// {
//     // if fmt args is null, return.
//     if (UNLIKELY(!fmt))
//         return;
// 
//     va_list va;
//     va_start(va, fmt);
//     do
//     {
//         // try detach detach format require buffers and resize it.
//         const UInt64 oldSize = o.size();
//         Int32 len =::vsnprintf(nullptr, 0, fmt, va);
//         if (len <= 0)
//             break;
// 
//         // exec format.
//         o.resize(oldSize + len);
//         len = ::vsnprintf(const_cast<Byte8 *>(o.data() + oldSize),
//                             len + 1,
//                             fmt,
//                             va);
// 
//         // len < 0 then o.size() - len > oldSize back to old string
//         if (UNLIKELY(oldSize != (o.size() - len)))
//         {
//             CRYSTAL_TRACE("wrong apend format and back to old string, oldSize:%llu, len:%d, new size:%llu", oldSize, len, o.size());
//             o.resize(oldSize);
//         }
//     } while (0);
//     
//     va_end(va);
// }
// 

// 流输出必须区分,指针,pod类型
template<typename _Ty, LibDataType::ENUMS _DataType>
struct KernelStringOutAdapter;

template<typename _Ty>
struct KernelStringOutAdapter<_Ty, LibDataType::BRIEF_DATA_TYPE>
{
public:
//     template<typename Rtn>
//     static Rtn &output(Rtn &o, _Ty &&input)
//     {
//         return o << input;
//     }
//     template<typename Rtn>
//     static Rtn &output(Rtn &o, _Ty &input)
//     {
//         return o << input;
//     }
    template<typename Rtn>
    static Rtn &output(Rtn &o, const _Ty &input)
    {
        return o += input;
    }
};
// Array 输出Address
template<typename _Ty>
struct KernelStringOutAdapter<_Ty, LibDataType::ARRAY_TYPE>
{
public:
//     template<typename Rtn>
//     static Rtn &output(Rtn &o, _Ty &&input)
//     {
//         return o.AppendFormat("%p", input);
//     }
//     template<typename Rtn>
//     static Rtn &output(Rtn &o, _Ty &input)
//     {
//         return o.AppendFormat("%p", input);
//     }
    template<typename Rtn>
    static Rtn &output(Rtn &o, const _Ty &input)
    {
        return o.AppendFormat("%p", input);
    }
};
// class 需要支持ToString() const;
template<typename _Ty>
struct KernelStringOutAdapter<_Ty, LibDataType::CLASS_TYPE>
{
public:
//     template<typename Rtn>
//     static Rtn &output(Rtn &o, _Ty &&input)
//     {
//         o += input.ToString();
//         return o;
//     }
//     template<typename Rtn>
//     static Rtn &output(Rtn &o, _Ty &input)
//     {
//         o += input.ToString();
//         return o;
//     }
    template<typename Rtn>
    static Rtn &output(Rtn &o, const _Ty &input)
    {
        o += input.ToString();
        return o;
    }
};

// Function 输出地址
template<typename _Ty>
struct KernelStringOutAdapter<_Ty, LibDataType::FUNCTION_TYPE>
{
public:
//     template<typename Rtn>
//     static Rtn &output(Rtn &o, _Ty &&input)
//     {
//         return o.AppendFormat("%p", input);
//     }
//     template<typename Rtn>
//     static Rtn &output(Rtn &o, _Ty &input)
//     {
//         return o.AppendFormat("%p", input);
//     }
    template<typename Rtn>
    static Rtn &output(Rtn &o, const _Ty &input)
    {
        return o.AppendFormat("%p", input);
    }
};


// Pointer 输出地址
template<typename _Ty>
struct KernelStringOutAdapter<_Ty, LibDataType::POINTER_TYPE>
{
public:
//     template<typename Rtn>
//     static Rtn &output(Rtn &o, _Ty &&input)
//     {
//         return o.AppendFormat("%p", input);
//     }
//     template<typename Rtn>
//     static Rtn &output(Rtn &o, _Ty &input)
//     {
//         return o.AppendFormat("%p", input);
//     }
    template<typename Rtn>
    static Rtn &output(Rtn &o, const _Ty &input)
    {
        return o.AppendFormat("%p", input);
    }
};

template<typename _Ty>
struct StringOutAdapter : public KernelStringOutAdapter<_Ty, LibTraitsDataType<_Ty>::value>
{};

KERNEL_END

#endif
