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
 * Description: Base62编码工具类实现.
*/

#include <pch.h>
#include <kernel/comp/Coder/LibBase62.h>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
  #include <WinSock2.h>
#endif

#include <limits.h>
#include <cstring>

KERNEL_BEGIN

void LibBase62::ToRadixArray(Int64 value, unsigned int *digits, Int32 len)
{
    // 处理负数: 取绝对值
    UInt64 uv = static_cast<UInt64>(value < 0 ? -value : value);

    for (Int32 i = len - 1; i >= 0; --i)
    {
        digits[i] = static_cast<unsigned int>(uv % RADIX);
        uv /= RADIX;
    }
}

Int64 LibBase62::FromRadixArray(const unsigned int *digits, Int32 len)
{
    UInt64 result = 0;
    for (Int32 i = 0; i < len; ++i)
    {
        result = result * RADIX + digits[i];
    }
    return static_cast<Int64>(result);
}

LibString LibBase62::Encode(Int64 value)
{
    unsigned int digits[SHORT_ID_LEN];
    ToRadixArray(value, digits, SHORT_ID_LEN);

    LibString result;
    result.resize(SHORT_ID_LEN);
    char *buf = const_cast<char *>(result.data());
    for (Int32 i = 0; i < SHORT_ID_LEN; ++i)
    {
        buf[i] = DigitToChar(digits[i]);
    }
    return result;
}

bool LibBase62::Decode(const LibString &str, Int64 &value)
{
    if (static_cast<Int32>(str.size()) != SHORT_ID_LEN)
        return false;

    unsigned int digits[SHORT_ID_LEN];
    for (Int32 i = 0; i < SHORT_ID_LEN; ++i)
    {
        unsigned int d = CharToDigit(str[i]);
        if (d == UINT_MAX)
            return false;
        digits[i] = d;
    }

    value = FromRadixArray(digits, SHORT_ID_LEN);
    return true;
}

KERNEL_END
