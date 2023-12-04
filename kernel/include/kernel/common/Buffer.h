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
 * Date: 2020-10-06 18:57:50
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_BUFFER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_BUFFER_H__

#pragma once

#include <kernel/common/BaseType.h>

// cache
#define BUFFER_LEN2         2
#define BUFFER_LEN4         4
#define BUFFER_LEN8         8
#define BUFFER_LEN16        16
#define BUFFER_LEN32        32
#define BUFFER_LEN64        64
#define BUFFER_LEN128       128
#define BUFFER_LEN256       256
#define BUFFER_LEN512       512
#define BUFFER_LEN1024      1024
typedef Byte8 BUFFER2[BUFFER_LEN2];
typedef Byte8 BUFFER4[BUFFER_LEN4];
typedef Byte8 BUFFER8[BUFFER_LEN8];
typedef Byte8 BUFFER16[BUFFER_LEN16];
typedef Byte8 BUFFER32[BUFFER_LEN32];
typedef Byte8 BUFFER64[BUFFER_LEN64];
typedef Byte8 BUFFER128[BUFFER_LEN128];
typedef Byte8 BUFFER256[BUFFER_LEN256];
typedef Byte8 BUFFER512[BUFFER_LEN512];
typedef Byte8 BUFFER1024[BUFFER_LEN1024];

#endif
