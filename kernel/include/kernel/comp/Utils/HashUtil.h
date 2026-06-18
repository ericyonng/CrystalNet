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

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_HASH_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_HASH_UTIL_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/common/BaseType.h>

#include "kernel/comp/LibString.h"

KERNEL_BEGIN

class KERNEL_EXPORT HashAlgorithm
{
public:
  enum ENUMS
  {
      Begin,

      BKDR = Begin,
      DJB,
      SDBM,
      RS,
      JS,
      PJW,
      ELF,
      AP,

      // 最佳, 冲突率低, 性能好
      MurmurHash3,
      // 64bit MurmurHash3
      MurmurHash3_64BIT,
      // 128位
      MurmurHash3_128BIT,

      End,

      Default = MurmurHash3,
  };

    /**
     * Get hash algorithm enum string.
     * @param[in] hashAlgo - the hash algorithm.
     * @return LLBC_CString - the enum string.
     */
    static LibString GetEnumStr(int hashAlgo);
};

struct KERNEL_EXPORT HashValue128bit
{
    UInt64 Hi;
    UInt64 Lo;
};

/**
 * \brief The hash algorithms encapsulation.
 */
class KERNEL_EXPORT HashUtil
{
public:
    /**
     * Hash specific bytes.
     * @param bytes - the will hash bytes.
     * @param size  - the will has bytes length.
     * @return uint32 - the hash value.
     */
    template <HashAlgorithm::ENUMS HashAlgo = HashAlgorithm::Default>
    static UInt32 Hash(const void *bytes, size_t size);

    /**
     * Hash specific bytes.
     * @param[in] hashAlgo  - the hash algorithm type.
     * @param[in] bytes     - the will has bytes.
     * @param[in] size      - the will hash bytes length.
     * @return uint32 - the hash value.
     */
    static UInt32 Hash(HashAlgorithm::ENUMS hashAlgo, const void *bytes, size_t size);

    // 128bit版本 MurmurHash3
    static UInt64 Hash64(const void *bytes, size_t size);
    static void Hash128(const void *bytes, size_t size, HashValue128bit &hashValue);
    static HashValue128bit Hash128(const void *bytes, size_t size);

private:
    /**
     * BKDR hash algorithm.
     */
    static UInt32 BKDRHash(const void *bytes, size_t size);

    /**
     * DJB hash algorithm.
     */
    static UInt32 DJBHash(const void *bytes, size_t size);

    /**
     * SDBM hash algorithm.
     */
    static UInt32 SDBMHash(const void *bytes, size_t size);

    /**
     * RS hash algorithm.
     */
    static UInt32 RSHash(const void *bytes, size_t size);

    /**
     * JS hash algorithm.
     */
    static UInt32 JSHash(const void *bytes, size_t size);

    /**
     * Peter J. Weinberger hash algorithm.
     */
    static UInt32 PJWHash(const void *bytes, size_t size);

    /**
     * ELF hash algorithm.
     */
    static UInt32 ELFHash(const void *bytes, size_t size);

    /**
     * AP hash algorithm.
     */
    static UInt32 APHash(const void *bytes, size_t size);

    /**
     * MurmurHash3 hash algorithm.
     */
    static UInt32 MurmurHash3Hash(const void *bytes, size_t size);

private:
    static  UInt64 getblock64 (const UInt64 * p, Int32 i);
    static UInt64 fmix64 ( UInt64 k );
};

template <HashAlgorithm::ENUMS HashAlgo>
ALWAYS_INLINE UInt32 HashUtil::Hash(const void *bytes, size_t size)
{
    if constexpr (HashAlgo == HashAlgorithm::BKDR)
        return BKDRHash(bytes, size);
    else if constexpr (HashAlgo == HashAlgorithm::DJB)
        return DJBHash(bytes, size);
    else if constexpr (HashAlgo == HashAlgorithm::SDBM)
        return SDBMHash(bytes, size);
    else if constexpr (HashAlgo == HashAlgorithm::RS)
        return RSHash(bytes, size);
    else if constexpr (HashAlgo == HashAlgorithm::JS)
        return JSHash(bytes, size);
    else if constexpr (HashAlgo == HashAlgorithm::PJW)
        return PJWHash(bytes, size);
    else if constexpr (HashAlgo == HashAlgorithm::ELF)
        return ELFHash(bytes, size);
    else if constexpr (HashAlgo == HashAlgorithm::AP)
        return APHash(bytes, size);
    else if constexpr (HashAlgo == HashAlgorithm::MurmurHash3)
        return MurmurHash3Hash(bytes, size);
    else
        static_assert("Invalid hash algorithm");

    return 0;
}

ALWAYS_INLINE UInt32 HashUtil::Hash(HashAlgorithm::ENUMS hashAlgo, const void *bytes, size_t size)
{
    if (hashAlgo == HashAlgorithm::BKDR)
        return BKDRHash(bytes, size);
    else if (hashAlgo == HashAlgorithm::DJB)
        return DJBHash(bytes, size);
    else if (hashAlgo == HashAlgorithm::SDBM)
        return SDBMHash(bytes, size);
    else if (hashAlgo == HashAlgorithm::RS)
        return RSHash(bytes, size);
    else if (hashAlgo == HashAlgorithm::JS)
        return JSHash(bytes, size);
    else if (hashAlgo == HashAlgorithm::PJW)
        return PJWHash(bytes, size);
    else if (hashAlgo == HashAlgorithm::ELF)
        return ELFHash(bytes, size);
    else if (hashAlgo == HashAlgorithm::AP)
        return APHash(bytes, size);
    else if (hashAlgo == HashAlgorithm::MurmurHash3)
        return MurmurHash3Hash(bytes, size);
    else
        return 0;
}

ALWAYS_INLINE UInt64 HashUtil::Hash64(const void *bytes, size_t size)
{
    HashValue128bit hashValue;
    Hash128(bytes, size, hashValue);
    return hashValue.Hi;
}


ALWAYS_INLINE UInt64 HashUtil::getblock64 (const UInt64 * p, Int32 i )
{
    return p[i];
}

ALWAYS_INLINE UInt64 HashUtil::fmix64 ( UInt64 k )
{
    k ^= k >> 33;
    k *= (0xff51afd7ed558ccdLLU);
    k ^= k >> 33;
    k *= (0xc4ceb9fe1a85ec53LLU);
    k ^= k >> 33;

    return k;
}

ALWAYS_INLINE void HashUtil::Hash128(const void *bytes, size_t size, HashValue128bit &hashValue)
{
    // 固定种子让输入对应一样的输出, 尤其跨进程
    constexpr UInt32 seed = 0x6384BA69;
    constexpr UInt64 c1 = 0x87c37b91114253d5LLU;
    constexpr UInt64 c2 = 0x4cf5ad432745937fLLU;

    const U8 *data = (const U8*)bytes;
    const Int32 nblocks = static_cast<Int32>(size / 16);

    hashValue.Hi = seed;
    hashValue.Lo = seed;
    auto &h1 = hashValue.Hi;
    auto &h2 = hashValue.Lo;

    //----------
    // body

    const UInt64 * blocks = (const UInt64 *)(data);
    for(Int32 i = 0; i < nblocks; ++i)
    {
        UInt64 k1 = getblock64(blocks,i*2+0);
        UInt64 k2 = getblock64(blocks,i*2+1);

        k1 *= c1; k1  = CRYSTAL_ROTL64(k1,31); k1 *= c2; h1 ^= k1;

        h1 = CRYSTAL_ROTL64(h1,27); h1 += h2; h1 = h1*5+0x52dce729;

        k2 *= c2; k2  = CRYSTAL_ROTL64(k2,33); k2 *= c1; h2 ^= k2;

        h2 = CRYSTAL_ROTL64(h2,31); h2 += h1; h2 = h2*5+0x38495ab5;
    }

    //----------
    // tail

    const U8 * tail = (const U8*)(data + nblocks*16);

    UInt64 k1 = 0;
    UInt64 k2 = 0;

    switch(size & 15)
    {
    case 15: k2 ^= ((UInt64)tail[14]) << 48;
    case 14: k2 ^= ((UInt64)tail[13]) << 40;
    case 13: k2 ^= ((UInt64)tail[12]) << 32;
    case 12: k2 ^= ((UInt64)tail[11]) << 24;
    case 11: k2 ^= ((UInt64)tail[10]) << 16;
    case 10: k2 ^= ((UInt64)tail[ 9]) << 8;
    case  9: k2 ^= ((UInt64)tail[ 8]) << 0;
        k2 *= c2; k2  = CRYSTAL_ROTL64(k2,33); k2 *= c1; h2 ^= k2;

    case  8: k1 ^= ((UInt64)tail[ 7]) << 56;
    case  7: k1 ^= ((UInt64)tail[ 6]) << 48;
    case  6: k1 ^= ((UInt64)tail[ 5]) << 40;
    case  5: k1 ^= ((UInt64)tail[ 4]) << 32;
    case  4: k1 ^= ((UInt64)tail[ 3]) << 24;
    case  3: k1 ^= ((UInt64)tail[ 2]) << 16;
    case  2: k1 ^= ((UInt64)tail[ 1]) << 8;
    case  1: k1 ^= ((UInt64)tail[ 0]) << 0;
        k1 *= c1; k1  = CRYSTAL_ROTL64(k1,31); k1 *= c2; h1 ^= k1;
    };

    //----------
    // finalization

    h1 ^= size; h2 ^= size;

    h1 += h2;
    h2 += h1;

    h1 = fmix64(h1);
    h2 = fmix64(h2);

    h1 += h2;
    h2 += h1;
}

ALWAYS_INLINE HashValue128bit HashUtil::Hash128(const void *bytes, size_t size)
{
    HashValue128bit hashValue;
    Hash128(bytes, size, hashValue);
    return hashValue;
}


KERNEL_END

#endif