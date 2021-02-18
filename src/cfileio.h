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
            u32 root_element;
        };
    };
};

int crefl_check_magic(void *addr);
void crefl_read_db(decl_db *db, const char *input_filename);
void crefl_write_db(decl_db *db, const char *output_filename);

#ifdef __cplusplus
}
#endif
