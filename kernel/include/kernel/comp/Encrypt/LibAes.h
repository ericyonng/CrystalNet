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
 * Date: 2021-02-01 22:49:05
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_ENCRYPT_LIB_AES_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_ENCRYPT_LIB_AES_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/Utils/CypherGeneratorUtil.h>
#include <kernel/comp/Encrypt/Defs.h>

KERNEL_BEGIN

class LibString;

class KERNEL_EXPORT LibAes
{
public:
    // 产生密钥 mode:CYPHER_TYPE
    static void GenerateKey(LibString &key, Int32 mode = LibAesDefs::AES_CYPHER_192);
    // 加密 需要是128bit 16字节的倍数 mode:CYPHER_TYPE
    static Int32 Encrypt_Data(const LibString &key, const LibString &plaintext, LibString &cyphertext, Int32 mode = LibAesDefs::AES_CYPHER_192);
    // 解密 需要是128bit 16字节的倍数 mode:CYPHER_TYPE
    static Int32 Decrypt_Data(const LibString &key,  const LibString &cyphertext, LibString &plaintext, Int32 mode = LibAesDefs::AES_CYPHER_192);

private:
    static Int32 GetAesKeyBytes(Int32 mode = LibAesDefs::AES_CYPHER_192);
};

inline void LibAes::GenerateKey(LibString &key, Int32 mode)
{
    const Int32 bytes = GetAesKeyBytes(mode);
    key.resize(static_cast<UInt64>(bytes));
    CypherGeneratorUtil::Gen(key, bytes);
}

inline Int32 LibAes::GetAesKeyBytes(Int32 mode)
{
    static const int keyBytes[] = {
        /* AES_CYPHER_128 */  16,
        /* AES_CYPHER_192 */  24,
        /* AES_CYPHER_256 */  32,
    };

    return keyBytes[mode];
}

KERNEL_END

#endif
