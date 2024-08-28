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
 * Date: 2020-12-30 00:32:43
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_IP_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_IP_UTIL_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/common/BaseType.h>

#include <kernel/comp/LibString.h>
#include <kernel/comp/Utils/Defs/IPUtilDef.h>

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    #ifndef MAX_PATH
        #define MAX_PATH  PATH_MAX
    #endif

    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netdb.h>
    #include <errno.h>
    #include <stdio.h>
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
 #include <ws2tcpip.h>
#endif

KERNEL_BEGIN

class KERNEL_EXPORT IPUtil
{
public:
    static Int32 GetLocalIP(LibString &ip, Int32 netCardNo = 0, bool isToBind = true, bool isStreamSock = true, bool isIpv4 = true);
    static Int32 GetIPByDomain(
        const Byte8 *domain,                                                    // 域名或主机名
    const Byte8 *service,                                                       // 端口号“80”等、服务名 如"ftp", "http"等
    LibString &ipOut,                                                           // 输出ip
    const std::set<LibString> &filter,                                          // ip过滤
    Int32 netCardNo = 0,                                                        // 网卡序号若获取的是本地的地址，为选择网址列表的某一个网址
    Int32 eFlags = IPUtilDef::AI_FLAGS_TYPE_AI_PASSIVE,                         // IPUtilDef::AI_FLAGS_TYPE 各个位的组合 默认AI_PASSIVE 即用于bind绑定 不设置则用于connect
    IPUtilDef::SOCK_TYPE eSockType = IPUtilDef::SOCK_TYPE_SOCK_STREAM,	        // 默认流数据
    IPUtilDef::FAMILY_TYPE eFamily = IPUtilDef::FAMILY_TYPE_AF_INET,            // 默认ipv4
    IPUtilDef::PROTOCOL_TYPE eProtocol = IPUtilDef::PROTOCOL_TYPE_IPPROTO_IP    // 默认任意协议 即ip协议
     );

    static Int32 GetNetCardList(Int32 family, std::vector<sockaddr_in> &addrList, Int32 sockType = IPUtilDef::SOCK_TYPE_SOCK_STREAM);

    // netCardNo是用于选择域名返回的ip列表，用于dns负载均衡，可以从多个ip总选择一个
    static Int32 GetIpByHostName(const LibString &hostName, LibString &ip, Int32 netCardNo = 0, bool isToBind = true, bool isStreamSock = true, bool isIpv4 = true);
    static Int32 GetIpByHostName(const LibString &hostName, LibString &ip, const std::set<LibString> &filter, Int32 netCardNo = 0, bool isToBind = true, bool isStreamSock = true, bool isIpv4 = true);
};

ALWAYS_INLINE Int32 IPUtil::GetLocalIP(LibString &ip, Int32 netCardNo, bool isToBind, bool isStreamSock, bool isIpv4)
{
    // 获得本机主机名+
    char hostname[MAX_PATH] = {0};
    if(gethostname(hostname, MAX_PATH) != 0)
        return false;

    //获取ip
    auto ret = GetIPByDomain(hostname
                             , NULL
                             , ip
                             , {}
                             , netCardNo
                             , isToBind ? IPUtilDef::AI_FLAGS_TYPE_AI_PASSIVE : IPUtilDef::AI_FLAGS_TYPE_NONE
                             , isStreamSock ? IPUtilDef::SOCK_TYPE_SOCK_STREAM : IPUtilDef::SOCK_TYPE_SOCK_DGRAM
                             , isIpv4 ? IPUtilDef::FAMILY_TYPE_AF_INET : IPUtilDef::FAMILY_TYPE_AF_INET6
                             , IPUtilDef::PROTOCOL_TYPE_IPPROTO_IP
    );

    if(ret != Status::Success)
        return ret;

    return Status::Success;
}

ALWAYS_INLINE Int32 IPUtil::GetNetCardList(Int32 family, std::vector<sockaddr_in> &addrList, Int32 sockType)
{
    // 获取地址的参数设置
    struct addrinfo hints = {0};
    hints.ai_family = family;
    hints.ai_flags = IPUtilDef::AI_FLAGS_TYPE::AI_FLAGS_TYPE_AI_PASSIVE;
    hints.ai_protocol = IPUtilDef::PROTOCOL_TYPE_IPPROTO_IP;   // 默认ip协议不动
    hints.ai_socktype = sockType;
    struct addrinfo *netCardRes = NULL;
    if(getaddrinfo(NULL, NULL, &hints, &netCardRes) != 0)
    {
        CRYSTAL_TRACE("getaddrinfo fail! faimly:%d, sockType:%d", family, sockType);
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        perror("getaddrinfo fail");
#endif
        return Status::IPUtil_GetAddrInfoFailed;
    }

    // 遍历网卡
    struct sockaddr_in *addr = NULL;
    struct addrinfo *cur = NULL;
    for(cur = netCardRes; cur != NULL; cur = cur->ai_next)
    {
        addr = (struct sockaddr_in *)cur->ai_addr;
        if(!addr)
            continue;

        addrList.push_back(*addr);
    }

    // 释放资源
    freeaddrinfo(netCardRes);
    return Status::Success;
}

ALWAYS_INLINE Int32 IPUtil::GetIpByHostName(const LibString &hostName, LibString &ip, Int32 netCardNo, bool isToBind, bool isStreamSock, bool isIpv4)
{
    //获取ip
    auto ret = GetIPByDomain(hostName.c_str()
                             , NULL
                             , ip
                             ,{}
                             , netCardNo
                             , isToBind ? IPUtilDef::AI_FLAGS_TYPE_AI_PASSIVE : IPUtilDef::AI_FLAGS_TYPE_NONE
                             , isStreamSock ? IPUtilDef::SOCK_TYPE_SOCK_STREAM : IPUtilDef::SOCK_TYPE_SOCK_DGRAM
                             , isIpv4 ? IPUtilDef::FAMILY_TYPE_AF_INET : IPUtilDef::FAMILY_TYPE_AF_INET6
                             , IPUtilDef::PROTOCOL_TYPE_IPPROTO_IP
    );

    if(ret != Status::Success)
        return ret;

    return Status::Success;
}

ALWAYS_INLINE Int32 IPUtil::GetIpByHostName(const LibString &hostName, LibString &ip, const std::set<LibString> &filter, Int32 netCardNo, bool isToBind, bool isStreamSock, bool isIpv4)
{
    //获取ip
    auto ret = GetIPByDomain(hostName.c_str()
                             , NULL
                             , ip
                             ,filter
                             , netCardNo
                             , isToBind ? IPUtilDef::AI_FLAGS_TYPE_AI_PASSIVE : IPUtilDef::AI_FLAGS_TYPE_NONE
                             , isStreamSock ? IPUtilDef::SOCK_TYPE_SOCK_STREAM : IPUtilDef::SOCK_TYPE_SOCK_DGRAM
                             , isIpv4 ? IPUtilDef::FAMILY_TYPE_AF_INET : IPUtilDef::FAMILY_TYPE_AF_INET6
                             , IPUtilDef::PROTOCOL_TYPE_IPPROTO_IP
    );

    if(ret != Status::Success)
        return ret;

    return Status::Success;
}


KERNEL_END

#endif
