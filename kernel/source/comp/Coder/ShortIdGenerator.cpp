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
 * Description: 短ID生成器实现.
*/

#include <pch.h>
#include <kernel/comp/Coder/ShortIdGenerator.h>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
  #include <WinSock2.h>
#endif

#include <kernel/comp/Coder/LibBase62.h>
#include <kernel/comp/Encrypt/LibFF1.h>
#include <kernel/comp/Log/log.h>

KERNEL_BEGIN

bool ShortIdGenerator::Generate(UInt64 id,
                                     const Byte8 *key, Int32 keyBits,
                                     const Byte8 *tweak, UInt32 tweakLen, LibString &outStr)
{
    if (UNLIKELY(!key))
    {
        CLOG_ERROR_GLOBAL(ShortIdGenerator, "key is null");
        return false;
    }

    // 1. UInt64 → 11位radix-62数字数组
    unsigned int inDigits[LibBase62::SHORT_ID_LEN];
    LibBase62::ToRadixArray(id, inDigits, LibBase62::SHORT_ID_LEN);

    // 2. FF1加密
    unsigned int outDigits[LibBase62::SHORT_ID_LEN];
    Int32 ret = LibFF1::Encrypt(key, keyBits, tweak, tweakLen,
                                inDigits, outDigits,
                                LibBase62::SHORT_ID_LEN, LibBase62::RADIX);
    if (UNLIKELY(ret != Status::Success))
    {
        CLOG_ERROR_GLOBAL(ShortIdGenerator, "FF1 encrypt fail err:%d id:%llu",  ret, id);
        return false;
    }

    // 3. 数字数组 → Base62字符
    outStr.resize(LibBase62::SHORT_ID_LEN);
    char *buf = const_cast<char *>(outStr.data());
    for (Int32 i = 0; i < LibBase62::SHORT_ID_LEN; ++i)
    {
        buf[i] = LibBase62::DigitToChar(outDigits[i]);
    }

    return !outStr.empty();
}

Int32 ShortIdGenerator::Parse(const LibString &shortId,
                              UInt64 &id,
                              const Byte8 *key, Int32 keyBits,
                              const Byte8 *tweak, UInt32 tweakLen)
{
    if (UNLIKELY(!key))
        return Status::ShortId_InvalidInput;

    const auto idLen = static_cast<Int32>(shortId.size());
    if (idLen > LibBase62::SHORT_ID_LEN)
        return Status::ShortId_InvalidShortIdLen;

    // 这个index是最大的'0'值索引(如果shortId小于 SHORT_ID_LEN, 给前面自动补'0'字符)
    const auto maxZeroIndex = LibBase62::SHORT_ID_LEN - idLen - 1;

    // 1. Base62字符 → 数字数组
    unsigned int inDigits[LibBase62::SHORT_ID_LEN];
    for (Int32 i = 0; i < LibBase62::SHORT_ID_LEN; ++i)
    {
        unsigned int d = LibBase62::CharToDigit('0');
        if (i > maxZeroIndex)
        {
            d = LibBase62::CharToDigit(shortId[i - (maxZeroIndex + 1)]);
        }
        if (d == UINT_MAX)
            return Status::ShortId_InvalidChar;
        inDigits[i] = d;
    }

    // 2. FF1解密
    unsigned int outDigits[LibBase62::SHORT_ID_LEN];
    Int32 ret = LibFF1::Decrypt(key, keyBits, tweak, tweakLen,
                                inDigits, outDigits,
                                LibBase62::SHORT_ID_LEN, LibBase62::RADIX);
    if (UNLIKELY(ret != Status::Success))
    {
        CLOG_ERROR_GLOBAL(ShortIdGenerator, "FF1 decrypt fail err:%d, shortId:%s", ret, shortId.c_str());
        return ret;
    }

    // 3. 数字数组 → UInt64
    if (!LibBase62::FromRadixArray(outDigits, id, LibBase62::SHORT_ID_LEN))
    {
        CLOG_WARN_GLOBAL(ShortIdGenerator, "FromRadixArray fail shortid:%s", shortId.c_str());
        return Status::ShortId_DecodedIdOverflow;
    }

    return Status::Success;
}

KERNEL_END
