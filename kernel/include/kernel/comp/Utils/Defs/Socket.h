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
 * Date: 2021-03-21 23:06:35
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_DEFS_SOCKET_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_DEFS_SOCKET_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>

#if CRYSTAL_TARGET_PLATFORM_LINUX
 #include <sys/socket.h>
#else
    #include <WinSock2.h>
#endif

#ifndef INVALID_SOCKET_IDD
#define INVALID_SOCKET_IDD  ~0ULL       // 无效套接字
#endif

#ifndef IS_VALID_SOCKET_IDD
#define IS_VALID_SOCKET_IDD(x)  (((x)!=INVALID_SOCKET_IDD)&&((x)!=0))   // 是否有效的socketIDD
#endif

#undef __LIB_TCP_SOCKTYPE__
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
#define __LIB_TCP_SOCKTYPE__  SOCK_STREAM
#else
#define  __LIB_TCP_SOCKTYPE__ (SOCK_STREAM | SOCK_CLOEXEC)       // SOCK_CLOEXEC是当服务器宕掉后还可以仍然使用端口避免设备掉线
#endif

KERNEL_BEGIN

class KERNEL_EXPORT SocketDefs
{
public:
    enum SOCKET_CACHE_TYPE
    {
        SOCKET_CACHE_TYPE_NONE = 0, //无效
        SOCKET_CACHE_TYPE_RECV,     // recv缓冲区
        SOCKET_CACHE_TYPE_SEND,     // send缓冲区
        SOCKET_CACHE_TYPE_END,
    };
};

struct KERNEL_EXPORT BriefAddrInfo
{
    POOL_CREATE_OBJ_DEFAULT(BriefAddrInfo);

    BriefAddrInfo();

    LibString _ip;
    UInt16 _port;
};

inline BriefAddrInfo::BriefAddrInfo()
    :_port(0)
{

}

struct KERNEL_EXPORT SocketOp
{
    bool noBlock;           // 设置非阻塞
    bool noDelay;           // 禁用nagle算法,降低延迟
    bool reuseAddr;         // close后端口重用
};


KERNEL_END

#endif