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
 * Date: 2021-01-11 02:32:23
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_DEFS_TEMPLATE_MACRO_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_DEFS_TEMPLATE_MACRO_H__

#pragma once

#include <kernel/common/compile.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Utils/TlsUtil.h>


/// 内存池创建对象便利宏
// 禁止直接使用 OBJ_POOL_CREATE 直接使用会大概率出现内存非法写入到其他对象内存区 ！！！！！！！！！！！！！！！！！！！！！
// WarningSeriously Dont Use objpoolcreate derectly!!!!!!!!!!
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

// ObjTypeNoTempArgs: 不带模版参数的类名
// ... 模版参数类型
#undef  __OBJ_POOL_CREATE_TEMPLATE
#define __OBJ_POOL_CREATE_TEMPLATE(createBufferWhenInit, initBlockNumPerBuffer, _objAlloctor, ObjTypeNoTempArgs, ...)   \
public:                                                                                                             \
                                                                                                                    \
        template<typename... Args>                                                                                  \
        static ALWAYS_INLINE ObjTypeNoTempArgs< __VA_ARGS__ > *New_##ObjTypeNoTempArgs(Args &&... args)             \
        {                                                                                                           \
            return GetAlloctor_##_objAlloctor().New(std::forward<Args>(args)...);                                   \
        }                                                                                                           \
                                                                                                                    \
        static ALWAYS_INLINE ObjTypeNoTempArgs< __VA_ARGS__ > *New_##ObjTypeNoTempArgs##NoConstruct()               \
        {                                                                                                           \
            return GetAlloctor_##_objAlloctor().NewNoConstruct();                                                   \
        }                                                                                                           \
                                                                                                                    \
        template<typename... Args>                                                                                  \
        static ALWAYS_INLINE ObjTypeNoTempArgs< __VA_ARGS__ > *New_##ObjTypeNoTempArgs##ByPtr(void *ptr, Args &&... args)         \
        {                                                                                                           \
            return GetAlloctor_##_objAlloctor().NewByPtr(ptr, std::forward<Args>(args)...);                         \
        }                                                                                                           \
                                                                                                                    \
        static ALWAYS_INLINE void Delete_##ObjTypeNoTempArgs(ObjTypeNoTempArgs< __VA_ARGS__ > *ptr)                 \
        {                                                                                                           \
            GetAlloctor_##_objAlloctor().Delete(ptr);                                                               \
        }                                                                                                           \
                                                                                                                    \
        static ALWAYS_INLINE void Delete_##ObjTypeNoTempArgs##NoDestructor(ObjTypeNoTempArgs< __VA_ARGS__ > *ptr)   \
        {                                                                                                           \
            GetAlloctor_##_objAlloctor().DeleteNoDestructor(ptr);                                                   \
        }                                                                                                           \
                                                                                                                    \
        ALWAYS_INLINE void AddRef_##ObjTypeNoTempArgs()                                                             \
        {                                                                                                           \
            GetAlloctor_##_objAlloctor().AddRef(this);                                                              \
        }                                                                                                           \
                                                                                                                    \
        static ALWAYS_INLINE KERNEL_NS::ObjAlloctor<ObjTypeNoTempArgs< __VA_ARGS__ >> &GetAlloctor_##_objAlloctor() \
        {                                                                                                           \
            static KERNEL_NS::SmartPtr<KERNEL_NS::ObjAlloctor<ObjTypeNoTempArgs< __VA_ARGS__ >>, KERNEL_NS::_Build::MT> staticAlloctor =     \
            new KERNEL_NS::ObjAlloctor<ObjTypeNoTempArgs< __VA_ARGS__ >>(initBlockNumPerBuffer                      \
            , KERNEL_NS::MemoryAlloctorConfig(sizeof(ObjTypeNoTempArgs< __VA_ARGS__ >), createBufferWhenInit));     \
                                                                                                                    \
            return *staticAlloctor;                                                                                   \
        }                                                                                                           \
                                                                                                                    \
        static ALWAYS_INLINE KERNEL_NS::ObjAlloctor<ObjTypeNoTempArgs< __VA_ARGS__ >> &GetThreadLocalAlloctor_##_objAlloctor()    \
        {                                                                                                           \
            DEF_STATIC_THREAD_LOCAL_DECLEAR KERNEL_NS::ObjAlloctor<ObjTypeNoTempArgs< __VA_ARGS__ >> *staticAlloctor =       \
            KERNEL_NS::TlsUtil::GetTlsStack()->New< KERNEL_NS::TlsObjectPool<KERNEL_NS::ObjAlloctor<ObjTypeNoTempArgs< __VA_ARGS__ >>> >()->GetPool(initBlockNumPerBuffer \
            , KERNEL_NS::MemoryAlloctorConfig(sizeof(ObjTypeNoTempArgs< __VA_ARGS__ >), createBufferWhenInit));     \
                                                                                                                    \
            return *staticAlloctor;                                                                                   \
        }                                                                                                           \
                                                                                                                    \
                                                                                                                    \
        template<typename... Args>                                                                                  \
        static ALWAYS_INLINE ObjTypeNoTempArgs< __VA_ARGS__ > *NewThreadLocal_##ObjTypeNoTempArgs(Args &&... args)                \
        {                                                                                                           \
            return GetThreadLocalAlloctor_##_objAlloctor().NewThreadLocal(std::forward<Args>(args)...);             \
        }                                                                                                           \
                                                                                                                    \
        static ALWAYS_INLINE ObjTypeNoTempArgs< __VA_ARGS__ > *NewThreadLocal_##ObjTypeNoTempArgs##NoConstruct()    \
        {                                                                                                           \
            return GetThreadLocalAlloctor_##_objAlloctor().NewNoConstructThreadLocal();                             \
        }                                                                                                           \
                                                                                                                    \
        template<typename... Args>                                                                                  \
        static ALWAYS_INLINE ObjTypeNoTempArgs< __VA_ARGS__ > *NewThreadLocal_##ObjTypeNoTempArgs##ByPtr(void *ptr, Args &&... args) \
        {                                                                                                           \
            return GetThreadLocalAlloctor_##_objAlloctor().NewByPtrThreadLocal(ptr, std::forward<Args>(args)...);   \
        }                                                                                                           \
                                                                                                                    \
        static ALWAYS_INLINE void DeleteThreadLocal_##ObjTypeNoTempArgs(ObjTypeNoTempArgs< __VA_ARGS__ > *ptr)      \
        {                                                                                                           \
            GetThreadLocalAlloctor_##_objAlloctor().DeleteThreadLocal(ptr);                                         \
        }                                                                                                           \
                                                                                                                    \
        static ALWAYS_INLINE void DeleteThreadLocal_##ObjTypeNoTempArgs##NoDestructor(ObjTypeNoTempArgs< __VA_ARGS__ > *ptr)      \
        {                                                                                                           \
            GetThreadLocalAlloctor_##_objAlloctor().DeleteNoDestructorThreadLocal(ptr);                             \
        }                                                                                                           \
                                                                                                                    \
        ALWAYS_INLINE void AddRefThreadLocal_##ObjTypeNoTempArgs()                                                  \
        {                                                                                                           \
            GetThreadLocalAlloctor_##_objAlloctor().AddRefThreadLocal(this);                                        \
        }                                                                                                           \
                                                                                                                    \
        template<typename... Args>                                                                                  \
        static ALWAYS_INLINE ObjTypeNoTempArgs< __VA_ARGS__ > *NewByAdapter_##ObjTypeNoTempArgs(KERNEL_NS::_Build::MT::Type, Args &&... args) \
        {                                                                                                           \
            return GetAlloctor_##_objAlloctor().New(std::forward<Args>(args)...);                                   \
        }                                                                                                           \
                                                                                                                    \
        static ALWAYS_INLINE ObjTypeNoTempArgs< __VA_ARGS__ > *NewByAdapter_##ObjTypeNoTempArgs##NoConstruct(KERNEL_NS::_Build::MT::Type)    \
        {                                                                                                           \
            return GetAlloctor_##_objAlloctor().NewNoConstruct();                                                   \
        }                                                                                                           \
                                                                                                                    \
        template<typename... Args>                                                                                  \
        static ALWAYS_INLINE ObjTypeNoTempArgs< __VA_ARGS__ > *NewByAdapter_##ObjTypeNoTempArgs##ByPtr(KERNEL_NS::_Build::MT::Type, void *ptr, Args &&... args)    \
        {                                                                                                           \
            return GetAlloctor_##_objAlloctor().NewByPtr(ptr, std::forward<Args>(args)...);                         \
        }                                                                                                           \
                                                                                                                    \
        static ALWAYS_INLINE void DeleteByAdapter_##ObjTypeNoTempArgs(KERNEL_NS::_Build::MT::Type, ObjTypeNoTempArgs< __VA_ARGS__ > *ptr)    \
        {                                                                                                           \
            GetAlloctor_##_objAlloctor().Delete(ptr);                                                               \
        }                                                                                                           \
                                                                                                                    \
        static ALWAYS_INLINE void DeleteByAdapter_##ObjTypeNoTempArgs##NoDestructor(KERNEL_NS::_Build::MT::Type, ObjTypeNoTempArgs< __VA_ARGS__ > *ptr)      \
        {                                                                                                           \
            GetAlloctor_##_objAlloctor().DeleteNoDestructor(ptr);                                                   \
        }                                                                                                           \
                                                                                                                    \
        ALWAYS_INLINE void AddRefByAdapter_##ObjTypeNoTempArgs(KERNEL_NS::_Build::MT::Type)                         \
        {                                                                                                           \
            GetAlloctor_##_objAlloctor().AddRef(this);                                                              \
        }                                                                                                           \
                                                                                                                    \
                                                                                                                    \
        template<typename... Args>                                                                                  \
        static ALWAYS_INLINE ObjTypeNoTempArgs< __VA_ARGS__ > *NewByAdapter_##ObjTypeNoTempArgs(KERNEL_NS::_Build::TL::Type, Args &&... args) \
        {                                                                                                           \
            return GetThreadLocalAlloctor_##_objAlloctor().NewThreadLocal(std::forward<Args>(args)...);             \
        }                                                                                                           \
                                                                                                                    \
        static ALWAYS_INLINE ObjTypeNoTempArgs< __VA_ARGS__ > *NewByAdapter_##ObjTypeNoTempArgs##NoConstruct(KERNEL_NS::_Build::TL::Type)    \
        {                                                                                                           \
            return GetThreadLocalAlloctor_##_objAlloctor().NewNoConstructThreadLocal();                             \
        }                                                                                                           \
                                                                                                                    \
        template<typename... Args>                                                                                  \
        static ALWAYS_INLINE ObjTypeNoTempArgs< __VA_ARGS__ > *NewByAdapter_##ObjTypeNoTempArgs##ByPtr(KERNEL_NS::_Build::TL::Type, void *ptr, Args &&... args)    \
        {                                                                                                           \
            return GetThreadLocalAlloctor_##_objAlloctor().NewByPtrThreadLocal(ptr, std::forward<Args>(args)...);   \
        }                                                                                                           \
                                                                                                                    \
        static ALWAYS_INLINE void DeleteByAdapter_##ObjTypeNoTempArgs(KERNEL_NS::_Build::TL::Type, ObjTypeNoTempArgs< __VA_ARGS__ > *ptr)    \
        {                                                                                                           \
            GetThreadLocalAlloctor_##_objAlloctor().DeleteThreadLocal(ptr);                                         \
        }                                                                                                           \
                                                                                                                    \
        static ALWAYS_INLINE void DeleteByAdapter_##ObjTypeNoTempArgs##NoDestructor(KERNEL_NS::_Build::TL::Type, ObjTypeNoTempArgs< __VA_ARGS__ > *ptr)      \
        {                                                                                                           \
            GetThreadLocalAlloctor_##_objAlloctor().DeleteNoDestructorThreadLocal(ptr);                             \
        }                                                                                                           \
                                                                                                                    \
        ALWAYS_INLINE void AddRefByAdapter_##ObjTypeNoTempArgs(KERNEL_NS::_Build::TL::Type)                         \
        {                                                                                                           \
            GetThreadLocalAlloctor_##_objAlloctor().AddRefThreadLocal(this);                                        \
        }
// 禁止直接使用 OBJ_POOL_CREATE_IMPL 直接使用会大概率出现内存非法写入到其他对象内存区 ！！！！！！！！！！！！！！



#endif
