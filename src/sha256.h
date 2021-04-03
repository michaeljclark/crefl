#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define sha256_block_size    64
#define sha256_hash_size     32
#define sha224_hash_size     28

struct sha256_ctx {
    uint32_t chain[8];
    uint8_t block[sha256_block_size];
    uint64_t nbytes;
    uint64_t digestlen;
};

typedef struct sha256_ctx sha256_ctx;

void sha224_init(sha256_ctx *ctx);
void sha256_init(sha256_ctx *ctx);
void sha256_update(sha256_ctx *ctx, const void *data, size_t len);
void sha256_final(sha256_ctx *ctx, unsigned char *result);

#define sha224_ctx sha256_ctx
#define sha224_update sha256_update
#define sha224_final sha256_final

#ifdef __cplusplus
}
#endif
