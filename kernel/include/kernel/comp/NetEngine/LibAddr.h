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
 * Date: 2021-09-13 00:22:01
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_LIB_ADDR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_LIB_ADDR_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/NetEngine/BriefSockAddr.h>
#include <kernel/comp/NetEngine/Defs/NetDefs.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class LibSocket;

class KERNEL_EXPORT LibAddr
{
    POOL_CREATE_OBJ_DEFAULT(LibAddr);
public:
    LibAddr(LibSocket *sock);
    virtual ~LibAddr();

public:
    // 更新与重置
    Int32 UpdateRemoteAddr();
    Int32 UpdateLocalAddr();
    void ResetRemoteAddr();
    void ResetLocalAddr();
    void BindAddrInfo(const BriefSockAddr &localAddr, const BriefSockAddr &remoteAddr);

    // 本地ip数据
    const BriefSockAddr &GetLocalBriefAddr() const;
    BriefSockAddr &GetLocalBriefAddr();
    const LibString &GetLocalIpStr() const;
    // 输出ip:port字符串
    const LibString &GetLocalAddrPortString() const;
    UInt16 GetLocalPort() const;

    // 远程ip数据
    const BriefSockAddr &GetRemoteBriefAddr() const;
    BriefSockAddr &GetRemoteBriefAddr();
    const LibString &GetRemoteIpStr() const;
    const LibString &GetRemoteAddrPortString() const;
    UInt16 GetRemotePort() const;

    LibString ToString() const;
    UInt16 GetFamily() const;

    void SetPrepareLocalIp(const Byte8 *ip);
    const LibString &GetPrepareLocalIp() const;
    void SetPrepareLocalPort(UInt16 port);
    UInt16 GetPrepareLocalPort() const;
    void SetPrepareLocalIpAndPort(const Byte8 *ip, UInt16 port);
    const LibString &GetPrepareLocalIpAndPort() const;

private:
    LibSocket *_sock;
    LibString _prepareLocalIp;    // 还没更新时设置的本地ip 执行update时会切到真实本地ip
    UInt16 _prepareLocalPort;     // 还没更新时设置的本地端口 执行update时会切到真实本地端口
    LibString _prepareLocalIpAndPort;   // 还没更新时ip:port

    BriefSockAddr _localAddr;   // 本地ip数据
    BriefSockAddr _remoteAddr;  // 远程ip数据
};

ALWAYS_INLINE void LibAddr::ResetRemoteAddr()
{
    _remoteAddr.Reset();
}

ALWAYS_INLINE void LibAddr::ResetLocalAddr()
{
    _localAddr.Reset();
}

ALWAYS_INLINE void LibAddr::BindAddrInfo(const BriefSockAddr &localAddr, const BriefSockAddr &remoteAddr)
{
    _localAddr = localAddr;
    _remoteAddr = remoteAddr;
    _prepareLocalIp = _localAddr._ip;
    _prepareLocalPort = _localAddr._port;
    _prepareLocalIpAndPort = _localAddr._ipAndPort;
}

ALWAYS_INLINE const BriefSockAddr &LibAddr::GetLocalBriefAddr() const
{
    return _localAddr;
}

ALWAYS_INLINE BriefSockAddr &LibAddr::GetLocalBriefAddr()
{
    return _localAddr;
}

ALWAYS_INLINE const LibString &LibAddr::GetLocalIpStr() const
{
    if(LIKELY(_localAddr._isUpdate))
        return _localAddr._ip;

    return _prepareLocalIp;
}

ALWAYS_INLINE const LibString &LibAddr::GetLocalAddrPortString() const
{
    if(LIKELY(_localAddr._isUpdate))
        return _localAddr._ipAndPort;

    return _prepareLocalIpAndPort;
}

ALWAYS_INLINE UInt16 LibAddr::GetLocalPort() const
{
    if(LIKELY(_localAddr._isUpdate))
        return _localAddr._port;

    return _prepareLocalPort;
}

ALWAYS_INLINE const BriefSockAddr &LibAddr::GetRemoteBriefAddr() const
{
    return _remoteAddr;
}

ALWAYS_INLINE BriefSockAddr &LibAddr::GetRemoteBriefAddr()
{
    return _remoteAddr;
}

ALWAYS_INLINE const LibString &LibAddr::GetRemoteIpStr() const
{
    return _remoteAddr._ip;
}

ALWAYS_INLINE const LibString &LibAddr::GetRemoteAddrPortString() const
{
    return _remoteAddr._ipAndPort;
}

ALWAYS_INLINE UInt16 LibAddr::GetRemotePort() const
{
    return _remoteAddr._port;   
}

ALWAYS_INLINE void LibAddr::SetPrepareLocalIp(const Byte8 *ip)
{
    _prepareLocalIp = ip;
}

ALWAYS_INLINE const LibString &LibAddr::GetPrepareLocalIp() const
{
    return _prepareLocalIp;
}

ALWAYS_INLINE void LibAddr::SetPrepareLocalPort(UInt16 port)
{
    _prepareLocalPort = port;
}

ALWAYS_INLINE UInt16 LibAddr::GetPrepareLocalPort() const
{
    return _prepareLocalPort;
}

ALWAYS_INLINE void LibAddr::SetPrepareLocalIpAndPort(const Byte8 *ip, UInt16 port)
{
    _prepareLocalIpAndPort.clear();
    _prepareLocalIpAndPort.AppendFormat("%s:%hu", ip, port);
}

ALWAYS_INLINE const LibString &LibAddr::GetPrepareLocalIpAndPort() const
{
    return _prepareLocalIpAndPort;
}

KERNEL_END

#endif
