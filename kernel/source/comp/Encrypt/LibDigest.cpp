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

KERNEL_BEGIN

bool LibDigest::MakeFileMd5(const LibString &file, LibString &md5)
{
    // 打开文件
    SmartPtr<FILE, AutoDelMethods::CustomDelete> fp = FileUtil::OpenFile(file.c_str(), false, "rb");
    if(!fp)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "open file fail file:%s"), file.c_str());
        return false;
    }
    
    // 释放资源
    fp.SetClosureDelegate([](void *ptr){
        auto p = reinterpret_cast<FILE *>(ptr);
        FileUtil::CloseFile(*p);
    });

    MD5_CTX ctx;
    if(!MakeMd5Init(ctx))
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "MakeMd5Init fail file:%s"), file.c_str());
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
        if(!KERNEL_NS::LibDigest::MakeMd5Continue(ctx, buffer, readBytes))
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "MakeMd5Continue fail file:%s"), file.c_str());
            isSuc = false;
            break;
        }

    }while(true);

    if(!isSuc)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "make md5 fail file:%s"), file.c_str());
        KERNEL_NS::LibDigest::MakeMd5Clean(ctx);
        return false;
    }

    if(!KERNEL_NS::LibDigest::MakeMd5Final(ctx, md5))
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "MakeMd5Final file:%s"), file.c_str());
        KERNEL_NS::LibDigest::MakeMd5Clean(ctx);
        return false;
    }
    
    KERNEL_NS::LibDigest::MakeMd5Clean(ctx);

    return true;
}

bool LibDigest::MakeFileSha1(const LibString &file, LibString &sha1Out)
{
    // 打开文件
    SmartPtr<FILE, AutoDelMethods::CustomDelete> fp = FileUtil::OpenFile(file.c_str(), false, "rb");
    if(!fp)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "open file fail file:%s"), file.c_str());
        return false;
    }
    
    // 释放资源
    fp.SetClosureDelegate([](void *ptr){
        auto p = reinterpret_cast<FILE *>(ptr);
        FileUtil::CloseFile(*p);
    });

    SHA_CTX ctx;
    if(!MakeSha1Init(ctx))
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "MakeSha1Init fail file:%s"), file.c_str());
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
        if(!KERNEL_NS::LibDigest::MakeSha1Continue(ctx, buffer, readBytes))
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "MakeSha1Continue fail file:%s"), file.c_str());
            isSuc = false;
            break;
        }

    }while(true);

    if(!isSuc)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "make sha1 fail file:%s"), file.c_str());
        KERNEL_NS::LibDigest::MakeSha1Clean(ctx);
        return false;
    }

    if(!KERNEL_NS::LibDigest::MakeSha1Final(ctx, sha1Out))
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "MakeSha1Final file:%s"), file.c_str());
        KERNEL_NS::LibDigest::MakeSha1Clean(ctx);
        return false;
    }
    
    KERNEL_NS::LibDigest::MakeSha1Clean(ctx);

    return true;
}

bool LibDigest::MakeFileSha224(const LibString &file, LibString &result)
{
    // 打开文件
    SmartPtr<FILE, AutoDelMethods::CustomDelete> fp = FileUtil::OpenFile(file.c_str(), false, "rb");
    if(!fp)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "open file fail file:%s"), file.c_str());
        return false;
    }
    
    // 释放资源
    fp.SetClosureDelegate([](void *ptr){
        auto p = reinterpret_cast<FILE *>(ptr);
        FileUtil::CloseFile(*p);
    });

    SHA256_CTX ctx;
    if(!MakeSha224Init(ctx))
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "MakeSha224Init fail file:%s"), file.c_str());
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
        if(!KERNEL_NS::LibDigest::MakeSha224Continue(ctx, buffer, readBytes))
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "MakeSha224Continue fail file:%s"), file.c_str());
            isSuc = false;
            break;
        }

    }while(true);

    if(!isSuc)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "make sha224 fail file:%s"), file.c_str());
        KERNEL_NS::LibDigest::MakeSha224Clean(ctx);
        return false;
    }

    if(!KERNEL_NS::LibDigest::MakeSha224Final(ctx, result))
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "MakeSha224Final file:%s"), file.c_str());
        KERNEL_NS::LibDigest::MakeSha224Clean(ctx);
        return false;
    }
    
    KERNEL_NS::LibDigest::MakeSha224Clean(ctx);

    return true;
}

bool LibDigest::MakeFileSha256(const LibString &file, LibString &result)
{
    // 打开文件
    SmartPtr<FILE, AutoDelMethods::CustomDelete> fp = FileUtil::OpenFile(file.c_str(), false, "rb");
    if(!fp)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "open file fail file:%s"), file.c_str());
        return false;
    }
    
    // 释放资源
    fp.SetClosureDelegate([](void *ptr){
        auto p = reinterpret_cast<FILE *>(ptr);
        FileUtil::CloseFile(*p);
    });

    SHA256_CTX ctx;
    if(!MakeSha256Init(ctx))
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "MakeSha256Init fail file:%s"), file.c_str());
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
        if(!KERNEL_NS::LibDigest::MakeSha256Continue(ctx, buffer, readBytes))
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "MakeSha256Continue fail file:%s"), file.c_str());
            isSuc = false;
            break;
        }

    }while(true);

    if(!isSuc)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "make sha256 fail file:%s"), file.c_str());
        KERNEL_NS::LibDigest::MakeSha256Clean(ctx);
        return false;
    }

    if(!KERNEL_NS::LibDigest::MakeSha256Final(ctx, result))
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "MakeSha256Final file:%s"), file.c_str());
        KERNEL_NS::LibDigest::MakeSha256Clean(ctx);
        return false;
    }
    
    KERNEL_NS::LibDigest::MakeSha256Clean(ctx);

    return true;
}

bool LibDigest::MakeFileSha384(const LibString &file, LibString &result)
{
    // 打开文件
    SmartPtr<FILE, AutoDelMethods::CustomDelete> fp = FileUtil::OpenFile(file.c_str(), false, "rb");
    if(!fp)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "open file fail file:%s"), file.c_str());
        return false;
    }
    
    // 释放资源
    fp.SetClosureDelegate([](void *ptr){
        auto p = reinterpret_cast<FILE *>(ptr);
        FileUtil::CloseFile(*p);
    });

    SHA512_CTX ctx;
    if(!MakeSha384Init(ctx))
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "MakeSha384Init fail file:%s"), file.c_str());
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
        if(!KERNEL_NS::LibDigest::MakeSha384Continue(ctx, buffer, readBytes))
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "MakeSha384Continue fail file:%s"), file.c_str());
            isSuc = false;
            break;
        }

    }while(true);

    if(!isSuc)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "make sha384 fail file:%s"), file.c_str());
        KERNEL_NS::LibDigest::MakeSha384Clean(ctx);
        return false;
    }

    if(!KERNEL_NS::LibDigest::MakeSha384Final(ctx, result))
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "MakeSha384Final file:%s"), file.c_str());
        KERNEL_NS::LibDigest::MakeSha384Clean(ctx);
        return false;
    }
    
    KERNEL_NS::LibDigest::MakeSha384Clean(ctx);

    return true;
}

bool LibDigest::MakeFileSha512(const LibString &file, LibString &result)
{
    // 打开文件
    SmartPtr<FILE, AutoDelMethods::CustomDelete> fp = FileUtil::OpenFile(file.c_str(), false, "rb");
    if(!fp)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "open file fail file:%s"), file.c_str());
        return false;
    }
    
    // 释放资源
    fp.SetClosureDelegate([](void *ptr){
        auto p = reinterpret_cast<FILE *>(ptr);
        FileUtil::CloseFile(*p);
    });

    SHA512_CTX ctx;
    if(!MakeSha512Init(ctx))
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "MakeSha512Init fail file:%s"), file.c_str());
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
        if(!KERNEL_NS::LibDigest::MakeSha512Continue(ctx, buffer, readBytes))
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "MakeSha512Continue fail file:%s"), file.c_str());
            isSuc = false;
            break;
        }

    }while(true);

    if(!isSuc)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "make sha512 fail file:%s"), file.c_str());
        KERNEL_NS::LibDigest::MakeSha512Clean(ctx);
        return false;
    }

    if(!KERNEL_NS::LibDigest::MakeSha512Final(ctx, result))
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(LibDigest, "MakeSha512Final file:%s"), file.c_str());
        KERNEL_NS::LibDigest::MakeSha512Clean(ctx);
        return false;
    }
    
    KERNEL_NS::LibDigest::MakeSha512Clean(ctx);

    return true;
}

KERNEL_END
