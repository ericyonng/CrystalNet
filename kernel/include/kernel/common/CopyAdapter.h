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
 * Date: 2022-12-16 22:25:46
 * Author: Eric Yonng
 * Description: 拷贝器
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_COPY_ADAPTER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_COPY_ADAPTER_H__

#pragma once

#include <kernel/common/macro.h>
#include <kernel/common/DataTypeAdapter.h>

KERNEL_BEGIN

class KERNEL_EXPORT CopyAdapter
{
public:
    template<typename T>
    static void Invoke(T &&target, T &&source);
    template<typename T>
    static void Invoke(T target[], T source[]);
};

template<typename T>
ALWAYS_INLINE void CopyAdapter::Invoke(T &&target, T&&source)
{
    target = source;
}

template<typename T>
ALWAYS_INLINE static void CopyAdapter::Invoke(T target[], T source[])
{
    ::memcpy(target, source, sizeof(source));
}

KERNEL_END

#endif
