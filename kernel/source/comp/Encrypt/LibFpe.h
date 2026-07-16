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
 * Date: 2026-07-16 10:57:05
 * Author: Eric Yonng
 * Description:
 * FPE(格式保留加密)加解密为密文长度与原文长度一致而设计,不需要对外暴露FPE接口
*/

#ifndef __CRYSTAL_NET_KERNEL_SOURCE_COMP_ENCRYPT_LIB_FPE_H__
#define __CRYSTAL_NET_KERNEL_SOURCE_COMP_ENCRYPT_LIB_FPE_H__

#pragma once

#include <openssl/aes.h>

# ifdef __cplusplus
extern "C" {
# endif

# define FPE_ENCRYPT 1
# define FPE_DECRYPT 0

# define FF1_ROUNDS 10
# define FF3_ROUNDS 8
# define FF3_TWEAK_SIZE 8

 struct fpe_key_st {
  unsigned int radix;
  unsigned int tweaklen;
  unsigned char *tweak;
  AES_KEY aes_enc_ctx;
 };

 typedef struct fpe_key_st FPE_KEY;

 /*** FF1 ***/
 int FPE_set_ff1_key(const unsigned char *userKey, const int bits, const unsigned char *tweak, const unsigned int tweaklen, const int radix, FPE_KEY *key);

 void FPE_unset_ff1_key(FPE_KEY *key);

 void FPE_ff1_encrypt(unsigned int *in, unsigned int *out, unsigned int inlen, FPE_KEY *key, const int enc);

 /*** FF3 ***/
 int FPE_set_ff3_key(const unsigned char *userKey, const int bits, const unsigned char *tweak, const unsigned int radix, FPE_KEY *key);

 void FPE_unset_ff3_key(FPE_KEY *key);

 void FPE_ff3_encrypt(unsigned int *in, unsigned int *out, unsigned int inlen, FPE_KEY *key, const int enc);

# ifdef __cplusplus
}
# endif

#endif
