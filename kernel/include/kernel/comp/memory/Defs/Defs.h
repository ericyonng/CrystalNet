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
 * Date: 2021-01-10 23:59:36
 * Author: Eric Yonng
 * Description: 
 * 1. OBJ_POOL_CREATE / OBJ_POOL_CREATE_IMPL 不可直接使用 直接使用会大概率出现内存非法写入到其他对象内存区
 * 2.请使用类继承相关的对象池宏
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_DEFS_DEFS_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_DEFS_DEFS_H__

#pragma once

#include <kernel/common/macro.h>
#include <kernel/common/LibObject.h>
#include <kernel/comp/memory/Defs/MemoryAlloctorConfig.h>
#include <kernel/comp/Utils/TlsUtil.h>
#include <kernel/comp/Tls/TlsObjectPool.h>
#include <kernel/comp/memory/ObjAlloctor.h>
#include <kernel/comp/memory/AlloctorInfoCollector.h>

// 禁用 new/delete,因为派生类会调用父类的new/delete,如果派生类没哟使用对象池宏,会导致父类的对象池分配子类,将导致内存错误
// void  *operator new(size_t bytes)                                                                         
// {                                                                                                         
//     _objAlloctor.Lock();                                                                                  
//     auto ptr = _objAlloctor.NewNoConstruct();                                                             
//     _objAlloctor.Unlock();                                                                                
//     return ptr;                                                                                           
// }                                                                                                         
                                                                                                    
// void   operator delete(void *ptr)                                                                         
// {                                                                                                         
//     _objAlloctor.Lock();                                                                                  
//     _objAlloctor.DeleteNoDestructor(ptr);                                                                 
//     _objAlloctor.Unlock();                                                                                
// }                                                                                                         
                                                                                                    

/// 内存池创建对象便利宏
// 禁止直接使用 __OBJ_POOL_CREATE 直接使用会大概率出现内存非法写入到其他对象内存区 ！！！！！！！！！！！！！！！！！！！！！
// WarningSeriously Dont Use objpoolcreate derectly!!!!!!!!!!
#undef  __OBJ_POOL_CREATE
#define __OBJ_POOL_CREATE(createBufferNumWhenInit, initBlockNumPerBuffer, ObjType, _objAlloctor)                                                             \
public:                                                                                                                             \
        template<typename... Args>                                                                                                  \
        static ALWAYS_INLINE ObjType *New_##ObjType(Args &&... args)                                                                \
        {                                                                                                                           \
            return _static##ObjType##Alloctor->New(std::forward<Args>(args)...);                                                    \
        }                                                                                                                           \
                                                                                                                                    \
        static ALWAYS_INLINE ObjType *New_##ObjType##NoConstruct()                                                                                \
        {                                                                                                                           \
            return _static##ObjType##Alloctor->NewNoConstruct();                                                                   \
        }                                                                                                                           \
                                                                                                                                    \
        template<typename... Args>                                                                                                  \
        static ALWAYS_INLINE ObjType *New_##ObjType##ByPtr(void *ptr, Args &&... args)                                                            \
        {                                                                                                                           \
            return _static##ObjType##Alloctor->NewByPtr(ptr, std::forward<Args>(args)...);                                         \
        }                                                                                                                           \
                                                                                                                                    \
        static ALWAYS_INLINE void Delete_##ObjType(ObjType *ptr)                                                                                  \
        {                                                                                                                           \
            _static##ObjType##Alloctor->Delete(ptr);                                                                               \
        }                                                                                                                           \
                                                                                                                                    \
        static ALWAYS_INLINE void Delete_##ObjType##NoDestructor(ObjType *ptr)                                                                    \
        {                                                                                                                           \
            _static##ObjType##Alloctor->DeleteNoDestructor(ptr);                                                                   \
        }                                                                                                                           \
                                                                                                                                    \
        ALWAYS_INLINE void AddRef_##ObjType()                                                                                                     \
        {                                                                                                                           \
            _static##ObjType##Alloctor->AddRef(this);                                                                              \
        }                                                                                                                           \
                                                                                                                                    \
        static ALWAYS_INLINE KERNEL_NS::ObjAlloctor<ObjType> &GetAlloctor_##_objAlloctor()                                          \
        {                                                                                                                           \
            return *_static##ObjType##Alloctor;                                                                                     \
        }                                                                                                                           \
        static ALWAYS_INLINE KERNEL_NS::ObjAlloctor<ObjType> &GetThreadLocalAlloctor_##_objAlloctor()                               \
        {                                                                                                                           \
            if(UNLIKELY(!_staticThreadLocal##ObjType##Alloctor))                                                                    \
            {                                                                                                                       \
                _staticThreadLocal##ObjType##Alloctor = KERNEL_NS::TlsUtil::GetTlsStack()->New<KERNEL_NS::TlsObjectPool<KERNEL_NS::ObjAlloctor<ObjType>> >()->GetPool(initBlockNumPerBuffer          \
            , KERNEL_NS::MemoryAlloctorConfig(sizeof(ObjType), createBufferNumWhenInit));                                           \
            }                                                                                                                       \
                                                                                                                                    \
            return *_staticThreadLocal##ObjType##Alloctor;                                                                          \
        }                                                                                                                           \
        template<typename... Args>                                                                                                  \
        static ALWAYS_INLINE ObjType *NewThreadLocal_##ObjType(Args &&... args)                                                                   \
        {                                                                                                                           \
            return GetThreadLocalAlloctor_##_objAlloctor().NewThreadLocal(std::forward<Args>(args)...);                              \
        }                                                                                                                           \
                                                                                                                                    \
        static ALWAYS_INLINE ObjType *NewThreadLocal_##ObjType##NoConstruct()                                                                     \
        {                                                                                                                           \
            return GetThreadLocalAlloctor_##_objAlloctor().NewNoConstructThreadLocal();                                             \
        }                                                                                                                           \
                                                                                                                                    \
        template<typename... Args>                                                                                                  \
        static ALWAYS_INLINE ObjType *NewThreadLocal_##ObjType##ByPtr(void *ptr, Args &&... args)                                   \
        {                                                                                                                           \
            return GetThreadLocalAlloctor_##_objAlloctor().NewByPtrThreadLocal(ptr, std::forward<Args>(args)...);                   \
        }                                                                                                                           \
                                                                                                                                    \
        static ALWAYS_INLINE void DeleteThreadLocal_##ObjType(ObjType *ptr)                                                         \
        {                                                                                                                           \
            GetThreadLocalAlloctor_##_objAlloctor().DeleteThreadLocal(ptr);                                                         \
        }                                                                                                                           \
                                                                                                                                    \
        static ALWAYS_INLINE void DeleteThreadLocal_##ObjType##NoDestructor(ObjType *ptr)                                           \
        {                                                                                                                           \
            GetThreadLocalAlloctor_##_objAlloctor().DeleteNoDestructorThreadLocal(ptr);                                             \
        }                                                                                                                           \
                                                                                                                                    \
        ALWAYS_INLINE void AddRefThreadLocal_##ObjType()                                                                            \
        {                                                                                                                           \
            GetThreadLocalAlloctor_##_objAlloctor().AddRefThreadLocal(this);                                                        \
        }                                                                                                                           \
                                                                                                                                    \
                                                                                                                                    \
        template<typename... Args>                                                                                                  \
        static ALWAYS_INLINE ObjType *NewByAdapter_##ObjType(KERNEL_NS::_Build::MT::Type,  Args &&... args)                         \
        {                                                                                                                           \
            return _static##ObjType##Alloctor->New(std::forward<Args>(args)...);                                                   \
        }                                                                                                                           \
                                                                                                                                    \
        static ALWAYS_INLINE ObjType *NewByAdapter_##ObjType##NoConstruct(KERNEL_NS::_Build::MT::Type)                              \
        {                                                                                                                           \
            return _static##ObjType##Alloctor->NewNoConstruct();                                                                   \
        }                                                                                                                           \
                                                                                                                                    \
        template<typename... Args>                                                                                                  \
        static ALWAYS_INLINE ObjType *NewByAdapter_##ObjType##ByPtr(KERNEL_NS::_Build::MT::Type, void *ptr, Args &&... args)        \
        {                                                                                                                           \
            return _static##ObjType##Alloctor->NewByPtr(ptr, std::forward<Args>(args)...);                                         \
        }                                                                                                                           \
                                                                                                                                    \
        static ALWAYS_INLINE void DeleteByAdapter_##ObjType(KERNEL_NS::_Build::MT::Type, ObjType *ptr)                              \
        {                                                                                                                           \
            _static##ObjType##Alloctor->Delete(ptr);                                                                               \
        }                                                                                                                           \
                                                                                                                                    \
        static ALWAYS_INLINE void DeleteByAdapter_##ObjType##NoDestructor(KERNEL_NS::_Build::MT::Type, ObjType *ptr)                \
        {                                                                                                                           \
            _static##ObjType##Alloctor->DeleteNoDestructor(ptr);                                                                   \
        }                                                                                                                           \
                                                                                                                                    \
        ALWAYS_INLINE void AddRefByAdapter_##ObjType(KERNEL_NS::_Build::MT::Type)                                                   \
        {                                                                                                                           \
            _static##ObjType##Alloctor->AddRef(this);                                                                              \
        }                                                                                                                           \
                                                                                                                                    \
                                                                                                                                    \
        template<typename... Args>                                                                                                  \
        static ALWAYS_INLINE ObjType *NewByAdapter_##ObjType(KERNEL_NS::_Build::TL::Type,  Args &&... args)                         \
        {                                                                                                                           \
            return GetThreadLocalAlloctor_##_objAlloctor().NewThreadLocal(std::forward<Args>(args)...);                             \
        }                                                                                                                           \
                                                                                                                                    \
        static ALWAYS_INLINE ObjType *NewByAdapter_##ObjType##NoConstruct(KERNEL_NS::_Build::TL::Type)                              \
        {                                                                                                                           \
            return GetThreadLocalAlloctor_##_objAlloctor().NewNoConstructThreadLocal();                                             \
        }                                                                                                                           \
                                                                                                                                    \
        template<typename... Args>                                                                                                  \
        static ALWAYS_INLINE ObjType *NewByAdapter_##ObjType##ByPtr(KERNEL_NS::_Build::TL::Type, void *ptr, Args &&... args)        \
        {                                                                                                                           \
            return GetThreadLocalAlloctor_##_objAlloctor().NewByPtrThreadLocal(ptr, std::forward<Args>(args)...);                   \
        }                                                                                                                           \
                                                                                                                                    \
        static ALWAYS_INLINE void DeleteByAdapter_##ObjType(KERNEL_NS::_Build::TL::Type, ObjType *ptr)                              \
        {                                                                                                                           \
            GetThreadLocalAlloctor_##_objAlloctor().DeleteThreadLocal(ptr);                                                         \
        }                                                                                                                           \
                                                                                                                                    \
        static ALWAYS_INLINE void DeleteByAdapter_##ObjType##NoDestructor(KERNEL_NS::_Build::TL::Type, ObjType *ptr)                \
        {                                                                                                                           \
            GetThreadLocalAlloctor_##_objAlloctor().DeleteNoDestructorThreadLocal(ptr);                                             \
        }                                                                                                                           \
                                                                                                                                    \
        ALWAYS_INLINE void AddRefByAdapter_##ObjType(KERNEL_NS::_Build::TL::Type)                                                   \
        {                                                                                                                           \
            GetThreadLocalAlloctor_##_objAlloctor().AddRefThreadLocal(this);                                                        \
        }                                                                                                                           \
        private:                                                                                                                    \
        static KERNEL_NS::ObjAlloctor<ObjType> *_static##ObjType##Alloctor;                                                         \
        DEF_STATIC_THREAD_LOCAL_DECLEAR KERNEL_NS::ObjAlloctor<ObjType> *_staticThreadLocal##ObjType##Alloctor;                     \


#undef __OBJ_POOL_CREATE_IMPL
#define __OBJ_POOL_CREATE_IMPL(ObjType, createBufferNumWhenInit, initBlockNumPerBuffer)                                             \
KERNEL_NS::ObjAlloctor<ObjType> *ObjType::_static##ObjType##Alloctor = new KERNEL_NS::ObjAlloctor<ObjType>(false, initBlockNumPerBuffer    \
            , KERNEL_NS::MemoryAlloctorConfig(sizeof(ObjType), createBufferNumWhenInit));                                           \
DEF_THREAD_LOCAL_DECLEAR KERNEL_NS::ObjAlloctor<ObjType> *ObjType::_staticThreadLocal##ObjType##Alloctor = NULL

// 禁止直接使用 OBJ_POOL_CREATE_IMPL 直接使用会大概率出现内存非法写入到其他对象内存区 ！！！！！！！！！！！！！！


////////////////////////////////////////////////////////////////////////////////////

/* 祖先类对象池创建 */
// ancestor
// 请祖先类与派生类选择性使用,
// 使用各自类的 ObjPoolCollect_ 自检与对象池信息收集，可以实现在编译器检查出基类是不是对象池创建的
// 若祖先类是对象池创建的那么派生出来的子类必须是对象池创建的,
// 否则会出现当前对象非法访问本类其他实例内存区，造成不可挽回的内存非法访问，
#undef __OBJ_POOL_CREATE_ANCESTOR
#define __OBJ_POOL_CREATE_ANCESTOR(createBufferNumWhenInit, initBlockNumPerBuffer, ObjType)                                                        \
__OBJ_POOL_CREATE(createBufferNumWhenInit, initBlockNumPerBuffer, ObjType, _##ObjType##objAlloctor);                                               \
public:                                                                                                                                         \
    ALWAYS_INLINE bool ObjPoolCollect_##ObjType(KERNEL_NS::AlloctorInfoCollector &collector);                                                   \
    {                                                                                                                                           \
        collector.Collect(GetAlloctor__##ObjType##objAlloctor()); return true;                                                                  \
    }                                                                                                                                           \
    ALWAYS_INLINE bool ObjPoolCollectThreadLocal_##ObjType(KERNEL_NS::AlloctorInfoCollector &collector);                                        \
    {                                                                                                                                           \
        collector.Collect(GetThreadLocalAlloctor__##ObjType##objAlloctor()); return true;                                                       \
    }


// 请祖先类与派生类选择性使用,
// 使用各自类的 ObjPoolCollect_ 自检与对象池信息收集，可以实现在编译器检查出基类是不是对象池创建的
// 若祖先类是对象池创建的那么派生出来的子类必须是对象池创建的,
// 否则会出现当前对象非法访问本类其他实例内存区，造成不可挽回的内存非法访问，
#undef __OBJ_POOL_CREATE_ANCESTOR_IMPL
#define __OBJ_POOL_CREATE_ANCESTOR_IMPL(ObjType)                                                                                                 \
ALWAYS_INLINE bool ObjType::ObjPoolCollect_##ObjType(KERNEL_NS::AlloctorInfoCollector &collector){ collector.Collect(GetAlloctor__##ObjType##objAlloctor()); return true; }


/* 派生类对象池创建 */
// derive
// 请祖先类与派生类选择性使用,
// 使用各自类的ObjPoolCollect_ 自检与对象池信息收集，可以实现在编译器检查出基类是不是对象池创建的
// 若祖先类是对象池创建的那么派生出来的子类必须是对象池创建的,
// 否则会出现当前对象非法访问本类其他实例内存区，造成不可挽回的内存非法访问，
#undef __OBJ_POOL_CREATE_OBJ
#define __OBJ_POOL_CREATE_OBJ(createBufferNumWhenInit, initBlockNumPerBuffer, ObjType)                                                                                                 \
__OBJ_POOL_CREATE(createBufferNumWhenInit, initBlockNumPerBuffer, ObjType, _##ObjType##objAlloctor)                                                                                              \
 public:\
    ALWAYS_INLINE bool ObjPoolDetectLeftPart_##ObjType(\

#undef __OBJ_POOL_CREATE_GEN_BEGIN
#define __OBJ_POOL_CREATE_GEN_BEGIN(ObjType)\
){return true;}\
 public:\
    ALWAYS_INLINE bool ObjPoolCollect_##ObjType(KERNEL_NS::AlloctorInfoCollector &collector)\
    {\
        collector.Collect(GetAlloctor__##ObjType##objAlloctor());\

// 添加通用
#undef __OBJ_POOL_CREATE_GEN_ADD_PARANT
#define __OBJ_POOL_CREATE_GEN_ADD_PARANT(ParantClass) \
collector.AddDecorate("Parant:\n");\
this->ObjPoolCollect_##ParantClass(collector);\
collector.AddDecorate("; \n");\

#undef __OBJ_POOL_CREATE_GEN_END
#define __OBJ_POOL_CREATE_GEN_END()\
return true;\

// THREAD LOCAL 部分
#undef __OBJ_POOL_CREATE_THREAD_LOCAL_BEGIN
#define __OBJ_POOL_CREATE_THREAD_LOCAL_BEGIN(ObjType)\
}\
 public:\
    ALWAYS_INLINE bool ObjPoolCollectThreadLocal_##ObjType(KERNEL_NS::AlloctorInfoCollector &collector)\
    {\
        collector.Collect(GetThreadLocalAlloctor__##ObjType##objAlloctor());\

// 添加通用
#undef __OBJ_POOL_CREATE_THREAD_LOCAL_ADD_PARANT
#define __OBJ_POOL_CREATE_THREAD_LOCAL_ADD_PARANT(ParantClass)\
collector.AddDecorate("Parant:\n");\
this->ObjPoolCollectThreadLocal_##ParantClass(collector);\
collector.AddDecorate("; \n");\

#undef __OBJ_POOL_CREATE_THREAD_LOCAL_END
#define __OBJ_POOL_CREATE_THREAD_LOCAL_END()\
return true;\
}


// 请祖先类与派生类选择性使用,
// 使用各自类的ObjPoolCollect_ 自检与对象池信息收集，可以实现在编译器检查出基类是不是对象池创建的
// 若祖先类是对象池创建的那么派生出来的子类必须是对象池创建的,
// 否则会出现当前对象非法访问本类其他实例内存区，造成不可挽回的内存非法访问，


/* 祖先类是模版类的 */
// ancestor

#include<kernel/comp/memory/Defs/TemplateMacro.h>

// 请祖先类与派生类选择性使用,
// 使用各自类的ObjPoolCollect_ 自检与对象池信息收集，可以实现在编译器检查出基类是不是对象池创建的
// 若祖先类是对象池创建的那么派生出来的子类必须是对象池创建的,
// 否则会出现当前对象非法访问本类其他实例内存区，造成不可挽回的内存非法访问，
// TempClassName是不带模版参数的模版类名字 ObjImplType是模版对各个参数进行实例化后的类型
#undef __OBJ_POOL_CREATE_TEMPLATE_ANCESTOR
#define __OBJ_POOL_CREATE_TEMPLATE_ANCESTOR(createBufferNumWhenInit, initBlockNumPerBuffer, TempClassName, ...)                                                                                   \
__OBJ_POOL_CREATE_TEMPLATE(createBufferNumWhenInit, initBlockNumPerBuffer, _##TempClassName##objAlloctor, TempClassName, ##__VA_ARGS__);                                                          \
ALWAYS_INLINE bool ObjPoolCollect_##TempClassName(KERNEL_NS::AlloctorInfoCollector &collector)                                                  \
{                                                                                                                                               \
    collector.Collect(GetAlloctor__##TempClassName##objAlloctor());                                                                             \
    return true;                                                                                                                                \
}                                                                                                                                               \
ALWAYS_INLINE bool ObjPoolCollectThreadLocal_##TempClassName(KERNEL_NS::AlloctorInfoCollector &collector)                                       \
{                                                                                                                                               \
    collector.Collect(GetThreadLocalAlloctor__##TempClassName##objAlloctor());                                                                  \
    return true;                                                                                                                                \
}

// 请祖先类与派生类选择性使用,
// 使用各自类的ObjPoolCollect_ 自检与对象池信息收集，可以实现在编译器检查出基类是不是对象池创建的
// 若祖先类是对象池创建的那么派生出来的子类必须是对象池创建的,
// 否则会出现当前对象非法访问本类其他实例内存区，造成不可挽回的内存非法访问，
// TempClassName是不带模版参数的模版类名字 ObjImplType是模版对各个参数进行实例化后的类型
// 外部必须自行带上template<typename T1, ...>



/* 派生类父类是模版类的对象池创建 */
// derive
// 请祖先类与派生类选择性使用,
// 使用各自类的ObjPoolCollect_ 自检与对象池信息收集，可以实现在编译器检查出基类是不是对象池创建的
// 若祖先类是对象池创建的那么派生出来的子类必须是对象池创建的,
// 否则会出现当前对象非法访问本类其他实例内存区，造成不可挽回的内存非法访问，
// ParentTempClassName是父类不戴模版参数的类名 ParentImplType是父类对象实例化后的对象
// TempClassName是子类不带参数的类型名,ObjImplType是带参数的实例化对象 
// 故意写不完整的函数是为了能够续上下一半
// void TempClassName##_objpooldetect() { ParentImplType::ParentTempClassName##_objpooldetect(); }
#undef __OBJ_POOL_CREATE_TEMPLATE_OBJ
#define __OBJ_POOL_CREATE_TEMPLATE_OBJ(createBufferNumWhenInit, initBlockNumPerBuffer, TempClassName, ...)           \
__OBJ_POOL_CREATE_TEMPLATE(createBufferNumWhenInit, initBlockNumPerBuffer, _##TempClassName##objAlloctor, TempClassName, ##__VA_ARGS__)      \
public:\
    ALWAYS_INLINE bool ObjPoolDetectLeftPart_##ObjType(\

// 通用部分
#undef __OBJ_POOL_CREATE_GEN_TEMPLATE_BEGIN
#define __OBJ_POOL_CREATE_GEN_TEMPLATE_BEGIN(ObjType)\
){return true;}\
 public:\
    ALWAYS_INLINE bool ObjPoolCollect_##ObjType(KERNEL_NS::AlloctorInfoCollector &collector)\
    {\
        collector.Collect(GetAlloctor__##ObjType##objAlloctor());\

// 添加通用
#undef __OBJ_POOL_CREATE_GEN_TEMPLATE_ADD_PARANT
#define __OBJ_POOL_CREATE_GEN_TEMPLATE_ADD_PARANT(ParantClass)\
collector.AddDecorate("Parant:\n");\
this->ObjPoolCollect_##ParantClass(collector);\
collector.AddDecorate("; \n");\

#undef __OBJ_POOL_CREATE_GEN_TEMPLATE_END
#define __OBJ_POOL_CREATE_GEN_TEMPLATE_END()\
return true;\

// THREAD LOCAL 部分
#undef __OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_BEGIN
#define __OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_BEGIN(ObjType)\
}\
 public:\
    ALWAYS_INLINE bool ObjPoolCollectThreadLocal_##ObjType(KERNEL_NS::AlloctorInfoCollector &collector)\
    {\
        collector.Collect(GetThreadLocalAlloctor__##ObjType##objAlloctor());\

// 添加通用
#undef __OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_ADD_PARANT
#define __OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_ADD_PARANT(ParantClass)\
collector.AddDecorate("Parant:\n");\
this->ObjPoolCollectThreadLocal_##ParantClass(collector);\
collector.AddDecorate("; \n");\

#undef __OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_END
#define __OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_END()\
return true;\
}

// 请祖先类与派生类选择性使用,
// 使用各自类的ObjPoolCollect_ 自检与对象池信息收集，可以实现在编译器检查出基类是不是对象池创建的
// 若祖先类是对象池创建的那么派生出来的子类必须是对象池创建的,
// 否则会出现当前对象非法访问本类其他实例内存区，造成不可挽回的内存非法访问，



#endif
