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
 * Date: 2025-01-23 00:22:15
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_XPROJECT_LOGIC_PLUGIN_LOGIC_PLUGIN_EXPORT_H__
#define __CRYSTAL_NET_XPROJECT_LOGIC_PLUGIN_LOGIC_PLUGIN_EXPORT_H__

#pragma once

// 设置文档格式utf8
// #pragma execution_character_set("utf-8")

// 使用动态库导出符号定义宏
#undef LOGIC_PLUGIN_EXTERN_DEFINE
#ifndef LOGIC_PLUGIN_EXPORT
    #ifndef LOGIC_PLUGIN_STATIC_LIB
        #ifdef _WIN32
            #define LOGIC_PLUGIN_EXPORT _declspec(dllexport)
            #define LOGIC_PLUGIN_EXTERN_DEFINE 
        #else
            #define LOGIC_PLUGIN_EXPORT __attribute__((__visibility__("default")))  // default 是默认导出符号（linux下）
            #define LOGIC_PLUGIN_EXTERN_DEFINE extern
        #endif
    #else
        // 静态库不需要导出
        #define LOGIC_PLUGIN_EXPORT
        #define LOGIC_PLUGIN_EXTERN_DEFINE extern
    #endif
#endif

// 当前模块在导入接口
#ifdef LOGIC_PLUGIN_IMPORT_LIB
    // 动态库的导入
    #ifndef LOGIC_PLUGIN_STATIC_LIB
        #undef LOGIC_PLUGIN_EXPORT
        #undef LOGIC_PLUGIN_EXTERN_DEFINE
        #ifdef _WIN32
            #define LOGIC_PLUGIN_EXPORT _declspec(dllimport)
            #define LOGIC_PLUGIN_EXTERN_DEFINE extern
        #else
            #define LOGIC_PLUGIN_EXPORT  __attribute__((__visibility__("default")))  // default 是默认导出符号（linux下）
            #define LOGIC_PLUGIN_EXTERN_DEFINE extern
        #endif
    #else
        // 静态库没有导入特性
        #ifndef LOGIC_PLUGIN_EXPORT
            #define LOGIC_PLUGIN_EXPORT 
        #endif
    #endif
#endif

// #pragma warning(disable:4251) // 模版类造成的警告
// #if CRYSTAL_TARGET_PLATFORM_WINDOWS
//     #pragma warning(disable:4819) // 屏蔽文件中存在汉字时的编码警告
// #endif

#define D_SCL_SECURE_NO_WARNINGS // disable warning C4996

#endif
