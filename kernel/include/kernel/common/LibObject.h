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
 * Date: 2021-08-22 21:19:07
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_LIB_OBJECT_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_LIB_OBJECT_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/common/type.h>

KERNEL_BEGIN

struct KERNEL_EXPORT _Build
{
    // 多线程版本
    struct KERNEL_EXPORT MT
    {
        enum Type
        {
            V = 0,
        };
    };

    // thread local 版本
    struct KERNEL_EXPORT TL
    {
        enum Type
        {
            V = 1,
        };
    };

    // 未知
    struct KERNEL_EXPORT UNKNOWN
    {
    };
};

// // 构建对象方法类型
// class KERNEL_EXPORT _Build
// {
// public:
//     // 多线程版本 multi thread
//     enum MT
//     {
//         V = 0,
//     };
//     // 线程本地版本thread local
//     enum TL
//     {
//         V = 0,
//     };
// };

// // 对象构建适配器
// namespace ObjBuild
// {
//     template<BuildType::ThreadType threadType, BuildType::ClassType classType>
//     struct Adapter{};
//     typedef KERNEL_EXPORT Adapter<BuildType::MULTI_THREAD, BuildType::NORMAL> NormalMultiAdapter;             // 普通类多线程版本
//     typedef KERNEL_EXPORT Adapter<BuildType::THREAD_LOCAL, BuildType::NORMAL> NormalThreadLocalAdapter;   // 普通类线程局部版本
//     typedef KERNEL_EXPORT Adapter<BuildType::MULTI_THREAD, BuildType::TEMPLATE> TemplateMultiAdapter;    // 泛型类多线程版本
//     typedef KERNEL_EXPORT Adapter<BuildType::THREAD_LOCAL, BuildType::TEMPLATE> TemplateThreadLocalAdapter;  // 泛型类线程局部版本

//     extern KERNEL_EXPORT const NormalMultiAdapter *_normalMultiAdapter;
//     extern KERNEL_EXPORT const NormalThreadLocalAdapter *_normalThreadLocalAdapter;
//     extern KERNEL_EXPORT const TemplateMultiAdapter *_templateMultiAdapter;
//     extern KERNEL_EXPORT const TemplateThreadLocalAdapter *_templateThreadLocalAdapter;
// };

KERNEL_END

using KernelBuildMT = KERNEL_NS::_Build::MT;
using KernelBuildTL = KERNEL_NS::_Build::TL;

#endif
