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
 * Date: 2021-03-25 15:12:43
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_LIB_PACKET_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_LIB_PACKET_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/LibStream.h>
#include <kernel/comp/NetEngine/BriefSockAddr.h>

KERNEL_BEGIN

class ICoder;

class KERNEL_EXPORT LibPacket
{
    POOL_CREATE_OBJ_DEFAULT(LibPacket);
public:
    LibPacket();
    virtual ~LibPacket();
    virtual void ReleaseUsingPool();

    // 包所属的会话
    UInt64 GetSessionId() const;
    void SetSessionId(UInt64 sessionId);

    // 地址
    const BriefSockAddr &GetLocalAddr() const;
    void SetLocalAddr(const BriefSockAddr &local);
    const BriefSockAddr &GetRemoteAddr() const;
    void SetRemoteAddr(const BriefSockAddr &remote);

    Int64 GetPacketId() const;
    void SetPacketId(Int64 packetId);

    Int32 GetOpcode() const;
    void SetOpcode(Int32 opcode);

    ICoder *GetCoder();
    const ICoder *GetCoder() const;
    template<typename T>
    T *GetCoder();
    template<typename T>
    const T *GetCoder() const;
    void SetCoder(ICoder *coder);
    ICoder *PopCoder();

    bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream);
    bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream);

    bool Decode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream);
    bool Decode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream);

    LibString ToString() const;

private:
    Int64 _packetId;               // seriaid + packetnum + seqid 最大一次性只能支持发送4GB数据
    UInt64 _sessionId;
    Int32 _opcode;
    BriefSockAddr _localAddr;       // 本地地址
    BriefSockAddr _remoteAddr;      // 远程地址
    ICoder *_coder;
};

ALWAYS_INLINE Int64 LibPacket::GetPacketId() const
{
    return _packetId;
} 

ALWAYS_INLINE UInt64 LibPacket::GetSessionId() const
{
    return _sessionId;
}

ALWAYS_INLINE void LibPacket::SetSessionId(UInt64 sessionId)
{
    _sessionId = sessionId;
}

ALWAYS_INLINE const BriefSockAddr &LibPacket::GetLocalAddr() const
{
    return _localAddr;
}

ALWAYS_INLINE void LibPacket::SetLocalAddr(const BriefSockAddr &local)
{
    _localAddr = local;
}

ALWAYS_INLINE const BriefSockAddr &LibPacket::GetRemoteAddr() const
{
    return _remoteAddr;
}

ALWAYS_INLINE void LibPacket::SetRemoteAddr(const BriefSockAddr &remote)
{
    _remoteAddr = remote;
}

ALWAYS_INLINE void LibPacket::SetPacketId(Int64 packetId)
{
    _packetId = packetId;
}

ALWAYS_INLINE Int32 LibPacket::GetOpcode() const
{
    return _opcode;
}

ALWAYS_INLINE void LibPacket::SetOpcode(Int32 opcode)
{
    _opcode = opcode;
}

ALWAYS_INLINE ICoder *LibPacket::GetCoder()
{
    return _coder;
}

ALWAYS_INLINE const ICoder *LibPacket::GetCoder() const
{
    return _coder;
}

template<typename T>
ALWAYS_INLINE T *LibPacket::GetCoder()
{
    return dynamic_cast<T *>(_coder);
}

template<typename T>
ALWAYS_INLINE const T *LibPacket::GetCoder() const
{
    return dynamic_cast<const T *>(_coder);
}

ALWAYS_INLINE ICoder *LibPacket::PopCoder()
{
    auto coder = _coder;
    _coder = NULL;
    return coder; 
}

ALWAYS_INLINE LibString LibPacket::ToString() const
{
    LibString info;
    info.AppendFormat("_packetId = [%llu], _sessionId=[%llu], local addr=[%s], remote addr=[%s], opcode:%d"
    , _packetId, _sessionId, _localAddr.ToString().c_str(), _remoteAddr.ToString().c_str(), _opcode);

    return info;
}

KERNEL_END

#endif
