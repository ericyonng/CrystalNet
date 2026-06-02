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
 * Date: 2026-06-02 12:48:22
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_BASIC_STRING_HELPER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_BASIC_STRING_HELPER_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>

KERNEL_BEGIN

class KERNEL_EXPORT BasicStringHelper
{
public:
  static ALWAYS_INLINE UInt64 CalcUtf8CharBytes(U8 ctrlChar)
  {
    if ((ctrlChar & (U8)0x80) == 0x00)
    {
     return 1;
    }
    // 110x xxxx
    // Encoding len: 2 bytes.
    else if ((ctrlChar & (U8)0xe0) == 0xc0)
    {
     return 2;
    }
    // 1110 xxxx
    // Encoding len: 3 bytes.
    else if ((ctrlChar & (U8)0xf0) == 0xe0)
    {
     return 3;
    }
    // 1111 0xxx
    // Encoding len: 4 bytes.
    else if ((ctrlChar & (U8)0xf8) == 0xf0)
    {
     return 4;
    }
    // 1111 10xx
    // Encoding len: 5 bytes.
    else if ((ctrlChar & (U8)0xfc) == 0xf8)
    {
     return 5;
    }
    // 1111 110x
    // Encoding len: 6 bytes.
    else if ((ctrlChar & (U8)0xfe) == 0xfc)
    {
     return 6;
    }

   return 0;
  }
};

KERNEL_END

#endif