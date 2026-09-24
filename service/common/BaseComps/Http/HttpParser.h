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
 * Description: http/1.1增量解析器: 负责报文边界切割(粘包/半包处理),
 *              支持Content-Length与chunked传输编码, 报文内容解析由HttpRequest/HttpResponse完成
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_HTTP_HTTP_PARSER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_HTTP_HTTP_PARSER_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/status.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Http/HttpDefs.h>

KERNEL_BEGIN

class KERNEL_EXPORT HttpParser
{
    POOL_CREATE_OBJ_DEFAULT(HttpParser);

public:
    HttpParser();
    ~HttpParser();
    void Release();

    // parseRequest: true按http请求切割, false按http响应切割
    void Init(bool parseRequest
        , UInt64 maxHeaderBytes = HttpDefaultLimit::MAX_HEADER_BYTES
        , UInt64 maxBodyBytes = HttpDefaultLimit::MAX_BODY_BYTES);
    void Reset();

    // 追加数据 返回: Status::Success成功, 其他为错误码
    Int32 Append(const Byte8 *data, UInt64 len);

    // 是否有完整报文
    bool HasCompleteMessage() const;
    // 完整报文字节数(含header与body)
    UInt64 GetCompleteBytes() const;

    // 当前缓存数据
    const Byte8 *GetData() const;
    UInt64 GetDataSize() const;

    // 弹出已完成的报文
    void PopComplete();

    // 当前报文header中是否带Expect: 100-continue且尚未应答(server侧在报文收齐前检查并应答一次)
    bool HasExpect100Pending() const;
    // 消费100-continue应答标记(应答后调用)
    void ConsumeExpect100();

    LibString ToString() const;

private:
    // 更新解析状态 返回: Status::Success成功, 其他为错误码
    Int32 _UpdateState();
    // 解析header区, 提取content-length/transfer-encoding 返回: Status::Success成功, 其他为错误码
    Int32 _ParseHeaders(UInt64 headerEndPos);
    // 检查chunked编码是否完整 返回: 完整报文字节数, 0表示数据不足
    UInt64 _CheckChunkedComplete(UInt64 bodyStart) const;

private:
    LibString _buf;
    bool _parseRequest;
    UInt64 _maxHeaderBytes;
    UInt64 _maxBodyBytes;

    UInt64 _headerBytes;        // header区字节数(含\r\n\r\n), 0表示header未收齐
    bool _chunked;              // transfer-encoding: chunked
    UInt64 _contentLength;      // content-length
    UInt64 _completeBytes;      // 完整报文字节数, 0表示未完整
    bool _expect100;            // header带Expect: 100-continue且未应答
};

ALWAYS_INLINE bool HttpParser::HasExpect100Pending() const
{
    return _expect100;
}

ALWAYS_INLINE void HttpParser::ConsumeExpect100()
{
    _expect100 = false;
}

ALWAYS_INLINE bool HttpParser::HasCompleteMessage() const
{
    return _completeBytes != 0;
}

ALWAYS_INLINE UInt64 HttpParser::GetCompleteBytes() const
{
    return _completeBytes;
}

ALWAYS_INLINE const Byte8 *HttpParser::GetData() const
{
    return _buf.data();
}

ALWAYS_INLINE UInt64 HttpParser::GetDataSize() const
{
    return static_cast<UInt64>(_buf.size());
}

KERNEL_END

#endif
