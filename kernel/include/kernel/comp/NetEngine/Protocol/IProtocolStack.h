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

protected:
    UInt64 _protoVersionNumber = 0;     // 协议版本号
    Int32 _type = 0;                    // 协议栈类型
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

KERNEL_END


#endif
