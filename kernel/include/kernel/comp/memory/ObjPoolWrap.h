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
 * Date: 2022-05-06 00:02:31
 * Author: Eric Yonng
 * Description: 提供给不能使用对象池宏进行创建的数据结构比如stl中的各种数据结构等等
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_OBJ_POOL_WRAP_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_OBJ_POOL_WRAP_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/Defs/MemoryAlloctorConfig.h>
#include <kernel/comp/memory/ObjAlloctor.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Utils/TlsUtil.h>

KERNEL_BEGIN

template<typename ObjType>
class ObjPoolWrap
{
public:
    template<typename... Args>
    static ObjType *NewByAdapter(_Build::MT::Type, Args &&... args)
    {
        return GetAlloctor().New(std::forward<Args>(args)...);
    }

    template<typename... Args>
    static ObjType *NewByAdapter(_Build::TL::Type, Args &&... args)
    {
        return GetThreadLocalAlloctor().NewThreadLocal(std::forward<Args>(args)...);
    }

    static void DeleteByAdapter(_Build::MT::Type, ObjType *ptr)
    {
        GetAlloctor().Delete(ptr);
    }
    static void DeleteByAdapter(_Build::TL::Type, ObjType *ptr)
    {
        GetThreadLocalAlloctor().DeleteThreadLocal(ptr);
    }

    // // 给没有构造与析构的用,或者不需要相关需求的对象用
    static ObjType *NewByAdapterNoConstructor(_Build::MT::Type)
    {
        return GetAlloctor().NewNoConstruct();
    }

    static ObjType *NewByAdapterNoConstructor(_Build::TL::Type)
    {
        return GetThreadLocalAlloctor().NewNoConstructThreadLocal();
    }

    static void DeleteByAdapterNoDestructor(_Build::MT::Type, ObjType *ptr)
    {
        GetAlloctor().DeleteNoDestructor(ptr);
    }
    static void DeleteByAdapterNoDestructor(_Build::TL::Type, ObjType *ptr)
    {
        GetThreadLocalAlloctor().DeleteNoDestructorThreadLocal(ptr);
    }
    

private:
    static ObjAlloctor<ObjType> &GetAlloctor()
    {                                                                                                                          
        static SmartPtr<ObjAlloctor<ObjType>, _Build::MT> staticAlloctor =                   
        new ObjAlloctor<ObjType>(MEMORY_BUFFER_BLOCK_INIT                                                               
        , MemoryAlloctorConfig(sizeof(ObjType), false));                                              
                                                                                                                                
        return *staticAlloctor;                                                                                                 
    }                                                                                                                           
    static ObjAlloctor<ObjType> &GetThreadLocalAlloctor()                               
    {                                                                                                                           
        DEF_STATIC_THREAD_LOCAL_DECLEAR ObjAlloctor<ObjType> *staticAlloctor =                                         
        TlsUtil::GetTlsStack()->New< TlsObjectPool<ObjAlloctor<ObjType>> >()->GetPool(MEMORY_BUFFER_BLOCK_INIT
        , MemoryAlloctorConfig(sizeof(ObjType), false));                                              
                                                                                                                                
        return *staticAlloctor;                                                                                                   
    }  
};

// 创建对象
#undef OBJ_POOL_WRAP_NEW
#define OBJ_POOL_WRAP_NEW(ObjType, BuildType, ...)                                          \
KERNEL_NS::ObjPoolWrap<ObjType>::NewByAdapter(BuildType::V, ##__VA_ARGS__)

// 移除对象
#undef OBJ_POOL_WRAP_DELETE
#define OBJ_POOL_WRAP_DELETE(ObjType, BuildType, ptr)                                       \
KERNEL_NS::ObjPoolWrap<ObjType>::DeleteByAdapter(BuildType::V, ptr)

// // 对于模版的分开拆解调用
// 一个模版参数
#undef OBJ_POOL_WRAP_TEMPLATE_NEW_P1
#define OBJ_POOL_WRAP_TEMPLATE_NEW_P1(cls, P1, BuildType, ...)                              \
KERNEL_NS::ObjPoolWrap<cls<P1>>::NewByAdapter(BuildType::V, ##__VA_ARGS__)

#undef OBJ_POOL_WRAP_TEMPLATE_DELETE_P1
#define OBJ_POOL_WRAP_TEMPLATE_DELETE_P1(cls, P1, BuildType, ptr)                           \
KERNEL_NS::ObjPoolWrap<cls<P1>>::DeleteByAdapter(BuildType::V, ptr)

// 两个模版参数
#undef OBJ_POOL_WRAP_TEMPLATE_NEW_P2
#define OBJ_POOL_WRAP_TEMPLATE_NEW_P2(cls, P1, P2, BuildType, ...)                          \
KERNEL_NS::ObjPoolWrap<cls<P1, P2>>::NewByAdapter(BuildType::V, ##__VA_ARGS__)

#undef OBJ_POOL_WRAP_TEMPLATE_DELETE_P2
#define OBJ_POOL_WRAP_TEMPLATE_DELETE_P2(cls, P1, P2, BuildType, ptr)                       \
KERNEL_NS::ObjPoolWrap<cls<P1, P2>>::DeleteByAdapter(BuildType::V, ptr)

// 三个模版参数
#undef OBJ_POOL_WRAP_TEMPLATE_NEW_P3
#define OBJ_POOL_WRAP_TEMPLATE_NEW_P3(cls, P1, P2, P3, BuildType, ...)                      \
KERNEL_NS::ObjPoolWrap<cls<P1, P2, P3>>::NewByAdapter(BuildType::V, ##__VA_ARGS__)

#undef OBJ_POOL_WRAP_TEMPLATE_DELETE_P3
#define OBJ_POOL_WRAP_TEMPLATE_DELETE_P3(cls, P1, P2, P3, BuildType, ptr)                   \
KERNEL_NS::ObjPoolWrap<cls<P1, P2, P3>>::DeleteByAdapter(BuildType::V, ptr)

// 四个模版参数
#undef OBJ_POOL_WRAP_TEMPLATE_NEW_P4
#define OBJ_POOL_WRAP_TEMPLATE_NEW_P4(cls, P1, P2, P3, P4, BuildType, ...)                  \
KERNEL_NS::ObjPoolWrap<cls<P1, P2, P3, P4>>::NewByAdapter(BuildType::V, ##__VA_ARGS__)

#undef OBJ_POOL_WRAP_TEMPLATE_DELETE_P4
#define OBJ_POOL_WRAP_TEMPLATE_DELETE_P4(cls, P1, P2, P3, P4, BuildType, ptr)               \
KERNEL_NS::ObjPoolWrap<cls<P1, P2, P3, P4>>::DeleteByAdapter(BuildType::V, ptr)

// 五个模版参数
#undef OBJ_POOL_WRAP_TEMPLATE_NEW_P5
#define OBJ_POOL_WRAP_TEMPLATE_NEW_P5(cls, P1, P2, P3, P4, P5, BuildType, ...)              \
KERNEL_NS::ObjPoolWrap<cls<P1, P2, P3, P4, P5>>::NewByAdapter(BuildType::V, ##__VA_ARGS__)

#undef OBJ_POOL_WRAP_TEMPLATE_DELETE_P5
#define OBJ_POOL_WRAP_TEMPLATE_DELETE_P5(cls, P1, P2, P3, P4, P5, BuildType, ptr)           \
KERNEL_NS::ObjPoolWrap<cls<P1, P2, P3, P4, P5>>::DeleteByAdapter(BuildType::V, ptr)

// 六个模版参数
#undef OBJ_POOL_WRAP_TEMPLATE_NEW_P6
#define OBJ_POOL_WRAP_TEMPLATE_NEW_P6(cls, P1, P2, P3, P4, P5, P6, BuildType, ...)          \
KERNEL_NS::ObjPoolWrap<cls<P1, P2, P3, P4, P5, P6>>::NewByAdapter(BuildType::V, ##__VA_ARGS__)

#undef OBJ_POOL_WRAP_TEMPLATE_DELETE_P6
#define OBJ_POOL_WRAP_TEMPLATE_DELETE_P6(cls, P1, P2, P3, P4, P5, P6, BuildType, ptr)       \
KERNEL_NS::ObjPoolWrap<cls<P1, P2, P3, P4, P5, P6>>::DeleteByAdapter(BuildType::V, ptr)



KERNEL_END

#endif
