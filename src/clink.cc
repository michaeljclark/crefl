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

#include <cstdio>
#include <cstring>
#include <cassert>
#include <cstdlib>

#include <string>

#include "cbits.h"
#include "cmodel.h"
#include "clink.h"

static const char *tag_delimeter = "+T=";
static const char *name_delimeter = "+N=";
static const char *props_delimeter = "+P=";
static const char *quantity_delimeter = "+Q=";
static const char *attr_delimeter = "+A=";
static const char *link_delimeter = "+L=";
static const char *next_delimeter = "+X=";

struct decl_sum
{
    sha256_ctx ctx;
};

static void crefl_hash_init(decl_sum *sum)
{
    sha256_init(&sum->ctx);
}

static void crefl_hash_absorb(decl_sum *sum, const char *str)
{
    sha256_update(&sum->ctx, str, strlen(str));
}

static void crefl_hash_final(decl_sum *sum, decl_hash *hash)
{
    sha256_final(&sum->ctx, (unsigned char*)hash->sum);
}

static void crefl_hash_update(decl_sum *sum, const void *data, size_t len)
{
    sha256_update(&sum->ctx, data, len);
}

static void crefl_hash_node_impl(decl_ref d, decl_sum *sum,
    decl_index *index, std::string prefix)
{
    decl_node *node = crefl_decl_ptr(d);
    decl_hash *hash;
    decl_ref next;

    crefl_hash_absorb(sum, tag_delimeter);
    crefl_hash_absorb(sum, crefl_tag_name(crefl_decl_tag(d)));
    crefl_hash_absorb(sum, name_delimeter);
    crefl_hash_absorb(sum, crefl_decl_name(d));
    crefl_hash_absorb(sum, props_delimeter);
    crefl_hash_update(sum, &node->_props, sizeof(node->_props));
    crefl_hash_absorb(sum, quantity_delimeter);
    crefl_hash_update(sum, &node->_quantity, sizeof(node->_quantity));

    if (node->_attr) {
        next = crefl_lookup(d.db, node->_attr);
        crefl_hash_absorb(sum, attr_delimeter);
        hash = crefl_node_hash(next, index, prefix);
        crefl_hash_update(sum, (const char*)hash->sum, sizeof(decl_hash));
    }
    if (node->_link) {
        switch (crefl_decl_tag(d)) {
        /*
         * follow `link` to child list `next` for container types:
         * 'set', 'enum', 'struct', 'union', 'function' are lists
         * containing: constants, types, fields and parameters.
         */
        case _decl_set:
        case _decl_enum:
        case _decl_struct:
        case _decl_union:
        case _decl_function:
            crefl_hash_absorb(sum, link_delimeter);
            next = crefl_lookup(d.db, node->_link);
            while (crefl_decl_idx(next))  {
                crefl_hash_absorb(sum, next_delimeter);
                decl_hash *hash = crefl_node_hash(next, index, prefix);
                crefl_hash_update(sum, (const char*)hash->sum, sizeof(decl_hash));
                next = crefl_decl_next(next);
            }
            break;
        /*
         * follow `link` to child element without processing `next`
         * for non container types such as 'field', 'pointer', 'array'
         * and 'param'. following `next` in these node types would cause
         * cycles from type references to adjacent anonymous types.
         */
        default:
            next = crefl_lookup(d.db, node->_link);
            if (crefl_entry_is_marked(crefl_entry_ref(index, next)) &&
                !crefl_entry_is_valid(crefl_entry_ref(index, next))) {
                /* we have a reference to a node that is being hashed */
                crefl_hash_absorb(sum, crefl_tag_name(crefl_decl_tag(next)));
                crefl_hash_absorb(sum, crefl_decl_name(next));
            } else {
                hash = crefl_node_hash(next, index, prefix);
                crefl_hash_update(sum, (const char*)hash->sum, sizeof(decl_hash));
            }
            break;
        }
    }
}

int crefl_entry_is_marked(decl_entry_ref er)
{
    return (crefl_entry_ptr(er)->props & decl_entry_marked) == decl_entry_marked;
}

int crefl_entry_is_valid(decl_entry_ref er)
{
    return (crefl_entry_ptr(er)->props & decl_entry_valid) == decl_entry_valid;
}

static const std::string sep = ".";

decl_hash * crefl_node_hash(decl_ref d, decl_index *index, std::string prefix)
{
    decl_entry_ref er = crefl_entry_ref(index, d);
    decl_entry *ent = crefl_entry_ptr(er);

    static bool anon_parenthesis = false;
    std::string osep = prefix.size() > 0 ? sep : "";

    switch (crefl_decl_tag(d)) {
    case _decl_array:
    case _decl_pointer:
        if (anon_parenthesis) {
            prefix += osep + std::string("(") + std::string(
                crefl_tag_name(crefl_decl_tag(d))) + std::string(")");
        }
        break;
    default:
        if (strlen(crefl_decl_name(d))) {
            prefix += osep + crefl_decl_name(d);
        } else if (anon_parenthesis) {
            prefix += osep + std::string("(") + std::string(
                crefl_tag_name(crefl_decl_tag(d))) + std::string(")");
        }
        break;
    }

    if ((ent->props & decl_entry_valid) != decl_entry_valid) {
        decl_sum sum;
        ent->props |= decl_entry_marked;
        crefl_hash_init(&sum);
        crefl_hash_node_impl(d, &sum, index, prefix);
        ent = crefl_entry_ptr(er); /* revalidate due to realloc */
        crefl_hash_final(&sum, &ent->hash);
        ent->fqn = crefl_entry_name_new(index, prefix.c_str());
        ent->props |= decl_entry_valid;
    }

    return &ent->hash;
}

decl_index * crefl_index_new()
{
    decl_index *index = (decl_index*)malloc(sizeof(decl_index));

    index->name_offset = 1; /* offset 0 holds empty string */
    index->name_size = 32;
    index->name = (char*)malloc(index->name_size);
    memset(index->name, 0, index->name_size);

    index->entry_offset = 1; /* offset 0 slot is empty */
    index->entry_size = 32;
    index->entry = (decl_entry*)malloc(sizeof(decl_entry) * index->entry_size);
    memset(index->entry, 0, sizeof(decl_entry) * index->entry_size);

    return index;
}

void crefl_index_destroy(decl_index *index)
{
    free(index->name);
    free(index->entry);
    free(index);
}

decl_entry_ref crefl_entry_ref(decl_index *index, decl_ref r)
{
    if (r.decl_idx >= index->entry_size) {
        size_t old_size = index->entry_size;
        index->entry_size = 1ull << (64 - clz(r.decl_idx));
        index->entry = (decl_entry*)realloc(index->entry,
            index->entry_size * sizeof(decl_entry));
        memset(index->entry + old_size, 0,
            (index->entry_size - old_size) * sizeof(decl_entry));
    }
    return decl_entry_ref { index, r.decl_idx };
}

decl_id crefl_entry_name_new(decl_index *index, const char *name)
{
    size_t len = strlen(name) + 1;
    if (len == 1) return 0;
    if (index->name_offset + len > index->name_size) {
        while (index->name_offset + len > index->name_size) {
            index->name_size <<= 1;
        }
        index->name = (char*)realloc(index->name, index->name_size);
    }
    size_t name_offset = index->name_offset;
    index->name_offset += len;
    memcpy(index->name + name_offset, name, len);
    return name_offset;
}

decl_entry * crefl_entry_ptr(decl_entry_ref d)
{
    return d.index->entry + d.offset;
}

const char* crefl_entry_fqn(decl_entry_ref d)
{
    return d.index->name + crefl_entry_ptr(d)->fqn;
}

void crefl_index_scan(decl_index *index, decl_db *db)
{
    decl_ref d = crefl_lookup(db, db->root_element);
    while (crefl_decl_idx(d))  {
        crefl_node_hash(d, index, "");
        d = crefl_decl_next(d);
    }
}
