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
 * Date: 2020-12-28 01:59:39
 * Author: Eric Yonng
 * Description: 
 *  // ai_flags的值的范围为0~7，取决于程序如何设置3个标志位，
    // 比如设置ai_flags为“AI_PASSIVE | AI_CANONNAME”，ai_flags值就为3。
    // 三个参数的含义分别为：
    // (1)AI_PASSIVE 当此标志置位时，表示调用者将在bind()函数调用中使用返回的地址结构,
    // 当此标志不置位时，表示将在connect()函数调用中使用。
    // 当节点名位NULL，且此标志置位，则返回的地址将是通配地址。
    // 如果节点名NULL，且此标志不置位，则返回的地址将是回环地址。
    // (2)AI_CANNONAME 当此标志置位时，在函数所返回的第一个addrinfo结构中的ai_cannoname成员中，
    // 应该包含一个以空字符结尾的字符串，字符串的内容是节点名的正规名。
    // (3)AI_NUMERICHOST 当此标志置位时，此标志表示调用中的节点名必须是一个数字地址字符串。
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_DEFS_IPUTIL_DEF_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_DEFS_IPUTIL_DEF_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/common/BaseType.h>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    #include <ws2ipdef.h>   // ipv6等
    #include <Ws2def.h>
    #include <ws2tcpip.h>
#endif

#if CRYSTAL_TARGET_PLATFORM_LINUX
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <unistd.h>
#endif

KERNEL_BEGIN

class KERNEL_EXPORT IPUtilDef
{
public: 
    enum FAMILY_TYPE
    {
        FAMILY_TYPE_UNSPEC = 0,     //  protocal nonrelationship
        #if CRYSTAL_TARGET_PLATFORM_WINDOWS
            FAMILY_TYPE_AF_INET = PF_INET,    //  IPV4 PF_INET
            FAMILY_TYPE_AF_INET6 = PF_INET6,	//  IPV6 PF_INET6
        #else
            FAMILY_TYPE_AF_INET = AF_INET,    //  IPV4 AF_INET
            FAMILY_TYPE_AF_INET6 = AF_INET6,	//  IPV6 AF_INET6
        #endif
    };

    enum PROTOCOL_TYPE
    {
        PROTOCOL_TYPE_IPPROTO_IP = 0,       //  ip protocal
     
     #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        PROTOCOL_TYPE_IPPROTO_IPV4 = IPPROTO_IPV4,     //  ipv4
        PROTOCOL_TYPE_IPPROTO_TCP = IPPROTO_TCP,      //  tcp
        PROTOCOL_TYPE_IPPROTO_UDP = IPPROTO_UDP,     //  udp
        PROTOCOL_TYPE_IPPROTO_IPV6 = IPPROTO_IPV6,    //  IPV6    
    #else
        PROTOCOL_TYPE_IPPROTO_IPV4 = 4,     //  ipv4
        PROTOCOL_TYPE_IPPROTO_TCP = 6,      //  tcp
        PROTOCOL_TYPE_IPPROTO_UDP = 17,     //  udp
        PROTOCOL_TYPE_IPPROTO_IPV6 = 41,    //  IPV6  
    #endif    
    };

    //  传输的套接字类型
    enum SOCK_TYPE
    {
        SOCK_TYPE_SOCK_STREAM = SOCK_STREAM,          // stream
        SOCK_TYPE_SOCK_DGRAM = SOCK_DGRAM,           // Datagram
    };

    // ai_flags
    enum AI_FLAGS_TYPE
    {
        AI_FLAGS_TYPE_NONE = 0,             // begin
        AI_FLAGS_TYPE_AI_PASSIVE = AI_PASSIVE,       // passive，use to bind，usually in server socket case
        AI_FLAGS_TYPE_AI_CANONNAME = AI_CANONNAME,     // use to get computer standard name
        AI_FLAGS_TYPE_AI_NUMERICHOST = AI_NUMERICHOST,   // addr is numeric string
        AI_FLAGS_TYPE_END = 8,              // ai_flags value range[0,8)
    };
};

KERNEL_END

#endif
