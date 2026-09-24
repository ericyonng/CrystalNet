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
 * Date: 2026-09-24 10:00:00
 * Author: CrystalNet
 * Description: http/https协议栈: 实现IProtocolStack挂接到网络引擎(IOCP/epoll),
 *              明文http直接解析, https通过openssl memory-BIO在协议栈层完成加解密,
 *              tls握手等记录通过HttpOpcode::RawData包直通发送, 不侵入网络引擎核心
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_HTTP_HTTP_PROTOCOL_STACK_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_HTTP_HTTP_PROTOCOL_STACK_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/comp/NetEngine/Protocol/IProtocolStack.h>
#include <kernel/comp/Http/HttpDefs.h>
#include <kernel/comp/Lock/Impl/SpinLock.h>
#include <kernel/comp/Delegate/IDelegate.h>

#include <unordered_map>

struct ssl_st;
typedef struct ssl_st SSL;

KERNEL_BEGIN

class HttpParser;
class HttpTlsContext;

// 每个会话的协议栈状态
struct HttpSessionState
{
    POOL_CREATE_OBJ_DEFAULT(HttpSessionState);

    HttpSessionState()
        :_parser(NULL)
        , _ssl(NULL)
        , _handshaked(false)
        , _sessionId(0)
        , _pollerId(0)
    {
    }

    HttpParser *_parser;        // http报文边界切割器
    SSL *_ssl;                  // tls会话(NULL表示明文)
    bool _handshaked;           // tls握手是否完成
    UInt64 _sessionId;          // 会话id
    UInt64 _pollerId;           // 会话所在poller
};

class KERNEL_EXPORT HttpProtocolStack : public IProtocolStack
{
public:
    // mode: HttpStackMode::Server/Client
    HttpProtocolStack(Int32 mode);
    virtual ~HttpProtocolStack();
    virtual void Release() override { delete this; }

    // 设置tls上下文(非空则启用https), 不接管所有权
    void SetTlsContext(HttpTlsContext *ctx);
    // tls等原始记录发送回调(pollerId, sessionId, packet), 必须设置
    void SetRawSendHandler(IDelegate<void, UInt64, UInt64, LibPacket *> *handler);
    // tls握手完成回调(sessionId, errCode), 客户端模式使用
    void SetTlsHandshakeHandler(IDelegate<void, UInt64, Int32> *handler);

    void SetMaxBodyBytes(UInt64 maxBodyBytes);

    // 客户端: 连接建立后发起tls握手(可在任意线程调用) 返回: Status::Success成功, 其他为错误码
    Int32 StartTlsClientHandshake(UInt64 sessionId, UInt64 pollerId);

    // 会话销毁时清理协议栈状态
    void OnSessionDestroy(UInt64 sessionId);

    bool IsTlsEnable() const;

public:
    virtual Int32 ParsingPacket(KERNEL_NS::LibSession *session
                                , KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream
                                , UInt64 &handledBytes
                                , UInt64 &packetCount
                                , LibList<LibList<LibPacket *> *, _Build::TL> *&recvPacketsBatch) override;

    virtual Int32 PacketsToBin(LibSession *session, LibPacket *packet, LibStream<_Build::TL> *stream, UInt64 &handledBytes) override;

    virtual ICoderFactory *GetCoderFactory(Int32 opcode) override { return NULL; }
    virtual void RegisterCoderFactory(Int32 opcode, ICoderFactory *factory) override;

private:
    HttpSessionState *_GetState(UInt64 sessionId);
    // 不存在则创建(按当前模式初始化parser)
    // needTls: server模式由_tlsCtx决定, client模式仅StartTlsClientHandshake时置true(https会话)
    HttpSessionState *_GetOrCreateState(UInt64 sessionId, UInt64 pollerId);
    // 为会话创建ssl(client模式延迟创建: https会话在StartTlsClientHandshake时调用)
    bool _EnsureSslCreated(HttpSessionState *st);

    // 驱动tls握手 返回: Status::Success成功(含进行中), 其他为错误码
    Int32 _DriveTlsHandshake(LibSession *session, HttpSessionState *st);
    // 排空wbio中的tls记录并通过raw包发送(需在协议栈线程上下文)
    void _FlushWbio(HttpSessionState *st);
    // 发送原始字节(tls记录直通, 不加密)
    void _SendRaw(UInt64 pollerId, UInt64 sessionId, const Byte8 *data, UInt64 len);
    // 发送http层原始字节(100 Continue/chunk块等, tls会话需加密)
    void _SendHttpLayerRaw(UInt64 pollerId, UInt64 sessionId, const Byte8 *data, UInt64 len);

    // 从parser中取出完整报文生成packet
    Int32 _PopMessages(LibSession *session
        , HttpSessionState *st
        , UInt64 &packetCount
        , LibList<LibList<LibPacket *> *, _Build::TL> *&recvPacketsBatch);

private:
    Int32 _mode;                                    // HttpStackMode
    HttpTlsContext *_tlsCtx;                        // tls上下文(不持有)
    IDelegate<void, UInt64, UInt64, LibPacket *> *_rawSendHandler;
    IDelegate<void, UInt64, Int32> *_tlsHandshakeHandler;
    UInt64 _maxBodyBytes;

    SpinLock _stateLck;
    std::unordered_map<UInt64, HttpSessionState *> _sessionIdRefState;
};

ALWAYS_INLINE bool HttpProtocolStack::IsTlsEnable() const
{
    return _tlsCtx != NULL;
}

KERNEL_END

#endif
