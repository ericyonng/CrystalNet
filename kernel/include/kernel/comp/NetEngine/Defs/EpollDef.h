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
 * Date: 2021-03-23 10:54:44
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_EPOLL_DEF_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_EPOLL_DEF_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS

#include <sys/epoll.h>

#undef __LIB_MAX_EVENT_NUM_TO_HANDLE_ONCE_DEF__
#define __LIB_MAX_EVENT_NUM_TO_HANDLE_ONCE_DEF__     128                         // 每次默认处理的最大事件上限

 // epoll能监听的事件最大上限 linux 2.6.8之后的版本这个参数没有作用,现在linux最大上限只跟内存以及文件描述符上限有关系,是动态的
#undef  __LIB_MAX_EPOLL_EVENT_NUM_LIMIT__
#define __LIB_MAX_EPOLL_EVENT_NUM_LIMIT__            10000
#undef  __ADD_LIB_EPOLL_EVENT_FLAGS_DEF__
#define __ADD_LIB_EPOLL_EVENT_FLAGS_DEF__            (EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR | EPOLLOUT)   // 关注epoll可读,断开,错误以及使用et模式
#undef  __ADD_LIB_EPOLL_ACCEPT_EVENT_FLAGS_DEF__
#define __ADD_LIB_EPOLL_ACCEPT_EVENT_FLAGS_DEF__     (EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR)
#undef  __DEL_LIB_EPOLL_EVENT_FLAGS_DEF__
#define __DEL_LIB_EPOLL_EVENT_FLAGS_DEF__            (EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR | EPOLLOUT)    // 从所有关注的事件中移除fd
#undef __ADD_LIB_EPOLL_EVENT_CONNECT_ONESHOT_FLAGS_DEF__
#define __ADD_LIB_EPOLL_EVENT_CONNECT_ONESHOT_FLAGS_DEF__    (EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR | EPOLLOUT | EPOLLONESHOT)        // 连接事件,采用LT模式
#undef __ADD_LIB_EPOLL_WAKE_UP_EVENT_FLAGS_DEF__
#define __ADD_LIB_EPOLL_WAKE_UP_EVENT_FLAGS_DEF__   (EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR)


KERNEL_BEGIN

class EpollWaitTimeOutType
{
public:
    enum
    {
        BlockNoTimeOut = -1,                // 没有超时的阻塞等待
        WakeupImmediately = 0,              // 立即被唤醒
    };
};

class EpollDefs
{
public:
    enum
    {
        InvalidFd = -1,                     // 无效文件描述符
        EpollError = -1,                    // epoll错误
    };
};

KERNEL_END

#endif

#endif
