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
 * Date: 2024-10-26 16:46:08
 * Author: Eric Yonng
 * Description: 移除类型中的const, volatile
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_REMOVE_CV_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_REMOVE_CV_H__

#pragma once

#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>

KERNEL_BEGIN

// 单独移除const
template <class _Ty>
struct RemoveConst 
{ // remove top-level const qualifier
    using type = _Ty;
};

template <class _Ty>
struct RemoveConst<const _Ty> 
{
    using type = _Ty;
};

template <class _Ty>
using RemoveConstType = typename RemoveConst<_Ty>::type;

template <class _Ty>
struct RemoveVolatile
{ // remove top-level volatile qualifier
    using type = _Ty;
};

template <class _Ty>
struct RemoveVolatile<volatile _Ty> 
{
    using type = _Ty;
};

template <class _Ty>
using RemoveVolatileType = typename RemoveVolatile<_Ty>::type;

template <class _Ty>
struct RemoveCV 
{ // remove top-level const and volatile qualifiers
    using type = _Ty;

    template <template <class> class _Fn>
    using _Apply = _Fn<_Ty>; // apply cv-qualifiers from the class template argument to _Fn<_Ty>
};

// 移除const
template <class _Ty>
struct RemoveCV<const _Ty> {
    using type = _Ty;

    template <template <class> class _Fn>
    using _Apply = const _Fn<_Ty>;
};

// 移除volatile
template <class _Ty>
struct RemoveCV<volatile _Ty> {
    using type = _Ty;

    template <template <class> class _Fn>
    using _Apply = volatile _Fn<_Ty>;
};

// 移除 const volatile
template <class _Ty>
struct RemoveCV<const volatile _Ty> {
    using type = _Ty;

    template <template <class> class _Fn>
    using _Apply = const volatile _Fn<_Ty>;
};

template <class _Ty>
using RemoveCVType = typename RemoveCV<_Ty>::type;

KERNEL_END

#endif
