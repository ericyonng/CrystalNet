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
 * Date: 2021-02-01 22:43:25
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_ENCRYPT_DEFS_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_ENCRYPT_DEFS_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <3rd/3rdForKernel.h>

// macro
#define LIB_AES_256_KEY_LEN     32       // 256bit/8bit

// aes定义
KERNEL_BEGIN

class KERNEL_EXPORT LibAesDefs
{
public:     
    // 密文类型
    enum CYPHER_TYPE
    {
        AES_CYPHER_128 = 0,             // 128bit加密
        AES_CYPHER_192,                 // 192bit
        AES_CYPHER_256,                 // 256bit
    };
};

class KERNEL_EXPORT LibRsaDefs
{
public:
    enum KEY_BITS
    {
        RSA_1024 = 1024,                // 1024加密
        RSA_2048 = 2048,                // 2048加密
    };

    enum PADDING_MODE
    {
        BEGIN = 0,
        PKCS1_PADDING = RSA_PKCS1_PADDING,
        NO_PADDING = RSA_NO_PADDING,
        PKCS1_OAEP_PADDING = RSA_PKCS1_OAEP_PADDING,
        End,
    };

    enum DIGEST_TYPE
    {
        SHA1   = NID_sha1,     // NID_sha1WithRsa?NID_sha1Rsa?NID_sha1WithRSAEncryption?
        SHA224 = NID_sha224,   // sha224签名
        SHA256 = NID_sha256,   // sha256签名
        SHA384 = NID_sha384,   // sha384签名
        SHA512 = NID_sha512,   // sha512签名
        MD5 = NID_md5,         // md5签名
    };

    static const std::set<Int32> *GetSupportBits()
    {
        static std::set<Int32> s_keyBits = {LibRsaDefs::RSA_1024, LibRsaDefs::RSA_2048};
        return &s_keyBits;
    }

    static Int32 GetPaddingSize(Int32 padding)
    {
        static const Int32 paddingSize[LibRsaDefs::End] = {
            -1,
            RSA_PKCS1_PADDING_SIZE, // PKCS1_PADDING size
            -1,
            0, // RSA_NO_PADDING 不填充
            41, // RSA_PKCS1_OAEP_PADDING 
        };

        return paddingSize[padding];
    }
};

KERNEL_END

#endif