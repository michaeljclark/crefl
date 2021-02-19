/*
 * crefl runtime library and compiler plug-in to support reflection in C.
 *
 * Copyright (c) 2020 Michael Clark <michaeljclark@mac.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct decl_db_hdr;
typedef struct decl_db_hdr decl_db_hdr;

/* decl db magic constant */
static const u8 decl_db_magic[8] = { 'c', 'r', 'e', 'f', 'l', '_', '0', '0' };

/* decl db header */
struct decl_db_hdr
{
    u8 magic[8];
    u32 decl_entry_count;
    u32 name_table_size;
    u32 root_element;
};

/* decl db magic and size */
int crefl_db_magic(const void *addr);
size_t crefl_db_size(decl_db *db);

/* decl db memory io */
int crefl_db_read_mem(decl_db *db, const uint8_t *buf, size_t input_sz);
int crefl_db_write_mem(decl_db *db, uint8_t *buf, size_t output_sz);

/* decl db file io */
int crefl_db_read_file(decl_db *db, const char *input_filename);
int crefl_db_write_file(decl_db *db, const char *output_filename);

#ifdef __cplusplus
}
#endif
