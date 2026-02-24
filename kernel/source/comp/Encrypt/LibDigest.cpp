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
 * Date: 2021-02-07 16:39:59
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Encrypt/LibDigest.h>
#include <kernel/comp/Utils/FileUtil.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Log/log.h>
#include <kernel/common/statics.h>

#include <stdio.h>
#include <3rd/openssl/openssl_include.h>

KERNEL_BEGIN

bool LibDigest::MakeMd5Init(void *ctx)
{
    auto err = MD5_Init((MD5_CTX *)ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("MD5_Init fail err[%d]", err);
        return false;
    }

    return true;
}

bool LibDigest::MakeMd5Continue(void *ctx, const Byte8 *src, UInt64 len)
{
    auto err = MD5_Update((MD5_CTX *)ctx, src, len);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("MD5_Update fail err[%d]", err);
        return false;
    }

    return true;
}

bool LibDigest::MakeMd5Final(void *ctx, LibString &digest)
{
    U8 bufferMd5[MD5_DIGEST_LENGTH] = {0};
    auto err = MD5_Final(&(bufferMd5[0]), (MD5_CTX *)ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("MD5_Final fail err[%d]", err);
        return false;
    }

    digest.AppendData(reinterpret_cast<const Byte8 *>(bufferMd5), MD5_DIGEST_LENGTH);

    return true;
}

void LibDigest::MakeMd5Clean(void *ctx)
{
    OPENSSL_cleanse(ctx, sizeof(*(MD5_CTX *)(ctx)));
}

bool LibDigest::MakeMd5(const Byte8 *src, UInt64 len, LibString &digest)
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

bool LibDigest::MakeFileMd5(const LibString &file, LibString &md5)
{
    // 打开文件
    SmartPtr<FILE, AutoDelMethods::CustomDelete> fp = FileUtil::OpenFile(file.c_str(), false, "rb");
    if(!fp)
    {
        CLOG_WARN_GLOBAL(LibDigest, "open file fail file:%s", file.c_str());
        return false;
    }
    
    // 释放资源
    fp.SetClosureDelegate([](void *ptr){
        auto p = reinterpret_cast<FILE *>(ptr);
        FileUtil::CloseFile(*p);
    });

    MD5_CTX ctx;
    if(!MakeMd5Init(&ctx))
    {
        CLOG_WARN_GLOBAL(LibDigest, "MakeMd5Init fail file:%s", file.c_str());
        return false;
    }

    SmartPtr<Byte8, AutoDelMethods::CustomDelete> cache = reinterpret_cast<Byte8 *>(KernelAllocMemory<_Build::TL>(256));
    cache.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<Byte8 *>(p);
        KernelFreeMemory<_Build::TL>(ptr);
    });
    auto buffer = cache.AsSelf();
    
    bool isSuc = true;
    do
    {
        auto readBytes = FileUtil::ReadFile(*fp, 255, buffer);
        if(readBytes == 0)
            break;

        buffer[readBytes] = 0;
        if(!KERNEL_NS::LibDigest::MakeMd5Continue(&ctx, buffer, readBytes))
        {
            CLOG_WARN_GLOBAL(LibDigest, "MakeMd5Continue fail file:%s", file.c_str());
            isSuc = false;
            break;
        }

    }while(true);

    if(!isSuc)
    {
        CLOG_WARN_GLOBAL(LibDigest, "make md5 fail file:%s", file.c_str());
        KERNEL_NS::LibDigest::MakeMd5Clean(&ctx);
        return false;
    }

    if(!KERNEL_NS::LibDigest::MakeMd5Final(&ctx, md5))
    {
        CLOG_WARN_GLOBAL(LibDigest, "MakeMd5Final file:%s", file.c_str());
        KERNEL_NS::LibDigest::MakeMd5Clean(&ctx);
        return false;
    }
    
    KERNEL_NS::LibDigest::MakeMd5Clean(&ctx);

    return true;
}

bool LibDigest::MakeSha1Init(void *ctx)
{
    auto err = SHA1_Init((SHA_CTX *)ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA1_Init fail err[%d]", err);
        return false;
    }

    return true;
}

bool LibDigest::MakeSha1Continue(void *ctx, const Byte8 *src, UInt64 len)
{
    auto err = SHA1_Update((SHA_CTX *)ctx, src,len);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA1_Update fail err[%d]", err);
        return false;
    }

    return true;
}

bool LibDigest::MakeSha1Final(void *ctx, LibString &digest)
{
    U8 digestBuffer[SHA_DIGEST_LENGTH] = {0};
    auto err = SHA1_Final(digestBuffer, (SHA_CTX *)ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA1_Final fail err[%d]", err);
        return false;
    }

    digest.AppendData(reinterpret_cast<const Byte8 *>(digestBuffer), SHA_DIGEST_LENGTH);
    return true;
}

void LibDigest::MakeSha1Clean(void *ctx)
{
    OPENSSL_cleanse(ctx, sizeof(*(SHA_CTX *)(ctx)));
}

bool LibDigest::MakeSha1(const Byte8 *src, UInt64 len, LibString &digest)
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

bool LibDigest::MakeFileSha1(const LibString &file, LibString &sha1Out)
{
    // 打开文件
    SmartPtr<FILE, AutoDelMethods::CustomDelete> fp = FileUtil::OpenFile(file.c_str(), false, "rb");
    if(!fp)
    {
        CLOG_WARN_GLOBAL(LibDigest, "open file fail file:%s", file.c_str());
        return false;
    }
    
    // 释放资源
    fp.SetClosureDelegate([](void *ptr){
        auto p = reinterpret_cast<FILE *>(ptr);
        FileUtil::CloseFile(*p);
    });

    SHA_CTX ctx;
    if(!MakeSha1Init(&ctx))
    {
        CLOG_WARN_GLOBAL(LibDigest, "MakeSha1Init fail file:%s", file.c_str());
        return false;
    }

    SmartPtr<Byte8, AutoDelMethods::CustomDelete> cache = reinterpret_cast<Byte8 *>(KernelAllocMemory<_Build::TL>(256));
    cache.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<Byte8 *>(p);
        KernelFreeMemory<_Build::TL>(ptr);
    });
    auto buffer = cache.AsSelf();

    bool isSuc = true;
    do
    {
        auto readBytes = FileUtil::ReadFile(*fp, 255, buffer);
        if(readBytes == 0)
            break;

        buffer[readBytes] = 0;
        if(!KERNEL_NS::LibDigest::MakeSha1Continue(&ctx, buffer, readBytes))
        {
            CLOG_WARN_GLOBAL(LibDigest, "MakeSha1Continue fail file:%s", file.c_str());
            isSuc = false;
            break;
        }

    }while(true);

    if(!isSuc)
    {
        CLOG_WARN_GLOBAL(LibDigest, "make sha1 fail file:%s", file.c_str());
        KERNEL_NS::LibDigest::MakeSha1Clean(&ctx);
        return false;
    }

    if(!KERNEL_NS::LibDigest::MakeSha1Final(&ctx, sha1Out))
    {
        CLOG_WARN_GLOBAL(LibDigest, "MakeSha1Final file:%s", file.c_str());
        KERNEL_NS::LibDigest::MakeSha1Clean(&ctx);
        return false;
    }
    
    KERNEL_NS::LibDigest::MakeSha1Clean(&ctx);

    return true;
}

bool LibDigest::MakeSha224Init(void *ctx)
{
    auto err = SHA224_Init((SHA256_CTX *)ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA224_Init fail err[%d]", err);
        return false;
    }

    return true;
}

bool LibDigest::MakeSha224Continue(void *ctx, const Byte8 *src, UInt64 len)
{
    auto err = SHA224_Update((SHA256_CTX *)ctx, src, len);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA224_Update fail err[%d]", err);
        return false;
    }

    return true;
}

bool LibDigest::MakeSha224Final(void *ctx, LibString &digest)
{
    U8 digestBuffer[SHA224_DIGEST_LENGTH] = {0};
    auto err = SHA224_Final(digestBuffer, (SHA256_CTX *)ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA224_Final fail err[%d]", err);
        return false;
    }

    digest.AppendData(reinterpret_cast<const Byte8 *>(digestBuffer), SHA224_DIGEST_LENGTH);

    return true;
}

void LibDigest::MakeSha224Clean(void *ctx)
{
    OPENSSL_cleanse((SHA256_CTX *)ctx, sizeof(*(SHA256_CTX *)(ctx)));
}

bool LibDigest::MakeSha224(const Byte8 *src, UInt64 len, LibString &digest)
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

bool LibDigest::MakeFileSha224(const LibString &file, LibString &result)
{
    // 打开文件
    SmartPtr<FILE, AutoDelMethods::CustomDelete> fp = FileUtil::OpenFile(file.c_str(), false, "rb");
    if(!fp)
    {
        CLOG_WARN_GLOBAL(LibDigest, "open file fail file:%s", file.c_str());
        return false;
    }
    
    // 释放资源
    fp.SetClosureDelegate([](void *ptr){
        auto p = reinterpret_cast<FILE *>(ptr);
        FileUtil::CloseFile(*p);
    });

    SHA256_CTX ctx;
    if(!MakeSha224Init(&ctx))
    {
        CLOG_WARN_GLOBAL(LibDigest, "MakeSha224Init fail file:%s", file.c_str());
        return false;
    }

    SmartPtr<Byte8, AutoDelMethods::CustomDelete> cache = reinterpret_cast<Byte8 *>(KernelAllocMemory<_Build::TL>(256));
    cache.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<Byte8 *>(p);
        KernelFreeMemory<_Build::TL>(ptr);
    });
    auto buffer = cache.AsSelf();

    bool isSuc = true;
    do
    {
        auto readBytes = FileUtil::ReadFile(*fp, 255, buffer);
        if(readBytes == 0)
            break;

        buffer[readBytes] = 0;
        if(!KERNEL_NS::LibDigest::MakeSha224Continue(&ctx, buffer, readBytes))
        {
            CLOG_WARN_GLOBAL(LibDigest, "MakeSha224Continue fail file:%s", file.c_str());
            isSuc = false;
            break;
        }

    }while(true);

    if(!isSuc)
    {
        CLOG_WARN_GLOBAL(LibDigest, "make sha224 fail file:%s", file.c_str());
        KERNEL_NS::LibDigest::MakeSha224Clean(&ctx);
        return false;
    }

    if(!KERNEL_NS::LibDigest::MakeSha224Final(&ctx, result))
    {
        CLOG_WARN_GLOBAL(LibDigest, "MakeSha224Final file:%s", file.c_str());
        KERNEL_NS::LibDigest::MakeSha224Clean(&ctx);
        return false;
    }
    
    KERNEL_NS::LibDigest::MakeSha224Clean(&ctx);

    return true;
}

bool LibDigest::MakeSha256Init(void *ctx)
{
    auto err = SHA256_Init((SHA256_CTX *)ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA256_Init fail err[%d]", err);
        return false;
    }

    return true;
}

bool LibDigest::MakeSha256Continue(void *ctx, const Byte8 *src, UInt64 len)
{
    auto err = SHA256_Update((SHA256_CTX *)ctx, src,len);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA256_Update fail err[%d]", err);
        return false;
    }

    return true;
}

bool LibDigest::MakeSha256Final(void *ctx, LibString &digest)
{
    U8 digestBuffer[SHA256_DIGEST_LENGTH] = {0};
    auto err = SHA256_Final(digestBuffer, (SHA256_CTX *)ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA256_Final fail err[%d]", err);
        return false;
    }

    digest.AppendData(reinterpret_cast<const Byte8 *>(digestBuffer), SHA256_DIGEST_LENGTH);

    return true;
}

void LibDigest::MakeSha256Clean(void *ctx)
{
    OPENSSL_cleanse((SHA256_CTX *)ctx, sizeof(*(SHA256_CTX *)(ctx)));
}

bool LibDigest::MakeSha256(const Byte8 *src, UInt64 len, LibString &digest)
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

bool LibDigest::MakeFileSha256(const LibString &file, LibString &result)
{
    // 打开文件
    SmartPtr<FILE, AutoDelMethods::CustomDelete> fp = FileUtil::OpenFile(file.c_str(), false, "rb");
    if(!fp)
    {
        CLOG_WARN_GLOBAL(LibDigest, "open file fail file:%s", file.c_str());
        return false;
    }
    
    // 释放资源
    fp.SetClosureDelegate([](void *ptr){
        auto p = reinterpret_cast<FILE *>(ptr);
        FileUtil::CloseFile(*p);
    });

    SHA256_CTX ctx;
    if(!MakeSha256Init(&ctx))
    {
        CLOG_WARN_GLOBAL(LibDigest, "MakeSha256Init fail file:%s", file.c_str());
        return false;
    }

    SmartPtr<Byte8, AutoDelMethods::CustomDelete> cache = reinterpret_cast<Byte8 *>(KernelAllocMemory<_Build::TL>(256));
    cache.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<Byte8 *>(p);
        KernelFreeMemory<_Build::TL>(ptr);
    });
    auto buffer = cache.AsSelf();

    bool isSuc = true;
    do
    {
        auto readBytes = FileUtil::ReadFile(*fp, 255, buffer);
        if(readBytes == 0)
            break;

        buffer[readBytes] = 0;
        if(!KERNEL_NS::LibDigest::MakeSha256Continue(&ctx, buffer, readBytes))
        {
            CLOG_WARN_GLOBAL(LibDigest, "MakeSha256Continue fail file:%s", file.c_str());
            isSuc = false;
            break;
        }

    }while(true);

    if(!isSuc)
    {
        CLOG_WARN_GLOBAL(LibDigest, "make sha256 fail file:%s", file.c_str());
        KERNEL_NS::LibDigest::MakeSha256Clean(&ctx);
        return false;
    }

    if(!KERNEL_NS::LibDigest::MakeSha256Final(&ctx, result))
    {
        CLOG_WARN_GLOBAL(LibDigest, "MakeSha256Final file:%s", file.c_str());
        KERNEL_NS::LibDigest::MakeSha256Clean(&ctx);
        return false;
    }
    
    KERNEL_NS::LibDigest::MakeSha256Clean(&ctx);

    return true;
}

bool LibDigest::MakeSha384Init(void *ctx)
{
    auto err = SHA384_Init((SHA512_CTX *)ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA384_Init fail err[%d]", err);
        return false;
    }

    return true;
}

bool LibDigest::MakeSha384Continue(void *ctx, const Byte8 *src, UInt64 len)
{
    auto err = SHA384_Update((SHA512_CTX *)ctx, src, len);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA384_Update fail err[%d]", err);
        return false;
    }

    return true;
}

bool LibDigest::MakeSha384Final(void *ctx, LibString &digest)
{
    U8 digestBuffer[SHA384_DIGEST_LENGTH] = {0};
    auto err = SHA384_Final(digestBuffer, (SHA512_CTX *)ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA384_Final fail err[%d]", err);
        return false;
    }

    digest.AppendData(reinterpret_cast<const Byte8 *>(digestBuffer), SHA384_DIGEST_LENGTH);
    return true;
}

void LibDigest::MakeSha384Clean(void *ctx)
{
    OPENSSL_cleanse((SHA512_CTX *)ctx, sizeof(*(SHA512_CTX *)(ctx)));
}

bool LibDigest::MakeSha384(const Byte8 *src, UInt64 len, LibString &digest)
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

bool LibDigest::MakeFileSha384(const LibString &file, LibString &result)
{
    // 打开文件
    SmartPtr<FILE, AutoDelMethods::CustomDelete> fp = FileUtil::OpenFile(file.c_str(), false, "rb");
    if(!fp)
    {
        CLOG_WARN_GLOBAL(LibDigest, "open file fail file:%s", file.c_str());
        return false;
    }
    
    // 释放资源
    fp.SetClosureDelegate([](void *ptr){
        auto p = reinterpret_cast<FILE *>(ptr);
        FileUtil::CloseFile(*p);
    });

    SHA512_CTX ctx;
    if(!MakeSha384Init(&ctx))
    {
        CLOG_WARN_GLOBAL(LibDigest, "MakeSha384Init fail file:%s", file.c_str());
        return false;
    }

    SmartPtr<Byte8, AutoDelMethods::CustomDelete> cache = reinterpret_cast<Byte8 *>(KernelAllocMemory<_Build::TL>(256));
    cache.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<Byte8 *>(p);
        KernelFreeMemory<_Build::TL>(ptr);
    });
    auto buffer = cache.AsSelf();
    
    bool isSuc = true;
    do
    {
        auto readBytes = FileUtil::ReadFile(*fp, 255, buffer);
        if(readBytes == 0)
            break;

        buffer[readBytes] = 0;
        if(!KERNEL_NS::LibDigest::MakeSha384Continue(&ctx, buffer, readBytes))
        {
            CLOG_WARN_GLOBAL(LibDigest, "MakeSha384Continue fail file:%s", file.c_str());
            isSuc = false;
            break;
        }

    }while(true);

    if(!isSuc)
    {
        CLOG_WARN_GLOBAL(LibDigest, "make sha384 fail file:%s", file.c_str());
        KERNEL_NS::LibDigest::MakeSha384Clean(&ctx);
        return false;
    }

    if(!KERNEL_NS::LibDigest::MakeSha384Final(&ctx, result))
    {
        CLOG_WARN_GLOBAL(LibDigest, "MakeSha384Final file:%s", file.c_str());
        KERNEL_NS::LibDigest::MakeSha384Clean(&ctx);
        return false;
    }
    
    KERNEL_NS::LibDigest::MakeSha384Clean(&ctx);

    return true;
}

bool LibDigest::MakeSha512Init(void *ctx)
{
    auto err = SHA512_Init((SHA512_CTX *)ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA512_Init fail err[%d]", err);
        return false;
    }

    return true;
}

bool LibDigest::MakeSha512Continue(void *ctx, const Byte8 *src, UInt64 len)
{
    auto err = SHA512_Update((SHA512_CTX *)ctx, src,len);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA512_Update fail err[%d]", err);
        return false;
    }

    return true;
}

bool LibDigest::MakeSha512Final(void *ctx, LibString &digest)
{
    U8 digestBuffer[SHA512_DIGEST_LENGTH] = {0};
    auto err = SHA512_Final(digestBuffer, (SHA512_CTX *)ctx);
    if(UNLIKELY(err != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("SHA512_Final fail err[%d]", err);
        return false;
    }

    digest.AppendData(reinterpret_cast<const Byte8 *>(digestBuffer), SHA512_DIGEST_LENGTH);

    return true;
}

void LibDigest::MakeSha512Clean(void *ctx)
{
    OPENSSL_cleanse((SHA512_CTX *)ctx, sizeof(*(SHA512_CTX *)(ctx)));
}

bool LibDigest::MakeSha512(const Byte8 *src, UInt64 len, LibString &digest)
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

bool LibDigest::MakeFileSha512(const LibString &file, LibString &result)
{
    // 打开文件
    SmartPtr<FILE, AutoDelMethods::CustomDelete> fp = FileUtil::OpenFile(file.c_str(), false, "rb");
    if(!fp)
    {
        CLOG_WARN_GLOBAL(LibDigest, "open file fail file:%s", file.c_str());
        return false;
    }
    
    // 释放资源
    fp.SetClosureDelegate([](void *ptr){
        auto p = reinterpret_cast<FILE *>(ptr);
        FileUtil::CloseFile(*p);
    });

    SHA512_CTX ctx;
    if(!MakeSha512Init(&ctx))
    {
        CLOG_WARN_GLOBAL(LibDigest, "MakeSha512Init fail file:%s", file.c_str());
        return false;
    }

    SmartPtr<Byte8, AutoDelMethods::CustomDelete> cache = reinterpret_cast<Byte8 *>(KernelAllocMemory<_Build::TL>(256));
    cache.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<Byte8 *>(p);
        KernelFreeMemory<_Build::TL>(ptr);
    });
    auto buffer = cache.AsSelf();
    
    bool isSuc = true;
    do
    {
        auto readBytes = FileUtil::ReadFile(*fp, 255, buffer);
        if(readBytes == 0)
            break;

        buffer[readBytes] = 0;
        if(!KERNEL_NS::LibDigest::MakeSha512Continue(&ctx, buffer, readBytes))
        {
            CLOG_WARN_GLOBAL(LibDigest, "MakeSha512Continue fail file:%s", file.c_str());
            isSuc = false;
            break;
        }

    }while(true);

    if(!isSuc)
    {
        CLOG_WARN_GLOBAL(LibDigest, "make sha512 fail file:%s", file.c_str());
        KERNEL_NS::LibDigest::MakeSha512Clean(&ctx);
        return false;
    }

    if(!KERNEL_NS::LibDigest::MakeSha512Final(&ctx, result))
    {
        CLOG_WARN_GLOBAL(LibDigest, "MakeSha512Final file:%s", file.c_str());
        KERNEL_NS::LibDigest::MakeSha512Clean(&ctx);
        return false;
    }
    
    KERNEL_NS::LibDigest::MakeSha512Clean(&ctx);

    return true;
}

KERNEL_END
