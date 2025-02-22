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
 * Date: 2021-01-10 23:02:07
 * Author: Eric Yonng
 * Description: 线程局部存储 线程局部存储，每个线程都有一个线程局部存储的slots,可以利用它实现当线程的全局共享
 * 1.需要在主线程初始化线程局部存储索引位 ，一个进程只需要初始化一次即可 CreateUtilTlsHandle/CreateTlsHandle
 * 2.crystal kernel库实现的是一个线程索引槽位会映射一张table来存储所有要使用的对象
 * 3.使用 TlsUtil 时必须配合库提供的 LibThread 避免 tls 内存泄漏
 * 4.因为与内存管理模块耦合所以不可与其他组件有交集
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_TLS_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_TLS_UTIL_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/comp/Utils/Defs/TlsDefs.h>
#include <kernel/comp/Tls/Defs.h>
#include <kernel/comp/Tls/TlsStack.h>
#include <kernel/comp/Tls/TlsMemoryAlloctor.h>
#include <kernel/comp/memory/MemoryDefs.h>
#include <kernel/comp/Tls/TlsCoDict.h>
#include <kernel/comp/Tls/TlsSmartPtr.h>
#include "kernel/comp/Tls/TlsPtr.h"

KERNEL_BEGIN

class MemoryPool;

class TlsMemoryPool;

template<typename ObjPoolType>
class TlsObjectPool;

class Poller;
class TlsCompsOwner;
class IdGenerator;

class KERNEL_EXPORT TlsUtil
{
public:
    // 公用的util的handle
    static TlsHandle &GetUtileTlsHandle();
    // 销毁线程局部存储
    static void DestroyUtilTlsHandle();

    // 创建tlsstack 默认只使用1MB空间
    static TlsStack<TlsStackSize::SIZE_1MB> *GetTlsStack(bool forceCreate = true); 
    static TlsDefaultObj *GetDefTls();
    static Poller *GetPoller();
    static IdGenerator *GetIdGenerator();
    static TlsCompsOwner *GetTlsCompsOwner();
    static void DestroyTlsStack();
    static void DestroyTlsStack(TlsStack<TlsStackSize::SIZE_1MB> *tlsTask);
    static void SetTlsValueNull();

    // 获取thread local MemoryAlloc
    template<typename MemAlloctorType, typename MemAloctorCfgType>
    static MemAlloctorType *GetMemoryAlloctor(UInt64 allocUnitBytes);
    template<typename MemAlloctorType, typename MemAloctorCfgType>
    static MemAlloctorType *CreateMemoryAlloctor(UInt64 allocUnitBytes, const std::string &source, UInt64 initBlockNumPerBuffer = MEMORY_BUFFER_BLOCK_INIT);

    // 获取thread local MemoryPoll
    static MemoryPool *GetMemoryPool();
    static MemoryPool *CreateMemoryPool(const std::string &reason);

    static void ClearTlsResource();

    template<typename ObjType>
    static TlsObjectPool<ObjType> *GetObjPool();

    // 当前线程所有协程管理
    static TlsCoDict *GetTlsCoDict();

    template<typename ObjType, AutoDelMethods::Way delMethod = AutoDelMethods::Delete>
    static TlsSmartPtr<ObjType, delMethod> *GetOrCreateSmartPtr();

    template<typename T>
    static TlsTargetPtr<T> *GetOrCreateTargetPtr();

private:
    static TlsMemoryAlloctor *GetTlsMemoryAlloctorHost();
    static TlsMemoryPool *GetTlsMemoryPoolHost();
    static TlsMemoryPool **GetTlsMemoryPoolHostThreadLocalAddr();
    // 指定tlshandle
    static TlsHandle CreateTlsHandle();
    static void DestroyTlsHandle(TlsHandle &tlsHandle);
    static IdGenerator *_GetIdGenerator();

};

ALWAYS_INLINE void TlsUtil::DestroyUtilTlsHandle()
{
    DestroyTlsHandle(GetUtileTlsHandle());
}

ALWAYS_INLINE TlsDefaultObj *TlsUtil::GetDefTls()
{
    auto tlsStack = GetTlsStack();
    return tlsStack->GetDef();
}

ALWAYS_INLINE TlsCompsOwner *TlsUtil::GetTlsCompsOwner()
{
    return GetDefTls()->_tlsComps;
}

ALWAYS_INLINE void TlsUtil::DestroyTlsStack()
{
    auto tlsStack = GetTlsStack(false);
    if(UNLIKELY(!tlsStack))
    {
        CRYSTAL_TRACE("repeat destroy tls stack thread id = [%llu]", KernelGetCurrentThreadId());
        return;
    }

    DestroyTlsStack(tlsStack);
}

template<typename MemAlloctorType, typename MemAloctorCfgType>
ALWAYS_INLINE MemAlloctorType *TlsUtil::GetMemoryAlloctor(UInt64 allocUnitBytes)
{
    // static_assert(false, "forbid use CreateMemoryAlloctor");
    TlsMemoryAlloctor *alloctorHost = GetTlsMemoryAlloctorHost();
    return alloctorHost->GetMemoryAlloctor<MemAlloctorType, MemAloctorCfgType>(allocUnitBytes);
}

template<typename MemAlloctorType, typename MemAloctorCfgType>
ALWAYS_INLINE MemAlloctorType *TlsUtil::CreateMemoryAlloctor(UInt64 allocUnitBytes, const std::string &source, UInt64 initBlockNumPerBuffer)
{
    // static_assert(false, "forbid use CreateMemoryAlloctor");
    TlsMemoryAlloctor *alloctorHost = GetTlsMemoryAlloctorHost();
    return alloctorHost->CreateMemoryAlloctor<MemAlloctorType, MemAloctorCfgType>(allocUnitBytes, initBlockNumPerBuffer, source);
}

ALWAYS_INLINE void TlsUtil::ClearTlsResource()
{
    // TODO:获取tls相关地址并重置成NULL

    // 1.清理设置tls pool NULL
    *GetTlsMemoryPoolHostThreadLocalAddr() = NULL;
}

template<typename ObjType>
ALWAYS_INLINE TlsObjectPool<ObjType> *TlsUtil::GetObjPool()
{
    DEF_STATIC_THREAD_LOCAL_DECLEAR TlsObjectPool<ObjType> * pool = NULL;
    if(UNLIKELY(pool == NULL))
    {
        pool = TlsUtil::GetTlsStack()->New<TlsObjectPool<ObjType>>();
    }

    return pool;
}

ALWAYS_INLINE TlsCoDict *TlsUtil::GetTlsCoDict()
{
    DEF_STATIC_THREAD_LOCAL_DECLEAR TlsCoDict *tlsCoDict = NULL;

    if(UNLIKELY(!tlsCoDict))
        tlsCoDict = TlsUtil::GetTlsStack()->New<TlsCoDict>();

    return tlsCoDict;
}

template<typename ObjType, AutoDelMethods::Way delMethod>
ALWAYS_INLINE TlsSmartPtr<ObjType, delMethod> *TlsUtil::GetOrCreateSmartPtr()
{
    DEF_STATIC_THREAD_LOCAL_DECLEAR TlsSmartPtr<ObjType, delMethod> *s_ptr = NULL;

    if(UNLIKELY(!s_ptr))
        s_ptr = TlsUtil::GetTlsStack()->New<TlsSmartPtr<ObjType, delMethod>>();

    return s_ptr;
}

template<typename T>
ALWAYS_INLINE TlsTargetPtr<T> *TlsUtil::GetOrCreateTargetPtr()
{
    DEF_STATIC_THREAD_LOCAL_DECLEAR TlsTargetPtr<T> *s_ptr = NULL;
    if(UNLIKELY(!s_ptr))
    {
        s_ptr = TlsUtil::GetTlsStack()->New<TlsTargetPtr<T>>();
    }

    return s_ptr;
}

ALWAYS_INLINE TlsMemoryAlloctor *TlsUtil::GetTlsMemoryAlloctorHost()
{
    DEF_STATIC_THREAD_LOCAL_DECLEAR TlsMemoryAlloctor *tlsAlloctor = NULL;
    if(UNLIKELY(!tlsAlloctor))
    {
        tlsAlloctor = TlsUtil::GetTlsStack()->New<TlsMemoryAlloctor>();
    }

    return tlsAlloctor;
}

ALWAYS_INLINE TlsMemoryPool *TlsUtil::GetTlsMemoryPoolHost()
{
    return *GetTlsMemoryPoolHostThreadLocalAddr();
}

ALWAYS_INLINE IdGenerator *TlsUtil::GetIdGenerator()
{
    DEF_STATIC_THREAD_LOCAL_DECLEAR IdGenerator *s_IdGen = NULL;
    if(LIKELY(s_IdGen))
        return s_IdGen;

    s_IdGen = _GetIdGenerator();

    return s_IdGen;
}

KERNEL_END

#endif

