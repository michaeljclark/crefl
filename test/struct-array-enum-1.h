#include <stdint.h>

enum { sha224_hash_size = 28 };

struct decl_hash;
typedef struct decl_hash decl_hash;

struct decl_hash {
	uint8_t sum[sha224_hash_size];
};
