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
 * Date: 2021-03-16 15:37:41
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_VARIANT_VARIANT_TRAITS_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_VARIANT_VARIANT_TRAITS_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/Utils/StringUtil.h>
#include <kernel/comp/Variant/Variant.h>

KERNEL_BEGIN

class KERNEL_EXPORT VariantTraits
{
public:
    /**
     * Assign operator.
     */
    static void assign(Variant &left, const Variant &right);

    /**
     * Relational operators.
     * ==, !=, <, >, <=, >=
     */
    static bool eq(const Variant &left, const Variant &right);
    static bool ne(const Variant &left, const Variant &right);
    static bool lt(const Variant &left, const Variant &right);
    static bool gt(const Variant &left, const Variant &right);
    static bool le(const Variant &left, const Variant &right);
    static bool ge(const Variant &left, const Variant &right);

    /**
     * Arithmetic operators.
     * +, -, *, /
     */
    static Variant add(const Variant &left, const Variant &right);
    static Variant sub(const Variant &left, const Variant &right);
    static Variant mul(const Variant &left, const Variant &right);
    static Variant div(const Variant &left, const Variant &right);

    /**
     * Arithmetic operators.
     * +=, -=, *=, /=
     */
    static void add_equal(Variant &left, const Variant &right);
    static void sub_equal(Variant &left, const Variant &right);
    static void mul_equal(Variant &left, const Variant &right);
    static void div_equal(Variant &left, const Variant &right);
};

KERNEL_END

#include <kernel/comp/Variant/VariantTraitsImpl.h>

#endif
