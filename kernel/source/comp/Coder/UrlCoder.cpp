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
 * Date: 2023-11-04 21:33:50
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Coder/UrlCoder.h>

KERNEL_BEGIN

static const Byte8 RFC3986_ReservedChars[] = {':', '/', '?', '#', '[', ']', '@', '!', '$', '&', '\'', '(', ')', '*', '+', ',', ';', '='};
static const Byte8 RFC3986_UnReservedSpecialChars[] = {'"', '-', '_', '.', '~'};
static const Int32 ReservedCharLen = sizeof(RFC3986_ReservedChars);
static const Int32 UnReservedCharLen = sizeof(RFC3986_UnReservedSpecialChars);

static const Byte8 ToHexChars[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
static Byte8 HexToDecimalValues[128] = {-1};

static ALWAYS_INLINE bool IsReservedChars(Byte8 c)
{
    auto ptr = RFC3986_ReservedChars;
    for(Int32 idx = 0; idx < ReservedCharLen; ++idx)
    {
        if(c == *ptr++)
            return true;
    }

    return false;
}

static ALWAYS_INLINE bool IsUnReservedChars(Byte8 c)
{
    // 数值和字母
    if( ((c >= 'A') && (c <= 'Z'))
        || ((c >= 'a') && (c <= 'z')) || 
        ((c >= '0') && (c <= '9')))
        return true;

    // 特殊字符
    auto ptr = RFC3986_UnReservedSpecialChars;
    for(Int32 idx = 0; idx < UnReservedCharLen; ++idx)
    {
        if(c == *ptr++)
            return true;
    }

    return false;
}

void UrlCoder::Init()
{
    // 16进制转码
    HexToDecimalValues[Int32('0')] = 0;
    HexToDecimalValues[Int32('1')] = 1;
    HexToDecimalValues[Int32('2')] = 2;
    HexToDecimalValues[Int32('3')] = 3;
    HexToDecimalValues[Int32('4')] = 4;
    HexToDecimalValues[Int32('5')] = 5;
    HexToDecimalValues[Int32('6')] = 6;
    HexToDecimalValues[Int32('7')] = 7;
    HexToDecimalValues[Int32('8')] = 8;
    HexToDecimalValues[Int32('9')] = 9;
    HexToDecimalValues[Int32('A')] = 10;
    HexToDecimalValues[Int32('B')] = 11;
    HexToDecimalValues[Int32('C')] = 12;
    HexToDecimalValues[Int32('D')] = 13;
    HexToDecimalValues[Int32('E')] = 14;
    HexToDecimalValues[Int32('F')] = 15;
}

// 除RFC3986_ReservedChars/RFC3986_UnReservedChars 之外都要百分号编码
bool UrlCoder::Encode(const Byte8 *src, UInt64 srcLen, Byte8 *target, UInt64 &targetLen, bool turnSpaceToPlusChar)
{
    // 保留字符/非保留字符不需要编码
    targetLen = 0;
    for(UInt64 idx = 0; idx < srcLen; ++idx)
    {
        // 不转码
        if(IsReservedChars(*src) || IsUnReservedChars(*src))
        {
            *(target++) = *(src++);
            ++targetLen;
            continue;
        }

        if(UNLIKELY(turnSpaceToPlusChar && (*src == ' ')))
        {
            *target++ = '+';
            ++targetLen;
            ++src;
            continue;
        }

        // 16进制编码
        *target++ = '%';
        // 高4位转16进制
        *target++ = ToHexChars[((U8)(*src)) >> 4];
        // 低4位转16进制
        *target++ = ToHexChars[(*src) & 0X0F];
        ++src;
        targetLen += 3;
    }

    return true;
}

bool UrlCoder::Decode(const Byte8 *src, UInt64 srcLen, Byte8 *target, UInt64 &targetLen, bool hasSpaceTurnToPlusChar)
{
    targetLen = 0;
    for(UInt64 idx = 0; idx < srcLen; )
    {
        if(*src == '%')
        {// 需要转码
            if(UNLIKELY((idx + 2) >= srcLen))
                return false;

            ++src;

            auto &hi = HexToDecimalValues[static_cast<Int32>(*src++)];
            auto &low = HexToDecimalValues[static_cast<Int32>(*src++)];
            if(UNLIKELY(hi < 0 || low < 0))
                return false;

            *(target++) = (hi << 4) | low;
            ++targetLen;
            idx += 3;
            continue;
        }

        if(UNLIKELY(hasSpaceTurnToPlusChar && (*src == '+')))
        {
            ++src;
            *target++ = ' ';
            ++targetLen;
            ++idx;
            continue;
        }

        // 既不是保留的又不是非保留的就是错误的
        if(UNLIKELY(!IsReservedChars(*src) && !IsUnReservedChars(*src)))
            return false;

        *(target++) = *(src++);
        ++targetLen;
        ++idx;
    }

    return true;
}


KERNEL_END
