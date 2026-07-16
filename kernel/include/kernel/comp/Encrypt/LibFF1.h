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
 * Description: FF1格式保留加密工具类. 封装第三方fpe.c (NIST SP 800-38G)
 *              密钥和tweak通过参数传入, 不持有状态, 每次调用创建局部FPE_KEY保证线程安全.
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_ENCRYPT_LIB_FF1_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_ENCRYPT_LIB_FF1_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/common/BaseType.h>

KERNEL_BEGIN

class KERNEL_EXPORT LibFF1
{
public:
    // FF1加密: 将输入radix进制数字数组加密为等长数字数组
    // key: AES密钥原始字节(16/24/32字节), keyBits: 128/192/256
    // tweak: 调整参数(可变长度, 可为nullptr当tweakLen=0时), tweakLen: tweak字节数
    // in: 输入数字数组(每位∈[0,radix-1]), out: 输出数字数组(与in等长)
    // len: 数字数组长度(FF1要求>=2), radix: 进制基数(范围[2, 65536])
    // 返回: Status::Success成功, 其他为错误码
    static Int32 Encrypt(const Byte8 *key, Int32 keyBits,
                         const Byte8 *tweak, UInt32 tweakLen,
                         const unsigned int *in, unsigned int *out,
                         UInt32 len, Int32 radix);

    // FF1解密: 逆操作
    static Int32 Decrypt(const Byte8 *key, Int32 keyBits,
                         const Byte8 *tweak, UInt32 tweakLen,
                         const unsigned int *in, unsigned int *out,
                         UInt32 len, Int32 radix);

    // 校验密钥位数是否合法
    static bool IsValidKeyBits(Int32 keyBits);
    // 获取密钥位数对应的字节长度
    static Int32 GetKeyBytes(Int32 keyBits);
};

ALWAYS_INLINE bool LibFF1::IsValidKeyBits(Int32 keyBits)
{
    return keyBits == 128 || keyBits == 192 || keyBits == 256;
}

ALWAYS_INLINE Int32 LibFF1::GetKeyBytes(Int32 keyBits)
{
    return keyBits / 8;
}

KERNEL_END

#endif
