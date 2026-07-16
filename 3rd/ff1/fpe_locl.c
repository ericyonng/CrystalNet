/*!
 * Source: https://github.com/0NG/Format-Preserving-Encryption
 * License: MIT License (Copyright (c) 2017)
 *
 * Internal helper for FF1: quick power computation.
 */

#include <assert.h>
#include "fpe_locl.h"

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
}
