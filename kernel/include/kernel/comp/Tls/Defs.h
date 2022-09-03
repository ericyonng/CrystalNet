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
 * Date: 2021-01-15 22:46:28
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_DEFS_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_DEFS_H__

#pragma once

#include <kernel/kernel_inc.h>

KERNEL_BEGIN

class KERNEL_EXPORT TlsStackSize
{
public:
    enum SizeType : UInt64
    {
        SIZE_MIN_UNIT = 16,         // tls对象最小16字节
        SIZE_1MB = 1048576,         // 1MB
        SIZE_2MB = 2097152,         // 2MB
        SIZE_4MB = 4194304,         // 4MB
        SIZE_8MB = 8388608,         // 8MB
        SIZE_16MB = 16777216,       // 16MB
        SIZE_32MB = 33554432,       // 32MB
        SIZE_64MB = 67108864,       // 64MB
        SIZE_128MB = 134217728,     // 128MB
        SIZE_256MB = 268435456,     // 256MB
        SIZE_512MB = 536870912,     // 512MB
    };
};

class KERNEL_EXPORT TlsDefs
{
public:
    enum ENUMS
    {
        LIB_RTTI_BUF_SIZE = 512,            // 运行时识别缓冲大小
    };
};

KERNEL_END

#endif
