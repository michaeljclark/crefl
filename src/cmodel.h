#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long s64;
typedef unsigned long long u64;

struct decl;
struct decl_db;
struct decl_ref;

typedef struct decl decl;
typedef struct decl_ref decl_ref;
typedef struct decl_db decl_db;
typedef union _raw _raw;
typedef struct _decl _decl;

typedef u32 _Tag;
typedef u32 _Set;
typedef u32 _Id;
typedef u64 _Size;

#define IDfmt "d"

extern const _decl _cvoid;
extern const _decl _cptr32;
extern const _decl _cptr64;
extern const _decl _cbool;
extern const _decl _uint1;
extern const _decl _sint1;
extern const _decl _uint8;
extern const _decl _sint8;
extern const _decl _uint16;
extern const _decl _sint16;
extern const _decl _uint32;
extern const _decl _sint32;
extern const _decl _uint64;
extern const _decl _sint64;
extern const _decl _uint128;
extern const _decl _sint128;
extern const _decl _float16;
extern const _decl _float32;
extern const _decl _float64;
extern const _decl _float128;

/*
 * decl base types
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
 * raw type
 */
union _raw
{
    s64    sx;
    u64    ux;
    char   cx[sizeof(void*)];
    void*  vx;
};

/*
 * decl struct
 */
struct _decl
{
    int _tag;
    int _width;
    int _attrs;
    const char* _name;
    const struct _decl **_elements;
};

/*
 * decl attrs
 */
enum _decl_attrs
{
    /*
     * bit[0:1] pad
     *
     * type bit packing information. the type can be packed to the
     * nearest power of two, the nearest byte, or the nearest bit.
     */

    _pad_pow2       = 0,  /* pad to nearest power of two */
    _pad_bit        = 1,  /* pad to nearest bit */
    _pad_byte       = 2,  /* pad to nearest byte */

    /*
     * bit[2:6] intrinsic type flags
     */

    _void           = 0,
    _integral       = 1 << 2,
    _real           = 1 << 3,

    _signed         = 1 << 4,
    _unsigned       = 1 << 5,
    _ieee754        = 1 << 6,

    _sint           = _integral | _signed,
    _uint           = _integral | _unsigned,
    _float          = _real     | _ieee754,

    /*
     * bit[30:31] general type flags
     */

    _bitfield       = 1 << 30,
    _top            = 1 << 31
};

struct decl_db
{
    char* data;  size_t data_offset,  data_size;
    decl *decl;  size_t decl_offset,  decl_size;
};

struct decl_ref
{
    decl_db *db; size_t decl_idx;
};

/*
 * - void       (* empty type *)
 * - typedef    (* alias to another type definition *)
 * - intrinsic  (* machine type quantified with width in bits *)
 * - set        (* machine type with many-of sequence of bit mask constants *)
 * - enum       (* machine type with one-of sequence of integral constants *)
 * - struct     (* sequence of non-overlapping types *)
 * - union      (* sequence of overlapping types *)
 * - field      (* named field within struct or union *)
 * - array      (* sequence of one type *)
 * - constant   (* named constant *)
 * - variable   (* named variable that is unique to each thread *)
 * - uniform    (* named variable that is uniform across threads *)
 * - function   (* function with input and output parameter list *)
 * - param      (* named parameter with link to next *)
 */

struct decl
{
    _Tag _tag;    /* tag: enum containing token for union member */
    _Set _attrs;  /* attrs: _pad_pow2,   _pad_bit,    _pad_byte, */
    _Id _name;    /*        _void,       _integral,   _real,     */
    _Id _next;    /*        _signed,     _unsigned,   _ieee754,  */

    union {
        struct { _Id _decl;               } _decl_typedef;
        struct { _Size _width;            } _decl_intrinsic;
        struct { _Id _link;               } _decl_set;
        struct { _Id _link;               } _decl_enum;
        struct { _Id _link;               } _decl_struct;
        struct { _Id _link;               } _decl_union;
        struct { _Id _decl; _Size _width; } _decl_field;
        struct { _Id _decl; _Size _size;  } _decl_array;
        struct { _Id _decl; _Size _value; } _decl_constant;
        struct { _Id _decl; _Size _addr;  } _decl_variable;
        struct { _Id _decl; _Size _addr;  } _decl_uniform;
        struct { _Id _link; _Size _addr;  } _decl_function;
        struct { _Id _decl;               } _decl_param;
    };
};

/*
 * reflection api
 */

_Tag decl_tag(decl *d);
_Set decl_attrs(decl *d);

int decl_is_top(decl *d);
int decl_is_type(decl *d);
int decl_is_typedef(decl *d);
int decl_is_intrinsic(decl *d);
int decl_is_set(decl *d);
int decl_is_enum(decl *d);
int decl_is_struct(decl *d);
int decl_is_union(decl *d);
int decl_is_field(decl *d);
int decl_is_array(decl *d);
int decl_is_constant(decl *d);
int decl_is_variable(decl *d);
int decl_is_uniform(decl *d);
int decl_is_function(decl *d);
int decl_is_param(decl *d);

decl_db * decl_db_new();
void decl_db_defaults(decl_db *db);
void decl_db_dump(decl_db *db);
void decl_db_destroy(decl_db *db);
decl_ref decl_new(decl_db *db, _Tag tag, _Set attrs);
decl_ref decl_at(decl_db *db, size_t decl_idx);
decl_ref decl_find_intrinsic(decl_db *db, _Set attrs, size_t width);
decl * decl_ptr(decl_ref r);
_Id decl_ref_idx(decl_ref d);
_Tag decl_ref_tag(decl_ref d);
_Set decl_ref_attrs(decl_ref d);
const char* decl_name_new(decl_ref d, const char *name);
const char* decl_name(decl_ref d);
int decl_types(decl_db *db, decl_ref *r, size_t *s);
int decl_constants(decl_db *db, decl_ref *r, size_t *s);
int decl_variables(decl_db *db, decl_ref *r, size_t *s);
int decl_uniforms(decl_db *db, decl_ref *r, size_t *s);
int decl_functions(decl_db *db, decl_ref *r, size_t *s);
size_t decl_type_width(decl_ref d);
size_t decl_intrinsic_width(decl_ref d);
size_t decl_struct_width(decl_ref d);
size_t decl_union_width(decl_ref d);
size_t decl_array_size(decl_ref d);
decl_ref decl_typedef_type(decl_ref d);
decl_ref decl_array_type(decl_ref d);
decl_ref decl_constant_type(decl_ref d);
decl_ref decl_variable_type(decl_ref d);
decl_ref decl_uniform_type(decl_ref d);
decl_ref decl_param_type(decl_ref d);
int decl_enum_constants(decl_ref d, decl_ref *r, size_t *s);
int decl_set_constants(decl_ref d, decl_ref *r, size_t *s);
int decl_struct_types(decl_ref d, decl_ref *r, size_t *s);
int decl_union_types(decl_ref d, decl_ref *r, size_t *s);
int decl_function_params(decl_ref d, decl_ref *r, size_t *s);
_Size decl_constant_value(decl_ref d);
void * decl_variable_addr(decl_ref d);
void * decl_uniform_addr(decl_ref d);
void * decl_function_addr(decl_ref d);

#ifdef __cplusplus
}
#endif
