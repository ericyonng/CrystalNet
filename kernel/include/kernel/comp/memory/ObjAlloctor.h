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

#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>
#include <kernel/comp/memory/MemoryDefs.h>
#include <kernel/comp/Delegate/IDelegate.h>
#include <kernel/comp/memory/MemoryAlloctor.h>
#include <kernel/comp/memory/Defs/MemoryAlloctorConfig.h>
#include <kernel/comp/Utils/RttiUtil.h>
#include <kernel/comp/MemoryMonitor/memorymonitor_inc.h>
#include <kernel/comp/LibString.h>
#include <kernel/common/Destructor.h>

KERNEL_BEGIN

template<typename ObjType>
class ObjAlloctor
{
    ObjAlloctor(const ObjAlloctor<ObjType>&) = delete;
    ObjAlloctor(ObjAlloctor<ObjType>&&) = delete;
    ObjAlloctor &operator = (const ObjAlloctor<ObjType>&) = delete;

public:
    using AgainstTemplateLazyInit = Int32;
    AgainstTemplateLazyInit _againstLazy = 0;

public:
    ObjAlloctor(bool isThreadLocal, UInt64 initBlockNumPerBuffer = MEMORY_BUFFER_BLOCK_INIT, const MemoryAlloctorConfig &alloctorCfg = MemoryAlloctorConfig(sizeof(ObjType)));
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
    bool _isThreadLocal;
};


template<typename ObjType>
ALWAYS_INLINE ObjAlloctor<ObjType>::ObjAlloctor(bool isThreadLocal, UInt64 initBlockNumPerBuffer, const MemoryAlloctorConfig &alloctorCfg)
    :_threadId(KernelGetCurrentThreadId())
    ,_name(RttiUtil::GetByType<ObjType>())
    ,_objSize(sizeof(ObjType))
    ,_alloctor(alloctorCfg)
    ,_objPoolPrintDelg(NULL)
    ,_isThreadLocal(isThreadLocal)
{
    _alloctor.Init(_isThreadLocal, initBlockNumPerBuffer, RttiUtil::GetByType<ObjType>().GetRaw());

    auto staticstics = MemoryMonitor::GetStatistics();
    _objPoolPrintDelg = DelegateFactory::Create<ObjAlloctor<ObjType>, UInt64, LibString &>(this, &ObjAlloctor<ObjType>::ToMonitor);
    staticstics->Lock();
    staticstics->GetDict().push_back(_objPoolPrintDelg);
    staticstics->Unlock();
}

template<typename ObjType>
ALWAYS_INLINE ObjAlloctor<ObjType>::~ObjAlloctor()
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
ALWAYS_INLINE void ObjAlloctor<ObjType>::Release()
{
    CRYSTAL_DELETE(this);
}

template<typename ObjType>
template<typename... Args>
ALWAYS_INLINE ObjType *ObjAlloctor<ObjType>::New(Args &&... args)
{
    // TODO:跨线程的因为性能问题暂时使用系统分配 TODO:对比下跨线程new/delete 与对象池/内存池new/delete性能, 考虑释放优先使用ThreadLocal本地分配
    // return new ObjType(std::forward<Args>(args)...);
    _alloctor.Lock();
    auto ptr = _alloctor.Alloc(_objSize);
    _alloctor.Unlock();
    return ::new(ptr)ObjType(std::forward<Args>(args)...);
}

template<typename ObjType>
ALWAYS_INLINE ObjType *ObjAlloctor<ObjType>::NewNoConstruct()
{
    // TODO:跨线程的因为性能问题暂时使用系统分配 TODO:对比下跨线程new/delete 与对象池/内存池new/delete性能, 考虑释放优先使用ThreadLocal本地分配
    // auto ptr = ::malloc(_objSize);
    _alloctor.Lock();
    auto ptr = _alloctor.Alloc(_objSize);
    _alloctor.Unlock();
    return  reinterpret_cast<ObjType *>(ptr);
}

template<typename ObjType>
template<typename... Args>
ALWAYS_INLINE ObjType *ObjAlloctor<ObjType>::NewByPtr(void *ptr, Args&&... args)
{
    return ::new(ptr)ObjType(std::forward<Args>(args)...);
}

template<typename ObjType>
ALWAYS_INLINE void ObjAlloctor<ObjType>::Delete(ObjType *ptr)
{
    auto memoryBlock = _alloctor.GetMemoryBlockBy(ptr);
    
    // 先析构后释放
    if(LIKELY(memoryBlock->_ref == 1))
    {
        Destructor::Invoke(ptr);
        // ptr->~ObjType();
    }

    _alloctor.Lock();
    _alloctor.Free(memoryBlock);
    _alloctor.Unlock();
}

template<typename ObjType>
ALWAYS_INLINE void ObjAlloctor<ObjType>::DeleteNoDestructor(void *ptr)
{
    // TODO:跨线程的因为性能问题暂时使用系统分配 TODO:对比下跨线程new/delete 与对象池/内存池new/delete性能, 考虑释放优先使用ThreadLocal本地分配
    // ::free(ptr);

    auto memoryBlock = _alloctor.GetMemoryBlockBy(ptr);

    _alloctor.Lock();
    _alloctor.Free(memoryBlock);
    _alloctor.Unlock();
}

template<typename ObjType>
ALWAYS_INLINE void ObjAlloctor<ObjType>::AddRef(void *ptr)
{
    _alloctor.AddRef(ptr);
}

template<typename ObjType>
template<typename... Args>
ALWAYS_INLINE ObjType *ObjAlloctor<ObjType>::NewThreadLocal(Args &&... args)
{
    auto ptr = _alloctor.Alloc(_objSize);
    return ::new(ptr)ObjType(std::forward<Args>(args)...);
}

template<typename ObjType>
ALWAYS_INLINE ObjType *ObjAlloctor<ObjType>::NewNoConstructThreadLocal()
{
    auto ptr = _alloctor.Alloc(_objSize);
    return  reinterpret_cast<ObjType *>(ptr);
}

template<typename ObjType>
template<typename... Args>
ALWAYS_INLINE ObjType *ObjAlloctor<ObjType>::NewByPtrThreadLocal(void *ptr, Args&&... args)
{
    return ::new(ptr)ObjType(std::forward<Args>(args)...);
}

template<typename ObjType>
ALWAYS_INLINE void ObjAlloctor<ObjType>::DeleteThreadLocal(ObjType *ptr)
{
    auto memoryBlock = _alloctor.GetMemoryBlockBy(ptr);

    // 先析构后释放
    if(LIKELY(memoryBlock->_ref == 1))
        Destructor::Invoke(ptr);

    _alloctor.Free(memoryBlock);
}

template<typename ObjType>
ALWAYS_INLINE void ObjAlloctor<ObjType>::DeleteNoDestructorThreadLocal(void *ptr)
{
    _alloctor.Free(_alloctor.GetMemoryBlockBy(ptr));
}

template<typename ObjType>
ALWAYS_INLINE void ObjAlloctor<ObjType>::AddRefThreadLocal(void *ptr)
{
    _alloctor.AddRef(ptr);
}

// 适配tls与多线程
template<typename ObjType>
template<typename BuildType, typename... Args>
ALWAYS_INLINE ObjType *ObjAlloctor<ObjType>::NewByAdapter(Args &&... args)
{
    return NewByAdapterBy(BuildType::V, std::forward<Args>(args)...);
}

template<typename ObjType>
template<typename BuildType>
ALWAYS_INLINE ObjType *ObjAlloctor<ObjType>::NewByAdapterNoConstruct()
{
    return NewByAdapterNoConstructBy(BuildType::V);
}

template<typename ObjType>
template<typename BuildType, typename... Args>
ALWAYS_INLINE ObjType *ObjAlloctor<ObjType>::NewByAdapterByPtr(void *ptr, Args &&... args)
{
    return NewByAdapterByPtrBy(BuildType::V, ptr, std::forward<Args>(args)...);
}

template<typename ObjType>
template<typename BuildType>
ALWAYS_INLINE void ObjAlloctor<ObjType>::DeleteByAdapter(ObjType *ptr)
{
    DeleteByAdapterBy(BuildType::V, ptr);
}

template<typename ObjType>
template<typename BuildType>
ALWAYS_INLINE void ObjAlloctor<ObjType>::DeleteByAdapterNoDestructor(void *ptr)
{
    DeleteByAdapterNoDestructorBy(BuildType::V, ptr);
}

template<typename ObjType>
template<typename BuildType>
ALWAYS_INLINE void ObjAlloctor<ObjType>::AddRefByAdapter(void *ptr)
{
    AddRefByAdapterBy(BuildType::V, ptr);
}

template<typename ObjType>
ALWAYS_INLINE LibString ObjAlloctor<ObjType>::ToString() const
{
    return UsingInfo();
}

template<typename ObjType>
ALWAYS_INLINE LibString ObjAlloctor<ObjType>::UsingInfo() const
{
    LibString info;
    info << "obj name:" << _name <<", obj size:" << _objSize << ", thread id:" << std::to_string(_threadId) << ", alloctor info: " << _alloctor.UsingInfo();
    return info;
}

template<typename ObjType>
ALWAYS_INLINE UInt64 ObjAlloctor<ObjType>::ToMonitor(LibString &str)
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
