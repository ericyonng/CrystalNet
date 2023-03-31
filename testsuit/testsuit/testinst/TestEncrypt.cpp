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
 * 
 * Author: Eric Yonng
 * Date: 2021-02-03 09:53:24
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestEncrypt.h>


// void TestEncrypt::Run() 
// {
//     KERNEL_NS::LibString textStr = "hello world!";
//     Int64 textSize = KERNEL_NS::MathUtil::GetLcm(static_cast<Int64>(textStr.size()), 16);
//     textStr.resize(static_cast<UInt64>(textSize));
//     std::cout << "textStr = " << textStr << std::endl;

//     // 1.aes加解密
//     Int32 err = Status::Success;
//     {
//         KERNEL_NS::LibString cypherText;
//         KERNEL_NS::LibString key;
//         KERNEL_NS::LibAes::GenerateKey(key);
//         KERNEL_NS::LibString keyView;
//         err = KERNEL_NS::StringUtil::ToHexStringView(key.data(), key.size(), keyView);
//         std::cout << " aes keyView = " << keyView << std::endl;

//         err = KERNEL_NS::LibAes::Encrypt_Data(key, textStr, cypherText);
//         std::cout << "err = " << err << " cypherText = " << cypherText << std::endl;
//         KERNEL_NS::LibString cypherView;
//         err = KERNEL_NS::StringUtil::ToHexStringView(cypherText.data(), cypherText.size(), cypherView);
//         std::cout << "err = " << err << " cypherView = " << cypherView << std::endl;

//         KERNEL_NS::LibString plainText;
//         err = KERNEL_NS::LibAes::Decrypt_Data(key, cypherText, plainText);
//         std::cout << "err = " << err << " plainText = " << plainText << "plain text == textStr? :"<< (plainText == textStr) << std::endl;
//     }

//     // 2.异或加解密
//     {
//         // 128bit密码
//         KERNEL_NS::LibString key;
//         key.resize(16);
//         KERNEL_NS::CypherGeneratorUtil::Gen(key, static_cast<Int32>(key.size()));
//         std::cout << " key = " << key << std::endl;
//         KERNEL_NS::LibString keyView;
//         err = KERNEL_NS::StringUtil::ToHexStringView(key.data(), key.size(), keyView);
//         std::cout << " keyView = " << keyView << std::endl;

//         KERNEL_NS::LibString cypherText;
//         cypherText.resize(textStr.size());
//         KERNEL_NS::XorEncrypt::EncryptStringBuffer(key, textStr, cypherText);
//         std::cout << " cypherText = " << cypherText << std::endl;
//         KERNEL_NS::LibString cypherView;
//         err = KERNEL_NS::StringUtil::ToHexStringView(cypherText.data(), cypherText.size(), cypherView);
//         std::cout << "err = " << err << " cypherView = " << cypherView << std::endl;

//         KERNEL_NS::LibString plainText;
//         plainText.resize(cypherText.size());
//         KERNEL_NS::XorEncrypt::DecryptStringBuffer(key, cypherText, plainText);
//         std::cout << " plainText = " << plainText << "plain text == textStr? :"<< (plainText == textStr) << std::endl;
//     }

//     // base64 编解码
//     {
//         KERNEL_NS::LibString str = "dlakjfsldfjlsakdjfkl";
//         KERNEL_NS::LibString base64String;
//         KERNEL_NS::LibBase64::Encode(str.data(), str.length(), base64String);
//         std::cout << "base 64 string :" << base64String << std::endl;

//         UInt64 decodeLen = KERNEL_NS::LibBase64::CalcDecodeLen(base64String.c_str(), base64String.length());
//         auto pool = KERNEL_NS::MemoryPool::GetDefaultInstance();
//         auto plainBuffer = static_cast<Byte8 *>(pool->Alloc(static_cast<UInt64>(decodeLen + 1)));
//         ++decodeLen;
//         KERNEL_NS::LibBase64::Decode(base64String.c_str(), base64String.length(), plainBuffer, decodeLen);
//         std::cout << "base64 decode string:"<< plainBuffer << std::endl;
//     }

//     // sha 摘要族
//     KERNEL_NS::LibString testFile = "./testEncrypt.txt";
//     {
//         KERNEL_NS::LibString str = "dlkajjd+++sdfjk//dfklsjl11";

//         // md5
//         {
//             KERNEL_NS::LibString digest;
//             KERNEL_NS::LibDigest::MakeMd5(str, digest);

//             KERNEL_NS::LibString multiSeg;
//             MD5_CTX ctx;
//             KERNEL_NS::LibDigest::MakeMd5Init(ctx);
//             const UInt64 strLen = static_cast<UInt64>(str.length());
//             UInt64 idx = 0;
//             for(; idx < strLen; idx += 5)
//             {
//                 if((idx + 5) >= strLen)
//                     break;

//                 KERNEL_NS::LibDigest::MakeMd5Continue(ctx, str.data() + idx, 5);
//             }

//             if(idx < strLen)
//                 KERNEL_NS::LibDigest::MakeMd5Continue(ctx, str.data() + idx, strLen - idx);

//             KERNEL_NS::LibDigest::MakeMd5Final(ctx, multiSeg);
//             KERNEL_NS::LibDigest::MakeMd5Clean(ctx);

//             std::cout << "MakeMd5 digest raw:" << digest << std::endl;
//             std::cout << "MakeMd5 multi seg digest raw:" << multiSeg << std::endl;
//             std::cout << "MakeMd5 multi seg digest is same with directly one:" << (multiSeg == digest) << std::endl;
//             digest = KERNEL_NS::LibBase64::Encode(digest);
//             multiSeg = KERNEL_NS::LibBase64::Encode(multiSeg);
//             std::cout << "MakeMd5 digest base64:" << digest << std::endl;
//             std::cout << "MakeMd5 multi seg digest base64:" << multiSeg << std::endl;

//             KERNEL_NS::LibString testMultiSeg;
//             KERNEL_NS::LibDigest::MakeFileMd5(testFile, testMultiSeg);
//             testMultiSeg = KERNEL_NS::LibBase64::Encode(testMultiSeg);
//             std::cout << "MakeFileMd5 testMultiSeg digest base64:" << testMultiSeg << ", result:" << (testMultiSeg == multiSeg) <<  std::endl;
//         }

//         // sha1
//         {
//             KERNEL_NS::LibString digest;
//             KERNEL_NS::LibString raw;
//             KERNEL_NS::LibDigest::MakeSha1(str, raw);

//             KERNEL_NS::LibString multiSeg;
//             SHA_CTX ctx;
//             KERNEL_NS::LibDigest::MakeSha1Init(ctx);
//             const UInt64 strLen = static_cast<UInt64>(str.length());
//             UInt64 idx = 0;
//             for(; idx < strLen; idx += 5)
//             {
//                 if((idx + 5) >= strLen)
//                     break;

//                 KERNEL_NS::LibDigest::MakeSha1Continue(ctx, str.data() + idx, 5);
//             }

//             if(idx < strLen)
//                 KERNEL_NS::LibDigest::MakeSha1Continue(ctx, str.data() + idx, strLen - idx);
//             KERNEL_NS::LibDigest::MakeSha1Final(ctx, multiSeg);
//             KERNEL_NS::LibDigest::MakeSha1Clean(ctx);

//             std::cout << "MakeSha1 digest raw:" << raw << std::endl;
//             std::cout << "MakeSha1 multi seg digest raw:" << multiSeg << std::endl;
//             std::cout << "MakeSha1 multi seg digest raw is the same with digest:" << (multiSeg == raw) << std::endl;
//             digest = KERNEL_NS::LibBase64::Encode(raw);
//             multiSeg = KERNEL_NS::LibBase64::Encode(multiSeg);
//             std::cout << "MakeSha1 digest base64:" << digest << std::endl;
//             std::cout << "MakeSha1 multiSeg base64:" << multiSeg << std::endl;
//             digest = KERNEL_NS::LibBase64::Decode(digest);
//             std::cout << "sha1 base decode is the same with raw:" << (digest == raw) << std::endl;

//             KERNEL_NS::LibString testMultiSeg;
//             KERNEL_NS::LibDigest::MakeFileSha1(testFile, testMultiSeg);
//             testMultiSeg = KERNEL_NS::LibBase64::Encode(testMultiSeg);
//             std::cout << "MakeFileSha1 testMultiSeg digest base64:" << testMultiSeg << ", result:" << (testMultiSeg == multiSeg) << std::endl;
//         }

//         // sha224
//         {
//             KERNEL_NS::LibString digest;
//             KERNEL_NS::LibDigest::MakeSha224(str, digest);

//             KERNEL_NS::LibString multiSeg;
//             SHA256_CTX ctx;
//             KERNEL_NS::LibDigest::MakeSha224Init(ctx);
//             const UInt64 strLen = static_cast<UInt64>(str.length());
//             UInt64 idx = 0;
//             for(; idx < strLen; idx += 5)
//             {
//                 if((idx + 5) >= strLen)
//                     break;

//                 KERNEL_NS::LibDigest::MakeSha224Continue(ctx, str.data() + idx, 5);
//             }

//             if(idx < strLen)
//                 KERNEL_NS::LibDigest::MakeSha224Continue(ctx, str.data() + idx, strLen - idx);
//             KERNEL_NS::LibDigest::MakeSha224Final(ctx, multiSeg);
//             KERNEL_NS::LibDigest::MakeSha224Clean(ctx);

//             std::cout << "MakeSha224 digest raw:" << digest << std::endl;
//             std::cout << "MakeSha224 multi seg digest raw:" << multiSeg << std::endl;
//             std::cout << "MakeSha224 multi seg digest raw is the same with digest:" << (multiSeg == digest) << std::endl;
//             digest = KERNEL_NS::LibBase64::Encode(digest);
//             multiSeg = KERNEL_NS::LibBase64::Encode(multiSeg);
//             std::cout << "MakeSha224 digest base64:" << digest << std::endl;
//             std::cout << "MakeSha224 digest base64:" << multiSeg << std::endl;

//             KERNEL_NS::LibString testMultiSeg;
//             KERNEL_NS::LibDigest::MakeFileSha224(testFile, testMultiSeg);
//             testMultiSeg = KERNEL_NS::LibBase64::Encode(testMultiSeg);
//             std::cout << "MakeFileSha224 testMultiSeg digest base64:" << testMultiSeg << ", result:" << (testMultiSeg == multiSeg) << std::endl;
//         }

//         // sha256
//         {
//             KERNEL_NS::LibString digest;
//             KERNEL_NS::LibDigest::MakeSha256(str, digest);

//             KERNEL_NS::LibString multiSeg;
//             SHA256_CTX ctx;
//             KERNEL_NS::LibDigest::MakeSha256Init(ctx);
//             const UInt64 strLen = static_cast<UInt64>(str.length());
//             UInt64 idx = 0;
//             for(; idx < strLen; idx += 5)
//             {
//                 if((idx + 5) >= strLen)
//                     break;

//                 KERNEL_NS::LibDigest::MakeSha256Continue(ctx, str.data() + idx, 5);
//             }

//             if(idx < strLen)
//                 KERNEL_NS::LibDigest::MakeSha256Continue(ctx, str.data() + idx, strLen - idx);
//             KERNEL_NS::LibDigest::MakeSha256Final(ctx, multiSeg);
//             KERNEL_NS::LibDigest::MakeSha256Clean(ctx);

//             std::cout << "MakeSha256 digest raw:" << digest << std::endl;
//             std::cout << "MakeSha256 multiSeg raw:" << multiSeg << std::endl;
//             std::cout << "MakeSha256 multiSeg raw is the same with digest:" << (multiSeg == digest) << std::endl;
//             digest = KERNEL_NS::LibBase64::Encode(digest);
//             multiSeg = KERNEL_NS::LibBase64::Encode(multiSeg);
//             std::cout << "MakeSha256 digest base64:" << digest << std::endl;
//             std::cout << "MakeSha256 multiSeg base64:" << multiSeg << std::endl;

//             KERNEL_NS::LibString testMultiSeg;
//             KERNEL_NS::LibDigest::MakeFileSha256(testFile, testMultiSeg);
//             testMultiSeg = KERNEL_NS::LibBase64::Encode(testMultiSeg);
//             std::cout << "MakeFileSha256 testMultiSeg digest base64:" << testMultiSeg << ", result:" << (testMultiSeg == multiSeg) << std::endl;
//         }

//         // sha384
//         {
//             KERNEL_NS::LibString digest;
//             KERNEL_NS::LibDigest::MakeSha384(str, digest);

//             KERNEL_NS::LibString multiSeg;
//             SHA512_CTX ctx;
//             KERNEL_NS::LibDigest::MakeSha384Init(ctx);
//             const UInt64 strLen = static_cast<UInt64>(str.length());
//             UInt64 idx = 0;
//             for(; idx < strLen; idx += 5)
//             {
//                 if((idx + 5) >= strLen)
//                     break;

//                 KERNEL_NS::LibDigest::MakeSha384Continue(ctx, str.data() + idx, 5);
//             }

//             if(idx < strLen)
//                 KERNEL_NS::LibDigest::MakeSha384Continue(ctx, str.data() + idx, strLen - idx);
//             KERNEL_NS::LibDigest::MakeSha384Final(ctx, multiSeg);
//             KERNEL_NS::LibDigest::MakeSha384Clean(ctx);

//             std::cout << "MakeSha384 digest raw:" << digest << std::endl;
//             std::cout << "MakeSha384 multiSeg raw:" << multiSeg << std::endl;
//             std::cout << "MakeSha384 multiSeg raw is the same with digest:" << (multiSeg == digest) << std::endl;
//             digest = KERNEL_NS::LibBase64::Encode(digest);
//             multiSeg = KERNEL_NS::LibBase64::Encode(multiSeg);
//             std::cout << "MakeSha384 digest base64:" << digest << std::endl;

//             KERNEL_NS::LibString testMultiSeg;
//             KERNEL_NS::LibDigest::MakeFileSha384(testFile, testMultiSeg);
//             testMultiSeg = KERNEL_NS::LibBase64::Encode(testMultiSeg);
//             std::cout << "MakeFileSha384 testMultiSeg digest base64:" << testMultiSeg << ", result:" << (testMultiSeg == multiSeg) << std::endl;
//         }

//         // sha512
//         {
//             KERNEL_NS::LibString digest;
//             KERNEL_NS::LibDigest::MakeSha512(str, digest);

//             KERNEL_NS::LibString multiSeg;
//             SHA512_CTX ctx;
//             KERNEL_NS::LibDigest::MakeSha512Init(ctx);
//             const UInt64 strLen = static_cast<UInt64>(str.length());
//             UInt64 idx = 0;
//             for(; idx < strLen; idx += 5)
//             {
//                 if((idx + 5) >= strLen)
//                     break;

//                 KERNEL_NS::LibDigest::MakeSha512Continue(ctx, str.data() + idx, 5);
//             }

//             if(idx < strLen)
//                 KERNEL_NS::LibDigest::MakeSha512Continue(ctx, str.data() + idx, strLen - idx);
//             KERNEL_NS::LibDigest::MakeSha512Final(ctx, multiSeg);
//             KERNEL_NS::LibDigest::MakeSha512Clean(ctx);

//             std::cout << "MakeSha512 digest raw:" << digest << std::endl;
//             std::cout << "MakeSha512 multiSeg raw:" << multiSeg << std::endl;
//             std::cout << "MakeSha512 multiSeg raw is the same with digest:" << (multiSeg == digest) << std::endl;
//             digest = KERNEL_NS::LibBase64::Encode(digest);
//             multiSeg = KERNEL_NS::LibBase64::Encode(multiSeg);
//             std::cout << "MakeSha512 digest base64:" << digest << std::endl;
//             std::cout << "MakeSha512 multiSeg base64:" << multiSeg << std::endl;

//             KERNEL_NS::LibString testMultiSeg;
//             KERNEL_NS::LibDigest::MakeFileSha512(testFile, testMultiSeg);
//             testMultiSeg = KERNEL_NS::LibBase64::Encode(testMultiSeg);
//             std::cout << "MakeFileSha512 testMultiSeg digest base64:" << testMultiSeg << ", result:" << (testMultiSeg == multiSeg) << std::endl;
//         }

//     }

//     // 3.rsa加解密
//     {
//         KERNEL_NS::LibRsa rsa;

//         rsa.GenKey();
//         std::cout << "pub key:" << rsa.GetPubKey() << std::endl;
//         std::cout << "private key:" << rsa.GetPrivateKey() << std::endl;

//         // 加密
//         auto pool = KERNEL_NS::MemoryPool::GetDefaultInstance();
//         auto cypherSize = rsa.CalcCypherSizeByPubKeyPlain(textStr.length());
//         auto cypherBuffer = static_cast<U8 *>(pool->Alloc(cypherSize));
//         auto cypherLen = rsa.PubKeyEncrypt(reinterpret_cast<const U8 *>(textStr.c_str())
//             , textStr.length(), cypherBuffer);

//         std::cout << "cypherBuffer:"<< cypherBuffer << std::endl;

//         auto plainSize = rsa.CalcPlainSizeByPrivKeyCypher(cypherLen);
//         auto plainTextBuffer = static_cast<U8 *>(pool->Alloc(plainSize));
//         auto plainLen = rsa.PrivateKeyDecrypt(cypherBuffer, cypherLen, plainTextBuffer);
//         plainTextBuffer[plainLen] = 0;
//         std::cout << "plainTextBuffer:" << plainTextBuffer << std::endl;

//         // priv加密
//         plainTextBuffer[0] = 0;
//         auto privEnBytes = rsa.PrivateKeyEncrypt(reinterpret_cast<const U8 *>(textStr.c_str()), textStr.length(), cypherBuffer);
//         // pub解密
//         auto pubDeBytes = rsa.PubKeyDecrypt(cypherBuffer, privEnBytes, plainTextBuffer);
//         plainTextBuffer[pubDeBytes] = 0;
//         std::cout << "plainTextBuffer:" << plainTextBuffer << std::endl;

//         pool->Free(plainTextBuffer);
//         pool->Free(cypherBuffer);
//     }

//     // rsa 签名验签
//     {
//         KERNEL_NS::LibString str = "dafdalskfjalskddddddddddddddddddddddddfjjjjjjjjjjjjjjjjjjjjdskskskskskskskskskskskskskskskskskskskskskskskskskskskkkkkklsslsldklafsdkflas;dfklasdfasdklfas;fkasasasasjdfksjasdfasjdfksjdfkjslkfjaskfskdfksdfksdjfksjfksjakjfksadfkajkadfjaksljfkalsjdkfjdsakjfksajdfjkasfjasjkfdjaksljfkasjdfkasjfkasjdkfsdkfjksdjfksjdfkjskdfjskdfskdfksjfkjdksajkfjkasjfkjaskdfjksjdfkasjkdfjaskdfj";

//         // rsa生成密钥对
//         KERNEL_NS::LibRsa rsa;
//         rsa.GenKey();

//         // sha1摘要签名验签
//         {
//             // 1.生成摘要
//             auto digest = KERNEL_NS::LibDigest::MakeSha1(str);
//             // 2.签名
//             KERNEL_NS::LibString signStr;
//             const Int32 digestType = static_cast<Int32>(KERNEL_NS::LibRsaDefs::SHA1);
//             UInt32 signLen = rsa.SignDigest(digestType, digest, signStr);
//             signStr.GetRaw().erase(signLen);
//             std::cout<< "signe str raw:" << signStr << std::endl;
//             auto base64Sign = KERNEL_NS::LibBase64::Encode(signStr);
//             std::cout<< "base64Sign:" << base64Sign << std::endl;

//             // 3.验签
//             bool ret = rsa.VerifyDigest(digestType, digest, signStr);
//             std::cout << "verify ret:" << ret << std::endl;
//         }

//         // sha224摘要签名验签
//         {
//             // 1.生成摘要
//             auto digest = KERNEL_NS::LibDigest::MakeSha224(str);
//             // 2.签名
//             KERNEL_NS::LibString signStr;
//             const Int32 digestType = static_cast<Int32>(KERNEL_NS::LibRsaDefs::SHA224);
//             UInt32 signLen = rsa.SignDigest(digestType, digest, signStr);
//             signStr.GetRaw().erase(signLen);
//             std::cout << "signe str raw:" << signStr << std::endl;
//             auto base64Sign = KERNEL_NS::LibBase64::Encode(signStr);
//             std::cout<< "base64Sign:" << base64Sign << std::endl;

//             // 3.验签
//             bool ret = rsa.VerifyDigest(digestType, digest, signStr);
//             std::cout << "verify ret:" << ret << std::endl;
//         }

//         // sha256摘要签名验签
//         {
//             // 1.生成摘要
//             auto digest = KERNEL_NS::LibDigest::MakeSha256(str);
//             KERNEL_NS::LibString digestHex;
//             KERNEL_NS::StringUtil::ToHexString(digest, digestHex);
//             g_Log->Info(LOGFMT_NON_OBJ_TAG(TestEncrypt, "digestHex:%s"), digestHex.c_str());


//             // 2.签名
//             KERNEL_NS::LibString signStr;
//             const Int32 digestType = static_cast<Int32>(KERNEL_NS::LibRsaDefs::SHA256);
//             UInt32 signLen = rsa.SignDigest(digestType, digest, signStr);
//             signStr.GetRaw().erase(signLen);
//             std::cout << "signe str raw:" << signStr << std::endl;
//             auto base64Sign = KERNEL_NS::LibBase64::Encode(signStr);
//             std::cout<< "base64Sign:" << base64Sign << std::endl;

//             // 3.验签
//             bool ret = rsa.VerifyDigest(digestType, digest, signStr);
//             std::cout << "verify ret:" << ret << std::endl;
//         }

//         // sha384摘要签名验签
//         {
//             // 1.生成摘要
//             auto digest = KERNEL_NS::LibDigest::MakeSha384(str);
//             // 2.签名
//             KERNEL_NS::LibString signStr;
//             const Int32 digestType = static_cast<Int32>(KERNEL_NS::LibRsaDefs::SHA384);
//             UInt32 signLen = rsa.SignDigest(digestType, digest, signStr);
//             signStr.GetRaw().erase(signLen);
//             std::cout << "signe str raw:" << signStr << std::endl;
//             auto base64Sign = KERNEL_NS::LibBase64::Encode(signStr);
//             std::cout<< "base64Sign:" << base64Sign << std::endl;

//             // 3.验签
//             bool ret = rsa.VerifyDigest(digestType, digest, signStr);
//             std::cout << "verify ret:" << ret << std::endl;
//         }

//         // sha512摘要签名验签
//         {
//             // 1.生成摘要
//             auto digest = KERNEL_NS::LibDigest::MakeSha512(str);
//             // 2.签名
//             KERNEL_NS::LibString signStr;
//             const Int32 digestType = static_cast<Int32>(KERNEL_NS::LibRsaDefs::SHA512);
//             UInt32 signLen = rsa.SignDigest(digestType, digest, signStr);
//             signStr.GetRaw().erase(signLen);
//             std::cout << "signe str raw:" << signStr << std::endl;
//             auto base64Sign = KERNEL_NS::LibBase64::Encode(signStr);
//             std::cout<< "base64Sign:" << base64Sign << std::endl;

//             // 3.验签
//             bool ret = rsa.VerifyDigest(digestType, digest, signStr);
//             std::cout << "verify ret:" << ret << std::endl;
//         }

//         // md5摘要签名验签
//         {
//             // 1.生成摘要
//             auto digest = KERNEL_NS::LibDigest::MakeMd5(str);
//             // 2.签名
//             KERNEL_NS::LibString signStr;
//             const Int32 digestType = static_cast<Int32>(KERNEL_NS::LibRsaDefs::MD5);
//             UInt32 signLen = rsa.SignDigest(digestType, digest, signStr);
//             signStr.GetRaw().erase(signLen);
//             std::cout << "signe str raw:" << signStr << std::endl;
//             auto base64Sign = KERNEL_NS::LibBase64::Encode(signStr);
//             std::cout<< "base64Sign:" << base64Sign << std::endl;

//             // 3.验签
//             bool ret = rsa.VerifyDigest(digestType, digest, signStr);
//             std::cout << "verify ret:" << ret << std::endl;
//         }
//     }
// }

void TestEncrypt::Run(int argc, char const *argv[]) 
{
     KERNEL_NS::LibString susParamsInfo, warnParamsInfo;
    
    KERNEL_NS::LibString targetFile;
    auto count = SERVICE_COMMON_NS::ParamsHandler::GetParams(argc, argv, susParamsInfo, warnParamsInfo, [&targetFile](const KERNEL_NS::LibString &key, const KERNEL_NS::LibString &value){
        if(key == "--file")
        {
            targetFile = value;
            return true;
        }

        return false;
    });

    if(!KERNEL_NS::FileUtil::IsFileExist(targetFile.c_str()))
    {
        return;
    }

    KERNEL_NS::LibString md5Out;
    if(!KERNEL_NS::LibDigest::MakeFileMd5(targetFile, md5Out))
    {
        return;
    }

    KERNEL_NS::LibString hexStr;
    KERNEL_NS::StringUtil::ToHexString(md5Out, hexStr);
    std::cout << hexStr << std::endl;
}

