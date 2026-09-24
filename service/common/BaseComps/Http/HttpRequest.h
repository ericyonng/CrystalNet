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
 * Description: http请求消息
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_HTTP_HTTP_REQUEST_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_HTTP_HTTP_REQUEST_H__

#pragma once

#include <kernel/comp/Http/HttpMessage.h>

KERNEL_BEGIN

class KERNEL_EXPORT HttpRequest : public HttpMessage
{
    POOL_CREATE_OBJ_DEFAULT_P1(HttpMessage, HttpRequest);

public:
    HttpRequest();
    virtual ~HttpRequest();
    virtual void Release() override;

    // 方法
    void SetMethod(Int32 method);
    void SetMethod(const LibString &method);
    Int32 GetMethod() const;
    const LibString &GetRawMethod() const;

    // 路径与query
    void SetPath(const LibString &path);
    const LibString &GetPath() const;
    void SetQuery(const LibString &query);
    const LibString &GetQuery() const;

    // 版本(默认HTTP/1.1)
    void SetVersion(const LibString &version);
    const LibString &GetVersion() const;

    // 深拷贝(含首行字段)
    void CopyFrom(const HttpRequest &other);

    virtual LibString ToString() const override;

protected:
    virtual LibString _BuildFirstLine() const override;
    virtual bool _ParseFirstLine(const Byte8 *line, UInt64 len) override;

private:
    Int32 _method;
    LibString _rawMethod;   // 原始方法字符串(未知方法时保留)
    LibString _path;
    LibString _query;
    LibString _version;
};

ALWAYS_INLINE void HttpRequest::SetMethod(Int32 method)
{
    _method = method;
    _rawMethod = HttpMethodType::ToString(method);
}

ALWAYS_INLINE Int32 HttpRequest::GetMethod() const
{
    return _method;
}

ALWAYS_INLINE const LibString &HttpRequest::GetRawMethod() const
{
    return _rawMethod;
}

ALWAYS_INLINE void HttpRequest::SetPath(const LibString &path)
{
    _path = path;
}

ALWAYS_INLINE const LibString &HttpRequest::GetPath() const
{
    return _path;
}

ALWAYS_INLINE void HttpRequest::SetQuery(const LibString &query)
{
    _query = query;
}

ALWAYS_INLINE const LibString &HttpRequest::GetQuery() const
{
    return _query;
}

ALWAYS_INLINE void HttpRequest::SetVersion(const LibString &version)
{
    _version = version;
}

ALWAYS_INLINE const LibString &HttpRequest::GetVersion() const
{
    return _version;
}

KERNEL_END

#endif
