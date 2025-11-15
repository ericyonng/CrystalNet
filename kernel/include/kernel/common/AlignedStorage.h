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
 * Date: 2025-11-15 11:32:13
 * Author: Eric Yonng
 * Description: 
*/
#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_ALIGNED_STORAGE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_ALIGNED_STORAGE_H__

#pragma once

#include <kernel/common/BaseType.h>
#include <kernel/common/BaseMacro.h>

KERNEL_BEGIN

// 内存对齐的内存块
template<size_t Len, size_t AlignBytes>
struct AlignedStorage
{
  struct alignas(AlignBytes) Type
  {
    U8 Data[Len];
  };
};

KERNEL_END

#endif
