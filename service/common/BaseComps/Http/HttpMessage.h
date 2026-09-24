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
 * Description: http消息基类(请求/响应公共部分: header/body)
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_HTTP_HTTP_MESSAGE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_HTTP_HTTP_MESSAGE_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/comp/NetEngine/Protocol/ICoder.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Http/HttpDefs.h>

#include <map>

KERNEL_BEGIN

class KERNEL_EXPORT HttpMessage : public ICoder
{
    POOL_CREATE_OBJ_DEFAULT_P1(ICoder, HttpMessage);

public:
    HttpMessage();
    virtual ~HttpMessage();

    // header操作(name统一按小写存储, 大小写不敏感)
    void SetHeader(const LibString &name, const LibString &value);
    const LibString *GetHeader(const LibString &name) const;
    bool HasHeader(const LibString &name) const;
    void RemoveHeader(const LibString &name);
    const std::map<LibString, LibString> &GetHeaders() const;

    // body
    void SetBody(const LibString &body);
    void SetBody(const Byte8 *data, UInt64 len);
    void SetBody(LibString &&body);
    const LibString &GetBody() const;

    // Connection: 默认http/1.1 keep-alive
    void SetKeepAlive(bool keepAlive);
    bool IsKeepAlive() const;

    // chunked发送: 开启后以Transfer-Encoding: chunked编码body
    // isStream=true时本次编码仅发送首部(不含body与结束块), 之后由HttpServer::SendChunk流式追加
    void SetChunkedSend(bool chunked, bool isStream = false);
    bool IsChunkedSend() const;
    bool IsChunkedStream() const;

    // 报文所属会话id(收方向由协议栈填充, 用于SendChunk等回写场景)
    void SetSessionId(UInt64 sessionId);
    UInt64 GetSessionId() const;

    // 深拷贝
    void CopyFrom(const HttpMessage &other);

    virtual LibString ToString() const;

public:
    virtual bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) const override;
    virtual bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) const override;

    virtual bool Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) override;
    virtual bool Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) override;
    virtual bool Decode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) override;
    virtual bool Decode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) override;

protected:
    // 首行由子类实现(请求行/状态行)
    virtual LibString _BuildFirstLine() const = 0;
    virtual bool _ParseFirstLine(const Byte8 *line, UInt64 len) = 0;

    template<typename StreamType>
    bool _EncodeTpl(StreamType &stream) const;
    template<typename StreamType>
    bool _DecodeTpl(StreamType &stream);

protected:
    std::map<LibString, LibString> _headers;    // name小写 => value
    LibString _body;
    bool _chunkedSend;                          // chunked发送
    bool _chunkedStream;                        // 流式chunked(仅发首部)
    UInt64 _sessionId;                          // 所属会话id
};

ALWAYS_INLINE bool HttpMessage::IsChunkedSend() const
{
    return _chunkedSend;
}

ALWAYS_INLINE bool HttpMessage::IsChunkedStream() const
{
    return _chunkedStream;
}

ALWAYS_INLINE UInt64 HttpMessage::GetSessionId() const
{
    return _sessionId;
}

ALWAYS_INLINE const std::map<LibString, LibString> &HttpMessage::GetHeaders() const
{
    return _headers;
}

ALWAYS_INLINE const LibString &HttpMessage::GetBody() const
{
    return _body;
}

KERNEL_END

#endif
