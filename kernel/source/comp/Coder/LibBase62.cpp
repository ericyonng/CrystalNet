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
#include <kernel/comp/Log/log.h>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
  #include <WinSock2.h>
#endif

#include <limits.h>
#include <cstring>

#include "kernel/comp/Log/LogMacro.h"

KERNEL_BEGIN
    void LibBase62::ToRadixArray(UInt64 value, unsigned int *digits, Int32 len)
{
    for (Int32 i = len - 1; i >= 0; --i)
    {
        digits[i] = static_cast<unsigned int>(value % RADIX);
        value /= RADIX;
    }
}

bool LibBase62::FromRadixArray(const unsigned int *digits, UInt64 &result, Int32 len)
{
    result = 0;
    for (Int32 i = 0; i < len; ++i)
    {
        const unsigned int d = digits[i];

        // 预检查: result * RADIX + d 是否会超出 UInt64 范围
        // 等价于 result > (MAX_UINT64 - d) / RADIX 则溢出
        // 事前检查避免无符号乘加溢出后结果不确定的问题
        if (result > (MAX_UINT64 - d) / RADIX)
        {
            return false;
        }

        result = result * RADIX + d;
    }
    return true;
}

LibString LibBase62::Encode(UInt64 value)
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

bool LibBase62::Decode(const LibString &str, UInt64 &value)
{
    const auto idLen = static_cast<Int32>(str.size());
    if (idLen > SHORT_ID_LEN)
        return false;

    // 这个index是最大的'0'值索引(如果shortId小于 SHORT_ID_LEN, 给前面自动补'0'字符)
    const auto maxZeroIndex = SHORT_ID_LEN - idLen - 1;

    unsigned int digits[SHORT_ID_LEN];
    for (Int32 i = 0; i < SHORT_ID_LEN; ++i)
    {
        unsigned int d = CharToDigit('0');
        if (i > maxZeroIndex)
        {
            d = CharToDigit(str[i - (maxZeroIndex + 1)]);
        }
        
        if (d == UINT_MAX)
            return false;
        digits[i] = d;
    }

    if (!FromRadixArray(digits, value,SHORT_ID_LEN))
    {
        CLOG_WARN_GLOBAL(LibBase62, "FromRadixArray fail str:%s", str.c_str());
        return false;
    }
    
    return true;
}

KERNEL_END
