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
 * Date: 2021-01-10 23:23:58
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Utils/TlsUtil.h>

KERNEL_BEGIN

// 定义全局的tlshandle

TlsHandle &TlsUtil::GetUtileTlsHandle()
{
    static TlsHandle g_UtilTlsHandle = CreateTlsHandle();
    return g_UtilTlsHandle;
}

// 创建tlsstack
TlsStack<TlsStackSize::SIZE_1MB> *TlsUtil::GetTlsStack(bool forceCreate)
{
    auto &handle = GetUtileTlsHandle();
    // if(UNLIKELY(handle == INVALID_TLS_HANDLE))
    // {
    //     CreateUtilTlsHandle();
    // }

    TlsStack<TlsStackSize::SIZE_1MB> *tlsStack = NULL;
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    tlsStack = reinterpret_cast<TlsStack<TlsStackSize::SIZE_1MB> *>(pthread_getspecific(handle));
#else
    tlsStack = reinterpret_cast<TlsStack<TlsStackSize::SIZE_1MB> *>(::TlsGetValue(handle));
#endif

    if(LIKELY(tlsStack || !forceCreate))
        return tlsStack;

    {
        // auto memPool = MemoryPool::GetDefaultInstance();
        // void *ptr = memPool->Alloc(__MEMORY_ALIGN__(sizeof(TlsStack<TlsStackSize::SIZE_1MB>)));
        // tlsStack = AllocUtil::NewByPtrNoConstructorParams<TlsStack<TlsStackSize::SIZE_1MB>>(ptr) ;

        Byte8 *ptr = new Byte8[__MEMORY_ALIGN__(sizeof(TlsStack<TlsStackSize::SIZE_1MB>))];
        tlsStack = AllocUtil::NewByPtrNoConstructorParams<TlsStack<TlsStackSize::SIZE_1MB>>(ptr) ;

        tlsStack->SetThreadId(SystemUtil::GetCurrentThreadId());

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        (void)pthread_setspecific(handle, tlsStack);
#else
        (void)::TlsSetValue(handle, tlsStack);
#endif
    }

    return tlsStack;
}

KERNEL_END
