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
 * Date: 2021-02-02 22:45:38
 * Author: Eric Yonng
 * Description: 异或加密,简单加密,应用于频繁交互的不是关键的数据
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_ENCRYPT_XOR_ENCRYPT_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_ENCRYPT_XOR_ENCRYPT_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class KERNEL_EXPORT XorEncrypt
{
public:
    static void Encrypt(const Byte8 *key, Int32 keySize, const Byte8 *plainText, Int32 plainSize, Byte8 *cypherText);
    static void Decrypt(const Byte8 *key, Int32 keySize, const Byte8 *cypherText, Int32 cypherSize, Byte8 *plainText);

    static void EncryptStringBuffer(const LibString &key, const LibString &plainText, LibString &cypherText);
    static void DecryptStringBuffer(const LibString &key, const LibString &cypherText, LibString &plainText);

};

inline void XorEncrypt::Encrypt(const Byte8 *key, Int32 keySize, const Byte8 *plainText, Int32 plainSize, Byte8 *cypherText)
{
    for(Int32 i = 0; i < plainSize; ++i)
        cypherText[i] = plainText[i] ^ key[i % keySize];
}

inline void XorEncrypt::Decrypt(const Byte8 *key, Int32 keySize, const Byte8 *cypherText, Int32 cypherSize, Byte8 *plainText)
{
    for(Int32 i = 0; i < cypherSize; ++i)
        plainText[i] = cypherText[i] ^ key[i % keySize];
}

inline void XorEncrypt::EncryptStringBuffer(const LibString &key, const LibString &plainText, LibString &cypherText)
{
    const Int32 keySize = static_cast<Int32>(key.size());
    const auto &plainRaw = plainText.GetRaw();
    const Int32 plainSize = static_cast<Int32>(plainRaw.size());
    auto &cypherRaw = cypherText.GetRaw();
    const auto &keyRaw = key.GetRaw();
    for(Int32 i = 0; i < plainSize; ++i)
        cypherRaw[i] = plainRaw[i] ^ keyRaw[i % keySize];
}

inline void XorEncrypt::DecryptStringBuffer(const LibString &key, const LibString &cypherText, LibString &plainText)
{
    const Int32 keySize = static_cast<Int32>(key.size());
    auto &plainRaw = plainText.GetRaw();
    const Int32 cypherSize = static_cast<Int32>(cypherText.size());
    const auto &cypherRaw = cypherText.GetRaw();
    const auto &keyRaw = key.GetRaw();

    for(Int32 i = 0; i < cypherSize; ++i)
        plainRaw[i] = cypherRaw[i] ^ keyRaw[i % keySize];
}

KERNEL_END

#endif
