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
 * Description: http消息基类实现
*/

#include <pch.h>
#include <kernel/comp/Http/HttpMessage.h>
#include <kernel/comp/LibStream.h>
#include <kernel/comp/Log/log.h>

KERNEL_BEGIN

namespace
{
    // header name转小写
    static LibString s_ToLowerName(const Byte8 *str, UInt64 len)
    {
        LibString ret;
        ret.reserve(len + 1);
        for (UInt64 idx = 0; idx < len; ++idx)
        {
            Byte8 ch = str[idx];
            if (ch >= 'A' && ch <= 'Z')
                ch = static_cast<Byte8>(ch - 'A' + 'a');

            ret += ch;
        }

        return ret;
    }

    // 去掉首尾空格与\t
    static LibString s_StripBlank(const Byte8 *str, UInt64 len)
    {
        UInt64 begin = 0;
        UInt64 end = len;
        while (begin < end && (str[begin] == ' ' || str[begin] == '\t'))
            ++begin;
        while (end > begin && (str[end - 1] == ' ' || str[end - 1] == '\t'))
            --end;

        LibString ret;
        ret.assign(str + begin, end - begin);
        return ret;
    }

    // chunked编码body解码: <hex size>\r\n<data>\r\n ... 0\r\n\r\n(trailer忽略)
    static bool s_DecodeChunkedBody(const Byte8 *data, UInt64 len, LibString &out)
    {
        out.clear();
        UInt64 pos = 0;
        while (pos < len)
        {
            // chunk size行
            UInt64 lineEnd = pos;
            while ((lineEnd + 1 < len) && !(data[lineEnd] == '\r' && data[lineEnd + 1] == '\n'))
                ++lineEnd;

            if (UNLIKELY(lineEnd + 1 >= len))
                return false;

            // hex解析(忽略;后的chunk扩展)
            UInt64 chunkSize = 0;
            bool hasDigit = false;
            for (UInt64 idx = pos; idx < lineEnd; ++idx)
            {
                const Byte8 ch = data[idx];
                if (ch == ';')
                    break;

                if (ch == ' ' || ch == '\t')
                    continue;

                UInt32 v = 0;
                if (ch >= '0' && ch <= '9')
                    v = static_cast<UInt32>(ch - '0');
                else if (ch >= 'a' && ch <= 'f')
                    v = static_cast<UInt32>(ch - 'a' + 10);
                else if (ch >= 'A' && ch <= 'F')
                    v = static_cast<UInt32>(ch - 'A' + 10);
                else
                    return false;

                chunkSize = chunkSize * 16 + v;
                hasDigit = true;
            }

            if (UNLIKELY(!hasDigit))
                return false;

            pos = lineEnd + 2;

            // 结束块(后续trailer与结尾\r\n忽略)
            if (chunkSize == 0)
                return true;

            if (UNLIKELY(pos + chunkSize + 2 > len))
                return false;

            out.append(data + pos, chunkSize);
            pos += chunkSize;

            // chunk数据后缀\r\n
            if (UNLIKELY((pos + 1 >= len) || (data[pos] != '\r') || (data[pos + 1] != '\n')))
                return false;

            pos += 2;
        }

        return true;
    }
}

HttpMessage::HttpMessage()
    :_chunkedSend(false)
    , _chunkedStream(false)
    , _sessionId(0)
{
}

HttpMessage::~HttpMessage()
{
}

void HttpMessage::SetHeader(const LibString &name, const LibString &value)
{
    _headers[s_ToLowerName(name.data(), static_cast<UInt64>(name.size()))] = value;
}

const LibString *HttpMessage::GetHeader(const LibString &name) const
{
    auto iter = _headers.find(s_ToLowerName(name.data(), static_cast<UInt64>(name.size())));
    return iter == _headers.end() ? NULL : &iter->second;
}

bool HttpMessage::HasHeader(const LibString &name) const
{
    return _headers.find(s_ToLowerName(name.data(), static_cast<UInt64>(name.size()))) != _headers.end();
}

void HttpMessage::RemoveHeader(const LibString &name)
{
    _headers.erase(s_ToLowerName(name.data(), static_cast<UInt64>(name.size())));
}

void HttpMessage::SetBody(const LibString &body)
{
    _body = body;
}

void HttpMessage::SetBody(const Byte8 *data, UInt64 len)
{
    _body.assign(data, len);
}

void HttpMessage::SetBody(LibString &&body)
{
    _body = std::move(body);
}

void HttpMessage::SetKeepAlive(bool keepAlive)
{
    SetHeader("Connection", keepAlive ? "keep-alive" : "close");
}

bool HttpMessage::IsKeepAlive() const
{
    auto *connection = GetHeader("Connection");
    if (connection && (*connection == "close"))
        return false;

    return true;
}

void HttpMessage::SetChunkedSend(bool chunked, bool isStream)
{
    _chunkedSend = chunked;
    _chunkedStream = chunked && isStream;
}

void HttpMessage::SetSessionId(UInt64 sessionId)
{
    _sessionId = sessionId;
}

void HttpMessage::CopyFrom(const HttpMessage &other)
{
    _headers = other._headers;
    _body = other._body;
    _chunkedSend = other._chunkedSend;
    _chunkedStream = other._chunkedStream;
    _sessionId = other._sessionId;
}

LibString HttpMessage::ToString() const
{
    LibString info;
    info.AppendFormat("headers count:%llu, body bytes:%llu"
        , static_cast<UInt64>(_headers.size()), static_cast<UInt64>(_body.size()));
    return info;
}

bool HttpMessage::Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) const
{
    return _EncodeTpl(stream);
}

bool HttpMessage::Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) const
{
    return _EncodeTpl(stream);
}

bool HttpMessage::Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream)
{
    return _DecodeTpl(const_cast<KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &>(stream));
}

bool HttpMessage::Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream)
{
    return _DecodeTpl(const_cast<KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &>(stream));
}

bool HttpMessage::Decode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream)
{
    return _DecodeTpl(stream);
}

bool HttpMessage::Decode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream)
{
    return _DecodeTpl(stream);
}

template<typename StreamType>
bool HttpMessage::_EncodeTpl(StreamType &stream) const
{
    LibString bin;
    const LibString firstLine = _BuildFirstLine();
    bin.reserve(firstLine.size() + _body.size() + 256);

    bin += firstLine;
    bin += "\r\n";

    for (auto &kv : _headers)
    {
        bin += kv.first;
        bin += ": ";
        bin += kv.second;
        bin += "\r\n";
    }

    if (_chunkedSend)
    {
        // chunked编码不携带Content-Length
        if (!HasHeader("Transfer-Encoding"))
        {
            bin += "transfer-encoding: chunked";
            bin += "\r\n";
        }
    }
    else if (!HasHeader("Content-Length"))
    {
        // 注意: AppendFormat的格式串中不要使用\r\n(LibString的format实现会吞掉\n)
        bin.AppendFormat("Content-Length: %llu", static_cast<UInt64>(_body.size()));
        bin += "\r\n";
    }

    bin += "\r\n";

    if (_chunkedSend)
    {
        if (_chunkedStream)
        {
            // 流式: 仅发送首部, 后续由SendChunk追加
        }
        else if (!_body.empty())
        {
            // 整包chunked: <hex len>\r\n<body>\r\n0\r\n\r\n
            bin.AppendFormat("%llx", static_cast<UInt64>(_body.size()));
            bin += "\r\n";
            bin.append(_body.data(), _body.size());
            bin += "\r\n0\r\n\r\n";
        }
        else
        {
            // 空body的chunked: 直接发结束块
            bin += "0\r\n\r\n";
        }
    }
    else
    {
        bin.append(_body.data(), _body.size());
    }

    return stream.Write(bin.data(), static_cast<Int64>(bin.size()));
}

template<typename StreamType>
bool HttpMessage::_DecodeTpl(StreamType &stream)
{
    const Byte8 *data = stream.GetReadBegin();
    const UInt64 len = static_cast<UInt64>(stream.GetReadableSize());

    // 定位header结束\r\n\r\n
    UInt64 headerEnd = 0;
    for (UInt64 idx = 0; idx + 3 < len; ++idx)
    {
        if (data[idx] == '\r' && data[idx + 1] == '\n' && data[idx + 2] == '\r' && data[idx + 3] == '\n')
        {
            headerEnd = idx + 4;
            break;
        }
    }

    if (UNLIKELY(headerEnd == 0))
    {
        if (g_Log)
        {
            CLOG_ERROR("http message have no header end, readable bytes:%llu", len);
        }

        return false;
    }

    // 首行
    const Byte8 *lineBegin = data;
    const Byte8 *cursor = data;
    UInt64 pos = 0;
    while (pos + 1 < headerEnd && !(data[pos] == '\r' && data[pos + 1] == '\n'))
        ++pos;

    if (UNLIKELY(pos + 1 >= headerEnd))
    {
        if (g_Log)
        {
            CLOG_ERROR("http message bad first line, header end:%llu", headerEnd);
        }

        return false;
    }

    if (!_ParseFirstLine(lineBegin, pos))
    {
        if (g_Log)
        {
            CLOG_ERROR("http message parse first line fail:%.*s", static_cast<Int32>(pos), lineBegin);
        }

        return false;
    }

    pos += 2;

    // headers
    _headers.clear();
    while (pos < headerEnd - 2)
    {
        // 找行尾
        UInt64 lineEndPos = pos;
        while (lineEndPos + 1 < headerEnd && !(data[lineEndPos] == '\r' && data[lineEndPos + 1] == '\n'))
            ++lineEndPos;

        if (lineEndPos == pos)
        {
            pos += 2;
            break;
        }

        // name: value
        UInt64 colonPos = pos;
        while (colonPos < lineEndPos && data[colonPos] != ':')
            ++colonPos;

        if (LIKELY(colonPos < lineEndPos))
        {
            const LibString name = s_ToLowerName(data + pos, colonPos - pos);
            const LibString value = s_StripBlank(data + colonPos + 1, lineEndPos - colonPos - 1);
            _headers[name] = value;
        }

        pos = lineEndPos + 2;
    }

    // body(剩余全部, transfer-encoding: chunked时需解码)
    if (len > headerEnd)
    {
        bool isChunked = false;
        auto *te = GetHeader("transfer-encoding");
        if (te)
        {
            for (UInt64 idx = 0; idx + 7 <= te->size(); ++idx)
            {
                bool match = true;
                for (UInt64 k = 0; k < 7; ++k)
                {
                    Byte8 ch = (*te)[idx + k];
                    if (ch >= 'A' && ch <= 'Z')
                        ch = static_cast<Byte8>(ch - 'A' + 'a');

                    if (ch != "chunked"[k])
                    {
                        match = false;
                        break;
                    }
                }

                if (match)
                {
                    isChunked = true;
                    break;
                }
            }
        }

        if (isChunked)
        {
            if (UNLIKELY(!s_DecodeChunkedBody(data + headerEnd, len - headerEnd, _body)))
            {
                if (g_Log)
                {
                    CLOG_ERROR("decode chunked body fail, body bytes:%llu", len - headerEnd);
                }

                return false;
            }
        }
        else
        {
            _body.assign(data + headerEnd, len - headerEnd);
        }
    }
    else
    {
        _body.clear();
    }

    stream.ShiftReadPos(static_cast<Int64>(len));
    return true;
}

KERNEL_END
