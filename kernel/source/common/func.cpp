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
#include <kernel/comp/Log/log.h>
#include <unordered_map>

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

void *KernelAllocTL(UInt64 memSize)
{
    return KernelGetTlsMemoryPool()->Alloc(memSize);
}

void *KernelAllocMT(UInt64 memSize)
{
    return  KernelGetDefaultMemoryPool()->Alloc(memSize);
}

void KernelAllocFreeTL(void *ptr)
{
    KernelGetTlsMemoryPool()->Free(ptr);
}

void KernelAllocFreeMT(void *ptr)
{
    KernelGetDefaultMemoryPool()->Free(ptr);
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

std::atomic<UInt64> &GetGlobalIdSrc()
{
    static std::atomic<UInt64> s_id;
    return s_id;
}

UInt64 GetCrystalModuleId()
{
    static const UInt64 id = GetGlobalIdSrc().fetch_add(1, std::memory_order_release) + 1;

// #if _DEBUG
//     if(g_Log)
//     {
//         CLOG_DEBUG_GLOBAL(SystemUtil, "CrystalKernel - GetCrystalModuleId:%llu", id);
//     }
// #endif
    
    return id;
}

std::set<UInt64> &GetCoroutineThreadSet(UInt64 threadId, UInt64 moduleId)
{
    static SpinLock s_Lck;
    static std::unordered_map<UInt64, std::unordered_map<UInt64, std::set<UInt64>>> s_threadIdRefModuleIdRefSet;

    s_Lck.Lock();
    auto iterThread = s_threadIdRefModuleIdRefSet.find(threadId);
    if(iterThread == s_threadIdRefModuleIdRefSet.end())
    {
        iterThread = s_threadIdRefModuleIdRefSet.emplace(std::move(threadId), std::unordered_map<UInt64, std::set<UInt64>>()).first;
    }
    auto &moduleIdRefSet = iterThread->second;
    s_Lck.Unlock();

    auto iterModule = moduleIdRefSet.find(moduleId);
    if(iterModule == moduleIdRefSet.end())
        iterModule = moduleIdRefSet.emplace(std::move(moduleId), std::set<UInt64>()).first;
    return iterModule->second;
}

std::set<UInt64> &GetCoroutineThreadLocalSet(UInt64 moduleId)
{
    static DEF_THREAD_LOCAL_DECLEAR std::set<UInt64> *s_Set = NULL;
    if(UNLIKELY(!s_Set))
        s_Set = &GetCoroutineThreadSet(KERNEL_NS::SystemUtil::GetCurrentThreadId(), moduleId);

    return *s_Set;
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


