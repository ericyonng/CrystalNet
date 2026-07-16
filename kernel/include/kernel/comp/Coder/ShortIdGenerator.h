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
 * Description: 短ID生成器. 将UInt64通过FF1格式保留加密后编码为11字符Base62短字符串.
 *              加密: UInt64 → 11位radix-62数字数组 → FF1加密 → Base62字符表映射 → 11字符字符串
 *              解密: 11字符字符串 → Base62字符表反映射 → FF1解密 → 11位radix-62数字数组 → UInt64
 *              密钥和tweak通过参数传入, 不持有状态, 线程安全.
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CODER_SHORT_ID_GENERATOR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CODER_SHORT_ID_GENERATOR_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/common/BaseType.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class KERNEL_EXPORT ShortIdGenerator
{
public:
    // UInt64 → FF1加密 → 11字符Base62短字符串
    // key: AES密钥(16/24/32字节), keyBits: 128/192/256
    // tweak: 调整参数(可变长度, 可为nullptr当tweakLen=0时), tweakLen: tweak字节数
    // 返回: 11字符Base62短字符串. 失败时返回空字符串(可通过日志查看错误)
    // 连续id不产生连续的短id(足够离散)
    static bool Generate(UInt64 id,
                              const Byte8 *key, Int32 keyBits,
                              const Byte8 *tweak, UInt32 tweakLen, LibString &outStr);

    // 不大于11字符Base62短字符串 → FF1解密 → UInt64
    // 成功返回Status::Success, 其他为错误码
    // shortId不够11个字符的左塞0
    static Int32 Parse(const LibString &shortId,
                       UInt64 &id,
                       const Byte8 *key, Int32 keyBits,
                       const Byte8 *tweak, UInt32 tweakLen);
};

KERNEL_END

#endif
