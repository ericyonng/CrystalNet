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
 * Date: 2021-09-25 21:55:12
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_MEM_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_MEM_UTIL_H__

#pragma once

#include <kernel/kernel_inc.h>

KERNEL_BEGIN

class KERNEL_EXPORT MemUtil
{
public:
    template<typename T>
    static void Memset(T *ptr, Int32 value, UInt64 bytes);

    template<typename T>
    static void Zeroset(T &podData);

    template<typename T>
    static void Memcpy(T *dest, void *src, UInt64 bytes);
};

template<typename T>
inline void MemUtil::Memset(T *ptr, Int32 value, UInt64 bytes)
{
    static_assert(std::is_pod<T>::value, "not pod type:");
    ::memset(ptr, value, bytes);
}

template<typename T>
inline void MemUtil::Zeroset(T &podData)
{
    static_assert(std::is_pod<T>::value, "not pod type");
    ::memset(&podData, 0, sizeof(podData));
}

template<typename T>
inline void MemUtil::Memcpy(T *dest, void *src, UInt64 bytes)
{
    static_assert(std::is_pod<T>::value, "not pod type");
    ::memcpy(dest, src, bytes);
}

KERNEL_END

#endif
