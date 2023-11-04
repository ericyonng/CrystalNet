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
 * Date: 2023-11-04 21:20:15
 * Author: Eric Yonng
 * Description: url 编辑码: 保留字符和非保留字符都不转码, 空格默认转码, 除非有特殊要求会转成+, 其他字符都需要转成16进制编码, 并且前缀加上%符
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CODER_URL_CODER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CODER_URL_CODER_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

// 默认RFC3986
class KERNEL_EXPORT UrlCoder
{
public:
    static void Init();
    // 特殊情况才需要空格转换成'+'
    static LibString Encode(const LibString &src, bool turnSpaceToPlusChar = false);
    static bool Encode(const LibString &src, LibString &target, bool turnSpaceToPlusChar = false);
    static bool Encode(const Byte8 *src, UInt64 srcLen, LibString &target, bool turnSpaceToPlusChar = false);
    static bool Encode(const Byte8 *src, UInt64 srcLen, Byte8 *target, UInt64 &targetLen, bool turnSpaceToPlusChar = false);

    static LibString Decode(const LibString &src, bool hasSpaceTurnToPlusChar = false);
    static bool Decode(const LibString &src, LibString &target, bool hasSpaceTurnToPlusChar = false);
    static bool Decode(const Byte8 *src, UInt64 srcLen, LibString &target, bool hasSpaceTurnToPlusChar = false);
    static bool Decode(const Byte8 *src, UInt64 srcLen, Byte8 *target, UInt64 &targetLen, bool hasSpaceTurnToPlusChar = false);
};

ALWAYS_INLINE LibString UrlCoder::Encode(const LibString &src, bool turnSpaceToPlusChar)
{
    LibString target;
    Encode(src, target, turnSpaceToPlusChar);

    return target;
}

ALWAYS_INLINE bool UrlCoder::Encode(const LibString &src, LibString &target, bool turnSpaceToPlusChar)
{
    return Encode(src.data(), static_cast<UInt64>(src.size()), target, turnSpaceToPlusChar);
}

ALWAYS_INLINE bool UrlCoder::Encode(const Byte8 *src, UInt64 srcLen, LibString &target, bool turnSpaceToPlusChar)
{
    UInt64 targetLen = srcLen * 3;
    target.resize(targetLen);
    auto ret = Encode(src, srcLen, target.data(), targetLen, turnSpaceToPlusChar);
    target.RemoveZeroTail();
    return ret;
}

ALWAYS_INLINE LibString UrlCoder::Decode(const LibString &src, bool hasSpaceTurnToPlusChar)
{
    LibString target;
    Decode(src, target, hasSpaceTurnToPlusChar);
    return target;
}

ALWAYS_INLINE bool UrlCoder::Decode(const LibString &src, LibString &target, bool hasSpaceTurnToPlusChar)
{
    return Decode(src.data(), static_cast<UInt64>(src.size()), target, hasSpaceTurnToPlusChar);
}

ALWAYS_INLINE bool UrlCoder::Decode(const Byte8 *src, UInt64 srcLen, LibString &target, bool hasSpaceTurnToPlusChar)
{
    target.resize(srcLen);
    UInt64 targetLen = srcLen;
    auto ret = Decode(src, srcLen, target.data(), targetLen, hasSpaceTurnToPlusChar);
    target.RemoveZeroTail();
    return ret;
}


KERNEL_END

#endif