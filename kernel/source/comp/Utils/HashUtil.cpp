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
 * Date: 2026-06-18 18:17:02
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Utils/HashUtil.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

static KERNEL_NS::LibString __g_HashAlgoEnumStrs[KERNEL_NS::HashAlgorithm::End + 1] = {
    "BKDR",
    "DJB",
    "SDBM",
    "RS",
    "JS",
    "PJW",
    "ELF",
    "AP",
    "MurmurHash3",

    "Unknown"
};

LibString HashAlgorithm::GetEnumStr(int hashAlgo)
{
    return hashAlgo >= Begin && hashAlgo < End ? __g_HashAlgoEnumStrs[hashAlgo] : __g_HashAlgoEnumStrs[HashAlgorithm::End];
}

UInt32 HashUtil::BKDRHash(const void * bytes, size_t size)
{
    constexpr UInt32 seed = 131; // 31 131 1313 13131 131313

    UInt32 hash = 0;
    const U8 *u8Buf = reinterpret_cast<const U8 *>(bytes);
    for (size_t i = 0; i < size; ++i)
        hash = hash * seed + u8Buf[i];

    return hash;
}

UInt32 HashUtil::DJBHash(const void *bytes, size_t size)
{
    UInt32 hash = 5381;
    const U8 *u8Bytes = reinterpret_cast<const U8 *>(bytes);
    for (size_t i = 0; i < size; ++i)
    {
        // Equivalent to: hash = hash[i - 1] * 33 + str[i]
        hash += (hash << 5) + u8Bytes[i];
    }

    return hash;
}

UInt32 HashUtil::SDBMHash(const void *bytes, size_t size)
{
    UInt32 hash = 0;
    const U8 *u8Bytes = reinterpret_cast<const U8 *>(bytes);
    for (size_t i = 0; i < size; ++i)
    {
        // Equivalent to: hash = 65599 * hash + (*bytes++);
        hash = u8Bytes[i] + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

UInt32 HashUtil::RSHash(const void *bytes, size_t size)
{
    UInt32 a = 63689;
    constexpr UInt32 b = 378551;

    UInt32 hash = 0;
    const U8 *u8Bytes = reinterpret_cast<const U8 *>(bytes);
    for (size_t i = 0; i < size; ++i)
    {
        hash = hash * a + (u8Bytes[i]);
        a *= b;
    }

    return hash;
}

UInt32 HashUtil::JSHash(const void *bytes, size_t size)
{
    UInt32 hash = 1315423911;
    const U8 *u8Bytes = reinterpret_cast<const U8 *>(bytes);
    for (size_t i = 0; i < size; ++i)
        hash ^= ((hash << 5) + u8Bytes[i] + (hash >> 2));

    return hash;
}

UInt32 HashUtil::PJWHash(const void *bytes, size_t size)
{
    constexpr UInt32 bitsInUInt32 = sizeof(UInt32) * 8;
    constexpr UInt32 threeQuarters = bitsInUInt32 * 3 / 4;
    constexpr UInt32 oneEighth = bitsInUInt32 / 8;
    constexpr UInt32 highBits = 0xffffffff << (bitsInUInt32 - oneEighth);

    UInt32 test;
    UInt32 hash = 0;
    const U8 *u8Bytes = reinterpret_cast<const U8 *>(bytes);
    for (size_t i = 0; i < size; ++i)
    {
        hash = (hash << oneEighth) + u8Bytes[i];
        if((test = hash & highBits) != 0)
            hash = (hash ^ (test >> threeQuarters)) & (~highBits);
    }

    return hash;
}

UInt32 HashUtil::ELFHash(const void *bytes, size_t size)
{
    UInt32 test;
    UInt32 hash = 0;
    const U8 *u8bytes = reinterpret_cast<const U8 *>(bytes);
    for (size_t i = 0; i < size; ++i)
    {
        hash = (hash << 4) + u8bytes[i];
        if((test = hash & 0xF0000000L) != 0)
        {
            hash ^= (test >> 24);
            hash &= ~test;
        }
    }

    return hash;
}

UInt32 HashUtil::APHash(const void *bytes, size_t size)
{
    UInt32 hash = 0;
    const U8 *u8bytes = reinterpret_cast<const U8 *>(bytes);
    for (size_t i = 0; i < size; ++i)
    {
        if((i & 1) == 0)
            hash ^= ((hash << 7) ^ u8bytes[i] ^ (hash >> 3));
        else
            hash ^= (~((hash << 11) ^ u8bytes[i] ^ (hash >> 5)));
    }

    return hash;
}

UInt32 HashUtil::MurmurHash3Hash(const void *bytes, size_t size)
{
    constexpr UInt32 c1 = 0xcc9e2d51;
    constexpr UInt32 c2 = 0x1b873593;
    constexpr UInt32 r1 = 15;
    constexpr UInt32 r2 = 13;
    constexpr UInt32 m = 5;
    constexpr UInt32 n = 0xe6546b64;

    UInt32 hash = 0;
    const U8 *u8Bytes = reinterpret_cast<const U8 *>(bytes);
    for (size_t i = 0; i < size; i += 4)
    {
        UInt32 k = 0;
        for (size_t j = 0; j < 4 && i + j < size; ++j)
            k |= u8Bytes[i + j] << (j * 8);

        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;

        hash ^= k;
        hash = (hash << r2) | (hash >> (32 - r2));
        hash = hash * m + n;
    }

    hash ^= size;
    hash ^= hash >> 16;
    hash *= 0x85ebca6b;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae35;
    hash ^= hash >> 16;

    return hash;
}

KERNEL_END
