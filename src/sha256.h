#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define sha256_block_size    64
#define sha256_hash_size     32

struct sha256_ctx {
    uint32_t chain[8];
    uint8_t block[sha256_block_size];
    uint64_t nbytes;
    uint64_t digestlen;
};

typedef struct sha256_ctx sha256_ctx;

void sha256_init(sha256_ctx *ctx);
void sha256_256_init(sha256_ctx *ctx);
void sha256_update(sha256_ctx *ctx, const void *data, size_t len);
void sha256_final(sha256_ctx *ctx, unsigned char *result);

#ifdef __cplusplus
}
#endif
