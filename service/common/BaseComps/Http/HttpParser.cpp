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
 * Description: http/1.1增量解析器实现
*/

#include <pch.h>
#include <kernel/comp/Http/HttpParser.h>
#include <kernel/comp/Log/log.h>

KERNEL_BEGIN

namespace
{
    // 不区分大小写的定长比较
    static bool s_StrCaseEqualN(const Byte8 *l, const Byte8 *r, UInt64 n)
    {
        for (UInt64 idx = 0; idx < n; ++idx)
        {
            Byte8 lc = l[idx];
            Byte8 rc = r[idx];
            if (lc >= 'A' && lc <= 'Z')
                lc = static_cast<Byte8>(lc - 'A' + 'a');
            if (rc >= 'A' && rc <= 'Z')
                rc = static_cast<Byte8>(rc - 'A' + 'a');

            if (lc != rc)
                return false;
        }

        return true;
    }
}

HttpParser::HttpParser()
    :_parseRequest(true)
    , _maxHeaderBytes(HttpDefaultLimit::MAX_HEADER_BYTES)
    , _maxBodyBytes(HttpDefaultLimit::MAX_BODY_BYTES)
    , _headerBytes(0)
    , _chunked(false)
    , _contentLength(0)
    , _completeBytes(0)
    , _expect100(false)
{
}

HttpParser::~HttpParser()
{
}

void HttpParser::Release()
{
    HttpParser::Delete_HttpParser(this);
}

void HttpParser::Init(bool parseRequest, UInt64 maxHeaderBytes, UInt64 maxBodyBytes)
{
    _parseRequest = parseRequest;
    _maxHeaderBytes = maxHeaderBytes == 0 ? HttpDefaultLimit::MAX_HEADER_BYTES : maxHeaderBytes;
    _maxBodyBytes = maxBodyBytes == 0 ? HttpDefaultLimit::MAX_BODY_BYTES : maxBodyBytes;
    Reset();
}

void HttpParser::Reset()
{
    _buf.clear();
    _headerBytes = 0;
    _chunked = false;
    _contentLength = 0;
    _completeBytes = 0;
    _expect100 = false;
}

Int32 HttpParser::Append(const Byte8 *data, UInt64 len)
{
    if (len != 0)
        _buf.append(data, len);

    return _UpdateState();
}

void HttpParser::PopComplete()
{
    if (_completeBytes == 0)
        return;

    _buf.erase(0, _completeBytes);
    _headerBytes = 0;
    _chunked = false;
    _contentLength = 0;
    _completeBytes = 0;
    _expect100 = false;

    _UpdateState();
}

LibString HttpParser::ToString() const
{
    LibString info;
    info.AppendFormat("parse request:%d, buffer bytes:%llu, header bytes:%llu, chunked:%d, content length:%llu, complete bytes:%llu"
        , _parseRequest ? 1 : 0, static_cast<UInt64>(_buf.size()), _headerBytes, _chunked ? 1 : 0, _contentLength, _completeBytes);
    return info;
}

Int32 HttpParser::_UpdateState()
{
    if (_completeBytes != 0)
        return Status::Success;

    // 1.定位header结束
    if (_headerBytes == 0)
    {
        const Byte8 *data = _buf.data();
        const UInt64 size = static_cast<UInt64>(_buf.size());

        UInt64 headerEnd = 0;
        for (UInt64 idx = 0; idx + 3 < size; ++idx)
        {
            if (data[idx] == '\r' && data[idx + 1] == '\n' && data[idx + 2] == '\r' && data[idx + 3] == '\n')
            {
                headerEnd = idx + 4;
                break;
            }
        }

        if (headerEnd == 0)
        {
            if (size > _maxHeaderBytes)
            {
                if (g_Log)
                {
                    CLOG_ERROR("http header over limit:%llu, current buffered:%llu", _maxHeaderBytes, size);
                }

                return Status::Http_HeaderTooLarge;
            }

            return Status::Success;
        }

        _headerBytes = headerEnd;

        const Int32 err = _ParseHeaders(headerEnd - 4);
        if (err != Status::Success)
            return err;
    }

    // 2.判定body边界
    if (_chunked)
    {
        const UInt64 total = _CheckChunkedComplete(_headerBytes);
        if (total == static_cast<UInt64>(-1))
        {
            if (g_Log)
            {
                CLOG_ERROR("http chunked body parse fail, parser:%s", ToString().c_str());
            }

            return Status::Http_BadMessage;
        }

        if (total == 0)
        {
            if ((static_cast<UInt64>(_buf.size()) - _headerBytes) > _maxBodyBytes)
            {
                if (g_Log)
                {
                    CLOG_ERROR("http chunked body over limit:%llu, parser:%s", _maxBodyBytes, ToString().c_str());
                }

                return Status::Http_BodyTooLarge;
            }

            return Status::Success;
        }

        _completeBytes = total;
        return Status::Success;
    }

    if (_contentLength > _maxBodyBytes)
    {
        if (g_Log)
        {
            CLOG_ERROR("http body over limit:%llu, content length:%llu", _maxBodyBytes, _contentLength);
        }

        return Status::Http_BodyTooLarge;
    }

    const UInt64 total = _headerBytes + _contentLength;
    if (static_cast<UInt64>(_buf.size()) >= total)
        _completeBytes = total;

    return Status::Success;
}

Int32 HttpParser::_ParseHeaders(UInt64 headerEndPos)
{
    // header区: [0, headerEndPos), 首行为请求行/状态行
    const Byte8 *data = _buf.data();

    // 跳过首行(行尾可能是header结束标记\r\n\r\n的第一个\r\n, 位于headerEndPos处, 扫描上界需放宽2字节)
    UInt64 pos = 0;
    while (pos + 1 < headerEndPos + 2 && !(data[pos] == '\r' && data[pos + 1] == '\n'))
        ++pos;

    if (UNLIKELY(pos + 1 >= headerEndPos + 2))
        return Status::Http_BadMessage;

    const UInt64 firstLineBegin = 0;
    const UInt64 firstLineEnd = pos;
    pos += 2;

    _chunked = false;
    _contentLength = 0;
    bool hasContentLength = false;

    while (pos < headerEndPos)
    {
        // 行尾可能是header结束标记\r\n\r\n的第一个\r\n(位于headerEndPos处), 扫描上界需放宽2字节
        UInt64 lineEnd = pos;
        while (lineEnd + 1 < headerEndPos + 2 && !(data[lineEnd] == '\r' && data[lineEnd + 1] == '\n'))
            ++lineEnd;

        if (lineEnd == pos)
            break;

        // name: value
        UInt64 colon = pos;
        while (colon < lineEnd && data[colon] != ':')
            ++colon;

        if (colon < lineEnd)
        {
            const UInt64 nameLen = colon - pos;
            // value去首尾空白
            UInt64 valueBegin = colon + 1;
            UInt64 valueEnd = lineEnd;
            while (valueBegin < valueEnd && (data[valueBegin] == ' ' || data[valueBegin] == '\t'))
                ++valueBegin;
            while (valueEnd > valueBegin && (data[valueEnd - 1] == ' ' || data[valueEnd - 1] == '\t'))
                --valueEnd;

            const UInt64 valueLen = valueEnd - valueBegin;

            if ((nameLen == 14) && s_StrCaseEqualN(data + pos, "content-length", 14))
            {
                hasContentLength = true;
                _contentLength = 0;
                for (UInt64 idx = 0; idx < valueLen; ++idx)
                {
                    const Byte8 ch = data[valueBegin + idx];
                    if (ch >= '0' && ch <= '9')
                        _contentLength = _contentLength * 10 + static_cast<UInt64>(ch - '0');
                }
            }
            else if ((nameLen == 17) && s_StrCaseEqualN(data + pos, "transfer-encoding", 17))
            {
                // value包含chunked(忽略大小写)
                for (UInt64 idx = 0; idx + 7 <= valueLen; ++idx)
                {
                    if (s_StrCaseEqualN(data + valueBegin + idx, "chunked", 7))
                    {
                        _chunked = true;
                        break;
                    }
                }
            }
            else if ((nameLen == 6) && s_StrCaseEqualN(data + pos, "expect", 6))
            {
                // Expect: 100-continue(忽略大小写)
                if ((valueLen == 12) && s_StrCaseEqualN(data + valueBegin, "100-continue", 12))
                    _expect100 = true;
            }
        }

        pos = lineEnd + 2;
    }

    if (_chunked)
        return Status::Success;

    if (hasContentLength)
        return Status::Success;

    // 无Content-Length无chunked: 请求一律视为无body
    if (_parseRequest)
    {
        _contentLength = 0;
        return Status::Success;
    }

    // 响应: 1xx/204/304无body, 其他必须携带长度(不支持以连接关闭标识body结束)
    Int32 statusCode = 0;
    const Byte8 *sp1 = static_cast<const Byte8 *>(::memchr(data + firstLineBegin, ' ', firstLineEnd - firstLineBegin));
    if (sp1)
    {
        const Byte8 *codeBegin = sp1 + 1;
        const UInt64 codeMaxLen = firstLineEnd - static_cast<UInt64>(codeBegin - data);
        for (UInt64 idx = 0; idx < codeMaxLen && idx < 3; ++idx)
        {
            const Byte8 ch = codeBegin[idx];
            if (ch < '0' || ch > '9')
                break;

            statusCode = statusCode * 10 + (ch - '0');
        }
    }

    if ((statusCode >= 100 && statusCode < 200) || statusCode == 204 || statusCode == 304)
    {
        _contentLength = 0;
        return Status::Success;
    }

    if (g_Log)
    {
        CLOG_ERROR("http response have no content-length and no chunked, not supported, first line:%.*s"
            , static_cast<Int32>(firstLineEnd - firstLineBegin), data + firstLineBegin);
    }

    return Status::Http_UnsupportedBody;
}

UInt64 HttpParser::_CheckChunkedComplete(UInt64 bodyStart) const
{
    // 返回: 0数据不足, (UInt64)-1格式错误, 其他为完整报文字节数
    const Byte8 *data = _buf.data();
    const UInt64 size = static_cast<UInt64>(_buf.size());

    UInt64 pos = bodyStart;
    for (;;)
    {
        // 找chunk size行
        UInt64 crlf = pos;
        while (crlf + 1 < size && !(data[crlf] == '\r' && data[crlf + 1] == '\n'))
            ++crlf;

        if (crlf + 1 >= size)
            return 0;

        // 解析hex size(忽略chunk extensions ';'之后内容)
        UInt64 chunkSize = 0;
        bool hasHex = false;
        for (UInt64 idx = pos; idx < crlf; ++idx)
        {
            const Byte8 ch = data[idx];
            if (ch == ';')
                break;

            if (ch == ' ' || ch == '\t')
                continue;

            Int32 hex = -1;
            if (ch >= '0' && ch <= '9')
                hex = ch - '0';
            else if (ch >= 'a' && ch <= 'f')
                hex = ch - 'a' + 10;
            else if (ch >= 'A' && ch <= 'F')
                hex = ch - 'A' + 10;
            else
                return static_cast<UInt64>(-1);

            hasHex = true;
            chunkSize = chunkSize * 16 + static_cast<UInt64>(hex);
            if (chunkSize > _maxBodyBytes)
                return static_cast<UInt64>(-1);
        }

        if (!hasHex)
            return static_cast<UInt64>(-1);

        pos = crlf + 2;

        // 最后一个chunk: 0\r\n + trailer + \r\n
        if (chunkSize == 0)
        {
            // 无trailer: 直接\r\n
            if (pos + 2 <= size && data[pos] == '\r' && data[pos + 1] == '\n')
                return pos + 2;

            // 有trailer: 找\r\n\r\n
            for (UInt64 idx = pos; idx + 3 < size; ++idx)
            {
                if (data[idx] == '\r' && data[idx + 1] == '\n' && data[idx + 2] == '\r' && data[idx + 3] == '\n')
                    return idx + 4;
            }

            return 0;
        }

        // 数据 + CRLF
        if (size - pos < chunkSize + 2)
            return 0;

        pos += chunkSize;
        if (data[pos] != '\r' || data[pos + 1] != '\n')
            return static_cast<UInt64>(-1);

        pos += 2;
    }
}

KERNEL_END
