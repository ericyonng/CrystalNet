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
 * Date: 2020-10-08 14:44:15
 * Author: Eric Yonng
 * Description: 
*/
#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_KERNEL_EXPORT_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_KERNEL_EXPORT_H__

#pragma once

// 设置文档格式utf8
// #pragma execution_character_set("utf-8")

#include <kernel/common/compile.h>

#undef KERNEL_EXTERN_DEFINE
#ifndef KERNEL_EXPORT
    #ifndef CRYSTAL_NET_STATIC_KERNEL_LIB
        #ifdef CRYSTAL_NET_KERNEL_LIB
            #if CRYSTAL_TARGET_PLATFORM_WINDOWS
                #define KERNEL_EXPORT _declspec(dllexport)
                #define KERNEL_EXTERN_DEFINE 
            #else
                #define KERNEL_EXPORT __attribute__((__visibility__("default")))  // default 是默认导出符号（linux下）
                #define KERNEL_EXTERN_DEFINE extern
            #endif
        #else
            #if CRYSTAL_TARGET_PLATFORM_WINDOWS
                #define KERNEL_EXPORT _declspec(dllimport)
                #define KERNEL_EXTERN_DEFINE extern

            #else
                #define KERNEL_EXPORT  __attribute__((__visibility__("default")))  // default 是默认导出符号（linux下）
                #define KERNEL_EXTERN_DEFINE extern
            #endif
        #endif
    #endif
    
#endif

// 不需要kernel库
#ifdef CRYSTAL_NET_NO_KERNEL_LIB
    #undef KERNEL_EXPORT
    #define KERNEL_EXPORT 
#endif

// #pragma warning(disable:4251) // 模版类造成的警告
// #if CRYSTAL_TARGET_PLATFORM_WINDOWS
//     #pragma warning(disable:4819) // 屏蔽文件中存在汉字时的编码警告
// #endif

#define D_SCL_SECURE_NO_WARNINGS // disable warning C4996

#endif
