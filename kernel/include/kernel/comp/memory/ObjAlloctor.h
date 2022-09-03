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
 * Date: 2020-11-30 00:25:22
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_OBJ_ALLOCTOR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_OBJ_ALLOCTOR_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/MemoryAlloctor.h>
#include <kernel/comp/memory/Defs/MemoryAlloctorConfig.h>
#include <kernel/comp/Utils/RttiUtil.h>
#include <kernel/comp/MemoryMonitor/memorymonitor_inc.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

template<typename ObjType>
class ObjAlloctor
{
    ObjAlloctor(const ObjAlloctor<ObjType>&) = delete;
    ObjAlloctor(ObjAlloctor<ObjType>&&) = delete;
    ObjAlloctor &operator = (const ObjAlloctor<ObjType>&) = delete;

public:
    ObjAlloctor(UInt64 initBlockNumPerBuffer = MEMORY_BUFFER_BLOCK_INIT, const MemoryAlloctorConfig &alloctorCfg = MemoryAlloctorConfig(sizeof(ObjType)));
    virtual ~ObjAlloctor();
    virtual void Release();

public:
    template<typename... Args>
    ObjType *New(Args &&... args);
    ObjType *NewNoConstruct();
    template<typename... Args>
    ObjType *NewByPtr(void *ptr, Args &&... args);

    void Delete(ObjType *ptr);
    void DeleteNoDestructor(void *ptr);    
    void AddRef(void *ptr);

    template<typename... Args>
    ObjType *NewThreadLocal(Args &&... args);
    ObjType *NewNoConstructThreadLocal();
    template<typename... Args>
    ObjType *NewByPtrThreadLocal(void *ptr, Args &&... args);

    void DeleteThreadLocal(ObjType *ptr);
    void DeleteNoDestructorThreadLocal(void *ptr);    
    void AddRefThreadLocal(void *ptr);

    // 适配tls与多线程
    template<typename BuildType, typename... Args>
    ObjType *NewByAdapter(Args &&... args);
    template<typename BuildType>
    ObjType *NewByAdapterNoConstruct();
    template<typename BuildType, typename... Args>
    ObjType *NewByAdapterByPtr(void *ptr, Args &&... args);
    
    template<typename BuildType>
    void DeleteByAdapter(ObjType *ptr);
    template<typename BuildType>
    void DeleteByAdapterNoDestructor(void *ptr);    
    template<typename BuildType>
    void AddRefByAdapter(void *ptr);

    // void Lock();
    // void Unlock();

    // TODO:后期需要替换
    LibString ToString() const;
    LibString UsingInfo() const;
    UInt64 ToMonitor(LibString &str);

    template<typename... Args>
    ObjType *NewByAdapterBy(_Build::MT::Type, Args &&... args);
    template<typename... Args>
    ObjType *NewByAdapterBy(_Build::TL::Type, Args &&... args);
    ObjType *NewByAdapterNoConstructBy(_Build::MT::Type);
    ObjType *NewByAdapterNoConstructBy(_Build::TL::Type);
    template<typename... Args>
    ObjType *NewByAdapterByPtrBy(_Build::MT::Type, void *ptr, Args &&... args);
    template<typename... Args>
    ObjType *NewByAdapterByPtrBy(_Build::TL::Type, void *ptr, Args &&... args);
    
    void DeleteByAdapterBy(_Build::MT::Type, ObjType *ptr);
    void DeleteByAdapterBy(_Build::TL::Type, ObjType *ptr);
    void DeleteByAdapterNoDestructorBy(_Build::MT::Type, void *ptr);    
    void DeleteByAdapterNoDestructorBy(_Build::TL::Type, void *ptr);    
    void AddRefByAdapterBy(_Build::MT::Type, void *ptr);
    void AddRefByAdapterBy(_Build::TL::Type, void *ptr);

protected:
    // 内存分配收集器
    friend class AlloctorInfoCollector;
    
    UInt64 _threadId;
    const LibString _name;
    const UInt64 _objSize;
    MemoryAlloctor _alloctor;
    IDelegate<UInt64, LibString &> *_objPoolPrintDelg;
};


template<typename ObjType>
inline ObjAlloctor<ObjType>::ObjAlloctor(UInt64 initBlockNumPerBuffer, const MemoryAlloctorConfig &alloctorCfg)
    :_threadId(KernelGetCurrentThreadId())
    ,_name(RttiUtil::GetByType<ObjType>())
    ,_objSize(sizeof(ObjType))
    ,_alloctor(alloctorCfg)
    ,_objPoolPrintDelg(NULL)
{
    _alloctor.Init(initBlockNumPerBuffer);

    auto staticstics = MemoryMonitor::GetStatistics();
    _objPoolPrintDelg = DelegateFactory::Create<ObjAlloctor<ObjType>, UInt64, LibString &>(this, &ObjAlloctor<ObjType>::ToMonitor);
    staticstics->Lock();
    staticstics->GetDict().push_back(_objPoolPrintDelg);
    staticstics->Unlock();
}

template<typename ObjType>
inline ObjAlloctor<ObjType>::~ObjAlloctor()
{
    if(LIKELY(_objPoolPrintDelg))
    {
        auto staticstics = MemoryMonitor::GetStatistics();
        staticstics->Lock();
        staticstics->Remove(_objPoolPrintDelg);
        staticstics->Unlock();
        _objPoolPrintDelg = NULL;
    }

    _alloctor.Destroy();
}

template<typename ObjType>
inline void ObjAlloctor<ObjType>::Release()
{
    CRYSTAL_DELETE(this);
}

template<typename ObjType>
template<typename... Args>
inline ObjType *ObjAlloctor<ObjType>::New(Args &&... args)
{
    _alloctor.Lock();
    auto ptr = _alloctor.Alloc(_objSize);
    _alloctor.Unlock();
    return ::new(ptr)ObjType(std::forward<Args>(args)...);
}

template<typename ObjType>
inline ObjType *ObjAlloctor<ObjType>::NewNoConstruct()
{
    _alloctor.Lock();
    auto ptr = _alloctor.Alloc(_objSize);
    _alloctor.Unlock();
    return  reinterpret_cast<ObjType *>(ptr);
}

template<typename ObjType>
template<typename... Args>
inline ObjType *ObjAlloctor<ObjType>::NewByPtr(void *ptr, Args&&... args)
{
    return ::new(ptr)ObjType(std::forward<Args>(args)...);
}

template<typename ObjType>
inline void ObjAlloctor<ObjType>::Delete(ObjType *ptr)
{
    // 先析构后释放
    ptr->~ObjType();

    _alloctor.Lock();
    _alloctor.Free(ptr);
    _alloctor.Unlock();
}

template<typename ObjType>
inline void ObjAlloctor<ObjType>::DeleteNoDestructor(void *ptr)
{
    _alloctor.Lock();
    _alloctor.Free(ptr);
    _alloctor.Unlock();
}

template<typename ObjType>
inline void ObjAlloctor<ObjType>::AddRef(void *ptr)
{
    _alloctor.AddRef(ptr);
}

template<typename ObjType>
template<typename... Args>
inline ObjType *ObjAlloctor<ObjType>::NewThreadLocal(Args &&... args)
{
    auto ptr = _alloctor.Alloc(_objSize);
    return ::new(ptr)ObjType(std::forward<Args>(args)...);
}

template<typename ObjType>
inline ObjType *ObjAlloctor<ObjType>::NewNoConstructThreadLocal()
{
    auto ptr = _alloctor.Alloc(_objSize);
    return  reinterpret_cast<ObjType *>(ptr);
}

template<typename ObjType>
template<typename... Args>
inline ObjType *ObjAlloctor<ObjType>::NewByPtrThreadLocal(void *ptr, Args&&... args)
{
    return ::new(ptr)ObjType(std::forward<Args>(args)...);
}

template<typename ObjType>
inline void ObjAlloctor<ObjType>::DeleteThreadLocal(ObjType *ptr)
{
    // 先析构后释放
    ptr->~ObjType();

    // c++ 11 支持thread_local
    // #if CRYSTAL_TARGET_PLATFORM_WINDOWS
    //     _alloctor.Lock();
    // #endif
    _alloctor.Free(ptr);
    // #if CRYSTAL_TARGET_PLATFORM_WINDOWS
    //     _alloctor.Unlock();
    // #endif
}

template<typename ObjType>
inline void ObjAlloctor<ObjType>::DeleteNoDestructorThreadLocal(void *ptr)
{
    _alloctor.Free(ptr);
}

template<typename ObjType>
inline void ObjAlloctor<ObjType>::AddRefThreadLocal(void *ptr)
{
    _alloctor.AddRef(ptr);
}

// 适配tls与多线程
template<typename ObjType>
template<typename BuildType, typename... Args>
inline ObjType *ObjAlloctor<ObjType>::NewByAdapter(Args &&... args)
{
    return NewByAdapterBy(BuildType::V, std::forward<Args>(args)...);
}

template<typename ObjType>
template<typename BuildType>
inline ObjType *ObjAlloctor<ObjType>::NewByAdapterNoConstruct()
{
    return NewByAdapterNoConstructBy(BuildType::V);
}

template<typename ObjType>
template<typename BuildType, typename... Args>
inline ObjType *ObjAlloctor<ObjType>::NewByAdapterByPtr(void *ptr, Args &&... args)
{
    return NewByAdapterByPtrBy(BuildType::V, ptr, std::forward<Args>(args)...);
}

template<typename ObjType>
template<typename BuildType>
inline void ObjAlloctor<ObjType>::DeleteByAdapter(ObjType *ptr)
{
    DeleteByAdapterBy(BuildType::V, ptr);
}

template<typename ObjType>
template<typename BuildType>
inline void ObjAlloctor<ObjType>::DeleteByAdapterNoDestructor(void *ptr)
{
    DeleteByAdapterNoDestructorBy(BuildType::V, ptr);
}

template<typename ObjType>
template<typename BuildType>
inline void ObjAlloctor<ObjType>::AddRefByAdapter(void *ptr)
{
    AddRefByAdapterBy(BuildType::V, ptr);
}

// template<typename ObjType>
// inline void ObjAlloctor<ObjType>::Lock()
// {
//     _alloctor.Lock();
// }

// template<typename ObjType>
// inline void ObjAlloctor<ObjType>::Unlock()
// {
//     _alloctor.Unlock();
// }


template<typename ObjType>
inline LibString ObjAlloctor<ObjType>::ToString() const
{
    return UsingInfo();
}

template<typename ObjType>
inline LibString ObjAlloctor<ObjType>::UsingInfo() const
{
    LibString info;
    // info << _name;
    // info << "obj name:";
    info << "obj name:" << _name <<", obj size:" << _objSize << ", thread id:" << std::to_string(_threadId) << ", alloctor info: " << _alloctor.UsingInfo();
    return info;
}

template<typename ObjType>
inline UInt64 ObjAlloctor<ObjType>::ToMonitor(LibString &str)
{
    str = UsingInfo();

    return _alloctor.GetTotalBytes();
}

template<typename ObjType>
template<typename... Args>
ALWAYS_INLINE ObjType *ObjAlloctor<ObjType>::NewByAdapterBy(_Build::MT::Type, Args &&... args)
{
    return New(std::forward<Args>(args)...);
}

template<typename ObjType>
template<typename... Args>
ALWAYS_INLINE ObjType *ObjAlloctor<ObjType>::NewByAdapterBy(_Build::TL::Type, Args &&... args)
{
    return NewThreadLocal(std::forward<Args>(args)...);
}

template<typename ObjType>
ALWAYS_INLINE ObjType *ObjAlloctor<ObjType>::NewByAdapterNoConstructBy(_Build::MT::Type)
{
    return NewNoConstruct();
}

template<typename ObjType>
ALWAYS_INLINE ObjType *ObjAlloctor<ObjType>::NewByAdapterNoConstructBy(_Build::TL::Type)
{
    return NewNoConstructThreadLocal();
}

template<typename ObjType>
template<typename... Args>
ALWAYS_INLINE ObjType *ObjAlloctor<ObjType>::NewByAdapterByPtrBy(_Build::MT::Type, void *ptr, Args &&... args)
{
    return NewByPtr(ptr, std::forward<Args>(args)...);
}

template<typename ObjType>
template<typename... Args>
ALWAYS_INLINE ObjType *ObjAlloctor<ObjType>::NewByAdapterByPtrBy(_Build::TL::Type, void *ptr, Args &&... args)
{
    return NewByPtr(ptr, std::forward<Args>(args)...);
}

template<typename ObjType>
ALWAYS_INLINE void ObjAlloctor<ObjType>::DeleteByAdapterBy(_Build::MT::Type, ObjType *ptr)
{
    Delete(ptr);
}

template<typename ObjType>
ALWAYS_INLINE void ObjAlloctor<ObjType>::DeleteByAdapterBy(_Build::TL::Type, ObjType *ptr)
{
    DeleteThreadLocal(ptr);
}

template<typename ObjType>
ALWAYS_INLINE void ObjAlloctor<ObjType>::DeleteByAdapterNoDestructorBy(_Build::MT::Type, void *ptr)
{
    DeleteNoDestructor(ptr);
}  

template<typename ObjType>
ALWAYS_INLINE void ObjAlloctor<ObjType>::DeleteByAdapterNoDestructorBy(_Build::TL::Type, void *ptr)
{
    DeleteNoDestructorThreadLocal(ptr);
}

template<typename ObjType>
ALWAYS_INLINE void ObjAlloctor<ObjType>::AddRefByAdapterBy(_Build::MT::Type, void *ptr)
{
    AddRef(ptr);
}

template<typename ObjType>
ALWAYS_INLINE void ObjAlloctor<ObjType>::AddRefByAdapterBy(_Build::TL::Type, void *ptr)
{
    AddRefThreadLocal(ptr);
}

KERNEL_END

#endif
