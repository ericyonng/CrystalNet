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
 * Description: http请求消息实现
*/

#include <pch.h>
#include <kernel/comp/Http/HttpRequest.h>

KERNEL_BEGIN

HttpRequest::HttpRequest()
    :_method(HttpMethodType::Get)
    , _rawMethod(HttpMethodType::ToString(HttpMethodType::Get))
    , _path("/")
    , _version("HTTP/1.1")
{
}

HttpRequest::~HttpRequest()
{
}

void HttpRequest::Release()
{
    HttpRequest::Delete_HttpRequest(this);
}

void HttpRequest::SetMethod(const LibString &method)
{
    _method = HttpMethodType::FromString(method.data(), method.size());
    _rawMethod = method;
}

void HttpRequest::CopyFrom(const HttpRequest &other)
{
    HttpMessage::CopyFrom(other);
    _method = other._method;
    _rawMethod = other._rawMethod;
    _path = other._path;
    _query = other._query;
    _version = other._version;
}

LibString HttpRequest::ToString() const
{
    LibString info;
    info.AppendFormat("%s %s", _rawMethod.c_str(), _path.c_str());
    if (!_query.empty())
        info.AppendFormat("?%s", _query.c_str());

    info.AppendFormat(" %s, %s", _version.c_str(), HttpMessage::ToString().c_str());
    return info;
}

LibString HttpRequest::_BuildFirstLine() const
{
    LibString firstLine;
    firstLine += _rawMethod;
    firstLine += ' ';
    firstLine += _path.empty() ? "/" : _path;
    if (!_query.empty())
    {
        firstLine += '?';
        firstLine += _query;
    }

    firstLine += ' ';
    firstLine += _version.empty() ? "HTTP/1.1" : _version;
    return firstLine;
}

bool HttpRequest::_ParseFirstLine(const Byte8 *line, UInt64 len)
{
    // METHOD SP request-target SP version
    const Byte8 *sp1 = static_cast<const Byte8 *>(::memchr(line, ' ', len));
    if (UNLIKELY(!sp1))
        return false;

    _method = HttpMethodType::FromString(line, static_cast<size_t>(sp1 - line));
    _rawMethod.assign(line, static_cast<size_t>(sp1 - line));

    const Byte8 *target = sp1 + 1;
    const UInt64 leftLen = len - static_cast<UInt64>(target - line);
    const Byte8 *sp2 = static_cast<const Byte8 *>(::memchr(target, ' ', leftLen));
    if (UNLIKELY(!sp2))
        return false;

    const UInt64 targetLen = static_cast<UInt64>(sp2 - target);
    const Byte8 *question = static_cast<const Byte8 *>(::memchr(target, '?', targetLen));
    if (question)
    {
        _path.assign(target, static_cast<size_t>(question - target));
        _query.assign(question + 1, static_cast<size_t>(targetLen - static_cast<UInt64>(question - target) - 1));
    }
    else
    {
        _path.assign(target, targetLen);
        _query.clear();
    }

    if (_path.empty())
        _path = "/";

    _version.assign(sp2 + 1, static_cast<size_t>(len - static_cast<UInt64>(sp2 + 1 - line)));
    return true;
}

KERNEL_END
