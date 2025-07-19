/*
 * cinf runtime library and compiler plug-in to support reflection in C.
 *
 * Copyright (c) 2020-2022 Michael Clark <michaeljclark@mac.com>
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
#include <cerrno>
#include <cassert>
#include <cstdlib>

#include <string>

#include <cinf/bits.h>
#include <cinf/model.h>
#include <cinf/link.h>
#include <cinf/util.h>
#include <cinf/hashmap.h>

/*
 * Crefl node hash algorithm
 *
 * Crefl contains a work-in-progress experimental model for C linkage that
 * composes the identity of function interfaces and their associated types
 * using Merkle Tree cryptographic hash sums. Merkel Tree style hash sums are
 * composed from the hierachical node properties of an abstract type tree, a
 * portion of the abstract syntax tree containing only the function interfaces
 * and type information.
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
 * nodes are assigned hashes with the following constraints and properties:
 *
 * - type hashes include information required to precisely identify types.
 * - type hashes are composed hierarchically in a deterministic traveral order
 *   recording successor and adjacency based on hash sum absorbtion order.
 * - type hashes are position invariant, but order preserving.
 * - type hashes absorb names but reference dependent nodes using hash sums.
 * - identical declarations with different ancestors have identical hash sums.
 * - types and interfaces link to dependencies without knowing their names.
 * - type hashes for incomplete types have different sums to complete types.
 * - semicolon is used as a delimeter as it does not occur in type names.
 * - SHA-224 is used because it is not subject to length extension attacks.
 * - type hashes for functions include parameter names and types. the index
 *   is decoupled so that alternative hashing algorithms can be used. e.g. a
 *   model used for linkage may omit parameter names.
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

static void cinf_hash_init(decl_sum *sum)
{
    sha224_init(&sum->ctx);
}

static void cinf_hash_absorb(decl_sum *sum, const char *str)
{
    sha224_update(&sum->ctx, str, strlen(str));
}

static void cinf_hash_final(decl_sum *sum, decl_hash *hash)
{
    sha224_final(&sum->ctx, (unsigned char*)hash->sum);
}

static void cinf_hash_update(decl_sum *sum, const void *data, size_t len)
{
    sha224_update(&sum->ctx, data, len);
}

decl_hash * cinf_node_hash(decl_index *index,
    decl_ref d, decl_ref p, std::string prefix);

static void cinf_hash_node_sum(decl_sum *sum, decl_index *index,
    decl_ref d, decl_ref p, std::string prefix)
{
    decl_node *node = cinf_decl_ptr(d);
    decl_hash *hash;
    decl_ref next;

    cinf_hash_absorb(sum, tag_delimeter);
    cinf_hash_absorb(sum, cinf_tag_name(cinf_decl_tag(d)));
    cinf_hash_absorb(sum, name_delimeter);
    cinf_hash_absorb(sum, cinf_decl_name(d));
    cinf_hash_absorb(sum, props_delimeter);
    cinf_hash_update(sum, &node->props, sizeof(node->props));
    cinf_hash_absorb(sum, quantity_delimeter);
    cinf_hash_update(sum, &node->quantity, sizeof(node->quantity));

    if (node->attr) {
        next = cinf_lookup(d.db, node->attr);
        cinf_hash_absorb(sum, attr_delimeter);
        hash = cinf_node_hash(index, next, d, prefix);
        cinf_hash_absorb(sum, hash_delimeter);
        cinf_hash_update(sum, (const char*)hash->sum, sizeof(decl_hash));
    }
    if (node->link) {
        switch (cinf_decl_tag(d)) {
        /*
         * follow `link` to child list for container types: 'object',
         * 'set', 'enum', 'struct', 'union', and 'function' are lists
         * containing: 'typedef', 'field', 'pointer', 'array', etc.
         */
        case decl_archive:
        case decl_source:
        case decl_enum:
        case decl_struct:
        case decl_union:
        case decl_function:
            cinf_hash_absorb(sum, link_delimeter);
            next = cinf_lookup(d.db, node->link);
            while (cinf_decl_idx(next))  {
                cinf_hash_absorb(sum, next_delimeter);
                decl_hash *hash = cinf_node_hash(index, next, d, prefix);
                cinf_hash_absorb(sum, hash_delimeter);
                cinf_hash_update(sum, (const char*)hash->sum, sizeof(decl_hash));
                next = cinf_decl_next(next);
            }
            break;
        /*
         * follow `link` to child element without processing `next`
         * for non container types such as 'typedef', 'field', 'pointer',
         * 'array' and 'param'. following `next` in these node types would
         * cause cycles from type references to adjacent anonymous types.
         */
        default:
            next = cinf_lookup(d.db, node->link);
            if (cinf_entry_is_marked(cinf_entry_ref(index, next)) &&
                !cinf_entry_is_valid(cinf_entry_ref(index, next))) {
                /* we have a reference to a node that is being hashed */
                cinf_hash_absorb(sum, cinf_tag_name(cinf_decl_tag(next)));
                cinf_hash_absorb(sum, cinf_decl_name(next));
            } else {
                hash = cinf_node_hash(index, next, d, prefix);
                cinf_hash_absorb(sum, hash_delimeter);
                cinf_hash_update(sum, (const char*)hash->sum, sizeof(decl_hash));
            }
            break;
        }
    }
    cinf_hash_absorb(sum, end_delimeter);
}

int cinf_entry_is_marked(decl_entry_ref er)
{
    return (cinf_entry_ptr(er)->props & decl_entry_marked) == decl_entry_marked;
}

int cinf_entry_is_valid(decl_entry_ref er)
{
    return (cinf_entry_ptr(er)->props & decl_entry_valid) == decl_entry_valid;
}

static const std::string sep = "::";

std::string cinf_node_name(decl_ref d, decl_ref p, std::string prefix)
{
    static bool anon_parenthesis = false;
    std::string osep = prefix.size() > 0 ? sep : "";

    if (cinf_is_source(p)) return cinf_decl_name(d);
    if (cinf_is_archive(p)) return cinf_decl_name(d);

    switch (cinf_decl_tag(d)) {
    case decl_array:
    case decl_pointer:
        if (anon_parenthesis) {
            return prefix + osep + std::string("(") + std::string(
                cinf_tag_name(cinf_decl_tag(d))) + std::string(")");
        } else {
            return prefix;
        }
        break;
    }

    if (strlen(cinf_decl_name(d))) {
        return prefix + osep + cinf_decl_name(d);
    } else if (anon_parenthesis) {
        return prefix + osep + std::string("(") + std::string(
            cinf_tag_name(cinf_decl_tag(d))) + std::string(")");
    } else {
        return prefix;
    }
}

decl_hash * cinf_node_hash(decl_index *index, decl_ref d, decl_ref p, std::string prefix)
{
    decl_entry_ref er = cinf_entry_ref(index, d);
    decl_entry *ent = cinf_entry_ptr(er);

    prefix = cinf_node_name(d, p, prefix);

    if ((ent->props & decl_entry_valid) != decl_entry_valid) {
        decl_sum sum;
        ent->props |= decl_entry_marked;
        cinf_hash_init(&sum);
        cinf_hash_node_sum(&sum, index, d, p, prefix);
        ent = cinf_entry_ptr(er); /* revalidate due to realloc */
        cinf_hash_final(&sum, &ent->hash);
        ent->fqn = cinf_entry_name_new(index, prefix.c_str());
        ent->props |= decl_entry_valid;
    }

    return &ent->hash;
}

decl_index * cinf_index_new()
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

void cinf_index_destroy(decl_index *index)
{
    free(index->name);
    free(index->entry);
    free(index);
}

decl_entry_ref cinf_entry_ref(decl_index *index, decl_ref r)
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

decl_id cinf_entry_name_new(decl_index *index, const char *name)
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

decl_entry * cinf_entry_ptr(decl_entry_ref d)
{
    return d.index->entry + d.offset;
}

const char* cinf_entry_fqn(decl_entry_ref d)
{
    return d.index->name + cinf_entry_ptr(d)->fqn;
}

void cinf_index_scan(decl_index *index, decl_db *db)
{
    decl_ref d = cinf_lookup(db, db->root_element);
    cinf_node_hash(index, d, cinf_decl_void(d), "");
}

static std::string hex_str(const uint8_t *data, size_t sz)
{
    std::string s;
    char hex[3];
    for (size_t i = 0; i < sz; i++) {
        snprintf(hex, sizeof(hex), "%02hhx", data[i]);
        s.append(hex);
    }
    return s;
}

struct hash_fn
{
    size_t operator()(const decl_hash &h) const { return ((size_t*)h.sum)[0]; }
};

bool operator==(const decl_hash &a, const decl_hash &b)
{
    return memcmp(a.sum, b.sum, sizeof(a.sum)) == 0;
}

struct cinf_link_state
{
    hashmap<decl_hash,decl_ref,hash_fn> *map;
    decl_db *db;
    decl_index *ld;
    decl_index *src_ld;
};

bool should_copy(decl_ref d)
{
    /* copy if not one of: 'set', 'enum', 'struct', 'union' and 'function' */
    decl_ref r = cinf_decl_link(d);
    return !(cinf_is_enum(d) || cinf_is_struct(d) ||
             cinf_is_union(d) || cinf_is_function(d));
}

decl_ref cinf_copy_node(cinf_link_state *state, decl_ref d, decl_ref p,
    bool is_child = false)
{
    decl_db *db = state->db;
    decl_node *node = cinf_decl_ptr(d);
    decl_entry_ref er = cinf_entry_ref(state->src_ld, d);
    decl_entry *ent = cinf_entry_ptr(er);
    decl_hash *hash = &ent->hash;
    decl_ref next, r, c, a, last = { db, 0 };

    /* always return direct references to intrinsics */
    if (cinf_decl_tag(d) == decl_intrinsic) {
        return decl_ref {db, cinf_decl_idx(d) };
    }

    /* lookup node in our hash table to decide whether to copy or alias */
    auto i = state->map->find(*hash);
    if (i == state->map->end() || should_copy(d)) {
        /* copy unseen nodes or non-collection nodes */
        r = cinf_decl_new(db, cinf_decl_tag(d));
        cinf_decl_ptr(r)->name = cinf_name_new(db, cinf_decl_name(d));
        cinf_decl_ptr(r)->props = cinf_decl_props(d);
        cinf_decl_ptr(r)->quantity = cinf_decl_qty(d);
        (*state->map)[*hash] = r;
    } else {
        /* return node directly if it is a child link */
        if (is_child) return decl_ref { db, i->second.decl_idx };
        /* otherwise alias node so we can override its next element */
        a = cinf_lookup(db, i->second.decl_idx);
        while (cinf_decl_tag(a) == decl_alias) {
            a = cinf_decl_link(a);
        }
        r = cinf_decl_new(db, decl_alias);
        cinf_decl_ptr(r)->name = cinf_name_new(db, cinf_decl_name(d));
        cinf_decl_ptr(r)->link = cinf_decl_idx(a);
        (*state->map)[*hash] = r;
        return r;
    }

    if (node->attr) {
        next = cinf_lookup(d.db, node->attr);
        c = cinf_copy_node(state, next, d);
        cinf_decl_ptr(r)->attr = cinf_decl_idx(c);
    }
    if (node->link) {
        switch (cinf_decl_tag(d)) {
        /*
         * follow `link` to child list for container types: 'object',
         * 'set', 'enum', 'struct', 'union', and 'function' are lists
         * containing: 'typedef', 'field', 'pointer', 'array', etc.
         */
        case decl_archive:
        case decl_source:
        case decl_enum:
        case decl_struct:
        case decl_union:
        case decl_function:
            next = cinf_lookup(d.db, node->link);
            while (cinf_decl_idx(next))  {
                c = cinf_copy_node(state, next, d);
                if (cinf_decl_idx(last)) cinf_decl_ptr(last)->next = cinf_decl_idx(c);
                else cinf_decl_ptr(r)->link = cinf_decl_idx(c);
                last = c;
                next = cinf_decl_next(next);
            }
            break;
        default:
            next = cinf_lookup(d.db, node->link);
            c = cinf_copy_node(state, next, d, true);
            cinf_decl_ptr(r)->link = cinf_decl_idx(c);
            break;
        }
    }

    return r;
}

int cinf_link_merge(decl_db *db, const char *name, decl_db **srcn, size_t n)
{
    hashmap<decl_hash,decl_ref,hash_fn> map;
    decl_index *ld = cinf_index_new();

    cinf_db_defaults(db);
    cinf_index_scan(ld, db);

    decl_ref r = cinf_decl_new(db, decl_archive);
    cinf_decl_ptr(r)->name = cinf_name_new(db,
        cinf_basename(name).c_str());
    db->root_element = cinf_decl_idx(r);

    decl_ref l { db, 0 };
    for (size_t i = 0; i < n; i++) {
        decl_index *src_ld = cinf_index_new();
        cinf_index_scan(src_ld, srcn[i]);
        cinf_link_state state{ &map, db, ld, src_ld };
        decl_ref d = cinf_lookup(srcn[i], srcn[i]->root_element);
        decl_ref p = cinf_decl_void(d);
        decl_ref o = cinf_copy_node(&state, d, p);
        if (cinf_decl_idx(l)) cinf_decl_ptr(l)->next = cinf_decl_idx(o);
        else cinf_decl_ptr(r)->link = cinf_decl_idx(o);
        l = o;
        cinf_index_destroy(src_ld);
    }

    cinf_index_destroy(ld);

    return 0;
}
