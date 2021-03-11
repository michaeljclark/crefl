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

#ifdef __cplusplus
extern "C" {
#endif

/*
 * # crefl reflection api
 *
 * the crefl API provides access to runtime reflection metadata for C
 * structure declarations with support for arbitrarily nested combinations
 * of: intrinsic, set, enum, struct, union, field, array, constant, variable.
 *
 * ## primary types
 *
 * - decl_node      - graph database declaration node type
 * - decl_db        - graph database containing the declarations
 * - decl_ref       - reference to a single declaration graph node
 * - decl_tag       - enumeration indicating graph node type
 * - decl_id        - indice of a graph node in the graph database
 * - decl_sz        - size type used for array size and bit widths
 * - decl_set       - type used to indicate many-of set enumerations
 * - decl_raw       - raw value used to store constants
 */

/*
 * decl base types
 */

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long s64;
typedef unsigned long long u64;
typedef float f32;
typedef double f64;

struct decl_node;
struct decl_db;
struct decl_ref;

typedef struct decl_node decl_node;
typedef struct decl_db decl_db;
typedef struct decl_ref decl_ref;
typedef union decl_raw decl_raw;

typedef u32 decl_tag;
typedef u32 decl_set;
typedef u32 decl_id;
typedef u64 decl_sz;

#define fmt_ID "%d"
#define fmt_SZ "%llu"
#define fmt_AD "0x%llx"

/*
 * # decl node
 *
 * primary data structure used to store a graph of reflection metadata.
 *
 * ## fields
 *
 * - decl_tag tag   - tagged union node type
 * - decl_set props - type specific properties
 * - decl_id name   - link to node name
 * - decl_id next   - link to next item
 * - decl_id link   - link to child item
 * - decl_id attr   - link to attribute list
 *
 * ## tags
 *
 * - void           - empty type
 * - intrinsic      - machine type with width in bits
 * - typedef        - alias to another type definition
 * - set            - machine type with many-of sequence
 * - enum           - machine type with one-of sequence
 * - struct         - sequence of non-overlapping types
 * - union          - sequence of overlapping types
 * - field          - top-level, struct and union field
 * - array          - sequence of one type
 * - pointer        - pointer type
 * - constant       - named constant
 * - function       - function with parameter list
 * - param          - named parameter with link to next
 * - attribute      - custom attribute name
 * - value          - custom attribute value
 *
 * ## properties
 *
 * - intrinsic      - sint, uint, float
 * - padding        - pad_pow2, pad_bit, pad_byte
 * - field          - bitfield
 * - qualifiers     - const, volatile, restrict
 * - binding        - local, global, weak
 * - visibility     - default, hidden
 *
 */
struct decl_node
{
    decl_tag _tag;
    decl_set _props;
    decl_id _name;
    decl_id _next;
    decl_id _link;
    decl_id _attr;

    /* quantifier used by intrinsic, set, enum, field, array, constant, etc */
    union {
        decl_sz _quantity;
        decl_sz _width;
        decl_sz _count;
        decl_sz _value;
        decl_sz _addr;
    };
};

/*
 * decl db
 *
 * reflection database containing decl nodes and symbol table.
 */
struct decl_db
{
    char* name;
    size_t name_builtin;
    size_t name_offset;
    size_t name_size;

    decl_node *decl;
    size_t decl_builtin;
    size_t decl_offset;
    size_t decl_size;

    decl_id root_element;
};

/*
 * decl ref
 *
 * reference to a single node in the reflection database.
 */
struct decl_ref
{
    decl_db *db;
    size_t decl_idx;
};

/*
 * decl tag
 *
 * union type tag indicating active properties in decl graph nodes.
 */
enum decl_tags
{
    _decl_none,
    _decl_intrinsic,
    _decl_typedef,
    _decl_set,
    _decl_enum,
    _decl_struct,
    _decl_union,
    _decl_field,
    _decl_array,
    _decl_pointer,
    _decl_constant,
    _decl_function,
    _decl_param,
    _decl_attribute,
    _decl_value
};

/*
 * decl props
 *
 * many-of set enumeration for graph node type specific properties such
 * as the intrinsic type, padding, qualifiers, binding and visibility.
 */
enum decl_props
{
    /* intrinsic type */
    _void           = 0,
    _integral       = 1 << 0,
    _real           = 1 << 1,
    _complex        = 1 << 2,
    _signed         = 1 << 3,
    _unsigned       = 1 << 4,
    _ieee754        = 1 << 5,

    _sint           = _integral | _signed,
    _uint           = _integral | _unsigned,
    _float          = _real     | _ieee754,
    _cfloat         = _complex  | _ieee754,

    /* padding */
    _pad_pow2       = 1 << 6,
    _pad_bit        = 1 << 7,
    _pad_byte       = 1 << 8,

    /* field */
    _bitfield       = 1 << 9,

    /* qualifiers */
    _const          = 1 << 10,
    _volatile       = 1 << 11,
    _restrict       = 1 << 12,

    /* binding */
    _local          = 1 << 13,
    _global         = 1 << 14,
    _weak           = 1 << 15,

    /* visibility */
    _default        = 1 << 16,
    _hidden         = 1 << 17,

    /* param */
    _in             = 1 << 18,
    _out            = 1 << 19,
};

/*
 * decl raw
 *
 * union to provide access to raw values for intrinsic types.
 */
union decl_raw
{
    u64   ux;
    s64   sx;
    void* px;
    u8    ub[8];
    s8    sb[8];
    u16   uw[4];
    s16   sw[4];
    u32   ud[2];
    s32   sd[2];
    u64   uq[1];
    s64   sq[1];
    f32   fs[2];
    f64   fd[1];
};

/*
 * # C Interface
 */

/*
 * decl types
 */
int crefl_is_any(decl_ref d);
int crefl_is_top(decl_ref d);
int crefl_is_type(decl_ref d);
int crefl_is_intrinsic(decl_ref d);
int crefl_is_typedef(decl_ref d);
int crefl_is_set(decl_ref d);
int crefl_is_enum(decl_ref d);
int crefl_is_struct(decl_ref d);
int crefl_is_union(decl_ref d);
int crefl_is_field(decl_ref d);
int crefl_is_array(decl_ref d);
int crefl_is_pointer(decl_ref d);
int crefl_is_constant(decl_ref d);
int crefl_is_function(decl_ref d);
int crefl_is_param(decl_ref d);
int crefl_is_attribute(decl_ref d);
int crefl_is_value(decl_ref d);

/*
 * decl database
 */
decl_db * crefl_db_new();
void crefl_db_defaults(decl_db *db);
void crefl_db_destroy(decl_db *db);

/*
 * decl properties
 */
decl_node * crefl_decl_ptr(decl_ref d);
decl_tag crefl_decl_tag(decl_ref d);
decl_set crefl_decl_props(decl_ref d);
decl_id crefl_decl_idx(decl_ref d);
decl_ref crefl_decl_next(decl_ref d);
decl_ref crefl_decl_attr(decl_ref d);

/*
 * decl allocation
 */
decl_id crefl_name_new(decl_db *db, const char *name);
decl_ref crefl_decl_new(decl_db *db, decl_tag tag);

/*
 * decl queries
 */
decl_ref crefl_intrinsic(decl_db *db, decl_set props, size_t width);
decl_ref crefl_lookup(decl_db *db, size_t decl_idx);
const char* crefl_tag_name(decl_tag tag);
const char* crefl_decl_name(decl_ref d);
int crefl_list_decls(decl_db *db, decl_ref *r, size_t *s);
int crefl_list_types(decl_db *db, decl_ref *r, size_t *s);
int crefl_list_fields(decl_db *db, decl_ref *r, size_t *s);
int crefl_list_functions(decl_db *db, decl_ref *r, size_t *s);
size_t crefl_type_width(decl_ref d);
size_t crefl_intrinsic_width(decl_ref d);
size_t crefl_struct_width(decl_ref d);
size_t crefl_union_width(decl_ref d);
size_t crefl_array_count(decl_ref d);
size_t crefl_pointer_width(decl_ref d);
decl_ref crefl_typedef_type(decl_ref d);
decl_ref crefl_field_type(decl_ref d);
decl_ref crefl_array_type(decl_ref d);
decl_ref crefl_pointer_type(decl_ref d);
decl_ref crefl_constant_type(decl_ref d);
decl_ref crefl_param_type(decl_ref d);
int crefl_enum_constants(decl_ref d, decl_ref *r, size_t *s);
int crefl_set_constants(decl_ref d, decl_ref *r, size_t *s);
int crefl_struct_fields(decl_ref d, decl_ref *r, size_t *s);
int crefl_union_fields(decl_ref d, decl_ref *r, size_t *s);
int crefl_function_params(decl_ref d, decl_ref *r, size_t *s);
decl_raw crefl_constant_value(decl_ref d);
void * crefl_function_addr(decl_ref d);

#ifdef __cplusplus
}
#endif
