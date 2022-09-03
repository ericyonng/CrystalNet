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

    static LibString MakeSha1(const LibString &src);
    static bool MakeSha1(const LibString &src, LibString &digest);
    static bool MakeSha1(const Byte8 *src, UInt64 len, LibString &digest);
    static LibString MakeSha224(const LibString &src);
    static bool MakeSha224(const LibString &src, LibString &digest);
    static bool MakeSha224(const Byte8 *src, UInt64 len, LibString &digest);
    static LibString MakeSha256(const LibString &src);
    static bool MakeSha256(const LibString &src, LibString &digest);
    static bool MakeSha256(const Byte8 *src, UInt64 len, LibString &digest);
    static LibString MakeSha384(const LibString &src);
    static bool MakeSha384(const LibString &src, LibString &digest);
    static bool MakeSha384(const Byte8 *src, UInt64 len, LibString &digest);
    static LibString MakeSha512(const LibString &src);
    static bool MakeSha512(const LibString &src, LibString &digest);
    static bool MakeSha512(const Byte8 *src, UInt64 len, LibString &digest);
};

inline LibString LibDigest::MakeMd5(const LibString &src)
{
    LibString digest;
    MakeMd5(src, digest);
    return digest;
}

inline bool LibDigest::MakeMd5(const LibString &src, LibString &digest)
{
    return MakeMd5(src.c_str(), src.length(), digest);
}

inline bool LibDigest::MakeMd5(const Byte8 *src, UInt64 len, LibString &digest)
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

inline LibString LibDigest::MakeSha1(const LibString &src)
{
    LibString digest;
    MakeSha1(src, digest);
    return digest;
}

inline bool LibDigest::MakeSha1(const LibString &src, LibString &digest)
{
    return MakeSha1(src.c_str(), src.length(), digest);
}

inline bool LibDigest::MakeSha1(const Byte8 *src, UInt64 len, LibString &digest)
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

inline LibString LibDigest::MakeSha224(const LibString &src)
{
    LibString digest;
    MakeSha224(src, digest);
    return digest;
}

inline bool LibDigest::MakeSha224(const LibString &src, LibString &digest)
{
    return MakeSha224(src.c_str(), src.length(), digest);
}

inline bool LibDigest::MakeSha224(const Byte8 *src, UInt64 len, LibString &digest)
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

inline LibString LibDigest::MakeSha256(const LibString &src)
{
    LibString digest;
    MakeSha256(src, digest);
    return digest;
}

inline bool LibDigest::MakeSha256(const LibString &src, LibString &digest)
{
    return MakeSha256(src.c_str(), src.length(), digest);
}

inline bool LibDigest::MakeSha256(const Byte8 *src, UInt64 len, LibString &digest)
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

inline LibString LibDigest::MakeSha384(const LibString &src)
{
    LibString digest;
    MakeSha384(src, digest);
    return digest;
}

inline bool LibDigest::MakeSha384(const LibString &src, LibString &digest)
{
    return MakeSha384(src.c_str(), src.length(), digest);
}

inline bool LibDigest::MakeSha384(const Byte8 *src, UInt64 len, LibString &digest)
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

inline LibString LibDigest::MakeSha512(const LibString &src)
{
    LibString digest;
    MakeSha512(src, digest);
    return digest;
}

inline bool LibDigest::MakeSha512(const LibString &src, LibString &digest)
{
    return MakeSha512(src.c_str(), src.length(), digest);
}

inline bool LibDigest::MakeSha512(const Byte8 *src, UInt64 len, LibString &digest)
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

KERNEL_END


#endif
