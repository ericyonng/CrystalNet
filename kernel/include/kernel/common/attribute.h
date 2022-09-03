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
 * Date: 2022-02-24 14:12:52
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_ATTRIBUTE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_ATTRIBUTE_H__

#pragma once

#include <kernel/common/compile.h>

#undef CRYSTAL_ATTRIBUTE_PURE
// gcc 或者clang下pure属性可以当作内联
#if CRYSTAL_CUR_COMP == CRYSTAL_COMP_GCC || CRYSTAL_CUR_COMP == CRYSTAL_COMP_CLANG
 #define CRYSTAL_ATTRIBUTE_PURE __attribute__((pure))
#else
 // gcc 或者clang下pure属性可以当作内联
 #define CRYSTAL_ATTRIBUTE_PURE
#endif

#endif
