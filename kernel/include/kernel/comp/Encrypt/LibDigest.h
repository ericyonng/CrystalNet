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
 * Author: Eric Yonng
 * Date: 2021-02-07 15:17:40
 * Description: 提供md5, sha1, sha224,sha384,sha512摘要算法
 * 可以一次性Make出来，也可以分片Make：
 * MakeXXXInit(ctx);
 * Int32 idx = 0;
 * for(;idx < strLen; idx += spanLen)
 * {
 *     if((idx + spanLen) >= strLen)
 *            break;
 * 
 *      MakeXXXContinue(ctx, str + idx, spanLen);
 * }
 * 
 * if(idx < strLen)
 *     MakeXXXContinue(ctx, str + idx, strLen - idx);
 * 
 * MakeXXXFinal(ctx, digest);
 * MakeXXXClean(ctx);
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_ENCRYPT_LIB_DIGEST_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_ENCRYPT_LIB_DIGEST_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>
#include <3rd/3rdForKernel.h>

KERNEL_BEGIN

class KERNEL_EXPORT LibDigest
{
public:
    static LibString MakeMd5(const LibString &src);
    static bool MakeMd5(const LibString &src, LibString &digest);
    static bool MakeMd5(const Byte8 *src, UInt64 len, LibString &digest);
    static bool MakeMd5Init(MD5_CTX &ctx);
    static bool MakeMd5Continue(MD5_CTX &ctx, const Byte8 *src, UInt64 len);
    static bool MakeMd5Final(MD5_CTX &ctx, LibString &digest);
    static void MakeMd5Clean(MD5_CTX &ctx);

    static LibString MakeSha1(const LibString &src);
    static bool MakeSha1(const LibString &src, LibString &digest);
    static bool MakeSha1(const Byte8 *src, UInt64 len, LibString &digest);
    static bool MakeSha1Init(SHA_CTX &ctx);
    static bool MakeSha1Continue(SHA_CTX &ctx, const Byte8 *src, UInt64 len);
    static bool MakeSha1Final(SHA_CTX &ctx, LibString &digest);
    static void MakeSha1Clean(SHA_CTX &ctx);

    static LibString MakeSha224(const LibString &src);
    static bool MakeSha224(const LibString &src, LibString &digest);
    static bool MakeSha224(const Byte8 *src, UInt64 len, LibString &digest);
    static bool MakeSha224Init(SHA256_CTX &ctx);
    static bool MakeSha224Continue(SHA256_CTX &ctx, const Byte8 *src, UInt64 len);
    static bool MakeSha224Final(SHA256_CTX &ctx, LibString &digest);
    static void MakeSha224Clean(SHA256_CTX &ctx);
    
    static LibString MakeSha256(const LibString &src);
    static bool MakeSha256(const LibString &src, LibString &digest);
    static bool MakeSha256(const Byte8 *src, UInt64 len, LibString &digest);
    static bool MakeSha256Init(SHA256_CTX &ctx);
    static bool MakeSha256Continue(SHA256_CTX &ctx, const Byte8 *src, UInt64 len);
    static bool MakeSha256Final(SHA256_CTX &ctx, LibString &digest);
    static void MakeSha256Clean(SHA256_CTX &ctx);
    
    static LibString MakeSha384(const LibString &src);
    static bool MakeSha384(const LibString &src, LibString &digest);
    static bool MakeSha384(const Byte8 *src, UInt64 len, LibString &digest);
    static bool MakeSha384Init(SHA512_CTX &ctx);
    static bool MakeSha384Continue(SHA512_CTX &ctx, const Byte8 *src, UInt64 len);
    static bool MakeSha384Final(SHA512_CTX &ctx, LibString &digest);
    static void MakeSha384Clean(SHA512_CTX &ctx);
    
    static LibString MakeSha512(const LibString &src);
    static bool MakeSha512(const LibString &src, LibString &digest);
    static bool MakeSha512(const Byte8 *src, UInt64 len, LibString &digest);
    static bool MakeSha512Init(SHA512_CTX &ctx);
    static bool MakeSha512Continue(SHA512_CTX &ctx, const Byte8 *src, UInt64 len);
    static bool MakeSha512Final(SHA512_CTX &ctx, LibString &digest);
    static void MakeSha512Clean(SHA512_CTX &ctx);
};

ALWAYS_INLINE LibString LibDigest::MakeMd5(const LibString &src)
{
    LibString digest;
    MakeMd5(src, digest);
    return digest;
}

ALWAYS_INLINE bool LibDigest::MakeMd5(const LibString &src, LibString &digest)
{
    return MakeMd5(src.c_str(), src.length(), digest);
}

ALWAYS_INLINE bool LibDigest::MakeMd5(const Byte8 *src, UInt64 len, LibString &digest)
{
    if(UNLIKELY(!len))
        return false;

    MD5_CTX c;
    auto err = MD5_Init(&c);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("MD5_Init fail err[%d]", err);
        return false;
    }

    err = MD5_Update(&c, src, len);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("MD5_Update fail err[%d]", err);
        return false;
    }

    U8 bufferMd5[MD5_DIGEST_LENGTH] = {0};
    err = MD5_Final(&(bufferMd5[0]), &c);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("MD5_Final fail err[%d]", err);
        return false;
    }

    digest.AppendData(reinterpret_cast<const Byte8 *>(bufferMd5), MD5_DIGEST_LENGTH);
    OPENSSL_cleanse(&c, sizeof(c));

    return true;
}

ALWAYS_INLINE bool LibDigest::MakeMd5Init(MD5_CTX &ctx)
{
    auto err = MD5_Init(&ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("MD5_Init fail err[%d]", err);
        return false;
    }

    return true;
}

ALWAYS_INLINE bool LibDigest::MakeMd5Continue(MD5_CTX &ctx, const Byte8 *src, UInt64 len)
{
    auto err = MD5_Update(&ctx, src, len);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("MD5_Update fail err[%d]", err);
        return false;
    }

    return true;
}

ALWAYS_INLINE bool LibDigest::MakeMd5Final(MD5_CTX &ctx, LibString &digest)
{
    U8 bufferMd5[MD5_DIGEST_LENGTH] = {0};
    auto err = MD5_Final(&(bufferMd5[0]), &ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("MD5_Final fail err[%d]", err);
        return false;
    }

    digest.AppendData(reinterpret_cast<const Byte8 *>(bufferMd5), MD5_DIGEST_LENGTH);

    return true;
}

ALWAYS_INLINE void LibDigest::MakeMd5Clean(MD5_CTX &ctx)
{
    OPENSSL_cleanse(&ctx, sizeof(ctx));
}

ALWAYS_INLINE LibString LibDigest::MakeSha1(const LibString &src)
{
    LibString digest;
    MakeSha1(src, digest);
    return digest;
}

ALWAYS_INLINE bool LibDigest::MakeSha1(const LibString &src, LibString &digest)
{
    return MakeSha1(src.c_str(), src.length(), digest);
}

ALWAYS_INLINE bool LibDigest::MakeSha1(const Byte8 *src, UInt64 len, LibString &digest)
{
    if(UNLIKELY(!len))
       return false;
    
    SHA_CTX stx;
    auto err = SHA1_Init(&stx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA1_Init fail err[%d]", err);
        return false;
    }

    err = SHA1_Update(&stx, src,len);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA1_Update fail err[%d]", err);
        return false;
    }

    U8 digestBuffer[SHA_DIGEST_LENGTH] = {0};
    err = SHA1_Final(digestBuffer, &stx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA1_Final fail err[%d]", err);
        return false;
    }

    digest.AppendData(reinterpret_cast<const Byte8 *>(digestBuffer), SHA_DIGEST_LENGTH);
    OPENSSL_cleanse(&stx, sizeof(stx));

    return true;
}

ALWAYS_INLINE bool LibDigest::MakeSha1Init(SHA_CTX &ctx)
{
    auto err = SHA1_Init(&ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA1_Init fail err[%d]", err);
        return false;
    }

    return true;
}

ALWAYS_INLINE bool LibDigest::MakeSha1Continue(SHA_CTX &ctx, const Byte8 *src, UInt64 len)
{
    auto err = SHA1_Update(&ctx, src,len);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA1_Update fail err[%d]", err);
        return false;
    }

    return true;
}

ALWAYS_INLINE bool LibDigest::MakeSha1Final(SHA_CTX &ctx, LibString &digest)
{
    U8 digestBuffer[SHA_DIGEST_LENGTH] = {0};
    auto err = SHA1_Final(digestBuffer, &ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA1_Final fail err[%d]", err);
        return false;
    }

    digest.AppendData(reinterpret_cast<const Byte8 *>(digestBuffer), SHA_DIGEST_LENGTH);
    return true;
}

ALWAYS_INLINE void LibDigest::MakeSha1Clean(SHA_CTX &ctx)
{
    OPENSSL_cleanse(&ctx, sizeof(ctx));
}

ALWAYS_INLINE LibString LibDigest::MakeSha224(const LibString &src)
{
    LibString digest;
    MakeSha224(src, digest);
    return digest;
}

ALWAYS_INLINE bool LibDigest::MakeSha224(const LibString &src, LibString &digest)
{
    return MakeSha224(src.c_str(), src.length(), digest);
}

ALWAYS_INLINE bool LibDigest::MakeSha224(const Byte8 *src, UInt64 len, LibString &digest)
{
    if(UNLIKELY(!len))
       return false;
    
    SHA256_CTX stx;
    auto err = SHA224_Init(&stx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA224_Init fail err[%d]", err);
        return false;
    }

    err = SHA224_Update(&stx, src,len);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA224_Update fail err[%d]", err);
        return false;
    }

    U8 digestBuffer[SHA224_DIGEST_LENGTH] = {0};
    err = SHA224_Final(digestBuffer, &stx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA224_Final fail err[%d]", err);
        return false;
    }

    digest.AppendData(reinterpret_cast<const Byte8 *>(digestBuffer), SHA224_DIGEST_LENGTH);
    OPENSSL_cleanse(&stx, sizeof(stx));

    return true;
}

ALWAYS_INLINE bool LibDigest::MakeSha224Init(SHA256_CTX &ctx)
{
    auto err = SHA224_Init(&ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA224_Init fail err[%d]", err);
        return false;
    }

    return true;
}

ALWAYS_INLINE bool LibDigest::MakeSha224Continue(SHA256_CTX &ctx, const Byte8 *src, UInt64 len)
{
    auto err = SHA224_Update(&ctx, src, len);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA224_Update fail err[%d]", err);
        return false;
    }

    return true;
}

ALWAYS_INLINE bool LibDigest::MakeSha224Final(SHA256_CTX &ctx, LibString &digest)
{
    U8 digestBuffer[SHA224_DIGEST_LENGTH] = {0};
    auto err = SHA224_Final(digestBuffer, &ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA224_Final fail err[%d]", err);
        return false;
    }

    digest.AppendData(reinterpret_cast<const Byte8 *>(digestBuffer), SHA224_DIGEST_LENGTH);

    return true;
}

ALWAYS_INLINE void LibDigest::MakeSha224Clean(SHA256_CTX &ctx)
{
    OPENSSL_cleanse(&ctx, sizeof(ctx));
}

ALWAYS_INLINE LibString LibDigest::MakeSha256(const LibString &src)
{
    LibString digest;
    MakeSha256(src, digest);
    return digest;
}

ALWAYS_INLINE bool LibDigest::MakeSha256(const LibString &src, LibString &digest)
{
    return MakeSha256(src.c_str(), src.length(), digest);
}

ALWAYS_INLINE bool LibDigest::MakeSha256(const Byte8 *src, UInt64 len, LibString &digest)
{
    if(UNLIKELY(!len))
       return false;
    
    SHA256_CTX stx;
    auto err = SHA256_Init(&stx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA256_Init fail err[%d]", err);
        return false;
    }

    err = SHA256_Update(&stx, src,len);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA256_Update fail err[%d]", err);
        return false;
    }

    U8 digestBuffer[SHA256_DIGEST_LENGTH] = {0};
    err = SHA256_Final(digestBuffer, &stx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA256_Final fail err[%d]", err);
        return false;
    }

    digest.AppendData(reinterpret_cast<const Byte8 *>(digestBuffer), SHA256_DIGEST_LENGTH);
    OPENSSL_cleanse(&stx, sizeof(stx));

    return true;
}

ALWAYS_INLINE bool LibDigest::MakeSha256Init(SHA256_CTX &ctx)
{
    auto err = SHA256_Init(&ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA256_Init fail err[%d]", err);
        return false;
    }

    return true;
}

ALWAYS_INLINE bool LibDigest::MakeSha256Continue(SHA256_CTX &ctx, const Byte8 *src, UInt64 len)
{
    auto err = SHA256_Update(&ctx, src,len);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA256_Update fail err[%d]", err);
        return false;
    }

    return true;
}

ALWAYS_INLINE bool LibDigest::MakeSha256Final(SHA256_CTX &ctx, LibString &digest)
{
    U8 digestBuffer[SHA256_DIGEST_LENGTH] = {0};
    auto err = SHA256_Final(digestBuffer, &ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA256_Final fail err[%d]", err);
        return false;
    }

    digest.AppendData(reinterpret_cast<const Byte8 *>(digestBuffer), SHA256_DIGEST_LENGTH);

    return true;
}

ALWAYS_INLINE void LibDigest::MakeSha256Clean(SHA256_CTX &ctx)
{
    OPENSSL_cleanse(&ctx, sizeof(ctx));
}

ALWAYS_INLINE LibString LibDigest::MakeSha384(const LibString &src)
{
    LibString digest;
    MakeSha384(src, digest);
    return digest;
}

ALWAYS_INLINE bool LibDigest::MakeSha384(const LibString &src, LibString &digest)
{
    return MakeSha384(src.c_str(), src.length(), digest);
}

ALWAYS_INLINE bool LibDigest::MakeSha384(const Byte8 *src, UInt64 len, LibString &digest)
{
    if(UNLIKELY(!len))
       return false;
    
    SHA512_CTX stx;
    auto err = SHA384_Init(&stx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA384_Init fail err[%d]", err);
        return false;
    }

    err = SHA384_Update(&stx, src,len);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA384_Update fail err[%d]", err);
        return false;
    }

    U8 digestBuffer[SHA384_DIGEST_LENGTH] = {0};
    err = SHA384_Final(digestBuffer, &stx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA384_Final fail err[%d]", err);
        return false;
    }

    digest.AppendData(reinterpret_cast<const Byte8 *>(digestBuffer), SHA384_DIGEST_LENGTH);
    OPENSSL_cleanse(&stx, sizeof(stx));

    return true;
}

ALWAYS_INLINE bool LibDigest::MakeSha384Init(SHA512_CTX &ctx)
{
    auto err = SHA384_Init(&ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA384_Init fail err[%d]", err);
        return false;
    }

    return true;
}

ALWAYS_INLINE bool LibDigest::MakeSha384Continue(SHA512_CTX &ctx, const Byte8 *src, UInt64 len)
{
    auto err = SHA384_Update(&ctx, src, len);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA384_Update fail err[%d]", err);
        return false;
    }

    return true;
}

ALWAYS_INLINE bool LibDigest::MakeSha384Final(SHA512_CTX &ctx, LibString &digest)
{
    U8 digestBuffer[SHA384_DIGEST_LENGTH] = {0};
    auto err = SHA384_Final(digestBuffer, &ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA384_Final fail err[%d]", err);
        return false;
    }

    digest.AppendData(reinterpret_cast<const Byte8 *>(digestBuffer), SHA384_DIGEST_LENGTH);
    return true;
}

ALWAYS_INLINE void LibDigest::MakeSha384Clean(SHA512_CTX &ctx)
{
    OPENSSL_cleanse(&ctx, sizeof(ctx));
}

ALWAYS_INLINE LibString LibDigest::MakeSha512(const LibString &src)
{
    LibString digest;
    MakeSha512(src, digest);
    return digest;
}

ALWAYS_INLINE bool LibDigest::MakeSha512(const LibString &src, LibString &digest)
{
    return MakeSha512(src.c_str(), src.length(), digest);
}

ALWAYS_INLINE bool LibDigest::MakeSha512(const Byte8 *src, UInt64 len, LibString &digest)
{
    if(UNLIKELY(!len))
       return false;
    
    SHA512_CTX stx;
    auto err = SHA512_Init(&stx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA512_Init fail err[%d]", err);
        return false;
    }

    err = SHA512_Update(&stx, src,len);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA512_Update fail err[%d]", err);
        return false;
    }

    U8 digestBuffer[SHA512_DIGEST_LENGTH] = {0};
    err = SHA512_Final(digestBuffer, &stx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA512_Final fail err[%d]", err);
        return false;
    }

    digest.AppendData(reinterpret_cast<const Byte8 *>(digestBuffer), SHA512_DIGEST_LENGTH);
    OPENSSL_cleanse(&stx, sizeof(stx));

    return true;
}

ALWAYS_INLINE bool LibDigest::MakeSha512Init(SHA512_CTX &ctx)
{
    auto err = SHA512_Init(&ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA512_Init fail err[%d]", err);
        return false;
    }

    return true;
}

ALWAYS_INLINE bool LibDigest::MakeSha512Continue(SHA512_CTX &ctx, const Byte8 *src, UInt64 len)
{
    auto err = SHA512_Update(&ctx, src,len);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA512_Update fail err[%d]", err);
        return false;
    }

    return true;
}

ALWAYS_INLINE bool LibDigest::MakeSha512Final(SHA512_CTX &ctx, LibString &digest)
{
    U8 digestBuffer[SHA512_DIGEST_LENGTH] = {0};
    auto err = SHA512_Final(digestBuffer, &ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA512_Final fail err[%d]", err);
        return false;
    }

    digest.AppendData(reinterpret_cast<const Byte8 *>(digestBuffer), SHA512_DIGEST_LENGTH);

    return true;
}

ALWAYS_INLINE void LibDigest::MakeSha512Clean(SHA512_CTX &ctx)
{
    OPENSSL_cleanse(&ctx, sizeof(ctx));
}

KERNEL_END


#endif
