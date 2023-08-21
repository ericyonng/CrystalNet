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
#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_PROTOCOL_IPROTOCOL_STACK_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_PROTOCOL_IPROTOCOL_STACK_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibList.h>
#include <kernel/comp/LibStream.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/LibTime.h>
#include <kernel/comp/Encrypt/Encrypt.h>
#include <kernel/comp/Coder/coder.h>
#include <kernel/comp/Utils/CypherGeneratorUtil.h>

KERNEL_BEGIN

class LibPacket;
class LibSession;

class KERNEL_EXPORT IProtocolStack 
{
public:
    IProtocolStack() {}
    virtual ~IProtocolStack() {}
    virtual void Release() = 0;
    
    // @param(buffer) : 原始数据缓冲区
    // @param(bufferSize) : 要处理的数据大小
    // @param(handledBytes) : 处理完成的字节数
    // @param(incompletedStream) : 未完成的stream
    // @param(parsedPacket) : 解析出来的完整包
    // @param(incompleteStreamOut) : 残余数据,不完整包,需要后到的数据组装成完整包,若有残余的包会从对象池中创建包,输出如果未完成的输入后变成已完成包,且后面没有产生未完成包会被置为NULL
    virtual Int32 ParsingPacket(KERNEL_NS::LibSession *session
                                , KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream
                                , UInt64 &handledBytes
                                , UInt64 &packetCount
                                , LibList<LibList<LibPacket *> *, _Build::TL> *&recvPacketsBatch) = 0;

    // 包转成二进制流
    // 这里会加上包头
    // 加解密标识
    // 对称密钥
    // 包校验码
    virtual Int32 PacketsToBin(LibSession *session, LibPacket *packet, LibStream<_Build::TL> *stream, UInt64 &handledBytes) = 0;

    virtual ICoderFactory *GetCoderFactory(Int32 opcode) = 0;
    virtual void RegisterCoderFactory(Int32 opcode, ICoderFactory *factory) = 0;

    void SetProtoVersionNumber(UInt64 ver);
    UInt64 GetProtoVersionNumber() const;

    Int32 GetType() const;
    virtual void SetOpenPorotoLog(bool enable){}

    LibRsa &GetParsingRsa();
    const LibRsa &GetParsingRsa() const;

    LibRsa &GetPacketToBinRsa();
    const LibRsa &GetPacketToBinRsa() const;

    void SetKeyExpireTimeIntervalMs(Int64 interval);
    bool UpdateKey();
    const KERNEL_NS::LibString &GetKey() const;
    const KERNEL_NS::LibString &GetCypherKey() const;
    const KERNEL_NS::LibString &GetBase64Key() const;

protected:
    UInt64 _protoVersionNumber = 0;     // 协议版本号
    Int32 _type = 0;                    // 协议栈类型
    LibRsa _parsingRsa;
    LibRsa _packetToBinRsa;

    KERNEL_NS::LibString _key;          // 生成的key
    KERNEL_NS::LibString _cypherKey;    // 加密后的key
    KERNEL_NS::LibString _base64Key;    // base64的key
    Int64 _expireTime = 0;                  // key过期时间
    Int64 _expireIntervalMs = 3000;         // key过期时间间隔
};

inline void IProtocolStack::SetProtoVersionNumber(UInt64 ver)
{
    _protoVersionNumber = ver;
}

inline UInt64 IProtocolStack::GetProtoVersionNumber() const
{
    return _protoVersionNumber;
}

inline Int32 IProtocolStack::GetType() const
{
    return _type;
}

ALWAYS_INLINE LibRsa &IProtocolStack::GetParsingRsa()
{
    return _parsingRsa;
}

ALWAYS_INLINE const LibRsa &IProtocolStack::GetParsingRsa() const
{
    return _parsingRsa;
}

ALWAYS_INLINE LibRsa &IProtocolStack::GetPacketToBinRsa()
{
    return _packetToBinRsa;
}

ALWAYS_INLINE const LibRsa &IProtocolStack::GetPacketToBinRsa() const
{
    return _packetToBinRsa;
}

ALWAYS_INLINE void IProtocolStack::SetKeyExpireTimeIntervalMs(Int64 interval)
{
    _expireIntervalMs = (interval > 0) ? interval : _expireIntervalMs;
}

ALWAYS_INLINE bool IProtocolStack::UpdateKey()
{
    const auto nowTime = LibTime::NowMilliTimestamp();
    if(LIKELY(_expireTime > nowTime))
        return true;

    _expireTime = nowTime + _expireIntervalMs;

    _key.clear();
    _base64Key.clear();
    _cypherKey.clear();
    KERNEL_NS::CypherGeneratorUtil::SpeedGen<KERNEL_NS::_Build::TL>(_key, KERNEL_NS::CypherGeneratorUtil::CYPHER_128BIT);
    
    if(_packetToBinRsa.IsPubEncryptPrivDecrypt())
    {
        _packetToBinRsa.PubKeyEncrypt(_key, _cypherKey);
    }
    else
    {
        _packetToBinRsa.PrivateKeyEncrypt(_key, _cypherKey);
    }

    if(UNLIKELY(_cypherKey.empty()))
    {
        _expireTime = nowTime;
        _key.clear();
        _base64Key.clear();
        return false;
    }

    KERNEL_NS::LibBase64::Encode(_cypherKey.data(), static_cast<UInt64>(_cypherKey.size()), _base64Key);

    return true;
}

ALWAYS_INLINE const KERNEL_NS::LibString &IProtocolStack::GetKey() const
{
    return _key;
}

ALWAYS_INLINE const KERNEL_NS::LibString &IProtocolStack::GetCypherKey() const
{
    return _cypherKey;
}

ALWAYS_INLINE const KERNEL_NS::LibString &IProtocolStack::GetBase64Key() const
{
    return _base64Key;
}

KERNEL_END


#endif
