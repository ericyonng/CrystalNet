/*!
 * MIT License
 *  
 * Copyright (c) 2020 Eric Yonng<120453674@qq.com>
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *  
 * 
 * Date: 2023-07-08 21:09:00
 * Author: Eric Yonng
 * Description: 
 *              1.自定义std::alloctor, 让stl容器的内存分配得到控制
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_STL_ALLOCTOR_EX_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_STL_ALLOCTOR_EX_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/type.h>
#include <kernel/common/libs.h>
#include <kernel/common/macro.h>
#include <kernel/common/LibObject.h>
#include <kernel/common/func.h>

KERNEL_BEGIN

// template <class _Ty>
// class STLAlloctorEx;

// KERNEL_END

// template <class _Ty, class _Other>
// extern bool operator==(const KERNEL_NS::STLAlloctorEx<_Ty>&, const KERNEL_NS::STLAlloctorEx<_Other>&) noexcept;

// template <class _Ty, class _Other>
// extern bool operator!=(const KERNEL_NS::STLAlloctorEx<_Ty>&, const KERNEL_NS::STLAlloctorEx<_Other>&) noexcept;

// KERNEL_BEGIN

// template <class _Ty>
// class STLAlloctorEx 
// {
// public:
//     static_assert(!std::is_const_v<_Ty>, "The C++ Standard forbids containers of const elements "
//                                     "because allocator<const T> is ill-formed.");

//     using _From_primary = STLAlloctorEx;

//     using value_type = _Ty;

//     typedef _Ty*        pointer;
//     typedef const _Ty*  const_pointer;
//     typedef _Ty&        reference;
//     typedef const _Ty&  const_reference;

//     using size_type       = size_t;
//     using difference_type = ptrdiff_t;

//     // 支持移动
//     using propagate_on_container_move_assignment = std::true_type;

//     // 不支持交换
//     using propagate_on_container_swap = std::false_type;
//     // 不支持拷贝
//     using propagate_on_container_copy_assignment = std::false_type;

//     using is_always_equal = std::true_type;

//     template <class _Other>
//     struct rebind 
//     {
//         using other = STLAlloctorEx<_Other>;
//     };

//     _Ty* address(_Ty& _Val) const noexcept 
//     {
//         return &(_Val);
//     }

//     const _Ty* address(const _Ty& _Val) const noexcept 
//     {
//         return &(_Val);
//     }

//     constexpr STLAlloctorEx() noexcept 
//     {

//     }

//     constexpr STLAlloctorEx(const STLAlloctorEx&) noexcept = default;

//     template <class _Other>
//     constexpr STLAlloctorEx(const STLAlloctorEx<_Other>&) noexcept {}

//     ALWAYS_INLINE ~STLAlloctorEx()       = default;
//     ALWAYS_INLINE STLAlloctorEx& operator=(const STLAlloctorEx&) = default;

//     ALWAYS_INLINE void deallocate(_Ty* const ptr, const size_t count) 
//     {
//         // TODO:需要测试
//         CRYSTAL_TRACE("stl delete count:%llu, ptr:%p, obj name:%s", static_cast<UInt64>(count), ptr, typeid(_Ty).name());
//         KernelFreeMemory<_Build::MT>(ptr);
//         // no overflow check on the following multiply; we assume _Allocate did that check
//     }

//     ALWAYS_INLINE _Ty* allocate(const size_t count) 
//     {
//         // TODO:需要测试
//         CRYSTAL_TRACE("stl alloc count:%llu, obj name:%s", static_cast<UInt64>(count), typeid(_Ty).name());
//         return reinterpret_cast<_Ty *>(KernelAllocMemory<_Build::MT>(static_cast<UInt64>(count * sizeof(_Ty))));
//     }

//     ALWAYS_INLINE _Ty* allocate(const size_t count, const void*) 
//     {
//         return allocate(count);
//     }

//     template <class _Objty, class... _Types>
//     ALWAYS_INLINE void construct(_Objty* const ptr, _Types&&... _Args) 
//     {
//         ::new (reinterpret_cast<void *>(ptr)) _Objty(std::forward<_Types>(_Args)...);
//     }

//     template <class _Uty>
//     ALWAYS_INLINE void destroy(_Uty* const ptr) 
//     {
//         Destructor::Invoke(ptr);
//     }

//     ALWAYS_INLINE size_t max_size() const noexcept 
//     {
//         return static_cast<size_t>(-1) / sizeof(_Ty);
//     }

//     ALWAYS_INLINE bool operator==(const STLAlloctorEx<_Ty> &other) const
//     {
//         return true;
//     }

//     ALWAYS_INLINE bool operator!=(const STLAlloctorEx<_Ty> &other) const
//     {
//         return false;
//     }
// };

// // CLASS allocator<void>
// template <>
// class STLAlloctorEx<void> 
// {
// public:
//     using value_type = void;
//     typedef void* pointer;
//     typedef const void* const_pointer;

//     using size_type       = size_t;
//     using difference_type = ptrdiff_t;

//     using propagate_on_container_move_assignment = std::true_type;

//     using is_always_equal = std::true_type;

//     template <class _Other>
//     struct rebind {
//         using other = STLAlloctorEx<_Other>;
//     };
// };


// // template <class _Tp>
// // class STLAlloctorEx
// // {
// // public:


// //     template<class _Tp1>
// //     struct rebind
// //     {
// //         typedef STLAlloctorEx<_Tp1> other;
// //     };

// //     STLAlloctorEx() { }

// //     STLAlloctorEx(const STLAlloctorEx&) { }

// //     template<typename _Tp1>
// //     STLAlloctorEx(const STLAlloctorEx<_Tp1>&) { }

// //     ~STLAlloctorEx(){ }

// //     //hint used for Locality, ref.[auslern], p189
// //     pointer allocate(size_type n, const_pointer hint = 0)
// //     {
// //         // TODO:需要测试
// //         CRYSTAL_TRACE("stl alloc count:%llu, hint:%p, obj name:%s", static_cast<UInt64>(n), hint, typeid(_Tp).name());
// //         return reinterpret_cast<pointer>(KernelAllocMemory<_Build::TL>(static_cast<UInt64>(n * sizeof(_Tp))));
// //     }

// //     void deallocate(pointer ptr, size_type n)
// //     {
// //         // TODO:需要测试
// //         CRYSTAL_TRACE("stl delete count:%llu, ptr:%p, obj name:%s", static_cast<UInt64>(n), ptr, typeid(_Tp).name());
// //         KernelFreeMemory<_Build::TL>(ptr);
// //     }
            
// //     void construct(pointer ptr, const _Tp& val) 
// //     {
// //         ::new((void *)ptr) _Tp(val); 
// //     }

// //     void destroy(pointer ptr) 
// //     {
// //         Destructor::Invoke(ptr);
// //     }

// //     pointer address(reference x){
// //         return (pointer)&x;
// //     }

// //     const_pointer const_address(const_reference x)
// //     {
// //         return (const_pointer)&x;
// //     }

// //     //#define UINT_MAX	0xFFFFFFFF
// //     size_type max_size()const 
// //     {
// //         return size_type(static_cast<UInt64>(-1)/sizeof(value_type));
// //     }
// // };

// KERNEL_END

// template <class _Ty, class _Other>
// ALWAYS_INLINE bool operator==(const KERNEL_NS::STLAlloctorEx<_Ty>&, const KERNEL_NS::STLAlloctorEx<_Other>&) noexcept 
// {
//     return true;
// }

// template <class _Ty, class _Other>
// ALWAYS_INLINE bool operator!=(const KERNEL_NS::STLAlloctorEx<_Ty>&, const KERNEL_NS::STLAlloctorEx<_Other>&) noexcept 
// {
//     return false;
// }

#endif
