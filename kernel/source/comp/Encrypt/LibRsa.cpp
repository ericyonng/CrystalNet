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
 * Date: 2021-02-03 14:31:58
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Encrypt/LibRsa.h>
#include <kernel/comp/memory/MemoryPool.h>
#include <kernel/comp/Coder/base64.h>


KERNEL_BEGIN
LibRsa::LibRsa()
:_pubRsa(NULL)
,_privateRsa(NULL)
,_padding(RSA_PKCS1_PADDING)
,_lastErr(ERR_LIB_NONE)
,_keyBits(0)
, _pubBeginEnd{"-----BEGIN RSA PUBLIC KEY-----", "-----END RSA PUBLIC KEY-----"}
, _privateBeginEnd{" -----BEGIN RSA PRIVATE KEY-----", " -----END RSA PRIVATE KEY-----"}
{

}

LibRsa::~LibRsa()
{
    if(_pubRsa)
        RSA_free(_pubRsa);

    if(_privateRsa)
        RSA_free(_privateRsa);
}

bool LibRsa::GenKey(Int32 keyBits)
{
    // 支持的key位数
    const std::set<Int32> *supportBits = LibRsaDefs::GetSupportBits();
    if(supportBits->find(keyBits) == supportBits->end())
    {
        CRYSTAL_TRACE("cant support key bits[%d]", keyBits);
        return false;
    }

    auto newBigNum = BN_new();
    _lastErr = BN_set_word(newBigNum, RSA_F4);
    if(UNLIKELY(_lastErr != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("BN_set_word big num RSA_F4 fail err[%d]", _lastErr);
        BN_free(newBigNum);
        return false;
    }

    // 生成密钥对
    auto newRsa = RSA_new();
    _lastErr = RSA_generate_key_ex(newRsa, keyBits, newBigNum, NULL);
    if(UNLIKELY(_lastErr != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("RSA_generate_key_ex fail err[%d]", _lastErr);
        BN_free(newBigNum);
        RSA_free(newRsa);
        return false;
    }

    // 拆分密钥对(生成bio 然后将密钥对导出到bio,再由bio分别创建两个密钥对应的rsa)
    auto privateBio = BIO_new(BIO_s_mem());
    auto pubBio = BIO_new(BIO_s_mem());

    _lastErr = PEM_write_bio_RSAPrivateKey(privateBio, newRsa, NULL, NULL, 0, NULL, NULL);
    if(UNLIKELY(_lastErr != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("PEM_write_bio_RSAPrivateKey fail err[%d]", _lastErr);
        BN_free(newBigNum);
        RSA_free(newRsa);
        BIO_free(privateBio);
        return false;
    }

    _lastErr = PEM_write_bio_RSAPublicKey(pubBio, newRsa);
    if(UNLIKELY(_lastErr != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("PEM_write_bio_RSAPublicKey fail err[%d]", _lastErr);
        BN_free(newBigNum);
        RSA_free(newRsa);
        BIO_free(privateBio);
        BIO_free(pubBio);
        return false;
    }

    // 获取key长度
    Int32 pubLen = static_cast<Int32>(BIO_pending(pubBio));
    Int32 privateLen = static_cast<Int32>(BIO_pending(privateBio));

    // 内存池
    auto pool = KernelGetTlsMemoryPool();

    // 读取公钥
    auto pubBuffer = static_cast<Byte8 *>(pool->Alloc(static_cast<UInt64>(pubLen)));
    pubLen = BIO_read(pubBio, pubBuffer, pubLen);
    if(UNLIKELY(pubLen <= 0))
    {
        CRYSTAL_TRACE("BIO_read pub key fail pubLen[%d]", pubLen);
        BN_free(newBigNum);
        RSA_free(newRsa);
        BIO_free(privateBio);
        BIO_free(pubBio);

        pool->Free(pubBuffer);
        return false;       
    }

    // 读取私钥
    auto privateBuffer = static_cast<Byte8 *>(pool->Alloc(static_cast<UInt64>(privateLen)));
    privateLen = BIO_read(privateBio, privateBuffer, privateLen);
    if(UNLIKELY(privateLen <= 0))
    {
        CRYSTAL_TRACE("BIO_read private key fail privateLen[%d]", privateLen);
        BN_free(newBigNum);
        RSA_free(newRsa);
        BIO_free(privateBio);
        BIO_free(pubBio);

        pool->Free(pubBuffer);
        pool->Free(privateBuffer);
        return false;       
    }

    // 存储密钥对
    pubBuffer[pubLen] = '\0';
    privateBuffer[privateLen] = '\0';
    _pubKey = pubBuffer;
    _privateKey = privateBuffer;

    // 内存释放
    BN_free(newBigNum);
    RSA_free(newRsa);
    BIO_free(privateBio);
    BIO_free(pubBio);
    pool->Free(pubBuffer);
    pool->Free(privateBuffer);

    // 生成对应的rsa
    if(!ImportKey(&_pubKey, &_privateKey))
    {
        CRYSTAL_TRACE("ImportKey fail err[%d]", _lastErr);
        BN_free(newBigNum);
        RSA_free(newRsa);
        BIO_free(privateBio);
        BIO_free(pubBio);

        pool->Free(pubBuffer);
        pool->Free(privateBuffer);
        return false;
    }

    _keyBits = keyBits;

    return true;
}

bool LibRsa::ImportKey(const LibString *pubKey, const LibString *privateKey)
{
    // 支持的key位数
    const std::set<Int32> *supportBits = LibRsaDefs::GetSupportBits();

    // 定义获取位数
    auto __getBits = [&supportBits](Int32 lenBits)->Int32
    {
        for(auto keyBit: *supportBits)
        {
            if( lenBits <= keyBit )
                return keyBit;
        }        

        return 0;
    };

    // 导入公钥
    Int32 pubBits = 0;
    if(pubKey)
    {
        _pubKey = *pubKey;

        // 释放公钥 TODO:需要检测内存增长情况,网上反馈有内存泄漏
        if(UNLIKELY(_pubRsa))
            RSA_free(_pubRsa);

        // 从已有的内存字符串创建bio
        BIO *bio = BIO_new_mem_buf(pubKey->c_str(), -1);
        // pkc#1 PEM_read_bio_RSAPublicKey pkc#8 PEM_read_bio_RSA_PUBKEY
        _pubRsa = PEM_read_bio_RSAPublicKey(bio, &_pubRsa, NULL, NULL);
        BIO_free(bio);

        if(!_pubRsa)
        {
            CRYSTAL_TRACE("pub rsa PEM_read_bio_RSA_PUBKEY fail");
            return false;
        }

        // key的位数
        Int32 lenBits = static_cast<Int32>(RSA_size(_pubRsa)) * 8;
        pubBits = __getBits(lenBits);
        if(UNLIKELY(!pubBits))
        {
            CRYSTAL_TRACE("pub key not surport lenBits = [%d]", lenBits);
            RSA_free(_pubRsa);
            _pubRsa = NULL;
            return false;
        }

        CRYSTAL_TRACE("import pubBits[%d]", pubBits);
    }

    // 导入私钥
    Int32 privateBits = 0;
    if(privateKey)
    {
        _privateKey = *privateKey;

        // 释放私钥 TODO:需要检测内存增长情况,网上反馈有内存泄漏
        if(UNLIKELY(_privateRsa))
            RSA_free(_privateRsa);

        BIO *bio = BIO_new_mem_buf(privateKey->c_str(), -1);
        _privateRsa = PEM_read_bio_RSAPrivateKey(bio, &_privateRsa, NULL, NULL);
        BIO_free(bio);
        
        if(!_privateRsa)
        {
            CRYSTAL_TRACE("private rsa PEM_read_bio_RSAPrivateKey fail");
            return false;
        }

        // key的位数
        Int32 lenBits = static_cast<Int32>(RSA_size(_privateRsa)) * 8;
        privateBits = __getBits(lenBits);
        if(UNLIKELY(!privateBits))
        {
            CRYSTAL_TRACE("private key not surport lenBits = [%d]", lenBits);

            if(_pubRsa)
            {
                RSA_free(_pubRsa);
                _pubRsa = NULL;
            }

            RSA_free(_privateRsa);
            _privateRsa = NULL;

            return false;
        }

        CRYSTAL_TRACE("import privateBits [%d]", privateBits);
    }

    // 私钥与公钥长度校验
    if(privateKey && pubKey && (privateBits != pubBits))
    {
        CRYSTAL_TRACE("private key pub key not match!!!! privateBits=[%d], pubBits=[%d]"
        , privateBits, pubBits);

        RSA_free(_pubRsa);
        _pubRsa = NULL;
        RSA_free(_privateRsa);
        _privateRsa = NULL;

        return false;
    }

    // 密钥位数
    if(pubBits)
        _keyBits = pubBits;
    if(privateBits)
        _keyBits = privateBits;

    return true;
}

KERNEL_END