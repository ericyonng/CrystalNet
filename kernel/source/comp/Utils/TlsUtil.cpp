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
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/memory/MemoryPool.h>
#include <kernel/comp/Tls/TlsMemoryPool.h>
#include <kernel/comp/Utils/AllocUtil.h>
#include <kernel/comp/Tls/TlsCompsOwner.h>
#include <kernel/comp/Poller/Poller.h>
#include <kernel/comp/IdGenerator/IdGenerator.h>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
 #include <processthreadsapi.h>
#endif

#if CRYSTAL_TARGET_PLATFORM_LINUX
 #include <pthread.h>
#endif

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

void TlsUtil::DestroyTlsStack(TlsStack<TlsStackSize::SIZE_1MB> *tlsStack)
{
    tlsStack->FreeAll();
    // memPool->Free(tlsStack);

    // 设置null
    if(tlsStack->GetThreadId() == SystemUtil::GetCurrentThreadId())
        SetTlsValueNull();

    Byte8 *ptr = reinterpret_cast<Byte8 *>(tlsStack);
    CRYSTAL_DELETE_SAFE(ptr);
}

MemoryPool *TlsUtil::GetMemoryPool()
{
    return GetTlsMemoryPoolHost()->GetPool<MemoryPool>();
}

MemoryPool *TlsUtil::CreateMemoryPool(const std::string &reason)
{
    return GetTlsMemoryPoolHost()->CreatePool<MemoryPool, InitMemoryPoolInfo>(reason);
}


TlsHandle TlsUtil::CreateTlsHandle()
{
    bool tlsCreated = false;
    TlsHandle tlsHandle = INVALID_TLS_HANDLE;
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    tlsCreated = (pthread_key_create(&tlsHandle, NULL) == 0) ? true : false;
#else
    tlsCreated = ((tlsHandle = ::TlsAlloc()) != TLS_OUT_OF_INDEXES) ? true : false;
#endif

    if(!tlsCreated)
        return INVALID_TLS_HANDLE;

    return tlsHandle;
}

void TlsUtil::DestroyTlsHandle(TlsHandle &tlsHandle)
{
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    (void)pthread_key_delete(tlsHandle);
#else
    (void)::TlsFree(tlsHandle);
#endif

    tlsHandle = INVALID_TLS_HANDLE;
}

void TlsUtil::SetTlsValueNull()
{
    // 设置null
    auto &handle = GetUtileTlsHandle();
    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
            (void)pthread_setspecific(handle, NULL);
    #else
            (void)::TlsSetValue(handle, NULL);
    #endif
}

TlsMemoryPool **TlsUtil::GetTlsMemoryPoolHostThreadLocalAddr()
{
    DEF_STATIC_THREAD_LOCAL_DECLEAR TlsMemoryPool *tlsPool = NULL;
    if(UNLIKELY(!tlsPool))
    {
        tlsPool = TlsUtil::GetTlsStack()->New<TlsMemoryPool>();
    }

    return &tlsPool;
}

Poller *TlsUtil::GetPoller()
{
    DEF_STATIC_THREAD_LOCAL_DECLEAR Poller *s_Poller = GetDefTls()->_tlsComps->GetComp<Poller>();
    return s_Poller;
}

IdGenerator *TlsUtil::_GetIdGenerator()
{
    auto defTls = GetDefTls();
    return defTls->_tlsComps->GetComp<IdGenerator>();
}


KERNEL_END
