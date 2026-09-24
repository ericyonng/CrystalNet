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
 * Description: 原始字节coder: 用于tls记录等需要不经编解码直通socket发送的数据
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_HTTP_HTTP_RAW_CODER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_HTTP_HTTP_RAW_CODER_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/comp/NetEngine/Protocol/ICoder.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class KERNEL_EXPORT HttpRawCoder : public ICoder
{
    POOL_CREATE_OBJ_DEFAULT_P1(ICoder, HttpRawCoder);

public:
    HttpRawCoder() {}
    virtual ~HttpRawCoder() {}
    virtual void Release() override;

    void SetData(const Byte8 *data, UInt64 len);
    void SetData(LibString &&data);
    const LibString &GetData() const;
    LibString &GetData();

    virtual bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) const override;
    virtual bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) const override;

    virtual bool Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) override;
    virtual bool Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) override;
    virtual bool Decode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) override;
    virtual bool Decode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) override;

    virtual LibString ToString() const;

private:
    LibString _data;
};

ALWAYS_INLINE void HttpRawCoder::SetData(const Byte8 *data, UInt64 len)
{
    _data.assign(data, len);
}

ALWAYS_INLINE void HttpRawCoder::SetData(LibString &&data)
{
    _data = std::move(data);
}

ALWAYS_INLINE const LibString &HttpRawCoder::GetData() const
{
    return _data;
}

ALWAYS_INLINE LibString &HttpRawCoder::GetData()
{
    return _data;
}

KERNEL_END

#endif
