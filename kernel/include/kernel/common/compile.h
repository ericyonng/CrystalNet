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
 * Author: Eric Yonng
 * Date: 2021-03-25 19:58:03
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_COMPILE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_COMPILE_H__

#pragma once

// ios/mac的宏定义 文件TargetConditionals.h
#ifdef __APPLE__
    #include "TargetConditionals.h"
#endif

// 各个平台宏定义
#define CRYSTAL_PLATFORM_UNKNOWN        0
#define CRYSTAL_PLATFORM_LINUX          1
#define CRYSTAL_PLATFORM_WINDOWS        2
#define CRYSTAL_PLATFORM_IPHONE         3
#define CRYSTAL_PLATFORM_MAC            4
#define CRYSTAL_PLATFORM_ANDROID        5

#define CRYSTAL_TARGET_PLATFORM         CRYSTAL_PLATFORM_UNKNOWN 

// 当前windows平台
#if !CRYSTAL_TARGET_PLATFORM && defined(_WIN32)
    #undef CRYSTAL_TARGET_PLATFORM
    #define CRYSTAL_TARGET_PLATFORM     CRYSTAL_PLATFORM_WINDOWS
#endif

// 当前linux平台
#if !CRYSTAL_TARGET_PLATFORM && defined(__linux__) && !defined(__ANDROID__)
    #undef CRYSTAL_TARGET_PLATFORM
    #define CRYSTAL_TARGET_PLATFORM     CRYSTAL_PLATFORM_LINUX
#endif

// 当前iPhone平台 iPhone(included Simulator) platform recognize.
#if !CRYSTAL_TARGET_PLATFORM && TARGET_OS_IPHONE
    #undef CRYSTAL_TARGET_PLATFORM
    #define CRYSTAL_TARGET_PLATFORM     CRYSTAL_PLATFORM_IPHONE
#endif

// Mac platform recognize.
#if !CRYSTAL_TARGET_PLATFORM && TARGET_OS_MAC
    #undef CRYSTAL_TARGET_PLATFORM
    #define CRYSTAL_TARGET_PLATFORM     CRYSTAL_PLATFORM_MAC
#endif

// 安卓平台
#if !CRYSTAL_TARGET_PLATFORM && defined(__ANDROID__)
    #undef CRYSTAL_TARGET_PLATFORM
    #define CRYSTAL_TARGET_PLATFORM CRYSTAL_PLATFORM_ANDROID
#endif

// 平台识别
#if !CRYSTAL_TARGET_PLATFORM 
    #error "Cannot recognize the target platform; are you targeting an unsuported platform?"
#endif

// 平台是否windows
#if CRYSTAL_TARGET_PLATFORM == CRYSTAL_PLATFORM_WINDOWS
    #undef CRYSTAL_TARGET_PLATFORM_WINDOWS
    #define CRYSTAL_TARGET_PLATFORM_WINDOWS     1
    #undef CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    #define CRYSTAL_TARGET_PLATFORM_NON_WINDOWS 0
#else
    #undef CRYSTAL_TARGET_PLATFORM_WINDOWS
    #define CRYSTAL_TARGET_PLATFORM_WINDOWS     0
    #undef CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    #define CRYSTAL_TARGET_PLATFORM_NON_WINDOWS 1
#endif

// 平台是否linux
#if CRYSTAL_TARGET_PLATFORM == CRYSTAL_PLATFORM_LINUX
    #undef CRYSTAL_TARGET_PLATFORM_LINUX
    #define CRYSTAL_TARGET_PLATFORM_LINUX   1
    #undef CRYSTAL_TARGET_PLATFORM_NON_LINUX
    #define CRYSTAL_TARGET_PLATFORM_NON_LINUX   0
#else
    #undef CRYSTAL_TARGET_PLATFORM_LINUX
    #define CRYSTAL_TARGET_PLATFORM_LINUX   0
    #undef CRYSTAL_TARGET_PLATFORM_NON_LINUX
    #define CRYSTAL_TARGET_PLATFORM_NON_LINUX   1
#endif

// 平台是否iPhone 要优先判断 因为TARGET_OS_IPHONE 是 TARGET_OS_MAC 的子集
#if CRYSTAL_TARGET_PLATFORM == CRYSTAL_PLATFORM_IPHONE
    #undef CRYSTAL_TARGET_PLATFORM_IPHONE
    #define CRYSTAL_TARGET_PLATFORM_IPHONE  1
    #undef CRYSTAL_TARGET_PLATFORM_NON_IPHONE
    #define CRYSTAL_TARGET_PLATFORM_NON_IPHONE 0
#else
    #undef CRYSTAL_TARGET_PLATFORM_IPHONE
    #define CRYSTAL_TARGET_PLATFORM_IPHONE  0
    #undef CRYSTAL_TARGET_PLATFORM_NON_IPHONE
    #define CRYSTAL_TARGET_PLATFORM_NON_IPHONE 1
#endif

// 平台是否Mac
#if CRYSTAL_TARGET_PLATFORM == CRYSTAL_PLATFORM_MAC
    #undef CRYSTAL_TARGET_PLATFORM_MAC
    #define CRYSTAL_TARGET_PLATFORM_MAC 1
    #undef CRYSTAL_TARGET_PLATFORM_NON_MAC
    #define CRYSTAL_TARGET_PLATFORM_NON_MAC 0
#else
    #undef CRYSTAL_TARGET_PLATFORM_MAC
    #define CRYSTAL_TARGET_PLATFORM_MAC 0
    #undef CRYSTAL_TARGET_PLATFORM_NON_MAC
    #define CRYSTAL_TARGET_PLATFORM_NON_MAC 1
#endif

// 平台是否ANDROID
#if CRYSTAL_TARGET_PLATFORM == CRYSTAL_PLATFORM_ANDROID
    #undef CRYSTAL_TARGET_PLATFORM_ANDROID
    #define CRYSTAL_TARGET_PLATFORM_ANDROID 1
    #undef CRYSTAL_TARGET_PLATFORM_NON_ANDROID
    #define CRYSTAL_TARGET_PLATFORM_NON_ANDROID 0
#else
    #undef CRYSTAL_TARGET_PLATFORM_ANDROID
    #define CRYSTAL_TARGET_PLATFORM_ANDROID 0
    #undef CRYSTAL_TARGET_PLATFORM_NON_ANDROID
    #define CRYSTAL_TARGET_PLATFORM_NON_ANDROID 1
#endif

// 处理器平台支持

// ==================================================================================
// Processer type enumeration.
//
#define CRYSTAL_PROCESSOR_UNKNOWN                              0
#define CRYSTAL_PROCESSOR_X86                                  1
#define CRYSTAL_PROCESSOR_X86_64                               2
#define CRYSTAL_PROCESSOR_ARM                                  3
#define CRYSTAL_PROCESSOR_ARM_THUMB                            4
#define CRYSTAL_PROCESSOR_ARM_64                               5

// ==================================================================================
// Recognize processors
// About processor predefined macros, see:
// All mainstream compilers: https://sourceforge.net/p/predef/wiki/Architectures/
// MSVC only:                https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=vs-2019
//
#if CRYSTAL_TARGET_PLATFORM_WINDOWS

 #if defined(_M_IX86)
    #define CRYSTAL_TARGET_PROCESSOR CRYSTAL_PROCESSOR_X86
 #elif defined(_M_X64) || defined(_M_AMD64)
    #define CRYSTAL_TARGET_PROCESSOR CRYSTAL_PROCESSOR_X86_64
 #elif defined(_M_ARM)
    #define CRYSTAL_TARGET_PROCESSOR CRYSTAL_PROCESSOR_ARM
 #elif defined(_M_ARMT)
    #define CRYSTAL_TARGET_PROCESSOR CRYSTAL_PROCESSOR_ARM_THUMB
 #elif defined(_M_ARM64)
    #define CRYSTAL_TARGET_PROCESSOR CRYSTAL_PROCESSOR_ARM_64
 #else // Unknown
    #define CRYSTAL_TARGET_PROCESSOR CRYSTAL_PROCESSOR_UNKNOWN
 #endif // defined(_M_IX86)

#else // CRYSTAL_TARGET_PLATFORM_NON_WINDOWS

 #if defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__)
  #define CRYSTAL_TARGET_PROCESSOR CRYSTAL_PROCESSOR_X86
 #elif defined(__x86_64__) || defined(__amd64__)
  #define CRYSTAL_TARGET_PROCESSOR CRYSTAL_PROCESSOR_X86_64
 #elif defined(__arm__)
  #define CRYSTAL_TARGET_PROCESSOR CRYSTAL_PROCESSOR_ARM
 #elif defined(__arm__) && defined(__thumb__)
  #define CRYSTAL_TARGET_PROCESSOR CRYSTAL_PROCESSOR_ARM_THUMB
 #elif defined(__aarch64__)
  #define CRYSTAL_TARGET_PROCESSOR CRYSTAL_PROCESSOR_ARM_64
 #else // Unknown
  #define CRYSTAL_TARGET_PROCESSOR CRYSTAL_PROCESSOR_UNKNOWN
 #endif // defined(__ix86__)

#endif // CRYSTAL_TARGET_PLATFORM_WIN32

#if CRYSTAL_TARGET_PROCESSOR == CRYSTAL_PROCESSOR_X86
    #define CRYSTAL_16BIT_PROCESSOR                               0
    #define CRYSTAL_32BIT_PROCESSOR                               1
    #define CRYSTAL_64BIT_PROCESSOR                               0
    #define CRYSTAL_TARGET_PROCESSOR_X86                          1
    #define CRYSTAL_TARGET_PROCESSOR_X86_64                       0
    #define CRYSTAL_TARGET_PROCESSOR_ARM                          0
    #define CRYSTAL_TARGET_PROCESSOR_ARM_THUMB                    0
    #define CRYSTAL_TARGET_PROCESSOR_ARM_64                       0
    #define CRYSTAL_TARGET_PROCESSOR_DESC                         "x86"
#elif CRYSTAL_TARGET_PROCESSOR == CRYSTAL_PROCESSOR_X86_64
    #define CRYSTAL_16BIT_PROCESSOR                               0
    #define CRYSTAL_32BIT_PROCESSOR                               0
    #define CRYSTAL_64BIT_PROCESSOR                               1
    #define CRYSTAL_TARGET_PROCESSOR_X86                          0
    #define CRYSTAL_TARGET_PROCESSOR_X86_64                       1
    #define CRYSTAL_TARGET_PROCESSOR_ARM                          0
    #define CRYSTAL_TARGET_PROCESSOR_ARM_THUMB                    0
    #define CRYSTAL_TARGET_PROCESSOR_ARM_64                       0
    #define CRYSTAL_TARGET_PROCESSOR_DESC                         "x86_64"
#elif CRYSTAL_TARGET_PROCESSOR == CRYSTAL_PROCESSOR_ARM
    #define CRYSTAL_16BIT_PROCESSOR                               0
    #define CRYSTAL_32BIT_PROCESSOR                               1
    #define CRYSTAL_64BIT_PROCESSOR                               0
    #define CRYSTAL_TARGET_PROCESSOR_X86                          0
    #define CRYSTAL_TARGET_PROCESSOR_X86_64                       0
    #define CRYSTAL_TARGET_PROCESSOR_ARM                          1
    #define CRYSTAL_TARGET_PROCESSOR_ARM_THUMB                    0
    #define CRYSTAL_TARGET_PROCESSOR_ARM_64                       0
    #define CRYSTAL_TARGET_PROCESSOR_DESC                         "arm"
#elif CRYSTAL_TARGET_PROCESSOR == CRYSTAL_PROCESSOR_ARM_THUMB
    #define CRYSTAL_16BIT_PROCESSOR                               1
    #define CRYSTAL_32BIT_PROCESSOR                               0
    #define CRYSTAL_64BIT_PROCESSOR                               0
    #define CRYSTAL_TARGET_PROCESSOR_X86                          0
    #define CRYSTAL_TARGET_PROCESSOR_X86_64                       0
    #define CRYSTAL_TARGET_PROCESSOR_ARM                          0
    #define CRYSTAL_TARGET_PROCESSOR_ARM_THUMB                    1
    #define CRYSTAL_TARGET_PROCESSOR_ARM_64                       0
    #define CRYSTAL_TARGET_PROCESSOR_DESC                         "thumb"
#elif CRYSTAL_TARGET_PROCESSOR == CRYSTAL_PROCESSOR_ARM_64
    #define CRYSTAL_16BIT_PROCESSOR                               0
    #define CRYSTAL_32BIT_PROCESSOR                               0
    #define CRYSTAL_64BIT_PROCESSOR                               1
    #define CRYSTAL_TARGET_PROCESSOR_X86                          0
    #define CRYSTAL_TARGET_PROCESSOR_X86_64                       0
    #define CRYSTAL_TARGET_PROCESSOR_ARM                          0
    #define CRYSTAL_TARGET_PROCESSOR_ARM_THUMB                    0
    #define CRYSTAL_TARGET_PROCESSOR_ARM_64                       1
    #define CRYSTAL_TARGET_PROCESSOR_DESC                         "arm64"
#else // Unknown processor
    #error "Cannot recognize the target processor; are you targeting an unsuported processor?"
#endif // CRYSTAL_TARGET_PROCESSOR == CRYSTAL_PROCESSOR_X86


// Compiler types macro define.
#define CRYSTAL_COMP_MSVC  0  // Microsoft visual c++ compiler(cl)
#define CRYSTAL_COMP_GCC   1  // GNU c compiler(gcc/g++)
#define CRYSTAL_COMP_CLANG 2  // CLang compiler(clang)(for now, could not determine)
#define CRYSTAL_COMP_OTHER 3  // Other compiles

// Determine current compiler.
// Microsoft visual c++ compiler.
// Version list:
//  MSVC++5.0   -> 1100
//  MSVC++6.0   -> 1200
//  MSVC++7.0   -> 1300
//  MSVC++7.1   -> 1310(Visual Studio 2003)
//  MSVC++8.0   -> 1400(Visual Studio 2005)
//  MSVC++9.0   -> 1500(Visual Studio 2008)
//  MSVC++10.0  -> 1600(Visual Studio 2010)
//  MSVC++11.0  -> 1700(Visual Studio 2012)
//  MSVC++12.0  -> 1800(Visual Studio 2013)
//  MSVC++14.0  -> 1900(Visual Studio 2015)
//  MSVC++15.0  -> 1910(Visual Studio 2017)
//  MSVC++16.0  -> 1920(Visual Studio 2019)
//  MSVC++17.0  -> 1930(Visual Studio 2022)
//  ... ...
#if CRYSTAL_TARGET_PLATFORM_WINDOWS && defined(_MSC_VER)
 #define CRYSTAL_CUR_COMP          CRYSTAL_COMP_MSVC
 #define CRYSTAL_CUR_COMP_DESC     "MSC"
 #define CRYSTAL_COMP_VER          _MSC_VER
 #define CRYSTAL_COMP_MAJOR_VER    _MSC_VER
 #define CRYSTAL_COMP_MINOR_VER    0
 #define CRYSTAL_COMP_PATCH_LEVEL  0
#endif // CRYSTAL_TARGET_PLATFORM_WINDOWS && defined(_MSC_VER)

// GNU C compiler.
// The compiler version is converted, if gcc version is 3.2.0
// the version value is 30200
#ifdef __GNUC__
 #define CRYSTAL_CUR_COMP          CRYSTAL_COMP_GCC
 #define CRYSTAL_CUR_COMP_DESC     "gcc"
 #define CRYSTAL_COMP_VER          (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
 #define CRYSTAL_COMP_MAJOR_VER    __GNUC__
 #define CRYSTAL_COMP_MINOR_VER    __GNUC_MINOR__
 #define CRYSTAL_COMP_PATCH_LEVEL  __GNUC_PATCHLEVEL__
#endif

// CLang compiler.
// The compile version is converted, like GUN C compiler
#if defined(__clang__) && !defined(CRYSTAL_CUR_COMP)
 #define CRYSTAL_CUR_COMP          CRYSTAL_COMP_CLANG
 #define CRYSTAL_CUR_COMP_DESC     "clang"
 #define CRYSTAL_COMP_VER          (__clang_major__ *10000 + __clang_minar__ *100 + __clang_patchlevel__)
 #define CRYSTAL_COMP_MAJOR_VER    __clang_major__
 #define CRYSTAL_COMP_MINOR_VER    __clang_minar__
 #define CRYSTAL_COMP_PATCH_LEVEL  __clang_patchlevel__
#endif

// Other compiles.
#ifndef CRYSTAL_CUR_COMP
 #warning "Cannot recognize the target compiler! please makesure you're using MSVC compiler or GNU c compiler or CLANG compiler to compiling llbc library!"
 #define CRYSTAL_CUR_COMP          CRYSTAL_COMP_OTHER
 #define CRYSTAL_CUR_COMP_DESC     "other"
 #define CRYSTAL_COMP_VER          0
 #define CRYSTAL_COMP_MAJOR_VER    0
 #define CRYSTAL_COMP_MINOR_VER    0
 #define CRYSTAL_COMP_PATCH_LEVEL  0
 #endif

// 临时选项
#include <kernel/common/tempcompile.h>

#endif
