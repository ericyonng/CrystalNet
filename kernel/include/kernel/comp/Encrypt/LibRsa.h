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
 * Date: 2021-02-03 14:24:13
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_ENCRYPT_LIB_RSA_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_ENCRYPT_LIB_RSA_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>
#include <3rd/3rdForKernel.h>
#include <kernel/comp/Encrypt/Defs.h>
#include <kernel/comp/Utils/FileUtil.h>

KERNEL_BEGIN

// key 指定pkc#1格式
class KERNEL_EXPORT LibRsa
{
private:
    enum PADDING_FORMAT_TYPE_POS: UInt32
    {
        PUB_PKC1_POS = 0,
        PUB_PKC8_POS,
    };

public:
    enum PADDING_FORMAT_TYPE : UInt32
    {
        PUB_PKC1_FLAG = 1U << LibRsa::PUB_PKC1_POS,
        PUB_PKC8_FLAG = 1U << LibRsa::PUB_PKC8_POS,
    };

    enum MODE_TYPE
    {
        // 公钥加密私钥解密模式
        PUB_ENCRYPT_PRIV_DECRYPT = 0,
        // 私钥加密公钥解密模式
        PRIV_ENCRYPT_PUB_DECRYPT = 1,
    };
public:
    LibRsa();
    ~LibRsa();

    // // 密钥
public:
    // 生成密钥对
    bool GenKey(Int32 keyBits = LibRsaDefs::RSA_1024, UInt32 flags = LibRsa::PUB_PKC1_FLAG, bool needImport = true);
    // 导入密钥 密钥必须是pkc#1字符串（带换行的）-----BEGIN RSA PUBLIC KEY-----
    bool ImportKey(const LibString *pubKey, const LibString *privateKey, UInt32 flags = LibRsa::PUB_PKC1_FLAG);

    // 获取key
    const LibString &GetPubPkc1Key() const;
    const LibString &GetPubPkc8Key() const;
    const LibString &GetPrivateKey() const;

    // 导出到文件
    void PubKeyToFile(FILE *fp);
    void PrivateKeyToFile(FILE *fp);

    // 设置加密模式
    void SetMode(Int32 mode);
    Int32 GetMode() const;
    bool IsPubEncryptPrivDecrypt() const;
    

    // 加解密前请保证有相应的key
public:
    // 只支持 RSA_PKCS1_PADDING 填充 11 BYTES
    // , RSA_PKCS1_OAEP_PADDING 填充 41 BYTES
    // , RSA_NO_PADDING 不填充
    bool SetPadding(Int32 padding = RSA_PKCS1_PADDING);
    Int32 GetLastErr() const;

    // 明文推算密文长度 公钥加密版本
    UInt64 CalcCypherSizeByPubKeyPlain(UInt64 plainTextSize) const;
    // 明文推算密文长度 私钥加密版本
    UInt64 CalcCypherSizeByPrivKeyPlain(UInt64 plainTextSize) const;
    // 密文推算明文长度 公钥解密版本
    UInt64 CalcPlainSizeByPubKeyCypher(UInt64 cypherLen) const;
    // 密文推算明文长度 私钥解密版本
    UInt64 CalcPlainSizeByPrivKeyCypher(UInt64 cypherLen) const;

    // 支持循环分组加/解密 明文/密文长度 与padding相关 <= RsaSize(rsa) - paddingSize
public:
    // 返回密文长度
    Int32 PubKeyEncrypt(const KERNEL_NS::LibString &plainText, KERNEL_NS::LibString &cypherText);
    Int32 PubKeyEncrypt(const U8 *plainText, Int64 plainTextLen, U8 *cypherText);
    // 返回明文长度
    Int32 PrivateKeyDecrypt(const KERNEL_NS::LibString &cypherText, KERNEL_NS::LibString &plainText);
    Int32 PrivateKeyDecrypt(const U8 *cypherText, Int64 cypherLen, U8 *plainText);
    Int32 PrivateKeyDecrypt(const U8 *cypherText, Int64 cypherLen, KERNEL_NS::LibString &plainText);
   
    // 返回密文长度
    Int32 PrivateKeyEncrypt(const KERNEL_NS::LibString &plainText, KERNEL_NS::LibString &cypherText);
    Int32 PrivateKeyEncrypt(const U8 *plainText, Int64 plainTextLen, U8 *cypherText);
    // 返回明文长度
    Int32 PubKeyDecrypt(const KERNEL_NS::LibString &cypherText, KERNEL_NS::LibString &plainText);
    Int32 PubKeyDecrypt(const U8 *cypherText, Int64 cypherLen, U8 *plainText);
    Int32 PubKeyDecrypt(const U8 *cypherText, Int64 cypherLen, KERNEL_NS::LibString &plainText);

    // 签名需要使用私钥,因为公钥是用来公开的,签名->数字签名,用于验证身份,如果公钥解密出来的结果与传过去的散列值一样说明是同一个人发的
    // digest:散列值可以是sha1,md5等openssl rsa支持的散列值算法
    // rsa只能签名sha1摘要（需要将原文生成sha1摘要）将摘要信息加密生成签名结果

    /*
    * 签名 :原文->生成摘要->SignDigest->签名后的数据(摘要通过私钥加密生成签名后的数据) 
    * 注意：这里的摘要是没有经过base64编码的,若要网络传输并显示可能会经过base64处理
    * @param(digestType) : LibRsaDefs::DIGEST_TYPE
    * @param(digest) : 摘要
    * @param(digestLen) : 摘要长度
    * @param(signBuffer) : 签名后数据
    * @param(signBufferSize) : 输入时是signeBuffer大小,输出signBuffer的数据长度
    * @return(UInt32) : 签名后signBuffer的长度
    */
    UInt32 SignDigest(Int32 digestType, const U8 *digest, UInt32 digestLen, U8 *signBuffer, UInt32 &signBufferSize);
    UInt32 SignDigest(Int32 digestType, const LibString &digest, LibString &signStr);
    // 验证sha1摘要 解密签名结果与摘要信息进行验证一样则验证成功
    /*
    * 验签:摘要数据,签名后的数据->VerifyDigest->result(公钥解密签名后的数据与摘要比对如果一样则身份确认)
    * @param(digestType) : LibRsaDefs::DIGEST_TYPE
    * @param(digest) : 摘要
    * @param(digestLen) : 摘要长度
    * @param(signData) : 经过签名后的数据
    * @param(signLen) : 签名数据长度
    */
    bool VerifyDigest(Int32 digestType, const U8 *digest, UInt32 digestLen, const U8 *signData, UInt32 signLen);
    bool VerifyDigest(Int32 digestType, const LibString &digest, const LibString &signStr);
    UInt32 CalcSignSize();

private:
    Int32 _mode;
    RSA *_pubRsa;
    RSA *_privateRsa;

    LibString _pubPkc1Key;      // 公钥
    LibString _pubPkc8Key;      // 公钥
    LibString _privateKey;  // 私钥
    Int32 _padding;         // rsa填充格式 默认 RSA_PKCS1_PADDING 需要计算填充占用的字节数
    Int32 _lastErr;         // rsa执行的最后错误码
    Int32 _keyBits;         // 密钥位数 LibRsaDefs::KEY_BITS
};

ALWAYS_INLINE const LibString &LibRsa::GetPubPkc1Key() const
{
    return _pubPkc1Key;
}

ALWAYS_INLINE const LibString &LibRsa::GetPubPkc8Key() const
{
    return _pubPkc8Key;
}

ALWAYS_INLINE const LibString &LibRsa::GetPrivateKey() const
{
    return _privateKey;
}

ALWAYS_INLINE void LibRsa::SetMode(Int32 mode)
{
    _mode = mode;
}

ALWAYS_INLINE Int32 LibRsa::GetMode() const
{
    return _mode;
}

ALWAYS_INLINE bool LibRsa::IsPubEncryptPrivDecrypt() const
{
    return _mode == LibRsa::PUB_ENCRYPT_PRIV_DECRYPT;
}

ALWAYS_INLINE void LibRsa::PubKeyToFile(FILE *fp)
{
    FileUtil::WriteFile(*fp, _pubPkc1Key);
    FileUtil::FlushFile(*fp);
}

ALWAYS_INLINE void LibRsa::PrivateKeyToFile(FILE *fp)
{
    FileUtil::WriteFile(*fp, _privateKey);
    FileUtil::FlushFile(*fp);
}

ALWAYS_INLINE bool LibRsa::SetPadding(Int32 padding)
{
    if(padding != RSA_PKCS1_PADDING && 
    padding != RSA_PKCS1_OAEP_PADDING && 
    padding != RSA_NO_PADDING)
    {
        CRYSTAL_TRACE("padding[%d], not support", padding);
        return false;
    }

    _padding = padding;

    return true;
}

ALWAYS_INLINE Int32 LibRsa::GetLastErr() const
{
    return _lastErr;
}

ALWAYS_INLINE UInt64 LibRsa::CalcCypherSizeByPubKeyPlain(UInt64 plainTextSize) const
{
    // 填充长度
    const Int32 paddingSize = LibRsaDefs::GetPaddingSize(_padding);
    // 模长即输出的长度
    const UInt64 modulusSize = static_cast<UInt64>(RSA_size(_pubRsa));
    // 求出有效限制的长度
    const UInt64 effectSize = static_cast<UInt64>(RSA_size(_pubRsa) - paddingSize);
    
    const auto multiple = plainTextSize / effectSize;
    const auto leftMultiple = (plainTextSize % effectSize) ? 1 : 0;
    return (multiple + leftMultiple) * modulusSize + 1;
}

ALWAYS_INLINE UInt64 LibRsa::CalcCypherSizeByPrivKeyPlain(UInt64 plainTextSize) const
{
    // 填充长度
    const Int32 paddingSize = LibRsaDefs::GetPaddingSize(_padding);
    // 模长即输出的长度
    const UInt64 modulusSize = static_cast<UInt64>(RSA_size(_privateRsa));
    // 求出有效限制的长度
    const UInt64 effectSize = static_cast<UInt64>(RSA_size(_privateRsa) - paddingSize);

    const auto multiple = plainTextSize / effectSize;
    const auto leftMultiple = (plainTextSize % effectSize) ? 1 : 0;
    return (multiple + leftMultiple) * modulusSize + 1;
}

ALWAYS_INLINE UInt64 LibRsa::CalcPlainSizeByPubKeyCypher(UInt64 cypherLen) const
{
    // 填充长度
    const Int32 paddingSize = LibRsaDefs::GetPaddingSize(_padding);
    // 模长即输出的长度
    const UInt64 modulusSize = static_cast<UInt64>(RSA_size(_pubRsa));
    // 求出有效限制的长度
    const UInt64 effectSize = static_cast<UInt64>(RSA_size(_pubRsa) - paddingSize);

    return (cypherLen / modulusSize) * effectSize + 1;
}

ALWAYS_INLINE UInt64 LibRsa::CalcPlainSizeByPrivKeyCypher(UInt64 cypherLen) const
{
    // 填充长度
    const Int32 paddingSize = LibRsaDefs::GetPaddingSize(_padding);
    // 模长即输出的长度
    const UInt64 modulusSize = static_cast<UInt64>(RSA_size(_privateRsa));
    // 求出有效限制的长度
    const UInt64 effectSize = static_cast<UInt64>(RSA_size(_privateRsa) - paddingSize);

    return (cypherLen / modulusSize) * effectSize + 1;
}

ALWAYS_INLINE Int32 LibRsa::PubKeyEncrypt(const KERNEL_NS::LibString &plainText, KERNEL_NS::LibString &cypherText)
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

ALWAYS_INLINE UInt32 LibRsa::SignDigest(Int32 digestType, const U8 *digest, UInt32 digestLen, U8 *signBuffer, UInt32 &signBufferSize)
{
    _lastErr = RSA_sign(digestType, digest, digestLen, signBuffer, &signBufferSize, _privateRsa);
    if(UNLIKELY(_lastErr != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("RSA_sign fail digestType[%d] signBufferSize[%u] _lastErr[%d]", digestType, signBufferSize, _lastErr);
        return 0;
    }

    return signBufferSize;
}

ALWAYS_INLINE UInt32 LibRsa::SignDigest(Int32 digestType, const LibString &digest, LibString &signStr)
{
    auto signLen = CalcSignSize();
    signStr.resize(static_cast<UInt64>(signLen));

    return SignDigest(digestType, reinterpret_cast<const U8 *>(digest.data()), static_cast<UInt32>(digest.size())
    , reinterpret_cast<U8 *>(const_cast<Byte8 *>(signStr.data())), signLen);
}

ALWAYS_INLINE bool LibRsa::VerifyDigest(Int32 digestType, const U8 *digest, UInt32 digestLen, const U8 *signData, UInt32 signLen)
{
    _lastErr = RSA_verify(digestType, digest, digestLen, signData, signLen,  _pubRsa);
    if(UNLIKELY(_lastErr != ERR_LIB_NONE))
    {
        CRYSTAL_TRACE("RSA_verify fail digestType[%d] _lastErr[%d]", digestType, _lastErr);
        return false;
    }

    return true;
}

ALWAYS_INLINE bool LibRsa::VerifyDigest(Int32 digestType, const LibString &digest, const LibString &signStr)
{
    return VerifyDigest(digestType, reinterpret_cast<const U8 *>(digest.data()), static_cast<UInt32>(digest.size())
    , reinterpret_cast<const U8 *>(signStr.data()), static_cast<UInt32>(signStr.size()));
}

ALWAYS_INLINE UInt32 LibRsa::CalcSignSize()
{
    return static_cast<UInt32>(RSA_size(_privateRsa) + 1);
}
KERNEL_END

#endif

