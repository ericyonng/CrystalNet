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
 * Date: 2021-02-02 22:29:40
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Encrypt/LibAes.h>
#include <3rd/3rdForKernel.h>


// // openssl
// #ifdef _WIN32

//     // include header
//     #ifdef _DEBUG
//         #include "3rd/openssl/staticlib/debug/include/openssl/md5.h"
//         #include "3rd/openssl/staticlib/debug/include/openssl/aes.h"
//     #else
//         #include "3rd/openssl/staticlib/release/include/openssl/md5.h"
//         #include "3rd/openssl/staticlib/release/include/openssl/aes.h"
//     #endif

//     // lib static lib
//     #ifdef _DEBUG
//         #pragma comment(lib, "staticlib\\debug\\lib\\libeay32.lib")
//         #pragma comment(lib, "staticlib\\debug\\lib\\ssleay32.lib")
//     #else
//         #pragma comment(lib, "staticlib\\release\\lib\\libeay32.lib")
//         #pragma comment(lib, "staticlib\\release\\lib\\ssleay32.lib")
//     #endif

// #else // linux

//     #include "openssl/md5.h"
//     #include "openssl/aes.h"

// #endif




KERNEL_BEGIN

Int32 LibAes::Encrypt_Data(const LibString &key, const LibString &plaintext, LibString &cyphertext, Int32 mode)
{
    const Int32 textSize = static_cast<Int32>(plaintext.size());
    if(UNLIKELY(plaintext.empty()))
        return Status::Aes_PlaintextIsEmpty;

    if(UNLIKELY(textSize < AES_BLOCK_SIZE))
        return Status::Aes_TextLengthNotEnough;

    if(UNLIKELY(textSize % AES_BLOCK_SIZE != 0))
        return Status::Aes_Not16BytesMultiple;

    Int32 i = 0;
    auto &cyphertestRaw = cyphertext.GetRaw();
    if(static_cast<Int32>(cyphertestRaw.size()) < textSize)
        cyphertestRaw.resize(textSize, 0);

    AES_KEY innerKey;
    AES_set_encrypt_key(reinterpret_cast<const unsigned char *>(key.c_str()), GetAesKeyBytes(mode) * 8, &innerKey);
    while(i < textSize)
    {
        AES_encrypt(reinterpret_cast<const unsigned char *>(&plaintext[i]), reinterpret_cast<unsigned char *>(&cyphertestRaw[i]), &innerKey);
        i += AES_BLOCK_SIZE;
    }

    return Status::Success;
}

Int32 LibAes::Decrypt_Data(const LibString &key,  const LibString &cyphertext, LibString &plaintext, Int32 mode)
{
    const Int32 textSize = static_cast<Int32>(cyphertext.size());
    if(UNLIKELY(cyphertext.empty()))
        return Status::Aes_CyphertextIsEmpty;

    if(UNLIKELY(textSize < AES_BLOCK_SIZE))
        return Status::Aes_TextLengthNotEnough;

    if(UNLIKELY(textSize % AES_BLOCK_SIZE != 0))
        return Status::Aes_Not16BytesMultiple;

    Int32 i = 0;
    auto &plaintextRaw = plaintext.GetRaw();
    if(UNLIKELY(static_cast<Int32>(plaintextRaw.size()) < textSize))
        plaintextRaw.resize(textSize, 0);

    AES_KEY innerKey;
    AES_set_decrypt_key(reinterpret_cast<const unsigned char *>(key.c_str()), GetAesKeyBytes(mode) * 8, &innerKey);
    while(i < textSize)
    {
        AES_decrypt(reinterpret_cast<const unsigned char *>(&cyphertext[i]), reinterpret_cast<unsigned char *>(&plaintextRaw[i]), &innerKey);
        i += AES_BLOCK_SIZE;
    }

    return Status::Success;
}

KERNEL_END