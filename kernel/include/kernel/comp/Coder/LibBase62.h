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
 * Date: 2026-07-16 01:30:00
 * Author: Eric Yonng
 * Description: Base62编码工具类. 将UInt64与11字符Base62字符串互相转换.
 *              字符表: 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz
 *              62^11 > 2^63, 11字符可覆盖整个UInt64空间.
 *              注意: 本类不包含FF1加密, 仅做纯Base62编码. 加密请使用ShortIdGenerator.
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CODER_LIB_BASE62_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CODER_LIB_BASE62_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/common/BaseType.h>
#include <kernel/comp/LibString.h>

#include <limits.h>

KERNEL_BEGIN

class KERNEL_EXPORT LibBase62
{
public:
    static constexpr Int32 RADIX = 62;
    static constexpr Int32 SHORT_ID_LEN = 11;  // 11个字符覆盖UInt64全空间
    // UInt64 最大值, 与项目 BigNum.h 风格一致
    static constexpr UInt64 MAX_UINT64 = 0xFFFFFFFFFFFFFFFFLLU;

    // UInt64 → 指定长度的radix-62数字数组(不足前导补0)
    // digits[0]为最高位, digits[len-1]为最低位
    static void ToRadixArray(UInt64 value, unsigned int *digits, Int32 len = SHORT_ID_LEN);
    // radix-62数字数组 → UInt64
    static bool FromRadixArray(const unsigned int *digits, UInt64 &result, Int32 len = SHORT_ID_LEN);

    // UInt64 → 11字符Base62字符串(不含FF1加密)
    static LibString Encode(UInt64 value);
    // 11字符Base62字符串 → UInt64, 成功返回true
    static bool Decode(const LibString &str, UInt64 &value);

    // 数字(0-61) → Base62字符, 非法digit返回'\0'
    static char DigitToChar(unsigned int digit);
    // Base62字符 → 数字(0-61), 非法字符返回UINT_MAX
    static unsigned int CharToDigit(char c);

private:
    static const char *GetCharset();
};

ALWAYS_INLINE const char *LibBase62::GetCharset()
{
    return "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
}

ALWAYS_INLINE char LibBase62::DigitToChar(unsigned int digit)
{
    if (digit >= 62)
        return '\0';
    return GetCharset()[digit];
}

ALWAYS_INLINE unsigned int LibBase62::CharToDigit(char c)
{
    if (c >= '0' && c <= '9')
        return static_cast<unsigned int>(c - '0');
    if (c >= 'A' && c <= 'Z')
        return static_cast<unsigned int>(c - 'A' + 10);
    if (c >= 'a' && c <= 'z')
        return static_cast<unsigned int>(c - 'a' + 36);
    return UINT_MAX;
}

KERNEL_END

#endif
