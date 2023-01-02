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
 * Date: 2023-01-02 17:09:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Utils/IPUtil.h>
#include <kernel/comp/Utils/SocketUtil.h>

KERNEL_BEGIN

Int32 IPUtil::GetIPByDomain(
    const char *domain                          // 域名或主机名
    , const char *service                       // 端口号“80”等、服务名 如"ftp", "http"等
    , LibString &ipAddrString                   // 输出ip
    , Int32 netCardNo                           // 网卡序号若获取的是本地的地址，为选择网址列表的某一个网址
    , Int32 eFlags                              // IPUtilDef::AI_FLAGS_TYPE 各个位的组合
                                                // 默认AI_PASSIVE 即用于bind绑定 不设置则用于connect

    , IPUtilDef::SOCK_TYPE eSockType            // IPUtilDef::SOCK_TYPE_SOCK_STREAM /* 默认流数据
    , IPUtilDef::FAMILY_TYPE eFamily            // IPUtilDef::FAMILY_TYPE_AF_INET /* 默认ipv4 */
    , IPUtilDef::PROTOCOL_TYPE eProtocol        // IPUtilDef::PROTOCOL_TYPE_IPPROTO_IP 默认任意协议 即ip协议 
    )
{
    // 不可同时为NULL
    if(UNLIKELY(!domain && !service))
        return Status::IPUtil_ParamError;

    
    // 获取地址的参数设置
    struct addrinfo hints = {0};
    hints.ai_family = eFamily;
    hints.ai_flags = eFlags;
    hints.ai_protocol = eProtocol;
    hints.ai_socktype = eSockType;
    struct addrinfo *netCardRes = NULL;
    if(getaddrinfo(domain, service, &hints, &netCardRes) != 0)
    {
        // throw std::logic_error("getaddrinfo failed");
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        perror("getaddrinfo fail");
#endif
        return Status::IPUtil_GetAddrInfoFailed;
    }

    // 遍历网卡
    BUFFER256 ipcache = {};
    Int32 cnt = 0;
    struct addrinfo *cur = NULL;
    UInt16 port = 0;
    Byte8 *ipPtr = ipcache;
    for(cur = netCardRes; cur != NULL; cur = cur->ai_next)
    {
        if(!cur->ai_addr)
            continue;

        if(eFamily == IPUtilDef::FAMILY_TYPE_AF_INET)
        {
            struct sockaddr_in *addr = (struct sockaddr_in *)cur->ai_addr;
            SocketUtil::GetAddrInfoFromNetInfo(*addr, BUFFER_LEN256, ipPtr, port);
        }   
        else
        {
            struct sockaddr_in6 *addr = (struct sockaddr_in6 *)cur->ai_addr;
            SocketUtil::GetAddrInfoFromNetInfo(*addr, BUFFER_LEN256, ipPtr, port);
        }

        if(netCardNo != cnt)
        {
            ++cnt;
            continue;
        }

        ipAddrString = ipcache;
        freeaddrinfo(netCardRes);
        return Status::Success;
    }

    // 释放资源
    freeaddrinfo(netCardRes);
    return Status::IPUtil_NotFound;
}
KERNEL_END
