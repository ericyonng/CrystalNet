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
 * Date: 2021-09-14 00:36:48
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_BriefSockAddr_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_BriefSockAddr_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/NetEngine/Defs/sockaddr_in.h>
#include <kernel/comp/Utils/MemUtil.h>

KERNEL_BEGIN

struct KERNEL_EXPORT BriefSockAddr
{
    POOL_CREATE_OBJ_DEFAULT(BriefSockAddr);

    BriefSockAddr(bool isIpv4)
        :_isUpdate(false)
        ,_addr(isIpv4)
        ,_port(0)
    {
        MemUtil::Zeroset(_addr._data);
        _ipNetByteOrder[0] = 0;
        _ipNetByteOrder[1] = 0;
    }

    LibString ToString() const
    {
        LibString info;
        info.AppendFormat("is update[%s], address[%s:%hu], _ipAndPort = [%s],network byte order ip[%llu,%llu]"
            , _isUpdate?"true":"false", _ip.c_str(), _port, _ipAndPort.c_str(), _ipNetByteOrder[1], _ipNetByteOrder[0]);
        
        return info;
    }

    void Reset()
    {
        _isUpdate = false;
        MemUtil::Zeroset(_addr._data);
        _ip.clear();
        _ipAndPort.clear();
        _port = 0;
        _ipNetByteOrder[0] = 0;
        _ipNetByteOrder[1] = 0;
    }

    bool _isUpdate;
    KernelSockAddrIn _addr;     // TODO:需要兼容ipv6 当配置时没有填充
    LibString _ip;              // ip字符串 当配置时需要填充
    LibString _ipAndPort;       // ip:port 当配置时需要填充
    UInt16 _port;               // 本地字节序
    UInt64 _ipNetByteOrder[2];  // 网络字节序 ipv4只需要_ipNetByteOrder[0], ipv6 需要16字节
};

KERNEL_END

#endif
