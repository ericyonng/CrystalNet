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
 * Date: 2020-12-30 00:57:32
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_DEFS_SYSTEM_UTIL_DEF_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_DEFS_SYSTEM_UTIL_DEF_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseType.h>
#include <kernel/common/macro.h>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
 #include "shlwapi.h"        // 控制台程序接口
#else
 #include <sys/resource.h>
#endif

KERNEL_BEGIN

class KERNEL_EXPORT LibConsoleColor
{
public:
   enum
    {
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        Fg_Black,
        Fg_Red,
        Fg_Green,
        Fg_Blue,
        Fg_Yellow,
        Fg_Purple,
        Fg_Cyan,
        Fg_White,

        Bg_Black,
        Bg_Red,
        Bg_Green,
        Bg_Blue,
        Bg_Yellow,
        Bg_Purple,
        Bg_Cyan,
        Bg_White,

        Highlight_Fg,
        Highlight_Bg,
#else // WIN32
        Fg_Black = 0,
        Fg_Red = FOREGROUND_RED,
        Fg_Green = FOREGROUND_GREEN,
        Fg_Blue = FOREGROUND_BLUE,
        Fg_Yellow = FOREGROUND_RED | FOREGROUND_GREEN,
        Fg_Purple = FOREGROUND_RED | FOREGROUND_BLUE,
        Fg_Cyan = FOREGROUND_BLUE | FOREGROUND_GREEN,
        Fg_White = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
        Fg_Gray = FOREGROUND_INTENSITY,
        Fg_LightYellow = Fg_Gray | Fg_Yellow,
        
        Bg_Black  = 0,
        Bg_Red    = BACKGROUND_RED,
        Bg_Green  = BACKGROUND_GREEN,
        Bg_Blue   = BACKGROUND_BLUE,
        Bg_Yellow = BACKGROUND_RED  | BACKGROUND_GREEN,
        Bg_Purple = BACKGROUND_RED  | BACKGROUND_BLUE,
        Bg_Cyan   = BACKGROUND_BLUE | BACKGROUND_GREEN,
        Bg_White  = BACKGROUND_RED  | BACKGROUND_GREEN | BACKGROUND_BLUE,

        Highlight_Fg = FOREGROUND_INTENSITY,
        Highlight_Bg = BACKGROUND_INTENSITY,
#endif

        Fg_Default = Fg_White,
        Bg_Default = Bg_Black
    };
};

// 进程占用的内存信息
struct KERNEL_EXPORT ProcessMemInfo
{
    size_t _maxHistorySetSize;              // 使用过的最大工作集，峰值内存占用
    size_t _curSetSize;                     // 当前工作集占用大小，当前进程占用的内存
    size_t _maxHistoryPagedPoolUsage;       // 使用过的最大分页池大小
    size_t _pagedPoolUsage;                 // 分页池大小
    size_t _maxHistoryNonPagedPoolUsage;    // 使用过的最大非分页池大小
    size_t _curNonPagedPoolUsage;           // 当前非分页池大小
    size_t _curPageFileUsage;               // 当前页交换文件使用大小
    size_t _maxHistoryPageFileUsage;        // 历史最大页交换文件使用大小
    size_t _processAllocMemoryUsage;        // 进程运行过程中申请的内存大小
};

// 见man setrlimit
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS

class KERNEL_EXPORT LinuxRlimitId
{
public:
   static const constexpr Int32 E_RLIMIT_AS = RLIMIT_AS;
   static const constexpr Int32 E_RLIMIT_CORE = RLIMIT_CORE;

    static const constexpr Int32 E_RLIMIT_CPU = RLIMIT_CPU;
    static const constexpr Int32 E_RLIMIT_DATA = RLIMIT_DATA;
    static const constexpr Int32 E_RLIMIT_FSIZE = RLIMIT_FSIZE;
    static const constexpr Int32 E_RLIMIT_LOCKS = RLIMIT_LOCKS;
    static const constexpr Int32 E_RLIMIT_MEMLOCK = RLIMIT_MEMLOCK;
    static const constexpr Int32 E_RLIMIT_NOFILE = RLIMIT_NOFILE;      // 限制文件描述符打开最大数量（当前进程）
    static const constexpr Int32 E_RLIMIT_NPROC = RLIMIT_NPROC;
    static const constexpr Int32 E_RLIMIT_RSS = RLIMIT_RSS;
    static const constexpr Int32 E_RLIMIT_RTPRIO = RLIMIT_RTPRIO;
    static const constexpr Int32 E_RLIMIT_RTTIME = RLIMIT_RTTIME;
    static const constexpr Int32 E_RLIMIT_SIGPENDING = RLIMIT_SIGPENDING;
    static const constexpr Int32 E_RLIMIT_STACK = RLIMIT_STACK;
};

#endif

// extern KERNEL_EXPORT Locker __g_consoleLock;

KERNEL_END

#endif
