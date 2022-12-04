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
 * Date: 2022-02-15 13:18:24
 * Author: Eric Yonng
 * Description: from kernel
*/

#include <pch.h>
#include <kernel/kernel.h>

#if CRYSTAL_TARGET_PLATFORM_LINUX

KERNEL_BEGIN

// template<>
// void *KernelAllocMemory<_Build::MT>(UInt64 memSize)
// {
//     auto pool = KernelMemoryPoolAdapter<_Build::MT>();
//     return pool->AllocAdapter<_Build::MT>(memSize);
// }

// template<>
// void *KernelAllocMemory<_Build::TL>(UInt64 memSize)
// {
//     auto pool = KernelMemoryPoolAdapter<_Build::TL>();
//     return pool->AllocAdapter<_Build::TL>(memSize);
// }

// template<>
// void KernelFreeMemory<_Build::MT>(void *ptr)
// {
//     auto pool = KernelMemoryPoolAdapter<_Build::MT>();
//     pool->FreeAdapter<_Build::MT>(ptr);
// }

// template<>
// void KernelFreeMemory<_Build::TL>(void *ptr)
// {
//     auto pool = KernelMemoryPoolAdapter<_Build::TL>();
//     pool->FreeAdapter<_Build::TL>(ptr);
// }

KERNEL_END

#endif
