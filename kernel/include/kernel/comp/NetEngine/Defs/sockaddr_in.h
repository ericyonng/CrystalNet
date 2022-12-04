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
 * Date: 2022-01-06 14:04:59
 * Author: Eric Yonng
 * Description: 
 *         linux
            struct sockaddr_in6 {
               sa_family_t     sin6_family;   // AF_INET6 
               in_port_t       sin6_port;     // port number 
               uint32_t        sin6_flowinfo; // IPv6 flow information 
               struct in6_addr sin6_addr;     // IPv6 address 
               uint32_t        sin6_scope_id; // Scope ID (new in 2.4) 
           };
           struct in6_addr {
               unsigned char   s6_addr[16];   // IPv6 address 
           };
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_NET_SOCKADDR_IN_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_NET_SOCKADDR_IN_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>

KERNEL_BEGIN

union KERNEL_EXPORT KernelSockAddrInRaw
{
    sockaddr_in _sinV4;
    sockaddr_in6 _sinV6;
};

class KERNEL_EXPORT KernelSockAddrIn
{
    POOL_CREATE_OBJ_DEFAULT(KernelSockAddrIn);

public:
    KernelSockAddrIn(bool isIpv4):_isIpv4(isIpv4){}
    ~KernelSockAddrIn(){}

    KernelSockAddrInRaw _data;
    bool _isIpv4;
};

KERNEL_END

#endif
