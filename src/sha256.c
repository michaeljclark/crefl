/*
 * Copyright (c) 2011 Stanford University.
 * Copyright (c) 2014 Cryptography Research, Inc.
 * Released under the MIT License.
 */

#include <string.h>

#include "stdendian.h"
#include "sha256.h"

static const uint32_t sha224_init_state[8] = {
	0xc1059ed8, 0x367cd507, 0x3070dd17, 0xf70e5939,
	0xffc00b31, 0x68581511, 0x64f98fa7, 0xbefa4fa4
};

static const uint32_t sha256_init_state[8] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

/*
 * SHA-256 round constants K are the first 32 bits of the fractional
 * parts of the cube roots of the first sixty-four prime numbers.
 *
 * for n = 0..63 sha256_k[n] = modf(primes[n] ** (1/3), 1.0)
 */
static const uint32_t sha256_k[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
	0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
	0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
	0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
	0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static inline uint32_t ror(uint32_t x, int d)
{
  return (x >> d) | (x << (32-d));
}

static inline uint32_t sigma0(uint32_t h1)
{
    return ror(h1, 2) ^ ror(h1, 13) ^ ror(h1, 22);
}

static inline uint32_t sigma1(uint32_t h4)
{
    return ror(h4, 6) ^ ror(h4, 11) ^ ror(h4, 25);
}

static inline uint32_t gamma0(uint32_t a)
{
    return ror(a, 7) ^ ror(a, 18) ^ (a >> 3);
}

static inline uint32_t gamma1(uint32_t b)
{
    return ror(b, 17) ^ ror(b, 19) ^ (b >> 10);
}

static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z)
{
    return z ^ (x & (y ^ z));
}

static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & y) ^ ((x ^ y) & z);
}

static void sha256_transform(sha256_ctx *ctx, const unsigned char *buf)
{
    uint32_t H[8], W[64], T0, T1;
    size_t i;

    for (i = 0; i < 8; i++) {
        H[i] = ctx->chain[i];
    }

    for (i=0; i<16; i++, buf += sizeof(uint32_t)) {
        W[i] = htobe32(*((uint32_t*)buf));
    }

    for (; i<64; i++) {
        W[i] = gamma1(W[i - 2]) + W[i - 7] + gamma0(W[i - 15]) + W[i - 16];
    }

    for (i=0; i<64; i++) {
        T0 = W[i] + H[7] + sigma1(H[4]) + ch(H[4], H[5], H[6]) + sha256_k[i];
        T1 = maj(H[0], H[1], H[2]) + sigma0(H[0]);
        H[7] = H[6];
        H[6] = H[5];
        H[5] = H[4];
        H[4] = H[3] + T0;
        H[3] = H[2];
        H[2] = H[1];
        H[1] = H[0];
        H[0] = T0 + T1;
    }

    for (i = 0; i < 8; i++) {
        ctx->chain[i] += H[i];
    }
}

void sha224_init(sha256_ctx *ctx)
{
    ctx->nbytes = 0;
    ctx->digestlen = sha224_hash_size;
    memcpy(ctx->chain, sha224_init_state, sizeof(sha224_init_state));
    memset(ctx->block, 0, sizeof(ctx->block));
}

void sha256_init(sha256_ctx *ctx)
{
    ctx->nbytes = 0;
    ctx->digestlen = sha256_hash_size;
    memcpy(ctx->chain, sha256_init_state, sizeof(sha256_init_state));
    memset(ctx->block, 0, sizeof(ctx->block));
}

void sha256_update(sha256_ctx *ctx, const void *data, size_t len)
{
    while (len) {
        uint64_t fill = ctx->nbytes % 64, accept = 64 - fill;
        if (accept > len) {
            accept = len;
        }
        ctx->nbytes += accept;
        memcpy(ctx->block + fill, data, accept);

        if (fill+accept == 64) {
            sha256_transform(ctx, ctx->block);
        }

        len -= accept;
        data = ((const char *)data + accept);
    }
}

void sha256_final(sha256_ctx *ctx, unsigned char *result)
{
    uint64_t fill = ctx->nbytes % 64, i;
    ctx->block[fill++] = 0x80;
    if (fill > 56) {
        memset(ctx->block + fill, 0, 64-fill);
        sha256_transform(ctx, ctx->block);
        fill = 0;
    }
    memset(ctx->block + fill, 0, 56-fill);

    uint64_t highCount = 0, lowCount = htobe64((ctx->nbytes * 8));
    memcpy(&ctx->block[56],&lowCount,8);
    sha256_transform(ctx, ctx->block);
    for (i=0; i<8; i++) {
        ctx->chain[i] = htobe32(ctx->chain[i]);
    }
    memcpy(result, ctx->chain, ctx->digestlen);
}
