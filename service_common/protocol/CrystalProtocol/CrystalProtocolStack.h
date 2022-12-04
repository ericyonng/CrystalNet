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
 * Date: 2022-06-27 13:20:46
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_PROTOCOL_CRYSTAL_PROTOCOL_CRYSTAL_PROTOCOL_STACK_H__
#define __CRYSTAL_NET_SERVICE_COMMON_PROTOCOL_CRYSTAL_PROTOCOL_CRYSTAL_PROTOCOL_STACK_H__

#pragma once

#include <kernel/kernel.h>
#include <service_common/common/common.h>
#include <service_common/protocol/CrystalProtocol/CrystalProtocolStackType.h>

SERVICE_COMMON_BEGIN

class CrystalProtocolStack : public KERNEL_NS::IProtocolStack
{
public:
    CrystalProtocolStack(Int32 type) { _type = type;}
    ~CrystalProtocolStack();

    virtual void Release() override { delete this; }

    // 解析包
    virtual Int32 ParsingPacket(KERNEL_NS::LibSession *session
                                , KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream
                                , UInt64 &handledBytes
                                , UInt64 &packetCount
                                , KERNEL_NS::LibList<KERNEL_NS::LibList<KERNEL_NS::LibPacket *> *, KERNEL_NS::_Build::TL> *&recvPacketsBatch);

    // 包转流
    virtual Int32 PacketsToBin(KERNEL_NS::LibSession *session
                                , KERNEL_NS::LibPacket *packet
                                , KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *stream
                                , UInt64 &handledBytes);

    virtual KERNEL_NS::ICoderFactory *GetCoderFactory(Int32 opcode);
    virtual void RegisterCoderFactory(Int32 opcode, KERNEL_NS::ICoderFactory *factory);

    void SetMaxMsgContentBytes(UInt64 maxBytesLimit);
    virtual void SetOpcodeNameParser(KERNEL_NS::IDelegate<const KERNEL_NS::LibString &, Int32> *parser) override;
    UInt64 GetMaxMsgContentBytes() const;

    void SetProtocolLogEnable(bool enable = true) { _enableProtocolLog = enable; }

private:
    KERNEL_NS::IDelegate<const KERNEL_NS::LibString &, Int32> *_opcodeNameParser = NULL;
    UInt64 _maxContenBytes = 0;
    bool _enableProtocolLog = false;
};

ALWAYS_INLINE void CrystalProtocolStack::SetMaxMsgContentBytes(UInt64 maxBytesLimit)
{
    _maxContenBytes = maxBytesLimit;
}

ALWAYS_INLINE UInt64 CrystalProtocolStack::GetMaxMsgContentBytes() const
{
    return _maxContenBytes;
}

SERVICE_COMMON_END

#endif
