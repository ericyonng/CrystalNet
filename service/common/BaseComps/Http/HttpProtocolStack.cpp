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
 * Description: http/https协议栈实现
*/

#include <pch.h>
#include <kernel/comp/Http/HttpProtocolStack.h>
#include <kernel/comp/Http/HttpParser.h>
#include <kernel/comp/Http/HttpRequest.h>
#include <kernel/comp/Http/HttpResponse.h>
#include <kernel/comp/Http/HttpRawCoder.h>
#include <kernel/comp/Http/HttpTlsContext.h>
#include <kernel/comp/NetEngine/LibPacket.h>
#include <kernel/comp/NetEngine/LibSocket.h>
#include <kernel/comp/NetEngine/LibAddr.h>
#include <kernel/comp/NetEngine/Poller/impl/Session/LibSession.h>
#include <kernel/comp/LibStream.h>
#include <kernel/comp/Log/log.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

KERNEL_BEGIN

namespace
{
    static LibString s_GetOpenSslErr()
    {
        LibString errInfo;
        unsigned long err = 0;
        while ((err = ::ERR_get_error()) != 0)
        {
            char buf[256] = {0};
            ::ERR_error_string_n(err, buf, sizeof(buf));
            errInfo.AppendFormat("%s;", buf);
        }

        return errInfo;
    }
}

HttpProtocolStack::HttpProtocolStack(Int32 mode)
    :_mode(mode)
    , _tlsCtx(NULL)
    , _rawSendHandler(NULL)
    , _tlsHandshakeHandler(NULL)
    , _maxBodyBytes(HttpDefaultLimit::MAX_BODY_BYTES)
{
    _type = (mode == HttpStackMode::Server) ? HttpProtocolStackType::HTTP : HttpProtocolStackType::HTTP;
}

HttpProtocolStack::~HttpProtocolStack()
{
    CRYSTAL_RELEASE_SAFE(_rawSendHandler);
    CRYSTAL_RELEASE_SAFE(_tlsHandshakeHandler);

    for (auto &iter : _sessionIdRefState)
    {
        auto *st = iter.second;
        if (st->_ssl)
            ::SSL_free(st->_ssl);
        if (st->_parser)
            HttpParser::Delete_HttpParser(st->_parser);

        HttpSessionState::Delete_HttpSessionState(st);
    }

    _sessionIdRefState.clear();
}

void HttpProtocolStack::SetTlsContext(HttpTlsContext *ctx)
{
    _tlsCtx = ctx;
    if (_tlsCtx)
        _type = HttpProtocolStackType::HTTPS;
}

void HttpProtocolStack::SetRawSendHandler(IDelegate<void, UInt64, UInt64, LibPacket *> *handler)
{
    CRYSTAL_RELEASE_SAFE(_rawSendHandler);
    _rawSendHandler = handler;
}

void HttpProtocolStack::SetTlsHandshakeHandler(IDelegate<void, UInt64, Int32> *handler)
{
    CRYSTAL_RELEASE_SAFE(_tlsHandshakeHandler);
    _tlsHandshakeHandler = handler;
}

void HttpProtocolStack::SetMaxBodyBytes(UInt64 maxBodyBytes)
{
    _maxBodyBytes = maxBodyBytes == 0 ? HttpDefaultLimit::MAX_BODY_BYTES : maxBodyBytes;
}

Int32 HttpProtocolStack::StartTlsClientHandshake(UInt64 sessionId, UInt64 pollerId)
{
    if (UNLIKELY(!_tlsCtx))
    {
        if (g_Log)
        {
            CLOG_ERROR("tls ctx is null.");
        }

        return Status::ParamError;
    }

    if (UNLIKELY(_mode != HttpStackMode::Client))
    {
        if (g_Log)
        {
            CLOG_ERROR("only client mode can start tls handshake, mode:%d", _mode);
        }

        return Status::ParamError;
    }

    auto *st = _GetOrCreateState(sessionId, pollerId);
    if (UNLIKELY(!st))
    {
        if (g_Log)
        {
            CLOG_ERROR("create session state fail, session id:%llu", sessionId);
        }

        return Status::Error;
    }

    // client模式的https会话延迟创建ssl(http会话不创建)
    if (UNLIKELY(!_EnsureSslCreated(st)))
    {
        if (g_Log)
        {
            CLOG_ERROR("ensure ssl created fail, session id:%llu", sessionId);
        }

        return Status::Error;
    }

    if (UNLIKELY(st->_handshaked))
        return Status::Repeat;

    const int ret = ::SSL_connect(st->_ssl);
    if (ret == 1)
    {
        st->_handshaked = true;
        _FlushWbio(st);

        if (_tlsHandshakeHandler)
            _tlsHandshakeHandler->Invoke(sessionId, Status::Success);

        return Status::Success;
    }

    const int sslErr = ::SSL_get_error(st->_ssl, ret);
    if (sslErr == SSL_ERROR_WANT_READ || sslErr == SSL_ERROR_WANT_WRITE)
    {
        _FlushWbio(st);
        return Status::Success;
    }

    if (g_Log)
    {
        CLOG_ERROR("ssl connect fail session id:%llu, ssl err:%d, openssl err:%s", sessionId, sslErr, s_GetOpenSslErr().c_str());
    }

    if (_tlsHandshakeHandler)
        _tlsHandshakeHandler->Invoke(sessionId, Status::Http_TlsFail);

    return Status::Http_TlsFail;
}

void HttpProtocolStack::OnSessionDestroy(UInt64 sessionId)
{
    HttpSessionState *st = NULL;
    {
        _stateLck.Lock();
        auto iter = _sessionIdRefState.find(sessionId);
        if (iter != _sessionIdRefState.end())
        {
            st = iter->second;
            _sessionIdRefState.erase(iter);
        }
        _stateLck.Unlock();
    }

    if (!st)
        return;

    if (st->_ssl)
        ::SSL_free(st->_ssl);
    if (st->_parser)
        HttpParser::Delete_HttpParser(st->_parser);

    HttpSessionState::Delete_HttpSessionState(st);
}

Int32 HttpProtocolStack::ParsingPacket(KERNEL_NS::LibSession *session
                            , KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream
                            , UInt64 &handledBytes
                            , UInt64 &packetCount
                            , LibList<LibList<LibPacket *> *, _Build::TL> *&recvPacketsBatch)
{
    const UInt64 sessionId = session->GetId();
    const UInt64 readable = static_cast<UInt64>(stream.GetReadableSize());
    if (readable == 0)
        return Status::Success;

    auto *st = _GetOrCreateState(sessionId, session->GetPollerId());
    if (UNLIKELY(!st))
        return Status::Error;

    if (st->_ssl)
    {
        // 1.密文入rbio
        ::BIO_write(SSL_get_rbio(st->_ssl), stream.GetReadBegin(), static_cast<int>(readable));
        handledBytes += readable;
        stream.ShiftReadPos(static_cast<Int64>(readable));

        // 2.驱动握手
        if (!st->_handshaked)
        {
            const Int32 err = _DriveTlsHandshake(session, st);
            if (err != Status::Success)
                return err;
        }

        // 3.读取明文
        if (st->_handshaked)
        {
            for (;;)
            {
                char buf[16384];
                const int n = ::SSL_read(st->_ssl, buf, static_cast<int>(sizeof(buf)));
                if (n > 0)
                {
                    const Int32 err = st->_parser->Append(reinterpret_cast<const Byte8 *>(buf), static_cast<UInt64>(n));
                    if (UNLIKELY(err != Status::Success))
                        return err;

                    continue;
                }

                const int sslErr = ::SSL_get_error(st->_ssl, n);
                if (sslErr == SSL_ERROR_WANT_READ || sslErr == SSL_ERROR_WANT_WRITE)
                    break;

                if (sslErr == SSL_ERROR_ZERO_RETURN)
                    break;

                if (g_Log)
                {
                    CLOG_ERROR("ssl read fail session id:%llu, ssl err:%d, openssl err:%s"
                        , sessionId, sslErr, s_GetOpenSslErr().c_str());
                }

                return Status::Http_TlsFail;
            }

            // 握手后可能产生待发送记录(new session ticket等)
            _FlushWbio(st);
        }
    }
    else
    {
        const Int32 err = st->_parser->Append(stream.GetReadBegin(), readable);
        if (UNLIKELY(err != Status::Success))
            return err;

        handledBytes += readable;
        stream.ShiftReadPos(static_cast<Int64>(readable));
    }

    // server: 请求头带Expect: 100-continue且body未收齐时, 应答100 Continue(每个报文仅一次)
    if ((_mode == HttpStackMode::Server) && st->_parser->HasExpect100Pending())
    {
        static const Byte8 s_100Continue[] = "HTTP/1.1 100 Continue\r\n\r\n";
        _SendHttpLayerRaw(st->_pollerId, st->_sessionId, s_100Continue, sizeof(s_100Continue) - 1);
        st->_parser->ConsumeExpect100();
    }

    return _PopMessages(session, st, packetCount, recvPacketsBatch);
}

Int32 HttpProtocolStack::PacketsToBin(LibSession *session, LibPacket *packet, LibStream<_Build::TL> *stream, UInt64 &handledBytes)
{
    auto *coder = packet->GetCoder();
    if (UNLIKELY(!coder))
    {
        if (g_Log)
        {
            CLOG_ERROR("packet have no coder, session id:%llu", session->GetId());
        }

        return Status::Error;
    }

    // 原始数据直通(tls握手记录等, 不做tls加密)
    if (packet->GetOpcode() == HttpOpcode::RawData)
    {
        auto *rawCoder = dynamic_cast<HttpRawCoder *>(coder);
        if (UNLIKELY(!rawCoder))
        {
            if (g_Log)
            {
                CLOG_ERROR("raw packet coder type miss match, session id:%llu", session->GetId());
            }

            return Status::Error;
        }

        const auto &data = rawCoder->GetData();
        if (!data.empty())
        {
            if (UNLIKELY(!stream->Write(data.data(), static_cast<Int64>(data.size()))))
                return Status::Error;

            handledBytes += static_cast<UInt64>(data.size());
        }

        return Status::Success;
    }

    // http层原始字节(100 Continue/chunk块等, tls会话需加密)
    LibString bin;
    if (packet->GetOpcode() == HttpOpcode::HttpLayerData)
    {
        auto *rawCoder = dynamic_cast<HttpRawCoder *>(coder);
        if (UNLIKELY(!rawCoder))
        {
            if (g_Log)
            {
                CLOG_ERROR("http layer packet coder type miss match, session id:%llu", session->GetId());
            }

            return Status::Error;
        }

        bin = rawCoder->GetData();
    }
    else
    {
        // http消息序列化
        LibStream<_Build::TL> tmpStream;
        tmpStream.Init(static_cast<Int64>(HttpDefaultLimit::MAX_HEADER_BYTES));
        if (UNLIKELY(!coder->Encode(tmpStream)))
            return Status::CoderFail;

        bin.assign(tmpStream.GetReadBegin(), static_cast<UInt64>(tmpStream.GetReadableSize()));
    }

    auto *st = _GetState(session->GetId());
    if (!st || !st->_ssl)
    {
        // 明文直接发送
        if (UNLIKELY(!stream->Write(bin.data(), static_cast<Int64>(bin.size()))))
            return Status::Error;

        handledBytes += static_cast<UInt64>(bin.size());
        return Status::Success;
    }

    if (UNLIKELY(!st->_handshaked))
    {
        if (g_Log)
        {
            CLOG_ERROR("tls session not handshaked, can not send http message, session id:%llu", session->GetId());
        }

        return Status::Http_TlsFail;
    }

    // tls加密发送
    UInt64 offset = 0;
    Int32 retry = 0;
    while (offset < bin.size())
    {
        const UInt64 left = bin.size() - offset;
        const int n = ::SSL_write(st->_ssl, bin.data() + offset, static_cast<int>(left < 16384 ? left : 16384));
        if (n > 0)
        {
            offset += static_cast<UInt64>(n);
            retry = 0;
            continue;
        }

        const int sslErr = ::SSL_get_error(st->_ssl, n);
        if ((sslErr == SSL_ERROR_WANT_WRITE || sslErr == SSL_ERROR_WANT_READ) && (++retry < 100))
            continue;

        if (g_Log)
        {
            CLOG_ERROR("ssl write fail session id:%llu, ssl err:%d, openssl err:%s"
                , session->GetId(), sslErr, s_GetOpenSslErr().c_str());
        }

        return Status::Http_TlsFail;
    }

    // 排空wbio密文到发送流
    for (;;)
    {
        char buf[16384];
        const int n = ::BIO_read(SSL_get_wbio(st->_ssl), buf, static_cast<int>(sizeof(buf)));
        if (n <= 0)
            break;

        if (UNLIKELY(!stream->Write(buf, n)))
            return Status::Error;

        handledBytes += static_cast<UInt64>(n);
    }

    return Status::Success;
}

void HttpProtocolStack::RegisterCoderFactory(Int32 opcode, ICoderFactory *factory)
{
    // http协议栈不需要coder工厂, 直接释放
    if (factory)
        factory->Release();
}

HttpSessionState *HttpProtocolStack::_GetState(UInt64 sessionId)
{
    _stateLck.Lock();
    auto iter = _sessionIdRefState.find(sessionId);
    auto *st = iter == _sessionIdRefState.end() ? NULL : iter->second;
    _stateLck.Unlock();
    return st;
}

HttpSessionState *HttpProtocolStack::_GetOrCreateState(UInt64 sessionId, UInt64 pollerId)
{
    {
        _stateLck.Lock();
        auto iter = _sessionIdRefState.find(sessionId);
        if (iter != _sessionIdRefState.end())
        {
            iter->second->_pollerId = pollerId;
            auto *st = iter->second;
            _stateLck.Unlock();
            return st;
        }
        _stateLck.Unlock();
    }

    auto *newState = HttpSessionState::New_HttpSessionState();
    newState->_sessionId = sessionId;
    newState->_pollerId = pollerId;
    newState->_parser = HttpParser::New_HttpParser();
    newState->_parser->Init(_mode == HttpStackMode::Server, HttpDefaultLimit::MAX_HEADER_BYTES, _maxBodyBytes);

    // 仅server模式在会话创建时建立ssl(server的listen明确区分http/https)
    // client模式由StartTlsClientHandshake延迟创建(http/https共用协议栈)
    if ((_mode == HttpStackMode::Server) && _tlsCtx && _tlsCtx->IsInited())
    {
        if (UNLIKELY(!_EnsureSslCreated(newState)))
        {
            if (g_Log)
            {
                CLOG_ERROR("create ssl fail session id:%llu", sessionId);
            }

            HttpParser::Delete_HttpParser(newState->_parser);
            HttpSessionState::Delete_HttpSessionState(newState);
            return NULL;
        }
    }

    _stateLck.Lock();
    auto ret = _sessionIdRefState.insert(std::make_pair(sessionId, newState));
    if (UNLIKELY(!ret.second))
    {
        // 并发下已存在, 释放新建的
        if (newState->_ssl)
            ::SSL_free(newState->_ssl);
        HttpParser::Delete_HttpParser(newState->_parser);
        HttpSessionState::Delete_HttpSessionState(newState);
    }
    auto *st = ret.first->second;
    _stateLck.Unlock();

    return st;
}

bool HttpProtocolStack::_EnsureSslCreated(HttpSessionState *st)
{
    if (LIKELY(st->_ssl))
        return true;

    if (UNLIKELY(!_tlsCtx || !_tlsCtx->IsInited()))
        return false;

    st->_ssl = ::SSL_new(_tlsCtx->GetCtx());
    if (UNLIKELY(!st->_ssl))
    {
        if (g_Log)
        {
            CLOG_ERROR("ssl new fail session id:%llu, openssl err:%s", st->_sessionId, s_GetOpenSslErr().c_str());
        }

        return false;
    }

    auto *rbio = ::BIO_new(::BIO_s_mem());
    auto *wbio = ::BIO_new(::BIO_s_mem());
    ::SSL_set_bio(st->_ssl, rbio, wbio);

    if (_mode == HttpStackMode::Server)
        ::SSL_set_accept_state(st->_ssl);
    else
        ::SSL_set_connect_state(st->_ssl);

    return true;
}

Int32 HttpProtocolStack::_DriveTlsHandshake(LibSession *session, HttpSessionState *st)
{
    const int ret = (_mode == HttpStackMode::Server) ? ::SSL_accept(st->_ssl) : ::SSL_connect(st->_ssl);
    if (ret == 1)
    {
        st->_handshaked = true;
        _FlushWbio(st);

        if (_mode == HttpStackMode::Client && _tlsHandshakeHandler)
            _tlsHandshakeHandler->Invoke(session->GetId(), Status::Success);

        return Status::Success;
    }

    const int sslErr = ::SSL_get_error(st->_ssl, ret);
    if (sslErr == SSL_ERROR_WANT_READ || sslErr == SSL_ERROR_WANT_WRITE)
    {
        _FlushWbio(st);
        return Status::Success;
    }

    if (g_Log)
    {
        CLOG_ERROR("tls handshake fail session id:%llu, ssl err:%d, openssl err:%s"
            , session->GetId(), sslErr, s_GetOpenSslErr().c_str());
    }

    if (_mode == HttpStackMode::Client && _tlsHandshakeHandler)
        _tlsHandshakeHandler->Invoke(session->GetId(), Status::Http_TlsFail);

    return Status::Http_TlsFail;
}

void HttpProtocolStack::_FlushWbio(HttpSessionState *st)
{
    if (!_rawSendHandler)
        return;

    LibString pending;
    for (;;)
    {
        char buf[16384];
        const int n = ::BIO_read(SSL_get_wbio(st->_ssl), buf, static_cast<int>(sizeof(buf)));
        if (n <= 0)
            break;

        pending.append(buf, static_cast<UInt64>(n));
    }

    if (!pending.empty())
        _SendRaw(st->_pollerId, st->_sessionId, pending.data(), static_cast<UInt64>(pending.size()));
}

void HttpProtocolStack::_SendRaw(UInt64 pollerId, UInt64 sessionId, const Byte8 *data, UInt64 len)
{
    auto *packet = LibPacket::New_LibPacket();
    packet->SetSessionId(sessionId);
    packet->SetOpcode(HttpOpcode::RawData);

    auto *coder = HttpRawCoder::New_HttpRawCoder();
    coder->SetData(data, len);
    packet->SetCoder(coder);

    _rawSendHandler->Invoke(pollerId, sessionId, packet);
}

void HttpProtocolStack::_SendHttpLayerRaw(UInt64 pollerId, UInt64 sessionId, const Byte8 *data, UInt64 len)
{
    if (!_rawSendHandler)
        return;

    auto *packet = LibPacket::New_LibPacket();
    packet->SetSessionId(sessionId);
    packet->SetOpcode(HttpOpcode::HttpLayerData);

    auto *coder = HttpRawCoder::New_HttpRawCoder();
    coder->SetData(data, len);
    packet->SetCoder(coder);

    _rawSendHandler->Invoke(pollerId, sessionId, packet);
}

Int32 HttpProtocolStack::_PopMessages(LibSession *session
    , HttpSessionState *st
    , UInt64 &packetCount
    , LibList<LibList<LibPacket *> *, _Build::TL> *&recvPacketsBatch)
{
    const auto &option = session->GetOption();

    while (st->_parser->HasCompleteMessage())
    {
        const UInt64 msgBytes = st->_parser->GetCompleteBytes();

        ICoder *coder = NULL;
        if (_mode == HttpStackMode::Server)
            coder = HttpRequest::New_HttpRequest();
        else
            coder = HttpResponse::New_HttpResponse();

        LibStream<_Build::TL> decodeStream;
        decodeStream.Attach(const_cast<Byte8 *>(st->_parser->GetData()), static_cast<Int64>(msgBytes), 0, static_cast<Int64>(msgBytes));
        if (UNLIKELY(!coder->Decode(decodeStream)))
        {
            if (g_Log)
            {
                CLOG_ERROR("decode http message fail session id:%llu, msg bytes:%llu", session->GetId(), msgBytes);
            }

            coder->Release();
            return Status::Http_BadMessage;
        }

        // 填充会话id(供SendChunk等回写场景使用)
        static_cast<HttpMessage *>(coder)->SetSessionId(session->GetId());

        auto *packet = LibPacket::New_LibPacket();
        packet->SetSessionId(session->GetId());

        auto *sock = session->GetSock();
        if (sock && sock->GetAddr())
        {
            packet->SetLocalAddr(sock->GetAddr()->GetLocalBriefAddr());
            packet->SetRemoteAddr(sock->GetAddr()->GetRemoteBriefAddr());
        }

        packet->SetOpcode(HttpOpcode::HttpMessage);
        packet->SetCoder(coder);

        // 按堆叠上限分包列表
        auto *parsedPacket = recvPacketsBatch->End() ? recvPacketsBatch->End()->_data : NULL;
        if (!parsedPacket || (option._sessionRecvPacketStackLimit && (parsedPacket->GetAmount() >= option._sessionRecvPacketStackLimit)))
        {
            parsedPacket = LibList<LibPacket *>::New_LibList();
            recvPacketsBatch->PushBack(parsedPacket);
        }

        parsedPacket->PushBack(packet);
        ++packetCount;

        st->_parser->PopComplete();
    }

    return Status::Success;
}

KERNEL_END
