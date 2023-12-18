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
 * Date: 2023-07-15 17:00:46
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_PROTOCOL_CRYSTAL_PROTOCOL_CRYSTAL_PROTOCOL_JSON_STACK_H__
#define __CRYSTAL_NET_SERVICE_COMMON_PROTOCOL_CRYSTAL_PROTOCOL_CRYSTAL_PROTOCOL_JSON_STACK_H__

#pragma once

#include <kernel/comp/NetEngine/Protocol/IProtocolStack.h>
#include <kernel/comp/NetEngine/Protocol/ICoder.h>
#include <kernel/comp/LibStream.h>
#include <kernel/comp/LibList.h>
#include <service_common/common/common.h>
#include <service_common/protocol/CrystalProtocol/CrystalProtocolStackType.h>

KERNEL_BEGIN

class LibSession;
class LibPacket;

KERNEL_END

SERVICE_COMMON_BEGIN

class CrystalProtocolJsonStack : public KERNEL_NS::IProtocolStack
{
public:
    CrystalProtocolJsonStack(Int32 type) { _type = type;}
    ~CrystalProtocolJsonStack();

    virtual void Release() override { delete this; }

    // 解析包
    virtual Int32 ParsingPacket(KERNEL_NS::LibSession *session
                                , KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream
                                , UInt64 &handledBytes
                                , UInt64 &packetCount
                                , KERNEL_NS::LibList<KERNEL_NS::LibList<KERNEL_NS::LibPacket *> *, KERNEL_NS::_Build::TL> *&recvPacketsBatch) override;

    // 包转流
    virtual Int32 PacketsToBin(KERNEL_NS::LibSession *session
                                , KERNEL_NS::LibPacket *packet
                                , KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *stream
                                , UInt64 &handledBytes) override;

    virtual KERNEL_NS::ICoderFactory *GetCoderFactory(Int32 opcode) override;
    virtual void RegisterCoderFactory(Int32 opcode, KERNEL_NS::ICoderFactory *factory) override;

    void SetMaxRecvMsgContentBytes(UInt64 maxBytesLimit);
    UInt64 GetMaxRecvMsgContentBytes() const;

    void SetOpenPorotoLog(bool enable = true) override { _enableProtocolLog = enable; }

private:
    UInt64 _maxRecvContenBytes = 0;
    bool _enableProtocolLog = false;
};

ALWAYS_INLINE void CrystalProtocolJsonStack::SetMaxRecvMsgContentBytes(UInt64 recvMsgContentBytesLimit)
{
    _maxRecvContenBytes = recvMsgContentBytesLimit;
}

ALWAYS_INLINE UInt64 CrystalProtocolJsonStack::GetMaxRecvMsgContentBytes() const
{
    return _maxRecvContenBytes;
}

SERVICE_COMMON_END

#endif
