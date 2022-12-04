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
 * Date: 2021-09-11 17:11:29
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/common/func.h>
#include <kernel/common/macro.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Utils/TlsUtil.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/memory/MemoryPool.h>
#include <kernel/comp/Lock/Lock.h>
#include <kernel/comp/Utils/BackTraceUtil.h>

KERNEL_BEGIN

MemoryPool *KernelGetTlsMemoryPool()
{
    DEF_STATIC_THREAD_LOCAL_DECLEAR MemoryPool *memPool = NULL;
    if(UNLIKELY(!memPool))
    {
        memPool = TlsUtil::GetMemoryPool() ? TlsUtil::GetMemoryPool() : TlsUtil::CreateMemoryPool("KernelGetTlsMemoryPool tls memory pool");
    }

    return memPool;
}

MemoryPool *KernelGetDefaultMemoryPool()
{
    return MemoryPool::GetDefaultInstance();
}

UInt64 KernelGetCurrentThreadId()
{
    return SystemUtil::GetCurrentThreadId();
}

template<>
void *KernelAllocMemory<_Build::MT>(UInt64 memSize)
{
    auto pool = KernelMemoryPoolAdapter<_Build::MT>();
    return pool->AllocAdapter<_Build::MT>(memSize);
}

template<>
void *KernelAllocMemory<_Build::TL>(UInt64 memSize)
{
    auto pool = KernelMemoryPoolAdapter<_Build::TL>();
    return pool->AllocAdapter<_Build::TL>(memSize);
}

template<>
void KernelFreeMemory<_Build::MT>(void *ptr)
{
    auto pool = KernelMemoryPoolAdapter<_Build::MT>();
    pool->FreeAdapter<_Build::MT>(ptr);
}

template<>
void KernelFreeMemory<_Build::TL>(void *ptr)
{
    auto pool = KernelMemoryPoolAdapter<_Build::TL>();
    pool->FreeAdapter<_Build::TL>(ptr);
}

SpinLock &GetConsoleLocker()
{
	static SpinLock s_consoleLocker;
	return s_consoleLocker;
}

void LockConsole()
{
    GetConsoleLocker().Lock();
}

void UnlockConsole()
{
    GetConsoleLocker().Unlock();
}

// SpinLock& GetBackTraceLock()
// {
//     static SpinLock g_lck;
//     return g_lck;
// }

// void LockBackTrace()
// {
//     GetBackTraceLock().Lock();
// }

// void UnlockBackTrace()
// {
//     GetBackTraceLock().Unlock();
// }

LibString &KernelAppendFormat(LibString &o, const Byte8 *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto fmtSize = o.CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    o.AppendFormatWithVaList(fmtSize, fmt, va);
    va_end(va);
    
    return o;
}

KERNEL_END

// std::atomic<Int64> g_TotalBytes = {0};
// std::string g_MemleakBackTrace;

// void *operator new(size_t bytes)
// {
//     g_TotalBytes.fetch_add(bytes);

//     // auto str = KERNEL_NS::BackTraceUtil::CrystalCaptureStackBackTrace();
//     printf("\noperator new bytes:%llu, \n", bytes);

//     return ::malloc(bytes);
// }

// void *operator new[](size_t bytes)
// {
//     g_TotalBytes.fetch_add(bytes);

//     // auto str = KERNEL_NS::BackTraceUtil::CrystalCaptureStackBackTrace();
//     printf("\noperator new[] bytes:%llu, \n", bytes);

//     return ::malloc(bytes);
// }

