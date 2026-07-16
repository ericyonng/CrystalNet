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
 * Description: 测试ShortId模块实现.
*/

#include <pch.h>
#include <testsuit/testinst/TestShortId.h>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
  #include <WinSock2.h>
#endif

#include <kernel/comp/Coder/LibBase62.h>
#include <kernel/comp/Coder/ShortIdGenerator.h>
#include <kernel/comp/Encrypt/LibFF1.h>
#include <kernel/comp/Log.h>
#include <kernel/comp/LibString.h>

#include <set>
#include <cstring>
#include <cstdlib>

// 测试用256位密钥(与NIST测试向量中的AES-256密钥一致, 仅用于测试)
static const char *TEST_KEY_HEX = "2B7E151628AED2A6ABF7158809CF4F3CEF4359D8D580AA4F7F036D6F04FC6A94";
// 测试用tweak(与NIST测试向量一致)
static const char *TEST_TWEAK_HEX = "39383736353433323130";

// NIST测试向量中字符到数字的映射(radix<=36时):
// '0'-'9' → 0-9, 'a'-'z' → 10-35
static unsigned int NistCharToDigit(char c)
{
    if (c >= '0' && c <= '9')
        return static_cast<unsigned int>(c - '0');
    if (c >= 'a' && c <= 'z')
        return static_cast<unsigned int>(c - 'a' + 10);
    return UINT_MAX;
}

static char NistDigitToChar(unsigned int d)
{
    if (d < 10)
        return static_cast<char>('0' + d);
    return static_cast<char>('a' + d - 10);
}

// 十六进制字符串转字节数组
static Int32 HexToBytes(const char *hex, Byte8 *out, Int32 maxOut)
{
    Int32 hexLen = static_cast<Int32>(strlen(hex));
    Int32 byteLen = hexLen / 2;
    if (byteLen > maxOut)
        return -1;

    for (Int32 i = 0; i < byteLen; ++i)
    {
        char tmp[3] = {hex[i * 2], hex[i * 2 + 1], 0};
        out[i] = static_cast<Byte8>(strtol(tmp, nullptr, 16));
    }
    return byteLen;
}

void TestShortId::Run()
{
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestShortId, "=================== ShortId Test Start ==================="));

    Int32 passCount = 0;
    Int32 failCount = 0;

    // ================================================================
    // 1. NIST FF1 官方测试向量验证
    // ================================================================
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestShortId, "--- 1. NIST FF1 Official Test Vectors ---"));

    // NIST FF1 测试向量: [radix, keyHex, tweakHex, plaintext, expectedCiphertext]
    struct NistTestVector
    {
        int radix;
        const char *keyHex;
        const char *tweakHex;
        const char *plaintext;
        const char *ciphertext;
    };

    NistTestVector nistVectors[] = {
        // AES-128
        {10, "2B7E151628AED2A6ABF7158809CF4F3C", "", "0123456789", "2433477484"},
        {10, "2B7E151628AED2A6ABF7158809CF4F3C", "39383736353433323130", "0123456789", "6124200773"},
        {36, "2B7E151628AED2A6ABF7158809CF4F3C", "3737373770717273373737", "0123456789abcdefghi", "a9tv40mll9kdu509eum"},
        // AES-192
        {10, "2B7E151628AED2A6ABF7158809CF4F3CEF4359D8D580AA4F", "", "0123456789", "2830668132"},
        {10, "2B7E151628AED2A6ABF7158809CF4F3CEF4359D8D580AA4F", "39383736353433323130", "0123456789", "2496655549"},
        {36, "2B7E151628AED2A6ABF7158809CF4F3CEF4359D8D580AA4F", "3737373770717273373737", "0123456789abcdefghi", "xbj3kv35jrawxv32ysr"},
        // AES-256
        {10, "2B7E151628AED2A6ABF7158809CF4F3CEF4359D8D580AA4F7F036D6F04FC6A94", "", "0123456789", "6657667009"},
        {10, "2B7E151628AED2A6ABF7158809CF4F3CEF4359D8D580AA4F7F036D6F04FC6A94", "39383736353433323130", "0123456789", "1001623463"},
        {36, "2B7E151628AED2A6ABF7158809CF4F3CEF4359D8D580AA4F7F036D6F04FC6A94", "3737373770717273373737", "0123456789abcdefghi", "xs8a0azh2avyalyzuwd"},
    };

    Int32 numNistVectors = static_cast<Int32>(sizeof(nistVectors) / sizeof(nistVectors[0]));
    for (Int32 idx = 0; idx < numNistVectors; ++idx)
    {
        const auto &tv = nistVectors[idx];

        // 解析key
        Byte8 key[64];
        Int32 keyLen = HexToBytes(tv.keyHex, key, sizeof(key));
        Int32 keyBits = keyLen * 8;

        // 解析tweak
        Byte8 tweak[64];
        Int32 tweakLen = 0;
        if (strlen(tv.tweakHex) > 0)
        {
            tweakLen = HexToBytes(tv.tweakHex, tweak, sizeof(tweak));
        }

        // 映射plaintext → 数字数组
        Int32 ptLen = static_cast<Int32>(strlen(tv.plaintext));
        unsigned int inDigits[256];
        unsigned int outDigits[256];
        for (Int32 i = 0; i < ptLen; ++i)
        {
            inDigits[i] = NistCharToDigit(tv.plaintext[i]);
        }

        // FF1加密
        Int32 ret = KERNEL_NS::LibFF1::Encrypt(key, keyBits,
            tweakLen > 0 ? tweak : nullptr, static_cast<UInt32>(tweakLen),
            inDigits, outDigits, static_cast<UInt32>(ptLen), tv.radix);

        if (ret != Status::Success)
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(TestShortId, "NIST FF1 encrypt fail case#%d err:%d"), idx, ret);
            ++failCount;
            continue;
        }

        // 映射output → 字符串
        char resultStr[256];
        for (Int32 i = 0; i < ptLen; ++i)
        {
            resultStr[i] = NistDigitToChar(outDigits[i]);
        }
        resultStr[ptLen] = '\0';

        bool match = (strcmp(resultStr, tv.ciphertext) == 0);
        if (match)
        {
            ++passCount;
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestShortId, "NIST FF1 case#%d PASS: radix=%d keyBits=%d plain=%s cipher=%s"),
                idx, tv.radix, keyBits, tv.plaintext, resultStr);
        }
        else
        {
            ++failCount;
            g_Log->Error(LOGFMT_NON_OBJ_TAG(TestShortId, "NIST FF1 case#%d FAIL: radix=%d expected=%s got=%s"),
                idx, tv.radix, tv.ciphertext, resultStr);
        }

        // 验证解密往返
        unsigned int decDigits[256];
        ret = KERNEL_NS::LibFF1::Decrypt(key, keyBits,
            tweakLen > 0 ? tweak : nullptr, static_cast<UInt32>(tweakLen),
            outDigits, decDigits, static_cast<UInt32>(ptLen), tv.radix);

        if (ret != Status::Success)
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(TestShortId, "NIST FF1 decrypt fail case#%d err:%d"), idx, ret);
            ++failCount;
            continue;
        }

        char decStr[256];
        for (Int32 i = 0; i < ptLen; ++i)
        {
            decStr[i] = NistDigitToChar(decDigits[i]);
        }
        decStr[ptLen] = '\0';

        bool decryptMatch = (strcmp(decStr, tv.plaintext) == 0);
        if (decryptMatch)
        {
            ++passCount;
        }
        else
        {
            ++failCount;
            g_Log->Error(LOGFMT_NON_OBJ_TAG(TestShortId, "NIST FF1 decrypt round-trip FAIL case#%d: expected=%s got=%s"),
                idx, tv.plaintext, decStr);
        }
    }

    // ================================================================
    // 2. Base62 编解码往返测试
    // ================================================================
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestShortId, "--- 2. Base62 Encode/Decode Round-Trip ---"));

    struct Base62TestCase
    {
        Int64 id;
        const char *desc;
    };

    Base62TestCase b62Cases[] = {
        {0, "zero"},
        {1, "one"},
        {61, "max_digit_61"},
        {62, "first_carry"},
        {3844, "62^2"},
        {238328, "62^3"},
        {916132832, "62^5"},
        {68719476735, "close_to_2^36"},
        {9223372036854775807LL, "INT64_MAX"},
        {1234567890123456789LL, "random_large"},
        {8589934591LL, "random_mid"},
        {42, "answer"},
    };

    Int32 numB62 = static_cast<Int32>(sizeof(b62Cases) / sizeof(b62Cases[0]));
    for (Int32 i = 0; i < numB62; ++i)
    {
        Int64 id = b62Cases[i].id;
        KERNEL_NS::LibString encoded = KERNEL_NS::LibBase62::Encode(id);

        // 验证长度
        if (static_cast<Int32>(encoded.size()) != KERNEL_NS::LibBase62::SHORT_ID_LEN)
        {
            ++failCount;
            g_Log->Error(LOGFMT_NON_OBJ_TAG(TestShortId, "Base62 encode wrong length for %s: expected %d got %llu"),
                b62Cases[i].desc, KERNEL_NS::LibBase62::SHORT_ID_LEN, static_cast<UInt64>(encoded.size()));
            continue;
        }

        // 验证解码
        Int64 decoded;
        bool ok = KERNEL_NS::LibBase62::Decode(encoded, decoded);
        if (!ok || decoded != id)
        {
            ++failCount;
            g_Log->Error(LOGFMT_NON_OBJ_TAG(TestShortId, "Base62 round-trip FAIL for %s: id=%lld encoded=%s decoded=%lld ok=%d"),
                b62Cases[i].desc, static_cast<long long>(id), encoded.c_str(), static_cast<long long>(decoded), ok);
        }
        else
        {
            ++passCount;
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestShortId, "Base62 case '%s' PASS: id=%lld → %s → %lld"),
                b62Cases[i].desc, static_cast<long long>(id), encoded.c_str(), static_cast<long long>(decoded));
        }
    }

    // 测试非法字符解码
    {
        KERNEL_NS::LibString badStr = "ABCDEFGHIJK"; // 11 chars, all valid
        Int64 val;
        bool ok = KERNEL_NS::LibBase62::Decode(badStr, val);
        if (ok)
        {
            ++passCount;
        }
        else
        {
            ++failCount;
            g_Log->Error(LOGFMT_NON_OBJ_TAG(TestShortId, "Base62 decode valid string FAIL: %s"), badStr.c_str());
        }

        // 非法字符
        KERNEL_NS::LibString badChar = "ABCDEFGHIJ!"; // 11 chars, last is invalid
        ok = KERNEL_NS::LibBase62::Decode(badChar, val);
        if (!ok)
        {
            ++passCount;
        }
        else
        {
            ++failCount;
            g_Log->Error(LOGFMT_NON_OBJ_TAG(TestShortId, "Base62 decode invalid char should fail but didn't: %s"), badChar.c_str());
        }

        // 长度不对
        KERNEL_NS::LibString shortStr = "ABC";
        ok = KERNEL_NS::LibBase62::Decode(shortStr, val);
        if (!ok)
        {
            ++passCount;
        }
        else
        {
            ++failCount;
            g_Log->Error(LOGFMT_NON_OBJ_TAG(TestShortId, "Base62 decode wrong length should fail but didn't: %s"), shortStr.c_str());
        }
    }

    // ================================================================
    // 3. ShortIdGenerator 加密-解密往返测试
    // ================================================================
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestShortId, "--- 3. ShortIdGenerator Encrypt/Decrypt Round-Trip ---"));

    // 准备密钥和tweak
    Byte8 key[32];
    HexToBytes(TEST_KEY_HEX, key, sizeof(key));
    Byte8 tweak[16];
    Int32 tweakLen = HexToBytes(TEST_TWEAK_HEX, tweak, sizeof(tweak));
    Int32 keyBits = 256;

    struct ShortIdTestCase
    {
        Int64 id;
        const char *desc;
    };

    ShortIdTestCase shortIdCases[] = {
        {0, "zero"},
        {1, "one"},
        {42, "answer"},
        {999999999LL, "small"},
        {1000000000LL, "1B"},
        {1234567890123LL, "mid"},
        {9223372036854775807LL, "INT64_MAX"},
        {6778338729212112781LL, "snowflake_sample"},
        {4567890123456789012LL, "random_large_2"},
        {1LL, "one_again"},
    };

    Int32 numShortId = static_cast<Int32>(sizeof(shortIdCases) / sizeof(shortIdCases[0]));
    for (Int32 i = 0; i < numShortId; ++i)
    {
        Int64 id = shortIdCases[i].id;
        KERNEL_NS::LibString shortId = KERNEL_NS::ShortIdGenerator::Generate(id, key, keyBits, tweak, static_cast<UInt32>(tweakLen));

        if (shortId.empty())
        {
            ++failCount;
            g_Log->Error(LOGFMT_NON_OBJ_TAG(TestShortId, "ShortId Generate FAIL for %s: id=%lld returned empty"),
                shortIdCases[i].desc, static_cast<long long>(id));
            continue;
        }

        // 验证长度
        if (static_cast<Int32>(shortId.size()) != KERNEL_NS::LibBase62::SHORT_ID_LEN)
        {
            ++failCount;
            g_Log->Error(LOGFMT_NON_OBJ_TAG(TestShortId, "ShortId Generate wrong length for %s: expected %d got %llu"),
                shortIdCases[i].desc, KERNEL_NS::LibBase62::SHORT_ID_LEN, static_cast<UInt64>(shortId.size()));
            continue;
        }

        // 解密验证
        Int64 parsedId;
        Int32 ret = KERNEL_NS::ShortIdGenerator::Parse(shortId, parsedId, key, keyBits, tweak, static_cast<UInt32>(tweakLen));

        if (ret != KERNEL_NS::Status::Success || parsedId != id)
        {
            ++failCount;
            g_Log->Error(LOGFMT_NON_OBJ_TAG(TestShortId, "ShortId round-trip FAIL for %s: id=%lld shortId=%s parsedId=%lld ret=%d"),
                shortIdCases[i].desc, static_cast<long long>(id), shortId.c_str(), static_cast<long long>(parsedId), ret);
        }
        else
        {
            ++passCount;
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestShortId, "ShortId case '%s' PASS: id=%lld → shortId=%s → parsedId=%lld"),
                shortIdCases[i].desc, static_cast<long long>(id), shortId.c_str(), static_cast<long long>(parsedId));
        }
    }

    // 测试空tweak的情况
    {
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestShortId, "--- 3b. ShortIdGenerator with empty tweak ---"));
        Int64 id = 1234567890123LL;
        KERNEL_NS::LibString shortId = KERNEL_NS::ShortIdGenerator::Generate(id, key, keyBits, nullptr, 0);

        if (shortId.empty())
        {
            ++failCount;
            g_Log->Error(LOGFMT_NON_OBJ_TAG(TestShortId, "ShortId with empty tweak Generate FAIL id=%lld"), static_cast<long long>(id));
        }
        else
        {
            Int64 parsedId;
            Int32 ret = KERNEL_NS::ShortIdGenerator::Parse(shortId, parsedId, key, keyBits, nullptr, 0);
            if (ret == KERNEL_NS::Status::Success && parsedId == id)
            {
                ++passCount;
                g_Log->Info(LOGFMT_NON_OBJ_TAG(TestShortId, "ShortId empty tweak PASS: id=%lld → %s → %lld"),
                    static_cast<long long>(id), shortId.c_str(), static_cast<long long>(parsedId));
            }
            else
            {
                ++failCount;
                g_Log->Error(LOGFMT_NON_OBJ_TAG(TestShortId, "ShortId empty tweak round-trip FAIL: id=%lld shortId=%s parsedId=%lld ret=%d"),
                    static_cast<long long>(id), shortId.c_str(), static_cast<long long>(parsedId), ret);
            }
        }
    }

    // ================================================================
    // 4. 唯一性测试(不同ID生成不同短ID)
    // ================================================================
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestShortId, "--- 4. Uniqueness Test ---"));

    std::set<KERNEL_NS::LibString> shortIdSet;
    Int32 uniquenessCount = 100000;
    Int32 duplicateCount = 0;

    for (Int32 i = 0; i < uniquenessCount; ++i)
    {
        Int64 id = static_cast<Int64>(i) * 92233720368547LL + i * 7 + 1;
        KERNEL_NS::LibString shortId = KERNEL_NS::ShortIdGenerator::Generate(id, key, keyBits, tweak, static_cast<UInt32>(tweakLen));

        if (shortId.empty())
        {
            ++failCount;
            g_Log->Error(LOGFMT_NON_OBJ_TAG(TestShortId, "ShortId Generate empty at uniqueness test i=%d"), i);
            continue;
        }

        auto result = shortIdSet.insert(shortId);
        if (!result.second)
        {
            ++duplicateCount;
            if (duplicateCount <= 5)
            {
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestShortId, "Duplicate shortId detected: id=%lld shortId=%s"),
                    static_cast<long long>(id), shortId.c_str());
            }
        }
    }

    if (duplicateCount == 0)
    {
        ++passCount;
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestShortId, "Uniqueness test PASS: %d IDs all generated unique shortIds"), uniquenessCount);
    }
    else
    {
        ++failCount;
        g_Log->Error(LOGFMT_NON_OBJ_TAG(TestShortId, "Uniqueness test FAIL: %d duplicates out of %d"), duplicateCount, uniquenessCount);
    }

    // ================================================================
    // 5. 连续ID不产生连续短ID(加密扩散性验证)
    // ================================================================
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestShortId, "--- 5. Cipher Diffusion Test ---"));

    Int64 baseId = 1000000;
    KERNEL_NS::LibString baseShortId = KERNEL_NS::ShortIdGenerator::Generate(baseId, key, keyBits, tweak, static_cast<UInt32>(tweakLen));
    KERNEL_NS::LibString nextShortId = KERNEL_NS::ShortIdGenerator::Generate(baseId + 1, key, keyBits, tweak, static_cast<UInt32>(tweakLen));

    Int32 diffCount = 0;
    for (Int32 i = 0; i < KERNEL_NS::LibBase62::SHORT_ID_LEN; ++i)
    {
        if (baseShortId[i] != nextShortId[i])
            ++diffCount;
    }

    if (diffCount >= 5) // 加密后至少5个字符不同(11个字符中)
    {
        ++passCount;
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestShortId, "Diffusion test PASS: id=%lld → %s, id=%lld → %s, %d chars differ"),
            static_cast<long long>(baseId), baseShortId.c_str(),
            static_cast<long long>(baseId + 1), nextShortId.c_str(), diffCount);
    }
    else
    {
        ++failCount;
        g_Log->Error(LOGFMT_NON_OBJ_TAG(TestShortId, "Diffusion test FAIL: consecutive IDs only differ in %d chars"), diffCount);
    }

    // ================================================================
    // 结果汇总
    // ================================================================
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestShortId, "=================== ShortId Test Summary ==================="));
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestShortId, "PASS: %d, FAIL: %d"), passCount, failCount);
    if (failCount == 0)
    {
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestShortId, "ALL TESTS PASSED!"));
    }
    else
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(TestShortId, "SOME TESTS FAILED!"));
    }
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestShortId, "=================== ShortId Test End ======================="));
}
