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
    InitMemoryPoolInfo(UInt64 miniAllockBlockBytes = MEMORY_POOL_MINI_BLOCK_BYTES, UInt64 maxAllockBlockBytes = MEMORY_POOL_MAX_BLOCK_BYTES);

    // MEMORY_POOL_MINI_BLOCK_BYTES 的最小公倍数
    UInt64 GetMin() const
    {
        return _miniAllockBlockBytes;
    }

    // 绝对内存池所能分配的最大内存块 MEMORY_POOL_MAX_BLOCK_BYTES 的最小公倍数
    UInt64 GetMax() const
    {
        return _maxAllockBlockBytes;
    }

    // 不修改以默认值执行
    MemoryAlloctorConfigs &GetAlloctorCfgsToModify()
    {
        return _alloctorCfgs;
    }

    const MemoryAlloctorConfigs &GetAlloctorCfgs() const
    {
        return _alloctorCfgs;
    }

private:
    UInt64 _miniAllockBlockBytes;           // 最小内存块大小
    UInt64 _maxAllockBlockBytes;            // 最大内存块大小
    MemoryAlloctorConfigs _alloctorCfgs;
};

class KERNEL_EXPORT MemoryPool
{
public:
    MemoryPool(const InitMemoryPoolInfo &initInfo = InitMemoryPoolInfo(), const std::string &reason = "");
    virtual ~MemoryPool();

public:
    /* 初始化与销毁以及单例线程安全 */
    // 单例对象底层不需要提供接口需要应用层自己实现  请谨慎使用特殊的内存池
    static MemoryPool *GetInstance(const InitMemoryPoolInfo &initInfo = InitMemoryPoolInfo());
    // 默认全局的内存池 避免不当使用导致内存在不同的内存池释放导致致命错误！！！
    static MemoryPool *GetDefaultInstance();
    Int32 Init();
    void Destroy();

    /* 线程安全 */
    template<typename PtrType>
    PtrType *Alloc(UInt64 bytes);
    void *Alloc(UInt64 bytes);
    void *Realloc(void *ptr, UInt64 bytes);
    template<typename PtrType>
    PtrType *Realloc(void *ptr, UInt64 bytes);
    void AddRef(void *ptr);
    void Free(void *ptr);

    // thread_local相关接口
    template<typename PtrType>
    PtrType *AllocThreadLocal(UInt64 bytes);
    void *AllocThreadLocal(UInt64 bytes);
    void *ReallocThreadLocal(void *ptr, UInt64 bytes);
    template<typename PtrType>
    PtrType *ReallocThreadLocal(void *ptr, UInt64 bytes);
    void AddRefThreadLocal(void *ptr);
    void FreeThreadLocal(void *ptr);

    // 适配tls与多线程
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
    // 最大可池中分配大小，超过的自动使用系统分配
    UInt64 GetMaxPoolAllocable() const;

private:
    void _InitMemRange(UInt64 begin, UInt64 end, MemoryAlloctor *alloctor);

private:
    std::vector<MemoryAlloctor *> _bytesPosRefAlloctors;            // 内存字节数对应一个分配器
    std::vector<MemoryAlloctor *> _alloctorArray;                   // 这里面的alloctor是唯一的不重复的，用于销毁分配器

    std::atomic_bool _isInit;                                       // 是否已初始化
    Locker _lck;
    const UInt64 _miniAllockBlockBytes;                             // 最小内存块大小
    const UInt64 _maxAllockBlockBytes;                              // 最大内存块大小
    const UInt64 _memoryBlockSizeAfterAlign;                        // 内存对齐后的内存块大小
    const MemoryAlloctorConfigs _alloctorCfgs;                      // 分配器配置
    IDelegate<UInt64, LibString &> *_monitorPrintDelg;                // 内存池日志回调
    const std::string _reason;
    UInt64 _threadId;
};

inline MemoryPool::~MemoryPool()
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

inline MemoryPool *MemoryPool::GetDefaultInstance()
{
    return GetInstance();
}


template<typename PtrType>
inline PtrType *MemoryPool::Alloc(UInt64 bytes)
{
    return reinterpret_cast<PtrType *>(Alloc(bytes));
}

template<typename PtrType>
inline PtrType *MemoryPool::Realloc(void *ptr, UInt64 bytes)
{
    return reinterpret_cast<PtrType *>(Realloc(ptr, bytes));
}

template<typename PtrType>
inline PtrType *MemoryPool::AllocThreadLocal(UInt64 bytes)
{
    return reinterpret_cast<PtrType *>(AllocThreadLocal(bytes));
}

template<typename PtrType>
inline PtrType *MemoryPool::ReallocThreadLocal(void *ptr, UInt64 bytes)
{
    return reinterpret_cast<PtrType *>(ReallocThreadLocal(ptr, bytes));
}

template<typename PtrType, typename BuildType>
inline PtrType *MemoryPool::AllocAdapter(UInt64 bytes)
{
    return reinterpret_cast<PtrType *>(AllocAdapter<BuildType>(bytes));
}

// tls版本
template<>
inline void *MemoryPool::AllocAdapter<_Build::TL>(UInt64 bytes)
{
    return AllocThreadLocal(bytes);
}

// MT版本
template<>
inline void *MemoryPool::AllocAdapter<_Build::MT>(UInt64 bytes)
{
    return Alloc(bytes);
}

template<>
inline void *MemoryPool::ReallocAdapter<_Build::TL>(void *ptr, UInt64 bytes)
{
    return ReallocThreadLocal(ptr, bytes);
}

template<>
inline void *MemoryPool::ReallocAdapter<_Build::MT>(void *ptr, UInt64 bytes)
{
    return Realloc(ptr, bytes);
}

template<typename PtrType, typename BuildType>
inline PtrType *MemoryPool::ReallocAdapter(void *ptr, UInt64 bytes)
{
    return reinterpret_cast<PtrType *>(ReallocAdapter<BuildType>(ptr, bytes));
}

template<>
inline void MemoryPool::AddRefAdapter<_Build::TL>(void *ptr)
{
    AddRefThreadLocal(ptr);
}

template<>
inline void MemoryPool::AddRefAdapter<_Build::MT>(void *ptr)
{
    AddRef(ptr);
}

template<>
inline void MemoryPool::FreeAdapter<_Build::TL>(void *ptr)
{
    FreeThreadLocal(ptr);
}

template<>
inline void MemoryPool::FreeAdapter<_Build::MT>(void *ptr)
{
    Free(ptr);
}

inline LibString MemoryPool::ToString() const
{
    return this->UsingInfo();
}

inline UInt64 MemoryPool::GetPoolThreadId() const
{
    return _threadId;
}

inline UInt64 MemoryPool::GetMaxPoolAllocable() const
{
    return _bytesPosRefAlloctors.size();
}

KERNEL_END

// 库不提供内存池初始化所有都得应用层自己实现初始化
// extern KERNEL_EXPORT std::atomic<KERNEL_NS::MemoryPool *> g_MemoryPool;

#endif
