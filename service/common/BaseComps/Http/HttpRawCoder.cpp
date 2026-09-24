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
 * Description: 原始字节coder实现
*/

#include <pch.h>
#include <kernel/comp/Http/HttpRawCoder.h>
#include <kernel/comp/LibStream.h>

KERNEL_BEGIN

void HttpRawCoder::Release()
{
    HttpRawCoder::Delete_HttpRawCoder(this);
}

bool HttpRawCoder::Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) const
{
    return stream.Write(_data.data(), static_cast<Int64>(_data.size()));
}

bool HttpRawCoder::Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) const
{
    return stream.Write(_data.data(), static_cast<Int64>(_data.size()));
}

bool HttpRawCoder::Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream)
{
    _data.assign(stream.GetReadBegin(), static_cast<UInt64>(stream.GetReadableSize()));
    return true;
}

bool HttpRawCoder::Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream)
{
    _data.assign(stream.GetReadBegin(), static_cast<UInt64>(stream.GetReadableSize()));
    return true;
}

bool HttpRawCoder::Decode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream)
{
    _data.assign(stream.GetReadBegin(), static_cast<UInt64>(stream.GetReadableSize()));
    return true;
}

bool HttpRawCoder::Decode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream)
{
    _data.assign(stream.GetReadBegin(), static_cast<UInt64>(stream.GetReadableSize()));
    return true;
}

LibString HttpRawCoder::ToString() const
{
    LibString info;
    info.AppendFormat("raw bytes:%llu", static_cast<UInt64>(_data.size()));
    return info;
}

KERNEL_END
