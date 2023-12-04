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

#include <kernel/comp/LibString.h>
#include <kernel/common/macro.h>
#include <kernel/kernel_export.h>
#include <kernel/common/BaseType.h>

KERNEL_BEGIN

class KERNEL_EXPORT LibDigest
{
public:
    static LibString MakeMd5(const LibString &src);
    static bool MakeMd5(const LibString &src, LibString &digest);
    static bool MakeMd5(const Byte8 *src, UInt64 len, LibString &digest);
    static bool MakeMd5Init(void *ctx);
    static bool MakeMd5Continue(void *ctx, const Byte8 *src, UInt64 len);
    static bool MakeMd5Final(void *ctx, LibString &digest);
    static void MakeMd5Clean(void *ctx);
    static bool MakeFileMd5(const LibString &file, LibString &md5);

    static LibString MakeSha1(const LibString &src);
    static bool MakeSha1(const LibString &src, LibString &digest);
    static bool MakeSha1(const Byte8 *src, UInt64 len, LibString &digest);
    static bool MakeSha1Init(void *ctx);
    static bool MakeSha1Continue(void *ctx, const Byte8 *src, UInt64 len);
    static bool MakeSha1Final(void *ctx, LibString &digest);
    static void MakeSha1Clean(void *ctx);
    static bool MakeFileSha1(const LibString &file, LibString &sha1Out);

    static LibString MakeSha224(const LibString &src);
    static bool MakeSha224(const LibString &src, LibString &digest);
    static bool MakeSha224(const Byte8 *src, UInt64 len, LibString &digest);
    static bool MakeSha224Init(void *ctx);
    static bool MakeSha224Continue(void *ctx, const Byte8 *src, UInt64 len);
    static bool MakeSha224Final(void *ctx, LibString &digest);
    static void MakeSha224Clean(void *ctx);
    static bool MakeFileSha224(const LibString &file, LibString &result);
    
    static LibString MakeSha256(const LibString &src);
    static bool MakeSha256(const LibString &src, LibString &digest);
    static bool MakeSha256(const Byte8 *src, UInt64 len, LibString &digest);
    static bool MakeSha256Init(void *ctx);
    static bool MakeSha256Continue(void *ctx, const Byte8 *src, UInt64 len);
    static bool MakeSha256Final(void *ctx, LibString &digest);
    static void MakeSha256Clean(void *ctx);
    static bool MakeFileSha256(const LibString &file, LibString &result);
    
    static LibString MakeSha384(const LibString &src);
    static bool MakeSha384(const LibString &src, LibString &digest);
    static bool MakeSha384(const Byte8 *src, UInt64 len, LibString &digest);
    static bool MakeSha384Init(void *ctx);
    static bool MakeSha384Continue(void *ctx, const Byte8 *src, UInt64 len);
    static bool MakeSha384Final(void *ctx, LibString &digest);
    static void MakeSha384Clean(void *ctx);
    static bool MakeFileSha384(const LibString &file, LibString &result);
    
    static LibString MakeSha512(const LibString &src);
    static bool MakeSha512(const LibString &src, LibString &digest);
    static bool MakeSha512(const Byte8 *src, UInt64 len, LibString &digest);
    static bool MakeSha512Init(void *ctx);
    static bool MakeSha512Continue(void *ctx, const Byte8 *src, UInt64 len);
    static bool MakeSha512Final(void *ctx, LibString &digest);
    static void MakeSha512Clean(void *ctx);
    static bool MakeFileSha512(const LibString &file, LibString &result);
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

KERNEL_END


#endif
