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
 * Date: 2020-11-21 21:12:09
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_MEMORY_POOL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_MEMORY_POOL_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/MemoryDefs.h>
#include <kernel/comp/Lock/Lock.h>
#include <kernel/comp/Utils/MathUtil.h>
#include <kernel/comp/memory/MemoryBlock.h>
#include <kernel/comp/memory/Defs/MemoryAlloctorConfig.h>
#include <kernel/comp/Delegate/Delegate.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class MemoryAlloctor;
class LibString;

struct KERNEL_EXPORT InitMemoryPoolInfo
{
    InitMemoryPoolInfo(UInt64 createMemoryBufferNumWhenInit = 0, UInt64 miniAllockBlockBytes = MEMORY_POOL_MINI_BLOCK_BYTES, UInt64 maxAllockBlockBytes = MEMORY_POOL_MAX_BLOCK_BYTES);

    // MEMORY_POOL_MINI_BLOCK_BYTES ??????????????????
    UInt64 GetMin() const
    {
        return _miniAllockBlockBytes;
    }

    // ????????????????????????????????????????????? MEMORY_POOL_MAX_BLOCK_BYTES ??????????????????
    UInt64 GetMax() const
    {
        return _maxAllockBlockBytes;
    }

    // ???????????????????????????
    MemoryAlloctorConfigs &GetAlloctorCfgsToModify()
    {
        return _alloctorCfgs;
    }

    const MemoryAlloctorConfigs &GetAlloctorCfgs() const
    {
        return _alloctorCfgs;
    }

private:
    UInt64 _miniAllockBlockBytes;           // ?????????????????????
    UInt64 _maxAllockBlockBytes;            // ?????????????????????
    MemoryAlloctorConfigs _alloctorCfgs;
};

class KERNEL_EXPORT MemoryPool
{
public:
    MemoryPool(const InitMemoryPoolInfo &initInfo = InitMemoryPoolInfo(), const std::string &reason = "");
    virtual ~MemoryPool();

public:
    /* ?????????????????????????????????????????? */
    // ??????????????????????????????????????????????????????????????????  ?????????????????????????????????
    static MemoryPool *GetInstance(const InitMemoryPoolInfo &initInfo = InitMemoryPoolInfo());
    // ???????????????????????? ????????????????????????????????????????????????????????????????????????????????????
    static MemoryPool *GetDefaultInstance();
    Int32 Init();
    void Destroy();

    /* ???????????? */
    template<typename PtrType>
    PtrType *Alloc(UInt64 bytes);
    void *Alloc(UInt64 bytes);
    void *Realloc(void *ptr, UInt64 bytes);
    template<typename PtrType>
    PtrType *Realloc(void *ptr, UInt64 bytes);
    void AddRef(void *ptr);
    void Free(void *ptr);

    // thread_local????????????
    template<typename PtrType>
    PtrType *AllocThreadLocal(UInt64 bytes);
    void *AllocThreadLocal(UInt64 bytes);
    void *ReallocThreadLocal(void *ptr, UInt64 bytes);
    template<typename PtrType>
    PtrType *ReallocThreadLocal(void *ptr, UInt64 bytes);
    void AddRefThreadLocal(void *ptr);
    void FreeThreadLocal(void *ptr);

    // ??????tls????????????
    template<typename PtrType, typename BuildType>
    PtrType *AllocAdapter(UInt64 bytes);
    template<typename BuildType>
    void *AllocAdapter(UInt64 bytes);
    template<typename BuildType>
    void *ReallocAdapter(void *ptr, UInt64 bytes);
    template<typename PtrType, typename BuildType>
    PtrType *ReallocAdapter(void *ptr, UInt64 bytes);
    template<typename BuildType>
    void AddRefAdapter(void *ptr);
    template<typename BuildType>
    void FreeAdapter(void *ptr);

    // void Lock();
    // void Unlock();
    LibString ToString() const;
    LibString UsingInfo() const;
    UInt64 ToMonitor(LibString &str);
    UInt64 GetPoolThreadId() const;
    // ???????????????????????????????????????????????????????????????
    UInt64 GetMaxPoolAllocable() const;

private:
    void _InitMemRange(UInt64 begin, UInt64 end, MemoryAlloctor *alloctor);

private:
    std::vector<MemoryAlloctor *> _bytesPosRefAlloctors;            // ????????????????????????????????????
    std::vector<MemoryAlloctor *> _alloctorArray;                   // ????????????alloctor????????????????????????????????????????????????

    std::atomic_bool _isInit;                                       // ??????????????????
    Locker _lck;
    const UInt64 _miniAllockBlockBytes;                             // ?????????????????????
    const UInt64 _maxAllockBlockBytes;                              // ?????????????????????
    const UInt64 _memoryBlockHeadSizeAfterAlign;                        // ?????????????????????????????????
    const MemoryAlloctorConfigs _alloctorCfgs;                      // ???????????????
    IDelegate<UInt64, LibString &> *_monitorPrintDelg;                // ?????????????????????
    const std::string _reason;
    UInt64 _threadId;
};

ALWAYS_INLINE MemoryPool::~MemoryPool()
{
    Destroy();
}

// inline void MemoryPool::Lock()
// {
//     _lck.Lock();
// }

// inline void MemoryPool::Unlock()
// {
//     _lck.Unlock();
// }

ALWAYS_INLINE MemoryPool *MemoryPool::GetDefaultInstance()
{
    return GetInstance();
}


template<typename PtrType>
ALWAYS_INLINE PtrType *MemoryPool::Alloc(UInt64 bytes)
{
    return reinterpret_cast<PtrType *>(Alloc(bytes));
}

template<typename PtrType>
ALWAYS_INLINE PtrType *MemoryPool::Realloc(void *ptr, UInt64 bytes)
{
    return reinterpret_cast<PtrType *>(Realloc(ptr, bytes));
}

template<typename PtrType>
ALWAYS_INLINE PtrType *MemoryPool::AllocThreadLocal(UInt64 bytes)
{
    return reinterpret_cast<PtrType *>(AllocThreadLocal(bytes));
}

template<typename PtrType>
ALWAYS_INLINE PtrType *MemoryPool::ReallocThreadLocal(void *ptr, UInt64 bytes)
{
    return reinterpret_cast<PtrType *>(ReallocThreadLocal(ptr, bytes));
}

template<typename PtrType, typename BuildType>
ALWAYS_INLINE PtrType *MemoryPool::AllocAdapter(UInt64 bytes)
{
    return reinterpret_cast<PtrType *>(AllocAdapter<BuildType>(bytes));
}

// tls??????
template<>
ALWAYS_INLINE void *MemoryPool::AllocAdapter<_Build::TL>(UInt64 bytes)
{
    return AllocThreadLocal(bytes);
}

// MT??????
template<>
ALWAYS_INLINE void *MemoryPool::AllocAdapter<_Build::MT>(UInt64 bytes)
{
    return Alloc(bytes);
}

template<>
ALWAYS_INLINE void *MemoryPool::ReallocAdapter<_Build::TL>(void *ptr, UInt64 bytes)
{
    return ReallocThreadLocal(ptr, bytes);
}

template<>
ALWAYS_INLINE void *MemoryPool::ReallocAdapter<_Build::MT>(void *ptr, UInt64 bytes)
{
    return Realloc(ptr, bytes);
}

template<typename PtrType, typename BuildType>
ALWAYS_INLINE PtrType *MemoryPool::ReallocAdapter(void *ptr, UInt64 bytes)
{
    return reinterpret_cast<PtrType *>(ReallocAdapter<BuildType>(ptr, bytes));
}

template<>
ALWAYS_INLINE void MemoryPool::AddRefAdapter<_Build::TL>(void *ptr)
{
    AddRefThreadLocal(ptr);
}

template<>
ALWAYS_INLINE void MemoryPool::AddRefAdapter<_Build::MT>(void *ptr)
{
    AddRef(ptr);
}

template<>
ALWAYS_INLINE void MemoryPool::FreeAdapter<_Build::TL>(void *ptr)
{
    FreeThreadLocal(ptr);
}

template<>
ALWAYS_INLINE void MemoryPool::FreeAdapter<_Build::MT>(void *ptr)
{
    Free(ptr);
}

ALWAYS_INLINE LibString MemoryPool::ToString() const
{
    return this->UsingInfo();
}

ALWAYS_INLINE UInt64 MemoryPool::GetPoolThreadId() const
{
    return _threadId;
}

ALWAYS_INLINE UInt64 MemoryPool::GetMaxPoolAllocable() const
{
    return _bytesPosRefAlloctors.size();
}

KERNEL_END

// ????????????????????????????????????????????????????????????????????????
// extern KERNEL_EXPORT std::atomic<KERNEL_NS::MemoryPool *> g_MemoryPool;

#endif
