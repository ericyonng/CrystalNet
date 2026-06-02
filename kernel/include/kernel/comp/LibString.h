/*!
 * MIT License
 *  
 * Copyright (c) 2020 Eric Yonng<120453674@qq.com>
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *  
 * 
 * Date: 2020-10-18 14:26:22
 * Author: Eric Yonng
 * Description: 默认使用Ascii码,打印日志时候应使用Utf8编码,提供ascii码转utf8接口
 *              有别于标准库,内存分配由用户指定分配器,带码点，数据包括一颗树和一个数组，支持utf8，支持append_format接口等，编码不同直接报错处理
 *              默认使用tls memmory pool
 *              ::std::basic_string<Byte8>::data只有在::std::basic_string<Byte8>内容大小使得capacity发生变化的时候才会失效
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LIB_STRING_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LIB_STRING_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/comp/LibBasicString.h>

KERNEL_BEGIN

using LibString = LibBasicString<Byte8> ;

KERNEL_END


namespace std
{

    /**
     * \brief The explicit specialization of std::hash<KERNEL_NS::LibBasicString> impl.
     * 
     */
    template <>
    struct KERNEL_EXPORT hash<KERNEL_NS::LibString>
    {
        // gcc 或者clang下pure属性可以当作内联
#if CRYSTAL_CUR_COMP == CRYSTAL_COMP_GCC || CRYSTAL_CUR_COMP == CRYSTAL_COMP_CLANG
        CRYSTAL_ATTRIBUTE_PURE
        #endif // Comp == Gcc or Comp == Clang
        size_t operator()(const KERNEL_NS::LibString &str) const noexcept
        {
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
            if (!str.empty())
                return ::std::_Hash_array_representation(str.data(), str.size());
            else
                return ::std::_Hash_representation(nullptr);
#else
            if (!str.empty())
                return ::std::_Hash_impl::hash(str.data(), str.size());
            else
                return ::std::_Hash_impl::hash(nullptr, 0);
#endif
        }
    };

}
#endif