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

#include "sha256.h"

#ifdef __cplusplus
extern "C" {
#endif

struct decl_hash;
struct decl_entry;
struct decl_entry_ref;
struct decl_index;

typedef struct decl_hash decl_hash;
typedef struct decl_entry decl_entry;
typedef struct decl_entry_ref decl_entry_ref;
typedef struct decl_index decl_index;

struct decl_hash
{
    uint8_t sum[sha224_hash_size];
};

enum decl_entry_props
{
    decl_entry_marked = 1,
    decl_entry_valid = 2
};

struct decl_entry
{
    decl_id fqn;
    decl_set props;
    decl_hash hash;
};

struct decl_entry_ref
{
    decl_index *index;
    size_t offset;
};

struct decl_index
{
    char* name;
    size_t name_offset;
    size_t name_size;

    decl_entry *entry;
    size_t entry_offset;
    size_t entry_size;
};

decl_index* crefl_index_new();
void crefl_index_destroy(decl_index *index);

decl_entry_ref crefl_entry_ref(decl_index *index, decl_ref r);
decl_id crefl_entry_name_new(decl_index *index, const char *name);

decl_entry * crefl_entry_ptr(decl_entry_ref d);
const char* crefl_entry_fqn(decl_entry_ref d);

int crefl_entry_is_marked(decl_entry_ref d);
int crefl_entry_is_valid(decl_entry_ref d);

void crefl_index_scan(decl_index *index, decl_db *db);
int crefl_link_merge(decl_db *dst, const char *name, decl_db **srcn, size_t n);

#ifdef __cplusplus
}
#endif
