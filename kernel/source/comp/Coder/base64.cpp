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
 * Date: 2021-02-06 21:18:58
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Coder/base64.h>

KERNEL_BEGIN

const char __b64Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                             "abcdefghijklmnopqrstuvwxyz"
                             "0123456789+/";


ALWAYS_INLINE void __A3ToA4(U8 *a3, U8 *a4)
{
    a4[0] = (a3[0] & 0xfc) >> 2;
    a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
    a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
    a4[3] = (a3[2] & 0x3f);
}

ALWAYS_INLINE void __A4ToA3(U8 *a4, U8 *a3)
{
    a3[0] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
    a3[1] = ((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2);
    a3[2] = ((a4[2] & 0x3) << 6) + a4[3];
}

ALWAYS_INLINE U8 __B64Lookup(Byte8 c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    else if (c >= 'a' && c <= 'z') return c - 71;
    else if (c >= '0' && c <= '9') return c + 4;
    else if (c == '+') return 62;
    else if (c == '/') return 63;
    else return -1;
}

KERNEL_END


KERNEL_BEGIN

bool LibBase64::Encode(const Byte8 *data, UInt64 len, Byte8 *outBuffer, UInt64 &outSize)
{
    if (UNLIKELY(outSize < CalcEncodeLen(len)))
    {
        CRYSTAL_TRACE("outBuffer size[%llu], not enough need[%llu]", outSize, CalcEncodeLen(len));
        return false;
    }

    U8 a3[3];
    U8 a4[4];
    Int32 i = 0, j;
    outSize = 0;
    while (len--)
    {
        a3[i++] = *(data++);
        if (i == 3)
        {
            __A3ToA4(a3, a4);
            for (i = 0; i < 4; ++i)
                outBuffer[outSize++] = __b64Alphabet[a4[i]];

            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j < 3; ++j)
            a3[j] = '\0';

        __A3ToA4(a3, a4);
        for (j = 0; j < i + 1; ++j)
            outBuffer[outSize++] = __b64Alphabet[a4[j]];

        while ((i++ < 3))
            outBuffer[outSize++] = CRYSTAL_BASE64_PADDING;
    }

    outBuffer[outSize] = '\0';
    return true;
}

bool LibBase64::Decode(const Byte8 *data, UInt64 inputLen, Byte8 *buffer, UInt64 &len)
{
    if (UNLIKELY(len < CalcDecodeLen(data, inputLen)))
    {
        CRYSTAL_TRACE("CalcDecodeLen fail buffer len[%llu] is enough need[%llu]", len, CalcDecodeLen(data, inputLen));
        return false;
    }

    int i = 0, j;
    unsigned char a3[3];
    unsigned char a4[4];

    len = 0;
    while (inputLen--)
    {
        if (*data == CRYSTAL_BASE64_PADDING)
            break;

        a4[i++] = *(data++);
        if (i == 4)
        {
            for (i = 0; i <4; ++i)
                a4[i] = __B64Lookup(a4[i]);

            __A4ToA3(a4, a3);
            for (i = 0; i < 3; ++i)
                buffer[len++] = a3[i];

            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j < 4; ++j)
            a4[j] = '\0';

        for (j = 0; j <4; ++j)
            a4[j] = __B64Lookup(a4[j]);

        __A4ToA3(a4, a3);

        for (j = 0; j < i - 1; ++j)
            buffer[len++] = a3[j];
    }

    buffer[len] = '\0';
    return true;
}

KERNEL_END