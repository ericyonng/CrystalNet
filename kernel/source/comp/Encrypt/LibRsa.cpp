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

#include <kernel/common/macro.h>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
  #include <WinSock2.h>
#endif

#include <kernel/comp/memory/MemoryPool.h>
#include <kernel/comp/Coder/base64.h>
#include <kernel/comp/Utils/BitUtil.h>
#include <kernel/comp/Utils/FileUtil.h>

// openssl 请注意内存释放
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/buffer.h>
#include <openssl/bn.h>

#include <kernel/comp/Encrypt/LibRsa.h>


KERNEL_BEGIN

const Int32 LibRsaDefs::PADDING_MODE_BEGIN = 0;
const Int32 LibRsaDefs::PADDING_MODE_PKCS1_PADDING = RSA_PKCS1_PADDING;
const Int32 LibRsaDefs::PADDING_MODE_NO_PADDING = RSA_NO_PADDING;
const Int32 LibRsaDefs::PADDING_MODE_PKCS1_OAEP_PADDING = RSA_PKCS1_OAEP_PADDING;
const Int32 LibRsaDefs::PADDING_MODE_END =  RSA_PKCS1_OAEP_PADDING + 1;

const Int32 LibRsaDefs::DIGEST_TYPE_SHA1 = NID_sha1;
const Int32 LibRsaDefs::DIGEST_TYPE_SHA224 = NID_sha224;
const Int32 LibRsaDefs::DIGEST_TYPE_SHA256 = NID_sha256;
const Int32 LibRsaDefs::DIGEST_TYPE_SHA384 = NID_sha384;
const Int32 LibRsaDefs::DIGEST_TYPE_SHA512 = NID_sha512;
const Int32 LibRsaDefs::DIGEST_TYPE_MD5 = NID_md5;

static ALWAYS_INLINE  Int32 GetPaddingSize(Int32 padding)
{
    static const Int32 paddingSize[LibRsaDefs::PADDING_MODE_END] = {
        -1,
        RSA_PKCS1_PADDING_SIZE, // PKCS1_PADDING size
        -1,
        0, // RSA_NO_PADDING 不填充
        41, // RSA_PKCS1_OAEP_PADDING 
    };

    return paddingSize[padding];
}

LibRsa::LibRsa()
:_mode(LibRsa::PUB_ENCRYPT_PRIV_DECRYPT)
,_pubRsa(NULL)
,_privateRsa(NULL)
,_padding(RSA_PKCS1_PADDING)
,_lastErr(ERR_LIB_NONE)
,_keyBits(0)
{

}

LibRsa::~LibRsa()
{
    if(_pubRsa)
        RSA_free((RSA *)_pubRsa);

    if(_privateRsa)
        RSA_free((RSA *)_privateRsa);
}

bool LibRsa::GenKey(Int32 keyBits, UInt32 flags, bool needImport)
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
    auto pubPkc1Bio = BIO_new(BIO_s_mem());
    auto pubPkc8Bio = BIO_new(BIO_s_mem());

    _lastErr = PEM_write_bio_RSAPrivateKey(privateBio, newRsa, NULL, NULL, 0, NULL, NULL);
    if(UNLIKELY(_lastErr != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("PEM_write_bio_RSAPrivateKey fail err[%d]", _lastErr);
        BN_free(newBigNum);
        RSA_free(newRsa);
        BIO_free(privateBio);
        BIO_free(pubPkc1Bio);
        BIO_free(pubPkc8Bio);
        return false;
    }

    // public key 是pkc1格式
    if((flags & LibRsa::PUB_PKC1_FLAG) == LibRsa::PUB_PKC1_FLAG)
    {
        _lastErr = PEM_write_bio_RSAPublicKey(pubPkc1Bio, newRsa);
        if(UNLIKELY(_lastErr != ERR_LIB_NONE))
        {
            CRYSTAL_TRACE("PEM_write_bio_RSAPublicKey fail err[%d]", _lastErr);
            BN_free(newBigNum);
            RSA_free(newRsa);
            BIO_free(privateBio);
            BIO_free(pubPkc1Bio);
            BIO_free(pubPkc8Bio);

            return false;
        }
    }
    
    if((flags & LibRsa::PUB_PKC8_FLAG) == LibRsa::PUB_PKC8_FLAG)
    {
        _lastErr = PEM_write_bio_RSA_PUBKEY(pubPkc8Bio, newRsa);
        if(UNLIKELY(_lastErr != ERR_LIB_NONE))
        {
            CRYSTAL_TRACE("PEM_write_bio_RSA_PUBKEY fail err[%d]", _lastErr);
            BN_free(newBigNum);
            RSA_free(newRsa);
            BIO_free(privateBio);
            BIO_free(pubPkc1Bio);
            BIO_free(pubPkc8Bio);

            return false;
        }
    }
    
    if((flags & LibRsa::PUB_PKC1_FLAG) != LibRsa::PUB_PKC1_FLAG && 
    ((flags & LibRsa::PUB_PKC8_FLAG) != LibRsa::PUB_PKC8_FLAG))
    {
        CRYSTAL_TRACE("bad flags [%x]", flags);
        BN_free(newBigNum);
        RSA_free(newRsa);
        BIO_free(privateBio);
        BIO_free(pubPkc1Bio);
        BIO_free(pubPkc8Bio);

        return false;
    }

    // 内存池
    auto pool = KernelGetTlsMemoryPool();

    // 获取key长度
    if((flags & LibRsa::PUB_PKC1_FLAG) == LibRsa::PUB_PKC1_FLAG)
    {
        Int32 pubPkc1Len = static_cast<Int32>(BIO_pending(pubPkc1Bio));
        // 读取公钥
        auto pubPkc1Buffer = static_cast<Byte8 *>(pool->Alloc(static_cast<UInt64>(pubPkc1Len)));
        pubPkc1Len = BIO_read(pubPkc1Bio, pubPkc1Buffer, pubPkc1Len);
        if(UNLIKELY(pubPkc1Len <= 0))
        {
            CRYSTAL_TRACE("BIO_read pub key fail pubLen[%d]", pubPkc1Len);
            BN_free(newBigNum);
            RSA_free(newRsa);
            BIO_free(privateBio);
            BIO_free(pubPkc1Bio);
            BIO_free(pubPkc8Bio);

            pool->Free(pubPkc1Buffer);
            return false;       
        }
        pubPkc1Buffer[pubPkc1Len] = '\0';
        _pubPkc1Key = pubPkc1Buffer;
        pool->Free(pubPkc1Buffer);

    }

    if((flags & LibRsa::PUB_PKC8_FLAG) == LibRsa::PUB_PKC8_FLAG)
    {
        Int32 pubPkc8Len = static_cast<Int32>(BIO_pending(pubPkc8Bio));
        // 读取公钥
        auto pubPkc8Buffer = static_cast<Byte8 *>(pool->Alloc(static_cast<UInt64>(pubPkc8Len)));
        pubPkc8Len = BIO_read(pubPkc8Bio, pubPkc8Buffer, pubPkc8Len);
        if(UNLIKELY(pubPkc8Len <= 0))
        {
            CRYSTAL_TRACE("BIO_read pub key fail pubLen[%d]", pubPkc8Len);
            BN_free(newBigNum);
            RSA_free(newRsa);
            BIO_free(privateBio);
            BIO_free(pubPkc1Bio);
            BIO_free(pubPkc8Bio);

            pool->Free(pubPkc8Buffer);
            return false;       
        }
        pubPkc8Buffer[pubPkc8Len] = '\0';
        _pubPkc8Key = pubPkc8Buffer;
        pool->Free(pubPkc8Buffer);
    }

    // 读取私钥
    Int32 privateLen = static_cast<Int32>(BIO_pending(privateBio));
    auto privateBuffer = static_cast<Byte8 *>(pool->Alloc(static_cast<UInt64>(privateLen)));
    privateLen = BIO_read(privateBio, privateBuffer, privateLen);
    if(UNLIKELY(privateLen <= 0))
    {
        CRYSTAL_TRACE("BIO_read private key fail privateLen[%d]", privateLen);
        BN_free(newBigNum);
        RSA_free(newRsa);
        BIO_free(privateBio);
        BIO_free(pubPkc1Bio);
        BIO_free(pubPkc8Bio);

        pool->Free(privateBuffer);
        return false;       
    }

    // 存储密钥对
    privateBuffer[privateLen] = '\0';
    _privateKey = privateBuffer;

    // 内存释放
    BN_free(newBigNum);
    RSA_free(newRsa);
    BIO_free(privateBio);
    BIO_free(pubPkc1Bio);
    BIO_free(pubPkc8Bio);

    pool->Free(privateBuffer);

    // 生成对应的rsa
    if(needImport)
    {
        if(!_pubPkc1Key.empty())
        {
            flags &= ~LibRsa::PUB_PKC8_FLAG;
            if(!ImportKey(&_pubPkc1Key, &_privateKey, flags))
            {
                CRYSTAL_TRACE("ImportKey fail err[%d]", _lastErr);
                return false;
            }
        }
        else if(!_pubPkc8Key.empty())
        {
            flags &= ~LibRsa::PUB_PKC1_FLAG;
            if(!ImportKey(&_pubPkc8Key, &_privateKey, flags))
            {
                CRYSTAL_TRACE("ImportKey fail err[%d]", _lastErr);
                return false;
            }
        }
    }

    _keyBits = keyBits;

    return true;
}

bool LibRsa::ImportKey(const LibString *pubKey, const LibString *privateKey, UInt32 flags)
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
    if(pubKey && !pubKey->empty())
    {
        if((flags & LibRsa::PUB_PKC1_FLAG) == LibRsa::PUB_PKC1_FLAG)
        {
            _pubPkc1Key = *pubKey;
        }
        else if((flags & LibRsa::PUB_PKC8_FLAG) == LibRsa::PUB_PKC8_FLAG)
        {
            _pubPkc8Key = *pubKey;
        }
        else
        {
            CRYSTAL_TRACE("bad flags flags:[%x]", flags);
            return false;
        }

        // 释放公钥 TODO:需要检测内存增长情况,网上反馈有内存泄漏
        if(UNLIKELY(_pubRsa))
            RSA_free((RSA *)_pubRsa);

        // 从已有的内存字符串创建bio
        BIO *bio = BIO_new_mem_buf(pubKey->c_str(), -1);

        if((flags & LibRsa::PUB_PKC1_FLAG) == LibRsa::PUB_PKC1_FLAG)
        {
            // pkc#1 PEM_read_bio_RSAPublicKey pkc#8 PEM_read_bio_RSA_PUBKEY
            _pubRsa = PEM_read_bio_RSAPublicKey(bio, (RSA **)(&_pubRsa), NULL, NULL);
            BIO_free(bio);

            if(!_pubRsa)
            {
                CRYSTAL_TRACE("pub rsa PEM_read_bio_RSAPublicKey fail");
                return false;
            }
        }
        else if((flags & LibRsa::PUB_PKC8_FLAG) == LibRsa::PUB_PKC8_FLAG)
        {
            // pkc#1 PEM_read_bio_RSAPublicKey pkc#8 PEM_read_bio_RSA_PUBKEY
            _pubRsa = PEM_read_bio_RSA_PUBKEY(bio, (RSA **)&_pubRsa, NULL, NULL);
            BIO_free(bio);

            if(!_pubRsa)
            {
                CRYSTAL_TRACE("pub rsa PEM_read_bio_RSA_PUBKEY fail");
                return false;
            }
        }
        else
        {
            CRYSTAL_TRACE("bad flags [%x]", flags);
            BIO_free(bio);
            return false;
        }

        // key的位数
        Int32 lenBits = static_cast<Int32>(RSA_size((RSA *)_pubRsa)) * 8;
        pubBits = __getBits(lenBits);
        if(UNLIKELY(!pubBits))
        {
            CRYSTAL_TRACE("pub key not surport lenBits = [%d]", lenBits);
            RSA_free((RSA *)_pubRsa);
            _pubRsa = NULL;
            return false;
        }

        // CRYSTAL_TRACE("import pubBits[%d]", pubBits);
    }

    // 导入私钥
    Int32 privateBits = 0;
    if(privateKey && !privateKey->empty())
    {
        _privateKey = *privateKey;

        // 释放私钥 TODO:需要检测内存增长情况,网上反馈有内存泄漏
        if(UNLIKELY(_privateRsa))
            RSA_free((RSA *)_privateRsa);

        BIO *bio = BIO_new_mem_buf(privateKey->c_str(), -1);
        _privateRsa = PEM_read_bio_RSAPrivateKey(bio, (RSA **)&_privateRsa, NULL, NULL);
        BIO_free(bio);
        
        if(!_privateRsa)
        {
            CRYSTAL_TRACE("private rsa PEM_read_bio_RSAPrivateKey fail");
            return false;
        }

        // key的位数
        Int32 lenBits = static_cast<Int32>(RSA_size((RSA *)_privateRsa)) * 8;
        privateBits = __getBits(lenBits);
        if(UNLIKELY(!privateBits))
        {
            CRYSTAL_TRACE("private key not surport lenBits = [%d]", lenBits);

            if(_pubRsa)
            {
                RSA_free((RSA *)_pubRsa);
                _pubRsa = NULL;
            }

            RSA_free((RSA *)_privateRsa);
            _privateRsa = NULL;

            return false;
        }

        // CRYSTAL_TRACE("import privateBits [%d]", privateBits);
    }

    // 私钥与公钥长度校验
    if(privateKey && (!privateKey->empty()) && pubKey && (!pubKey->empty()) && (privateBits != pubBits))
    {
        CRYSTAL_TRACE("private key pub key not match!!!! privateBits=[%d], pubBits=[%d]"
        , privateBits, pubBits);

        RSA_free((RSA *)_pubRsa);
        _pubRsa = NULL;
        RSA_free((RSA *)_privateRsa);
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

Int32 LibRsa::PubKeyEncrypt(const U8 *plainText, Int64 plainTextLen, U8 *cypherText)
{
    // 填充长度
    const Int32 paddingSize = GetPaddingSize(_padding);
    // 求出有效限制的长度
    const Int64 effectSize = static_cast<Int64>(RSA_size((RSA *)_pubRsa) - paddingSize);
    
    // 循环分组加密
    UInt64 blockSizeToHandle = 0;
    Int32 totalBytes = 0;
    while (plainTextLen)
    {
        blockSizeToHandle = (plainTextLen < effectSize) ? plainTextLen : effectSize;
        Int32 enBytes = RSA_public_encrypt(static_cast<Int32>(blockSizeToHandle), plainText
        , cypherText, (RSA *)_pubRsa, _padding);

        if(UNLIKELY(enBytes <= 0))
        {
            CRYSTAL_TRACE("RSA_public_encrypt fail err = [%d] "
            " blockSizeToHandle[%llu] plainTextSize[%llu] effectSize[%lld] "
            "paddingSize[%d] totalBytes[%d]"
            , _lastErr, blockSizeToHandle, plainTextLen
            , effectSize, paddingSize, totalBytes);

            return -1;
        }

        plainText += blockSizeToHandle;
        cypherText += enBytes;              // 加密后输出modulusSize长度
        totalBytes += enBytes;
        plainTextLen -= blockSizeToHandle;
    }

    if (UNLIKELY(plainTextLen))
        CRYSTAL_TRACE("PubKeyEncrypt left plain text plainTextLen[%lld]", plainTextLen);

    return totalBytes;
}

Int32 LibRsa::PrivateKeyDecrypt(const KERNEL_NS::LibString &cypherText, KERNEL_NS::LibString &plainText)
{
    const auto cypherLen = static_cast<UInt64>(cypherText.size());
    auto plainSize = CalcPlainSizeByPrivKeyCypher(cypherLen);
    plainText.resize(plainSize);
    
    U8 *p = (U8 *)(plainText.data());
    auto finalBytes = PrivateKeyDecrypt((const U8 *)(cypherText.data()), static_cast<Int64>(cypherLen), p);
    if(UNLIKELY(finalBytes <= 0))
    {
        plainText.clear();
    }
    else
    {
        plainText.resize(finalBytes);
    }

    return finalBytes;
}

Int32 LibRsa::PrivateKeyDecrypt(const U8 *cypherText, Int64 cypherLen, U8 *plainText)
{
    // 模长即输出的长度
    const Int64 modulusSize = static_cast<Int64>(RSA_size((RSA *)_privateRsa));

    // 循环分组解密
    Int32 totalDecodeBytes = 0;
    while (cypherLen)
    {
        Int32 decodeLen = RSA_private_decrypt(static_cast<Int32>(modulusSize), cypherText
            , plainText, (RSA *)_privateRsa, _padding);
        if(UNLIKELY(decodeLen <= 0))
        {
            CRYSTAL_TRACE("RSA_private_decrypt fail totalDecodeBytes = [%d]", totalDecodeBytes);
            return -1;
        }

        cypherText += modulusSize;
        cypherLen -= modulusSize;
        plainText += decodeLen;
        totalDecodeBytes += decodeLen;
    }

    if(UNLIKELY(cypherLen))
        CRYSTAL_TRACE("left cypher text size[%lld]", cypherLen);

    return totalDecodeBytes;
}

Int32 LibRsa::PrivateKeyDecrypt(const U8 *cypherText, Int64 cypherLen, KERNEL_NS::LibString &plainText)
{
    auto plainSize = CalcPlainSizeByPrivKeyCypher(cypherLen);
    plainText.resize(plainSize);
    
    U8 *p = (U8 *)(plainText.data());
    auto finalBytes = PrivateKeyDecrypt(cypherText, cypherLen, p);
    if(UNLIKELY(finalBytes <= 0))
    {
        plainText.clear();
    }
    else
    {
        plainText.resize(finalBytes);
    }

    return finalBytes;
}

Int32 LibRsa::PrivateKeyEncrypt(const KERNEL_NS::LibString &plainText, KERNEL_NS::LibString &cypherText)
{
    auto len = static_cast<UInt64>(plainText.size());
    auto calcLen = CalcCypherSizeByPrivKeyPlain(len);
    cypherText.resize(calcLen);
    
    U8 * finalCypher = (U8 *)(cypherText.data());
    auto finalBytes = PrivateKeyEncrypt((const U8 *)(plainText.data()), static_cast<Int64>(len), finalCypher);
    if(UNLIKELY(finalBytes <= 0))
    {
        cypherText.clear();
    }
    else
    {
        cypherText.resize(finalBytes);
    }

    return finalBytes;
}

Int32 LibRsa::PrivateKeyEncrypt(const U8 *plainText, Int64 plainTextLen, U8 *cypherText)
{
        // 填充长度
    const Int32 paddingSize = GetPaddingSize(_padding);
    // 求出有效限制的长度
    const Int64 effectSize = static_cast<Int64>(RSA_size((RSA *)_privateRsa) - paddingSize);
    
    // 循环分组加密
    UInt64 blockSizeToHandle = 0;
    Int32 totalBytes = 0;
    while (plainTextLen)
    {
        blockSizeToHandle = (plainTextLen < effectSize) ? plainTextLen : effectSize;
        auto enBytes = RSA_private_encrypt(static_cast<Int32>(blockSizeToHandle), plainText
        , cypherText, (RSA *)_privateRsa, _padding);

        if(UNLIKELY(enBytes <= 0))
        {
            cypherText[0] = 0;
            CRYSTAL_TRACE("RSA_private_encrypt fail err = [%d] "
            " blockSizeToHandle[%llu] plainTextSize[%llu] effectSize[%lld] "
            "paddingSize[%d] totalBytes[%d]"
            , _lastErr, blockSizeToHandle, plainTextLen
            , effectSize, paddingSize, totalBytes);

            return -1;
        }

        plainText += blockSizeToHandle;
        cypherText += enBytes;              // 加密后输出modulusSize长度
        totalBytes += enBytes;
        plainTextLen -= blockSizeToHandle;
    }

    if (UNLIKELY(plainTextLen))
        CRYSTAL_TRACE("PrivateKeyEncrypt left plain text plainTextLen[%lld]", plainTextLen);

    return totalBytes;
}

Int32 LibRsa::PubKeyDecrypt(const KERNEL_NS::LibString &cypherText, KERNEL_NS::LibString &plainText)
{
    const auto cypherLen = static_cast<UInt64>(cypherText.size());
    auto plainSize = CalcPlainSizeByPubKeyCypher(cypherLen);
    plainText.resize(plainSize);
    
    U8 *p = (U8 *)(plainText.data());
    auto finalBytes = PubKeyDecrypt((const U8 *)(cypherText.data()), static_cast<Int64>(cypherLen), p);
    if(UNLIKELY(finalBytes <= 0))
    {
        plainText.clear();
    }
    else
    {
        plainText.resize(finalBytes);
    }

    return finalBytes;
}

Int32 LibRsa::PubKeyDecrypt(const U8 *cypherText, Int64 cypherLen, U8 *plainText)
{
    // 模长即输出的长度
    const Int64 modulusSize = static_cast<UInt64>(RSA_size((RSA *)_pubRsa));

    // 循环分组解密
    Int32 totalDecodeBytes = 0;
    while (cypherLen)
    {
        auto decodeLen = RSA_public_decrypt(static_cast<Int32>(modulusSize), cypherText, plainText, (RSA *)_pubRsa, _padding);
        if(UNLIKELY(decodeLen <= 0))
        {
            CRYSTAL_TRACE("RSA_public_decrypt fail totalDecodeBytes = [%d]", totalDecodeBytes);
            return -1;
        }

        cypherText += modulusSize;
        cypherLen -= modulusSize;
        plainText += decodeLen;
        totalDecodeBytes += decodeLen;

    }

    if(UNLIKELY(cypherLen))
        CRYSTAL_TRACE("left cypher text size[%lld]", cypherLen);

    return totalDecodeBytes;
}

Int32 LibRsa::PubKeyDecrypt(const U8 *cypherText, Int64 cypherLen, KERNEL_NS::LibString &plainText)
{
    auto plainSize = CalcPlainSizeByPubKeyCypher(cypherLen);
    plainText.resize(plainSize);
    
    U8 *p = (U8 *)(plainText.data());
    auto finalBytes = PubKeyDecrypt(cypherText, cypherLen, p);
    if(UNLIKELY(finalBytes <= 0))
    {
        plainText.clear();
    }
    else
    {
        plainText.resize(finalBytes);
    }

    return finalBytes;
}

void LibRsa::PubKeyToFile(FILE *fp)
{
    FileUtil::WriteFile(*fp, _pubPkc1Key);
    FileUtil::FlushFile(*fp);
}

void LibRsa::PrivateKeyToFile(FILE *fp)
{
    FileUtil::WriteFile(*fp, _privateKey);
    FileUtil::FlushFile(*fp);
}

UInt64 LibRsa::CalcCypherSizeByPubKeyPlain(UInt64 plainTextSize) const
{
    // 填充长度
    const Int32 paddingSize = GetPaddingSize(_padding);
    // 模长即输出的长度
    const UInt64 modulusSize = static_cast<UInt64>(RSA_size((RSA *)_pubRsa));
    // 求出有效限制的长度
    const UInt64 effectSize = static_cast<UInt64>(RSA_size((RSA *)_pubRsa) - paddingSize);
    
    const auto multiple = plainTextSize / effectSize;
    const auto leftMultiple = (plainTextSize % effectSize) ? 1 : 0;
    return (multiple + leftMultiple) * modulusSize + 1;
}

UInt64 LibRsa::CalcCypherSizeByPrivKeyPlain(UInt64 plainTextSize) const
{
    // 填充长度
    const Int32 paddingSize = GetPaddingSize(_padding);
    // 模长即输出的长度
    const UInt64 modulusSize = static_cast<UInt64>(RSA_size((RSA *)_privateRsa));
    // 求出有效限制的长度
    const UInt64 effectSize = static_cast<UInt64>(RSA_size((RSA *)_privateRsa) - paddingSize);

    const auto multiple = plainTextSize / effectSize;
    const auto leftMultiple = (plainTextSize % effectSize) ? 1 : 0;
    return (multiple + leftMultiple) * modulusSize + 1;
}

UInt64 LibRsa::CalcPlainSizeByPubKeyCypher(UInt64 cypherLen) const
{
    // 填充长度
    const Int32 paddingSize = GetPaddingSize(_padding);
    // 模长即输出的长度
    const UInt64 modulusSize = static_cast<UInt64>(RSA_size((RSA *)_pubRsa));
    // 求出有效限制的长度
    const UInt64 effectSize = static_cast<UInt64>(RSA_size((RSA *)_pubRsa) - paddingSize);

    return (cypherLen / modulusSize) * effectSize + 1;
}

UInt64 LibRsa::CalcPlainSizeByPrivKeyCypher(UInt64 cypherLen) const
{
    // 填充长度
    const Int32 paddingSize = GetPaddingSize(_padding);
    // 模长即输出的长度
    const UInt64 modulusSize = static_cast<UInt64>(RSA_size((RSA *)_privateRsa));
    // 求出有效限制的长度
    const UInt64 effectSize = static_cast<UInt64>(RSA_size((RSA *)_privateRsa) - paddingSize);

    return (cypherLen / modulusSize) * effectSize + 1;
}

Int32 LibRsa::PubKeyEncrypt(const KERNEL_NS::LibString &plainText, KERNEL_NS::LibString &cypherText)
{
    auto len = static_cast<UInt64>(plainText.size());
    auto calcLen = CalcCypherSizeByPubKeyPlain(len);
    cypherText.resize(calcLen);
    
    U8 * finalCypher = (U8 *)(cypherText.data());
    auto finalBytes = PubKeyEncrypt((const U8 *)(plainText.data()), static_cast<Int64>(len), finalCypher);
    if(UNLIKELY(finalBytes <= 0))
    {
        cypherText.clear();
    }
    else
    {
        cypherText.resize(finalBytes);
    }

    return finalBytes;
}

UInt32 LibRsa::SignDigest(Int32 digestType, const U8 *digest, UInt32 digestLen, U8 *signBuffer, UInt32 &signBufferSize)
{
    _lastErr = RSA_sign(digestType, digest, digestLen, signBuffer, &signBufferSize, (RSA *)_privateRsa);
    if(UNLIKELY(_lastErr != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("RSA_sign fail digestType[%d] signBufferSize[%u] _lastErr[%d]", digestType, signBufferSize, _lastErr);
        return 0;
    }

    return signBufferSize;
}

bool LibRsa::VerifyDigest(Int32 digestType, const U8 *digest, UInt32 digestLen, const U8 *signData, UInt32 signLen)
{
    _lastErr = RSA_verify(digestType, digest, digestLen, signData, signLen, (RSA *)_pubRsa);
    if(UNLIKELY(_lastErr != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("RSA_verify fail digestType[%d] _lastErr[%d]", digestType, _lastErr);
        return false;
    }

    return true;
}

UInt32 LibRsa::CalcSignSize()
{
    return static_cast<UInt32>(RSA_size((RSA *)_privateRsa) + 1);
}
KERNEL_END