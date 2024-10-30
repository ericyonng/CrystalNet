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

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_REMOVE_CVREF_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_REMOVE_CVREF_H__

#pragma once

#include <kernel/common/RemoveCV.h>
#include <kernel/common/RemoveReference.h>

KERNEL_BEGIN

template <class _Ty>
using _RemoveCvRefType = RemoveCVType<RemoveReferenceType<_Ty>>;

template <class _Ty>
using RemoveCvRefType = _RemoveCvRefType<_Ty>;

template <class _Ty>
struct RemoveCvRef {
    using type = RemoveCvRefType<_Ty>;
};

KERNEL_END

#endif
