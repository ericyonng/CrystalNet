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
 * Description: http响应消息
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_HTTP_HTTP_RESPONSE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_HTTP_HTTP_RESPONSE_H__

#pragma once

#include <kernel/comp/Http/HttpMessage.h>

KERNEL_BEGIN

class KERNEL_EXPORT HttpResponse : public HttpMessage
{
    POOL_CREATE_OBJ_DEFAULT_P1(HttpMessage, HttpResponse);

public:
    HttpResponse();
    virtual ~HttpResponse();
    virtual void Release() override;

    // 状态码/原因短语
    void SetStatusCode(Int32 statusCode);
    Int32 GetStatusCode() const;
    void SetReason(const LibString &reason);
    const LibString &GetReason() const;

    // 版本(默认HTTP/1.1)
    void SetVersion(const LibString &version);
    const LibString &GetVersion() const;

    // 深拷贝(含首行字段)
    void CopyFrom(const HttpResponse &other);

    virtual LibString ToString() const override;

protected:
    virtual LibString _BuildFirstLine() const override;
    virtual bool _ParseFirstLine(const Byte8 *line, UInt64 len) override;

private:
    LibString _version;
    Int32 _statusCode;
    LibString _reason;
};

ALWAYS_INLINE void HttpResponse::SetStatusCode(Int32 statusCode)
{
    _statusCode = statusCode;
}

ALWAYS_INLINE Int32 HttpResponse::GetStatusCode() const
{
    return _statusCode;
}

ALWAYS_INLINE void HttpResponse::SetReason(const LibString &reason)
{
    _reason = reason;
}

ALWAYS_INLINE const LibString &HttpResponse::GetReason() const
{
    return _reason;
}

ALWAYS_INLINE void HttpResponse::SetVersion(const LibString &version)
{
    _version = version;
}

ALWAYS_INLINE const LibString &HttpResponse::GetVersion() const
{
    return _version;
}

KERNEL_END

#endif
