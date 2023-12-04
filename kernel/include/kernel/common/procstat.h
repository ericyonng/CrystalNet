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
 * Author: Eric Yonng
 * Date: 2021-03-18 19:06:19
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_PROC_STAT_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_PROC_STAT_H__

#pragma once

#include <kernel/common/BaseMacro.h>

KERNEL_BEGIN

class ProcStat
{
public:
    enum
    {
        CPU_NAME = 0,
        USER = 1,
        NICE = 2,
        SYSTEM = 3,
        IDLE = 4,
        IOWAIT = 5,
        IRQ = 6,
        SOFTIRQ = 7,
        STEAL = 8,
        GUEST = 9,
        GUEST_NICE = 10,
    };
};

class ProcPidStat
{
public:
    enum
    {
        PID                    = 1,
        COMM                   = 2,    // 文件名
        STATE                  = 3,    // 进程状态
        PPID                   = 4,    // 父进程pid
        PGRP                   = 5,    // 父进程pid
        SESSION                = 6,    // 会话组id
        TTY_NR                 = 7,    // 控制终端
        TPGID                  = 8,    // 前台进程组id 控制过程的终端
        FLAGS                  = 9,    // 内核标记      
        MINFLT                 = 10,    
        CMINFLT                = 11,
        MAJFLT                 = 12,
        CMAJFLT                = 13,
        UTIME                  = 14,
        STIME                  = 15,
        CUTIME                 = 16,
        CSTIME                 = 17,
        PRIORITY               = 18,
        NICE                   = 19,
        NUM_THREADS            = 20,
        ITREAL_VALUE           = 21,
        STARTTIME              = 22,
        VSIZE                  = 23,
        RSS                    = 24,
        RSS_LIM                = 25,
        START_CODE             = 26,
        END_CODE               = 27,
        START_STACK            = 28,
        KST_KESP               = 29,
        KST_KEIP               = 30,
        SIGNAL                 = 31,
        BLOCKED                = 32,
        SIGIGNORE              = 33,
        SIGCATCH               = 34,
        WCHAN                  = 35,
        NSWAP                  = 36,
        CNSWAP                 = 37,
        EXIT_SIGNAL            = 38,
        PROCESSOR              = 39,
        RT_PRIORITY            = 40,
        POLICY                 = 41,
        DELAY_ACCT_BLKIO_TICKS = 42,
        GUEST_TIME             = 43,
        CGUEST_TIME            = 44,
        START_DATA             = 45,
        END_DATA               = 46,
        START_BRK              = 47,
        ARG_START              = 48,
        ARG_END                = 49,
        ENV_START              = 50,
        ENV_END                = 51,
        EXIT_CODE              = 52,

    };
};

KERNEL_END

#endif
