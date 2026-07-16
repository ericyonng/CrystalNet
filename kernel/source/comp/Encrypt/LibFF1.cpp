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
 * Date: 2026-07-16 01:30:00
 * Author: Eric Yonng
 * Description: FF1格式保留加密工具类实现.
*/

#include <pch.h>
#include <kernel/comp/Encrypt/LibFF1.h>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
  #include <WinSock2.h>
#endif

#include <kernel/comp/Log.h>
#include <fpe.h>

KERNEL_BEGIN

Int32 LibFF1::Encrypt(const Byte8 *key, Int32 keyBits,
                      const Byte8 *tweak, UInt32 tweakLen,
                      const unsigned int *in, unsigned int *out,
                      UInt32 len, Int32 radix)
{
    if (UNLIKELY(!IsValidKeyBits(keyBits)))
        return Status::Ff1_InvalidKeyBits;

    if (UNLIKELY(!key))
        return Status::Ff1_InvalidKeyBits;

    if (UNLIKELY(!in || !out))
        return Status::Ff1_InputLenTooShort;

    if (UNLIKELY(len < 2))
        return Status::Ff1_InputLenTooShort;

    if (UNLIKELY(radix < 2 || radix > 65536))
        return Status::Ff1_RadixNotSupported;

    // 每次调用创建局部FPE_KEY, 保证线程安全
    FPE_KEY fpeKey;
    const unsigned char *tweakPtr = tweakLen > 0 ? reinterpret_cast<const unsigned char *>(tweak) : nullptr;

    int ret = FPE_set_ff1_key(
        reinterpret_cast<const unsigned char *>(key),
        keyBits,
        tweakPtr,
        tweakLen,
        radix,
        &fpeKey);

    if (UNLIKELY(ret != 0))
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(LibFF1, "FPE_set_ff1_key fail ret:%d keyBits:%d radix:%d"), ret, keyBits, radix);
        return Status::Ff1_SetKeyFail;
    }

    FPE_ff1_encrypt(
        const_cast<unsigned int *>(in),
        out,
        len,
        &fpeKey,
        FPE_ENCRYPT);

    FPE_unset_ff1_key(&fpeKey);

    return Status::Success;
}

Int32 LibFF1::Decrypt(const Byte8 *key, Int32 keyBits,
                      const Byte8 *tweak, UInt32 tweakLen,
                      const unsigned int *in, unsigned int *out,
                      UInt32 len, Int32 radix)
{
    if (UNLIKELY(!IsValidKeyBits(keyBits)))
        return Status::Ff1_InvalidKeyBits;

    if (UNLIKELY(!key))
        return Status::Ff1_InvalidKeyBits;

    if (UNLIKELY(!in || !out))
        return Status::Ff1_InputLenTooShort;

    if (UNLIKELY(len < 2))
        return Status::Ff1_InputLenTooShort;

    if (UNLIKELY(radix < 2 || radix > 65536))
        return Status::Ff1_RadixNotSupported;

    FPE_KEY fpeKey;
    const unsigned char *tweakPtr = tweakLen > 0 ? reinterpret_cast<const unsigned char *>(tweak) : nullptr;

    int ret = FPE_set_ff1_key(
        reinterpret_cast<const unsigned char *>(key),
        keyBits,
        tweakPtr,
        tweakLen,
        radix,
        &fpeKey);

    if (UNLIKELY(ret != 0))
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(LibFF1, "FPE_set_ff1_key fail ret:%d keyBits:%d radix:%d"), ret, keyBits, radix);
        return Status::Ff1_SetKeyFail;
    }

    FPE_ff1_encrypt(
        const_cast<unsigned int *>(in),
        out,
        len,
        &fpeKey,
        FPE_DECRYPT);

    FPE_unset_ff1_key(&fpeKey);

    return Status::Success;
}

KERNEL_END
