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
#include "cutil.h"
#include "hashmap.h"

/*
 * node hash algorithm
 *
 * nodes are hashed with the following template where $(var-name) has been
 * substituted with the node property of the same name and H(id) refers to
 * the bytes of the hash of the node with that id:
 *
 * - (T=$(tag);N=$(name);P=$(props);Q=$(quantity)[;A=<H($(attr))>][;L=H($(link))...])
 *
 * e.g. the integral signed intrinsic 'i64'
 *
 * - (T=intrinsic;N=ulong;P=5;Q=64)
 *
 * nodes have a unique SHA-224 hash with the following constraints:
 *
 * - hashes include all identity information excluding internally assigned
 *   ids and next links. this is so that node hashes are position invariant.
 * - identical declarations in different modules will have identical hashes.
 * - adjacency information is included based on the order nodes are absorbed.
 * - while a node's hash sum includes its own name directly, links to
 *   dependent nodes absorb the hash sum of the dependent not its name.
 * - nodes can thus link to dependencies without knowing their names.
 * - semicolon is used as a delimeter as it does not occur in type names.
 * - SHA-224 is used because it is not subject to length extension attacks.
 */
static const char *tag_delimeter      = "(T=";
static const char *name_delimeter     = ";N=";
static const char *props_delimeter    = ";P=";
static const char *quantity_delimeter = ";Q=";
static const char *attr_delimeter     = ";A=";
static const char *link_delimeter     = ";L=";
static const char *next_delimeter     = ";X=";
static const char *hash_delimeter     = ";H=";
static const char *end_delimeter      = ")";

struct decl_sum
{
    sha224_ctx ctx;
};

static void crefl_hash_init(decl_sum *sum)
{
    sha224_init(&sum->ctx);
}

static void crefl_hash_absorb(decl_sum *sum, const char *str)
{
    sha224_update(&sum->ctx, str, strlen(str));
}

static void crefl_hash_final(decl_sum *sum, decl_hash *hash)
{
    sha224_final(&sum->ctx, (unsigned char*)hash->sum);
}

static void crefl_hash_update(decl_sum *sum, const void *data, size_t len)
{
    sha224_update(&sum->ctx, data, len);
}

decl_hash * crefl_node_hash(decl_index *index,
    decl_ref d, decl_ref p, std::string prefix);

static void crefl_hash_node_sum(decl_sum *sum, decl_index *index,
    decl_ref d, decl_ref p, std::string prefix)
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
        hash = crefl_node_hash(index, next, d, prefix);
        crefl_hash_absorb(sum, hash_delimeter);
        crefl_hash_update(sum, (const char*)hash->sum, sizeof(decl_hash));
    }
    if (node->_link) {
        switch (crefl_decl_tag(d)) {
        /*
         * follow `link` to child list for container types: 'object',
         * 'set', 'enum', 'struct', 'union', and 'function' are lists
         * containing: 'typedef', 'field', 'pointer', 'array', etc.
         */
        case _decl_archive:
        case _decl_source:
        case _decl_set:
        case _decl_enum:
        case _decl_struct:
        case _decl_union:
        case _decl_function:
            crefl_hash_absorb(sum, link_delimeter);
            next = crefl_lookup(d.db, node->_link);
            while (crefl_decl_idx(next))  {
                crefl_hash_absorb(sum, next_delimeter);
                decl_hash *hash = crefl_node_hash(index, next, d, prefix);
                crefl_hash_absorb(sum, hash_delimeter);
                crefl_hash_update(sum, (const char*)hash->sum, sizeof(decl_hash));
                next = crefl_decl_next(next);
            }
            break;
        /*
         * follow `link` to child element without processing `next`
         * for non container types such as 'typedef', 'field', 'pointer',
         * 'array' and 'param'. following `next` in these node types would
         * cause cycles from type references to adjacent anonymous types.
         */
        default:
            next = crefl_lookup(d.db, node->_link);
            if (crefl_entry_is_marked(crefl_entry_ref(index, next)) &&
                !crefl_entry_is_valid(crefl_entry_ref(index, next))) {
                /* we have a reference to a node that is being hashed */
                crefl_hash_absorb(sum, crefl_tag_name(crefl_decl_tag(next)));
                crefl_hash_absorb(sum, crefl_decl_name(next));
            } else {
                hash = crefl_node_hash(index, next, d, prefix);
                crefl_hash_absorb(sum, hash_delimeter);
                crefl_hash_update(sum, (const char*)hash->sum, sizeof(decl_hash));
            }
            break;
        }
    }
    crefl_hash_absorb(sum, end_delimeter);
}

int crefl_entry_is_marked(decl_entry_ref er)
{
    return (crefl_entry_ptr(er)->props & decl_entry_marked) == decl_entry_marked;
}

int crefl_entry_is_valid(decl_entry_ref er)
{
    return (crefl_entry_ptr(er)->props & decl_entry_valid) == decl_entry_valid;
}

static const std::string sep = "::";

std::string crefl_node_name(decl_ref d, decl_ref p, std::string prefix)
{
    static bool anon_parenthesis = false;
    std::string osep = prefix.size() > 0 ? sep : "";

    if (crefl_is_source(p)) return crefl_decl_name(d);
    if (crefl_is_archive(p)) return crefl_decl_name(d);

    switch (crefl_decl_tag(d)) {
    case _decl_array:
    case _decl_pointer:
        if (anon_parenthesis) {
            return prefix + osep + std::string("(") + std::string(
                crefl_tag_name(crefl_decl_tag(d))) + std::string(")");
        } else {
            return prefix;
        }
        break;
    }

    if (strlen(crefl_decl_name(d))) {
        return prefix + osep + crefl_decl_name(d);
    } else if (anon_parenthesis) {
        return prefix + osep + std::string("(") + std::string(
            crefl_tag_name(crefl_decl_tag(d))) + std::string(")");
    } else {
        return prefix;
    }
}

decl_hash * crefl_node_hash(decl_index *index, decl_ref d, decl_ref p, std::string prefix)
{
    decl_entry_ref er = crefl_entry_ref(index, d);
    decl_entry *ent = crefl_entry_ptr(er);

    prefix = crefl_node_name(d, p, prefix);

    if ((ent->props & decl_entry_valid) != decl_entry_valid) {
        decl_sum sum;
        ent->props |= decl_entry_marked;
        crefl_hash_init(&sum);
        crefl_hash_node_sum(&sum, index, d, p, prefix);
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
    crefl_node_hash(index, d, crefl_decl_void(d), "");
}

static std::string _hex_str(const uint8_t *data, size_t sz)
{
    std::string s;
    char hex[3];
    for (size_t i = 0; i < sz; i++) {
        snprintf(hex, sizeof(hex), "%02hhx", data[i]);
        s.append(hex);
    }
    return s;
}

struct _hash_fn
{
    size_t operator()(const decl_hash &h) const { return ((size_t*)h.sum)[0]; }
};

bool operator==(const decl_hash &a, const decl_hash &b)
{
    return memcmp(a.sum, b.sum, sizeof(a.sum)) == 0;
}

struct crefl_link_state
{
    hashmap<decl_hash,decl_ref,_hash_fn> *map;
    decl_db *db;
    decl_index *ld;
    decl_index *src_ld;
};

bool _should_copy(decl_ref d)
{
    /* copy if not one of: 'set', 'enum', 'struct', 'union' and 'function' */
    decl_ref r = crefl_decl_link(d);
    return !(crefl_is_set(d) || crefl_is_enum(d) ||
             crefl_is_struct(d) || crefl_is_union(d) ||
             crefl_is_function(d));
}

decl_ref crefl_copy_node(crefl_link_state *state, decl_ref d, decl_ref p,
    bool _is_child = false)
{
    decl_db *db = state->db;
    decl_node *node = crefl_decl_ptr(d);
    decl_entry_ref er = crefl_entry_ref(state->src_ld, d);
    decl_entry *ent = crefl_entry_ptr(er);
    decl_hash *hash = &ent->hash;
    decl_ref next, r, c, last = { db, 0 };

    /* always return direct references to intrinsics */
    if (crefl_decl_tag(d) == _decl_intrinsic) {
        return decl_ref {db, crefl_decl_idx(d) };
    }

    /* lookup node in our hash table to decide whether to copy or alias */
    auto i = state->map->find(*hash);
    if (i == state->map->end() || _should_copy(d)) {
        /* copy unseen nodes or non-collection nodes */
        r = crefl_decl_new(db, crefl_decl_tag(d));
        crefl_decl_ptr(r)->_name = crefl_name_new(db, crefl_decl_name(d));
        crefl_decl_ptr(r)->_props = crefl_decl_props(d);
        crefl_decl_ptr(r)->_quantity = crefl_decl_qty(d);
        (*state->map)[*hash] = r;
    } else {
        /* return node directly if it is a child link */
        if (_is_child) return decl_ref { db, i->second.decl_idx };
        /* otherwise alias node so we can override its next element */
        r = crefl_decl_new(db, _decl_alias);
        crefl_decl_ptr(r)->_name = crefl_name_new(db, crefl_decl_name(d));
        crefl_decl_ptr(r)->_link = i->second.decl_idx;
        (*state->map)[*hash] = r;
        return r;
    }

    if (node->_attr) {
        next = crefl_lookup(d.db, node->_attr);
        c = crefl_copy_node(state, next, d);
        crefl_decl_ptr(r)->_attr = crefl_decl_idx(c);
    }
    if (node->_link) {
        switch (crefl_decl_tag(d)) {
        /*
         * follow `link` to child list for container types: 'object',
         * 'set', 'enum', 'struct', 'union', and 'function' are lists
         * containing: 'typedef', 'field', 'pointer', 'array', etc.
         */
        case _decl_archive:
        case _decl_source:
        case _decl_set:
        case _decl_enum:
        case _decl_struct:
        case _decl_union:
        case _decl_function:
            next = crefl_lookup(d.db, node->_link);
            while (crefl_decl_idx(next))  {
                c = crefl_copy_node(state, next, d);
                if (crefl_decl_idx(last)) crefl_decl_ptr(last)->_next = crefl_decl_idx(c);
                else crefl_decl_ptr(r)->_link = crefl_decl_idx(c);
                last = c;
                next = crefl_decl_next(next);
            }
            break;
        default:
            next = crefl_lookup(d.db, node->_link);
            c = crefl_copy_node(state, next, d, true);
            crefl_decl_ptr(r)->_link = crefl_decl_idx(c);
            break;
        }
    }

    return r;
}

int crefl_link_merge(decl_db *db, const char *name, decl_db **srcn, size_t n)
{
    hashmap<decl_hash,decl_ref,_hash_fn> map;
    decl_index *ld = crefl_index_new();

    crefl_db_defaults(db);
    crefl_index_scan(ld, db);

    decl_ref r = crefl_decl_new(db, _decl_archive);
    crefl_decl_ptr(r)->_name = crefl_name_new(db,
        crefl_basename(name).c_str());
    db->root_element = crefl_decl_idx(r);

    decl_ref l { db, 0 };
    for (size_t i = 0; i < n; i++) {
        decl_index *src_ld = crefl_index_new();
        crefl_index_scan(src_ld, srcn[i]);
        crefl_link_state state{ &map, db, ld, src_ld };
        decl_ref d = crefl_lookup(srcn[i], srcn[i]->root_element);
        decl_ref p = crefl_decl_void(d);
        decl_ref o = crefl_copy_node(&state, d, p);
        if (crefl_decl_idx(l)) crefl_decl_ptr(l)->_next = crefl_decl_idx(o);
        else crefl_decl_ptr(r)->_link = crefl_decl_idx(o);
        l = o;
        crefl_index_destroy(src_ld);
    }

    crefl_index_destroy(ld);

    return 0;
}
