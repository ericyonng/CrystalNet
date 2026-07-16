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
*/

#include <pch.h>
#include "FpeLocal.h"

#include <assert.h>

# ifdef __cplusplus
extern "C" {
# endif

// quick power: result = x ^ e
void pow_uv(BIGNUM *pow_u, BIGNUM *pow_v, unsigned int x, int u, int v, BN_CTX *ctx)
{
 BN_CTX_start(ctx);
 BIGNUM *base = BN_CTX_get(ctx),
        *e = BN_CTX_get(ctx);

 BN_set_word(base, x);
 if (u > v) {
  BN_set_word(e, v);
  BN_exp(pow_v, base, e, ctx);
  BN_mul(pow_u, pow_v, base, ctx);
 } else {
  BN_set_word(e, u);
  BN_exp(pow_u, base, e, ctx);
  if (u == v)    BN_copy(pow_v, pow_u);
  else    BN_mul(pow_v, pow_u, base, ctx);
 }

 BN_CTX_end(ctx);
 return;

 /*
 // old veresion, classical quick power
 mpz_t temp;
 mpz_init_set_ui(result, 1);
 mpz_init_set_ui(temp, x);
 while (e) {
     if (e & 1)    mpz_mul(result, result, temp);
     mpz_mul(temp, temp, temp);
     e >>= 1;
 }
 mpz_clear(temp);
 return;
 */
}

# ifdef __cplusplus
}
# endif