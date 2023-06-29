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
 * Date: 2020-10-06 18:55:19
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_LIBS_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_LIBS_H__

#pragma once

#include <kernel/common/compile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atomic>
#include <limits>   
#include <numeric>  // 随机数等
#include <thread>
#include <algorithm>
#include <stdexcept>
#include <deque>
#include <set>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>        // 与map的不同在于内建的是hash表而不是红黑树
#include <unordered_set>
#include <mutex>
#include <assert.h> 
#include <iostream>
#include <string>
#include <stdarg.h>     // c风格格式化vs_arg等接口
#include <chrono>       // 时间
#include <forward_list> // std::forward等
#include <fstream>
#include <time.h>
#include <stddef.h>
#include <regex>
#include <random>
#include <stack>
#include <queue>
#include <tuple>
#include <type_traits>  // is_class, is_pod等
#include <bitset>
#include <array>
#include <iosfwd>
#include <sstream>  // stringstream
#include <float.h>
#include <functional>
#include <memory>
#include <setjmp.h>
// #include <yvals_core.h> // 使用_NODISCARD


// 平台有关库
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    #include <crtdbg.h>
    #include <WinSock2.h>
    #include <ws2def.h>
    #include <ws2tcpip.h>
    #include <fcntl.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #pragma comment(lib,"ws2_32.lib")
    #include<MSWSock.h>
    #pragma comment (lib, "MSWSock.lib")

    #include "shlwapi.h"        // 控制台程序接口
    #pragma comment(lib,"shlwapi.lib")

    // 线程等
    #include <process.h>
    #include <tchar.h>

    // 系统信息
    #include "Psapi.h"
    #include "tlhelp32.h"
    #include "sysinfoapi.h"
    #include <iphlpapi.h>   // 网卡
    #pragma comment(lib,"Iphlpapi.lib")
    
    #include<direct.h>      // mkdir func

    #include <ws2ipdef.h>   // ipv6等
    #include <profileapi.h> // cpucounter
    #include <intrin.h> // 获取cpuid
    #include <signal.h> // 信号处理
    #include <io.h>          // access func 遍历目录

#else
    #include <linux/version.h>  // 内核版本宏等 LINUX_VERSION_CODE/KERNEL_VERSION等
    #include <unistd.h>
    #include <limits.h> // 含有PATH_MAX
    #include <errno.h>
    
    // linux socket环境 以及相关网络接口
    #include <sys/types.h>
    #include <sys/socket.h> // 含有 getaddrinfo等
    #include <netdb.h>
    #include <signal.h>
    #include<sys/param.h>

    // 资源
    #include <sys/resource.h>
    #include <netinet/tcp.h>
    #include <netinet/in.h>

    #include <semaphore.h>  // 信号量
    #include <pthread.h>    // 线程库
    #include <sys/time.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <sys/eventfd.h>    // 多线程多进程事件通知机制

    // ip相关
    #include <arpa/inet.h>

    // 生成uuid
    #include <uuid/uuid.h> // 真正在uuid/uuid.h
    // #include <linux/uuid.h> // 真正在uuid/uuid.h

    // syscall 系统调用获取硬件时钟,单调递增,不随用户调整而变化,不受ntp影响 系统调用会有上下文切换开销
    #include <sys/syscall.h>
    // 包含sysinfo结构体信息
    #include <linux/kernel.h>
    #include <sys/sysinfo.h>

    // linux下随机数相关
    #include <tr1/random>

    // linux下类型识别接口相关
    #include <cxxabi.h>
    // linux下堆栈追踪头文件
    #include <execinfo.h>

    // event
    #include <sys/epoll.h>

    // 支持获取网卡信息
    #include	<net/if.h>

    #include <sys/sysctl.h> // sysctl函数
    #include <net/route.h>  // 路由表 ifa_msghdr等
    // #include <net/if_dl.h> // 数据结构 sockaddr_dl 需要安装unp才可以，废弃

    #include <cpuid.h>  // 获取cpuid
    #include <x86intrin.h>  // rdtsc/rdtscp支持
    #include <sys/socket.h> // 支持socklen_t等
    #include <sys/ioctl.h> // ioctl

    // 遍历目录
    #include <dirent.h>

#endif

#endif
