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

struct decl;
struct decl_db;
struct decl_ref;

typedef struct decl decl;
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
 * decl tag
 *
 * union type tag indicating active properties in decl graph nodes.
 */
enum {
    _decl_void,
    _decl_typedef,
    _decl_intrinsic,
    _decl_set,
    _decl_enum,
    _decl_struct,
    _decl_union,
    _decl_field,
    _decl_array,
    _decl_constant,
    _decl_variable,
    _decl_uniform,
    _decl_function,
    _decl_param,
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
 * decl attrs
 *
 * many-of set enumeration for graph node type specific attributes such
 * as the intrinsic type, padding, qualifiers, binding and visibility.
 */
enum decl_attrs
{
    /* intrinsic type */
    _void           = 0,
    _integral       = 1 << 0,
    _real           = 1 << 1,
    _signed         = 1 << 2,
    _unsigned       = 1 << 3,
    _ieee754        = 1 << 4,

    _sint           = _integral | _signed,
    _uint           = _integral | _unsigned,
    _float          = _real     | _ieee754,

    /* padding */
    _pad_pow2       = 1 << 5,
    _pad_bit        = 1 << 6,
    _pad_byte       = 1 << 7,

    /* qualifiers */
    _bitfield       = 1 << 8,
    _const          = 1 << 9,
    _volatile       = 1 << 10,
    _restrict       = 1 << 11,

    /* binding */
    _local          = 1 << 12,
    _global         = 1 << 13,
    _weak           = 1 << 14,

    /* visibility */
    _default        = 1 << 15,
    _hidden         = 1 << 16,

    /* general */
    _top            = 1 << 31
};

/*
 * decl db
 *
 * reflection database containing decl nodes and symbol table.
 */
struct decl_db
{
    char* data;  size_t data_offset,  data_size;
    decl *decl;  size_t decl_offset,  decl_size;
};

/*
 * decl ref
 *
 * reference to a single node in the reflection database.
 */
struct decl_ref
{
    decl_db *db; size_t decl_idx;
};

/*
 * decl
 *
 * primary data structure used to store a graph of reflection metadata.
 * decl is comprised of a set of shared fields, including name and index
 * of the next node. node type specific properties are stored in a union.
 *
 * decl fields
 *
 * - decl_tag tag   - union type tag indicating active properties
 * - decl_set attrs - type specific declaration attributes
 * - decl_id name   - offset of name in symbol table
 * - decl_id next   - pointer to next node in a sequence
 *
 * attributes values
 *
 * - intrinsic      - sint, uint, float
 * - padding        - pad_pow2, pad_bit, pad_byte
 * - field          - bitfield
 * - general        - top
 *
 * tag descriptions
 *
 * - void           - empty type
 * - typedef        - alias to another type definition
 * - intrinsic      - machine type quantified with width in bits
 * - set            - machine type with many-of sequence of bit mask constants
 * - enum           - machine type with one-of sequence of integral constants
 * - struct         - sequence of non-overlapping types
 * - union          - sequence of overlapping types
 * - field          - named field within struct or union
 * - array          - sequence of one type
 * - constant       - named constant
 * - variable       - named variable that is unique to each thread
 * - uniform        - named variable that is uniform across threads
 * - function       - function with input and output parameter list
 * - param          - named parameter with link to next
 *
 */
struct decl
{
    decl_tag _tag;
    decl_set _attrs;
    decl_id _name;
    decl_id _next;

    union {
        struct { decl_id _decl;                 } _decl_typedef;
        struct { decl_sz _width;                } _decl_intrinsic;
        struct { decl_id _link;                 } _decl_set;
        struct { decl_id _link;                 } _decl_enum;
        struct { decl_id _link;                 } _decl_struct;
        struct { decl_id _link;                 } _decl_union;
        struct { decl_id _decl; decl_sz _width; } _decl_field;
        struct { decl_id _decl; decl_sz _size;  } _decl_array;
        struct { decl_id _decl; decl_sz _value; } _decl_constant;
        struct { decl_id _decl; decl_sz _addr;  } _decl_variable;
        struct { decl_id _decl; decl_sz _addr;  } _decl_uniform;
        struct { decl_id _link; decl_sz _addr;  } _decl_function;
        struct { decl_id _decl;                 } _decl_param;
    };
};

/*
 * crefl reflection api
 *
 * the crefl API provides access to runtime reflection metadata for C
 * structure declarations with support for arbitrarily nested combinations
 * of intrinsic, set, enum, struct, union, field, array, constant, variable,
 * uniform and function declarations.
 *
 * primary types
 *
 * - decl_db        - graph database containing the declarations
 * - decl_ref       - reference to a single declaration graph node
 * - decl_tag       - enumeration indicating graph node type
 * - decl_id        - indice of a graph node in the graph database
 * - decl_sz        - size type used for array size and bit widths
 * - decl_set       - type used to indicate many-of set enumerations
 */

/*
 * decl types
 */
int crefl_is_top(decl_ref d);
int crefl_is_type(decl_ref d);
int crefl_is_typedef(decl_ref d);
int crefl_is_intrinsic(decl_ref d);
int crefl_is_set(decl_ref d);
int crefl_is_setr(decl_ref d);
int crefl_is_enum(decl_ref d);
int crefl_is_struct(decl_ref d);
int crefl_is_union(decl_ref d);
int crefl_is_field(decl_ref d);
int crefl_is_array(decl_ref d);
int crefl_is_constant(decl_ref d);
int crefl_is_variable(decl_ref d);
int crefl_is_uniform(decl_ref d);
int crefl_is_function(decl_ref d);
int crefl_is_param(decl_ref d);

/*
 * decl database
 */
decl_db * crefl_db_new();
void crefl_db_defaults(decl_db *db);
void crefl_db_destroy(decl_db *db);

/*
 * decl properties
 */
decl * crefl_ptr(decl_ref d);
decl_tag crefl_tag(decl_ref d);
decl_set crefl_attrs(decl_ref d);
decl_id crefl_idx(decl_ref d);
decl_ref crefl_next(decl_ref d);
decl_ref crefl_new(decl_db *db, decl_tag tag, decl_set attrs);
const char* crefl_name_new(decl_ref d, const char *name);

/*
 * decl queries
 */
decl_ref crefl_find_intrinsic(decl_db *db, decl_set attrs, size_t width);
decl_ref crefl_lookup(decl_db *db, size_t decl_idx);
const char * crefl_tag_name(decl_tag tag);
const char* crefl_name(decl_ref d);
int crefl_types(decl_db *db, decl_ref *r, size_t *s);
int crefl_constants(decl_db *db, decl_ref *r, size_t *s);
int crefl_variables(decl_db *db, decl_ref *r, size_t *s);
int crefl_uniforms(decl_db *db, decl_ref *r, size_t *s);
int crefl_functions(decl_db *db, decl_ref *r, size_t *s);
size_t crefl_type_width(decl_ref d);
size_t crefl_intrinsic_width(decl_ref d);
size_t crefl_struct_width(decl_ref d);
size_t crefl_union_width(decl_ref d);
size_t crefl_array_size(decl_ref d);
decl_ref crefl_typedef_type(decl_ref d);
decl_ref crefl_array_type(decl_ref d);
decl_ref crefl_constant_type(decl_ref d);
decl_ref crefl_variable_type(decl_ref d);
decl_ref crefl_uniform_type(decl_ref d);
decl_ref crefl_param_type(decl_ref d);
int crefl_enum_constants(decl_ref d, decl_ref *r, size_t *s);
int crefl_set_constants(decl_ref d, decl_ref *r, size_t *s);
int crefl_struct_types(decl_ref d, decl_ref *r, size_t *s);
int crefl_union_types(decl_ref d, decl_ref *r, size_t *s);
int crefl_function_params(decl_ref d, decl_ref *r, size_t *s);
decl_raw crefl_constant_value(decl_ref d);
void * crefl_variable_addr(decl_ref d);
void * crefl_uniform_addr(decl_ref d);
void * crefl_function_addr(decl_ref d);

#ifdef __cplusplus
}
#endif
