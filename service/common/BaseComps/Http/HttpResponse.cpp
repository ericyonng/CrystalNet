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
 * Description: http响应消息实现
*/

#include <pch.h>
#include <kernel/comp/Http/HttpResponse.h>

KERNEL_BEGIN

HttpResponse::HttpResponse()
    :_version("HTTP/1.1")
    , _statusCode(HttpStatusCode::Ok)
    , _reason(HttpStatusCode::Reason(HttpStatusCode::Ok))
{
}

HttpResponse::~HttpResponse()
{
}

void HttpResponse::Release()
{
    HttpResponse::Delete_HttpResponse(this);
}

void HttpResponse::CopyFrom(const HttpResponse &other)
{
    HttpMessage::CopyFrom(other);
    _version = other._version;
    _statusCode = other._statusCode;
    _reason = other._reason;
}

LibString HttpResponse::ToString() const
{
    LibString info;
    info.AppendFormat("%s %d %s, %s", _version.c_str(), _statusCode, _reason.c_str(), HttpMessage::ToString().c_str());
    return info;
}

LibString HttpResponse::_BuildFirstLine() const
{
    LibString firstLine;
    firstLine += _version.empty() ? "HTTP/1.1" : _version;
    firstLine += ' ';
    firstLine.AppendFormat("%d", _statusCode);
    firstLine += ' ';
    firstLine += _reason.empty() ? HttpStatusCode::Reason(_statusCode) : _reason;
    return firstLine;
}

bool HttpResponse::_ParseFirstLine(const Byte8 *line, UInt64 len)
{
    // version SP status-code SP reason-phrase
    const Byte8 *sp1 = static_cast<const Byte8 *>(::memchr(line, ' ', len));
    if (UNLIKELY(!sp1))
        return false;

    _version.assign(line, static_cast<size_t>(sp1 - line));

    const Byte8 *codeBegin = sp1 + 1;
    const UInt64 leftLen = len - static_cast<UInt64>(codeBegin - line);
    const Byte8 *sp2 = static_cast<const Byte8 *>(::memchr(codeBegin, ' ', leftLen));

    const UInt64 codeLen = sp2 ? static_cast<UInt64>(sp2 - codeBegin) : leftLen;
    if (UNLIKELY(codeLen == 0))
        return false;

    _statusCode = 0;
    for (UInt64 idx = 0; idx < codeLen; ++idx)
    {
        const Byte8 ch = codeBegin[idx];
        if (UNLIKELY(ch < '0' || ch > '9'))
            return false;

        _statusCode = _statusCode * 10 + (ch - '0');
    }

    if (sp2 && (static_cast<UInt64>(sp2 + 1 - line) < len))
        _reason.assign(sp2 + 1, static_cast<size_t>(len - static_cast<UInt64>(sp2 + 1 - line)));
    else
        _reason = HttpStatusCode::Reason(_statusCode);

    return true;
}

KERNEL_END
