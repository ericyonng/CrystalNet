/*!
 * Source: https://github.com/0NG/Format-Preserving-Encryption
 * License: MIT License (Copyright (c) 2017)
 *
 * FF1/FF3 Format-Preserving Encryption implementation following
 * NIST Special Publication 800-38G recommendation.
 *
 * This implementation is based on OpenSSL's BIGNUM and AES primitives.
 * Only FF1 is compiled into this project (ff3.c is excluded).
 */

#ifndef HEADER_FPE_H
# define HEADER_FPE_H

# include <openssl/aes.h>

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

/*** FF3 (declared for API completeness; ff3.c is not compiled) ***/
int FPE_set_ff3_key(const unsigned char *userKey, const int bits, const unsigned char *tweak, const unsigned int radix, FPE_KEY *key);

void FPE_unset_ff3_key(FPE_KEY *key);

void FPE_ff3_encrypt(unsigned int *in, unsigned int *out, unsigned int inlen, FPE_KEY *key, const int enc);

# ifdef __cplusplus
}
# endif

#endif
