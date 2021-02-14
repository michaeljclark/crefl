#pragma once

#ifdef __cplusplus
extern "C" {
#endif

static const u8 decl_db_magic[8] = { 'c', 'r', 'e', 'f', 'l', '_', '0', '0' };

struct decl_db_hdr;
typedef struct decl_db_hdr decl_db_hdr;

struct decl_db_hdr
{
    union {
        char hdr[32];
        struct {
            u8 magic[8];
            u32 decl_entry_count;
            u32 symbol_table_size;
        };
    };
};

int crefl_check_magic(void *addr);
void crefl_read_db(decl_db *db, const char *input_filename);
void crefl_write_db(decl_db *db, const char *output_filename);

#ifdef __cplusplus
}
#endif
