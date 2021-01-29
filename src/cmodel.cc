#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>

#include "cmodel.h"

const _decl _cvoid =    { _decl_intrinsic,  0, _void  | _pad_bit,  "cvoid" };
const _decl _cptr32 =   { _decl_intrinsic, 32, _void  | _pad_pow2, "cptr32" };
const _decl _cptr64 =   { _decl_intrinsic, 64, _void  | _pad_pow2, "cptr64" };
const _decl _cbool =    { _decl_intrinsic,  1, _sint  | _pad_byte, "cbool" };
const _decl _uint1 =    { _decl_intrinsic,  1, _uint  | _pad_pow2, "u1" };
const _decl _sint1 =    { _decl_intrinsic,  1, _sint  | _pad_pow2, "i1" };
const _decl _uint8 =    { _decl_intrinsic,  8, _uint  | _pad_pow2, "u8" };
const _decl _sint8 =    { _decl_intrinsic,  8, _sint  | _pad_pow2, "i8" };
const _decl _uint16 =   { _decl_intrinsic, 16, _uint  | _pad_pow2, "u16" };
const _decl _sint16 =   { _decl_intrinsic, 16, _sint  | _pad_pow2, "i16" };
const _decl _uint32 =   { _decl_intrinsic, 32, _uint  | _pad_pow2, "u32" };
const _decl _sint32 =   { _decl_intrinsic, 32, _sint  | _pad_pow2, "i32" };
const _decl _uint64 =   { _decl_intrinsic, 64, _uint  | _pad_pow2, "u64" };
const _decl _sint64 =   { _decl_intrinsic, 64, _sint  | _pad_pow2, "i64" };
const _decl _uint128 =  { _decl_intrinsic, 128,_uint  | _pad_pow2, "u128" };
const _decl _sint128 =  { _decl_intrinsic, 128,_sint  | _pad_pow2, "i128" };
const _decl _float16 =  { _decl_intrinsic, 16, _float | _pad_pow2, "f16" };
const _decl _float32 =  { _decl_intrinsic, 32, _float | _pad_pow2, "f32" };
const _decl _float64 =  { _decl_intrinsic, 64, _float | _pad_pow2, "f64" };
const _decl _float128 = { _decl_intrinsic, 128,_float | _pad_pow2, "f128" };

const _decl* _vec2h_el[] = { &_float16, &_float16, 0 };
const _decl* _vec2f_el[] = { &_float32, &_float32, 0 };
const _decl* _vec2d_el[] = { &_float64, &_float64, 0 };
const _decl* _vec2t_el[] = { &_float128,&_float128,0 };
const _decl* _vec3h_el[] = { &_float16, &_float16, &_float16, 0 };
const _decl* _vec3f_el[] = { &_float32, &_float32, &_float32, 0 };
const _decl* _vec3d_el[] = { &_float64, &_float64, &_float64, 0 };
const _decl* _vec3t_el[] = { &_float128,&_float128,&_float128,0 };
const _decl* _vec4h_el[] = { &_float16, &_float16, &_float16, &_float16, 0 };
const _decl* _vec4f_el[] = { &_float32, &_float32, &_float32, &_float32, 0 };
const _decl* _vec4d_el[] = { &_float64, &_float64, &_float64, &_float64, 0 };
const _decl* _vec4t_el[] = { &_float128,&_float128,&_float128,&_float128,0 };

const _decl _vec2h = { _decl_struct, 32,  _pad_pow2, "vec2h", _vec2h_el };
const _decl _vec2f = { _decl_struct, 64,  _pad_pow2, "vec2f", _vec2f_el };
const _decl _vec2d = { _decl_struct, 128, _pad_pow2, "vec2d", _vec2d_el };
const _decl _vec2t = { _decl_struct, 256, _pad_pow2, "vec2t", _vec2t_el };
const _decl _vec3h = { _decl_struct, 48,  _pad_pow2, "vec3h", _vec3h_el };
const _decl _vec3f = { _decl_struct, 96,  _pad_pow2, "vec3f", _vec3f_el };
const _decl _vec3d = { _decl_struct, 192, _pad_pow2, "vec3d", _vec3d_el };
const _decl _vec3t = { _decl_struct, 384, _pad_pow2, "vec3t", _vec3t_el };
const _decl _vec4h = { _decl_struct, 64,  _pad_pow2, "vec4h", _vec4h_el };
const _decl _vec4f = { _decl_struct, 128, _pad_pow2, "vec4f", _vec4f_el };
const _decl _vec4d = { _decl_struct, 256, _pad_pow2, "vec4d", _vec4d_el };
const _decl _vec4t = { _decl_struct, 512, _pad_pow2, "vec4t", _vec4t_el };

const _decl* _vec2b_el[] = { &_sint8,  &_sint8,  0 };
const _decl* _vec2s_el[] = { &_sint16, &_sint16, 0 };
const _decl* _vec2i_el[] = { &_sint32, &_sint32, 0 };
const _decl* _vec2l_el[] = { &_sint64, &_sint64, 0 };
const _decl* _vec3b_el[] = { &_sint8,  &_sint8,  &_sint8,  0 };
const _decl* _vec3s_el[] = { &_sint16, &_sint16, &_sint16, 0 };
const _decl* _vec3i_el[] = { &_sint32, &_sint32, &_sint32, 0 };
const _decl* _vec3l_el[] = { &_sint64, &_sint64, &_sint64, 0 };
const _decl* _vec4b_el[] = { &_sint8,  &_sint8,  &_sint8,  &_sint8,  0 };
const _decl* _vec4s_el[] = { &_sint16, &_sint16, &_sint16, &_sint16, 0 };
const _decl* _vec4i_el[] = { &_sint32, &_sint32, &_sint32, &_sint32, 0 };
const _decl* _vec4l_el[] = { &_sint64, &_sint64, &_sint64, &_sint64, 0 };

const _decl _vec2b = { _decl_struct, 16,  _pad_pow2, "vec2b", _vec2b_el };
const _decl _vec2s = { _decl_struct, 32,  _pad_pow2, "vec2s", _vec2s_el };
const _decl _vec2i = { _decl_struct, 64,  _pad_pow2, "vec2i", _vec2i_el };
const _decl _vec2l = { _decl_struct, 128, _pad_pow2, "vec2l", _vec2l_el };
const _decl _vec3b = { _decl_struct, 24,  _pad_pow2, "vec3b", _vec3b_el };
const _decl _vec3s = { _decl_struct, 48,  _pad_pow2, "vec3s", _vec3s_el };
const _decl _vec3i = { _decl_struct, 96,  _pad_pow2, "vec3i", _vec3i_el };
const _decl _vec3l = { _decl_struct, 192, _pad_pow2, "vec3l", _vec3l_el };
const _decl _vec4b = { _decl_struct, 32,  _pad_pow2, "vec4b", _vec4b_el };
const _decl _vec4s = { _decl_struct, 64,  _pad_pow2, "vec4s", _vec4s_el };
const _decl _vec4i = { _decl_struct, 128, _pad_pow2, "vec4i", _vec4i_el };
const _decl _vec4l = { _decl_struct, 256, _pad_pow2, "vec4l", _vec4l_el };

const _decl* _vec2ub_el[] = { &_uint8,  &_uint8,  0 };
const _decl* _vec2us_el[] = { &_uint16, &_uint16, 0 };
const _decl* _vec2ui_el[] = { &_uint32, &_uint32, 0 };
const _decl* _vec2ul_el[] = { &_uint64, &_uint64, 0 };
const _decl* _vec3ub_el[] = { &_uint8,  &_uint8,  &_uint8,  0 };
const _decl* _vec3us_el[] = { &_uint16, &_uint16, &_uint16, 0 };
const _decl* _vec3ui_el[] = { &_uint32, &_uint32, &_uint32, 0 };
const _decl* _vec3ul_el[] = { &_uint64, &_uint64, &_uint64, 0 };
const _decl* _vec4ub_el[] = { &_uint8,  &_uint8,  &_uint8,  &_uint8,  0 };
const _decl* _vec4us_el[] = { &_uint16, &_uint16, &_uint16, &_uint16, 0 };
const _decl* _vec4ui_el[] = { &_uint32, &_uint32, &_uint32, &_uint32, 0 };
const _decl* _vec4ul_el[] = { &_uint64, &_uint64, &_uint64, &_uint64, 0 };

const _decl _vec2ub = { _decl_struct, 16,  _pad_pow2, "vec2ub", _vec2ub_el };
const _decl _vec2us = { _decl_struct, 32,  _pad_pow2, "vec2us", _vec2us_el };
const _decl _vec2ui = { _decl_struct, 64,  _pad_pow2, "vec2ui", _vec2ui_el };
const _decl _vec2ul = { _decl_struct, 128, _pad_pow2, "vec2ul", _vec2ul_el };
const _decl _vec3ub = { _decl_struct, 24,  _pad_pow2, "vec3ub", _vec3ub_el };
const _decl _vec3us = { _decl_struct, 48,  _pad_pow2, "vec3us", _vec3us_el };
const _decl _vec3ui = { _decl_struct, 96,  _pad_pow2, "vec3ui", _vec3ui_el };
const _decl _vec3ul = { _decl_struct, 192, _pad_pow2, "vec3ul", _vec3ul_el };
const _decl _vec4ub = { _decl_struct, 32,  _pad_pow2, "vec4ub", _vec4ub_el };
const _decl _vec4us = { _decl_struct, 64,  _pad_pow2, "vec4us", _vec4us_el };
const _decl _vec4ui = { _decl_struct, 128, _pad_pow2, "vec4ui", _vec4ui_el };
const _decl _vec4ul = { _decl_struct, 256, _pad_pow2, "vec4ul", _vec4ul_el };

const _decl *all_types[] = {
    &_cvoid,        &_cptr32,       &_cptr64,       &_cbool,
    &_uint1,        &_sint1,        &_uint8,        &_sint8,
    &_uint16,       &_sint16,       &_uint32,       &_sint32,
    &_uint64,       &_sint64,       &_uint128,      &_sint128,
    &_float16,      &_float32,      &_float64,      &_float128,
#if 0
    &_vec2h,        &_vec2f,        &_vec2d,        &_vec2t,
    &_vec3h,        &_vec3f,        &_vec3d,        &_vec3t,
    &_vec4h,        &_vec4f,        &_vec4d,        &_vec4t,
    &_vec2b,        &_vec2s,        &_vec2i,        &_vec2l,
    &_vec3b,        &_vec3s,        &_vec3i,        &_vec3l,
    &_vec4b,        &_vec4s,        &_vec4i,        &_vec4l,
    &_vec2ub,       &_vec2us,       &_vec2ui,       &_vec2ul,
    &_vec3ub,       &_vec3us,       &_vec3ui,       &_vec3ul,
    &_vec4ub,       &_vec4us,       &_vec4ui,       &_vec4ul,
#endif
};


/*
 * decl helpers
 */

_Tag decl_tag(decl *d) { return d->_tag; }
_Set decl_attrs(decl *d) { return d->_attrs; }

int decl_is_top(decl *d)
{
    return (decl_attrs(d) & _top) > 0;
}

int decl_is_type(decl *d)
{
    _Tag t = decl_tag(d);
    return t == _decl_typedef || t == _decl_intrinsic ||
           t == _decl_set     || t == _decl_enum      ||
           t == _decl_struct  || t == _decl_union     ||
           t == _decl_field   || t == _decl_array;
}

int decl_is_typedef(decl *d) { return decl_tag(d) == _decl_typedef; }
int decl_is_intrinsic(decl *d) { return decl_tag(d) == _decl_intrinsic; }
int decl_is_set(decl *d) { return decl_tag(d) == _decl_set; }
int decl_is_enum(decl *d) { return decl_tag(d) == _decl_enum; }
int decl_is_struct(decl *d) { return decl_tag(d) == _decl_struct; }
int decl_is_union(decl *d) { return decl_tag(d) == _decl_union; }
int decl_is_field(decl *d) { return decl_tag(d) == _decl_field; }
int decl_is_array(decl *d) { return decl_tag(d) == _decl_array; }
int decl_is_constant(decl *d) { return decl_tag(d) == _decl_constant; }
int decl_is_variable(decl *d) { return decl_tag(d) == _decl_variable; }
int decl_is_uniform(decl *d) { return decl_tag(d) == _decl_uniform; }
int decl_is_function(decl *d) { return decl_tag(d) == _decl_function; }
int decl_is_param(decl *d) { return decl_tag(d) == _decl_param; }

/*
 * decl relflection
 */

#define array_size(arr) ((sizeof(arr)/sizeof(arr[0])))

static const char * decl_tag_names_arr[] = {
    "void",
    "typedef",
    "intrinsic",
    "set",
    "enum",
    "struct",
    "union",
    "field",
    "array",
    "constant",
    "variable",
    "uniform",
    "function",
    "param"
};

static const char * decl_tag_name(_Tag tag)
{
    if (tag < array_size(decl_tag_names_arr)) {
        return decl_tag_names_arr[tag];
    } else {
        return "<unknown>";
    }
}

decl_ref decl_at(decl_db *db, size_t decl_idx)
{
    return decl_ref { db, decl_idx };
}

decl * decl_ptr(decl_ref d)
{
    return d.db->decl + d.decl_idx;
}

_Id decl_ref_idx(decl_ref d) { return d.decl_idx; }

_Tag decl_ref_tag(decl_ref d) { return (d.db->decl + d.decl_idx)->_tag; }
_Set decl_ref_attrs(decl_ref d) { return (d.db->decl + d.decl_idx)->_attrs; }

decl_db * decl_db_new()
{
    decl_db *db = (decl_db*)malloc(sizeof(decl_db));

    db->data_offset = 1; /* offset 0 holds empty string */
    db->data_size = 128;
    db->data = (char*)malloc(db->data_size);
    memset(db->data, 0, db->data_size);

    db->decl_offset = 1; /* offset 0 slot is empty */
    db->decl_size = 128;
    db->decl = (decl*)malloc(sizeof(decl) * db->decl_size);
    memset(db->decl, 0, sizeof(decl) * db->decl_size);

    return db;
}

void decl_db_defaults(decl_db *db)
{
    for (size_t i = 0; i < array_size(all_types); i++) {
        const _decl *d = all_types[i];
        if (d->_tag == _decl_intrinsic) {
            decl_ref r = decl_new(db, _decl_intrinsic, _top | d->_attrs);
            decl_name_new(r, d->_name);
            decl_ptr(r)->_decl_intrinsic._width = d->_width;
        }
    }
}

static std::string _pretty_name(std::string l, decl_db *db, size_t decl_idx)
{
    decl_ref d = decl_at(db, decl_idx);
    std::string s;
    s += l;
    s += "=[";
    s += decl_tag_name(decl_ref_tag(d));
    s += ":";
    s += std::to_string(decl_ref_idx(d));
    s += ",";
    if (strlen(decl_name(d)) > 0) {
        s += "(\"";
        s += decl_name(d);
        s += "\")";
    } else {
        s += "(anonymous)";
    }
    s += "]";
    return s;
}

void decl_db_dump(decl_db *db)
{
    printf("%-5s %-5s %-10s %-14s %-14s\n",
        "id", "next", "type", "name", "details");
    printf("%-5s %-5s %-10s %-14s %-14s\n",
        "-----", "-----", "----------", "--------------", "--------------");
    for (size_t i = 0; i < db->decl_offset; i++) {
        char buf[256];
        decl_ref r = decl_at(db, i);
        decl *d = decl_ptr(r);
        switch (decl_ref_tag(r)) {
        case _decl_typedef:
            snprintf(buf, sizeof(buf), "%s",
                _pretty_name("decl", db, d->_decl_typedef._decl).c_str());
            break;
        case _decl_intrinsic:
            snprintf(buf, sizeof(buf), "width=%llu",
                d->_decl_intrinsic._width);
            break;
        case _decl_set:
            snprintf(buf, sizeof(buf), "%s",
                _pretty_name("link", db, d->_decl_set._link).c_str());
            break;
        case _decl_enum:
            snprintf(buf, sizeof(buf), "%s",
                _pretty_name("link", db, d->_decl_enum._link).c_str());
            break;
        case _decl_struct:
            snprintf(buf, sizeof(buf), "%s",
                _pretty_name("link", db, d->_decl_struct._link).c_str());
            break;
        case _decl_union:
            snprintf(buf, sizeof(buf), "%s",
                _pretty_name("link", db, d->_decl_union._link).c_str());
            break;
        case _decl_field:
            if ((decl_ref_attrs(r) & _bitfield) > 0) {
                snprintf(buf, sizeof(buf), "%s width=%llu",
                    _pretty_name("decl", db, d->_decl_field._decl).c_str(),
                    d->_decl_field._width);
            } else {
                snprintf(buf, sizeof(buf), "%s",
                    _pretty_name("decl", db, d->_decl_field._decl).c_str());
            }
            break;
        case _decl_array:
            snprintf(buf, sizeof(buf), "%s size=%llu",
                _pretty_name("decl", db, d->_decl_array._decl).c_str(),
                d->_decl_array._size);
            break;
        case _decl_constant:
            snprintf(buf, sizeof(buf), "%s value=%lld",
                _pretty_name("decl", db, d->_decl_constant._decl).c_str(),
                d->_decl_constant._value);
            break;
        case _decl_variable:
            snprintf(buf, sizeof(buf), "%s addr=0x%llx",
                _pretty_name("decl", db, d->_decl_variable._decl).c_str(),
                d->_decl_variable._addr);
            break;
        case _decl_uniform:
            snprintf(buf, sizeof(buf), "%s addr=0x%llx",
                _pretty_name("decl", db, d->_decl_uniform._decl).c_str(),
                d->_decl_uniform._addr);
            break;
        case _decl_function:
            snprintf(buf, sizeof(buf), "%s addr=0x%llx",
                _pretty_name("link", db, d->_decl_function._link).c_str(),
                d->_decl_function._addr);
            break;
        case _decl_param:
            snprintf(buf, sizeof(buf), "%s",
                _pretty_name("decl", db, d->_decl_param._decl).c_str());
            break;
        default: buf[0] = '\0'; break;
        }
        printf("%-5zu %-5d %-10s %-14s %-14s\n", i, d->_next,
            decl_tag_name(decl_ref_tag(r)),
            strlen(decl_name(r)) > 0 ? decl_name(r) : "(anonymous)", buf);
    }
    printf("%-5s %-5s %-10s %-14s %-14s\n",
        "-----", "-----", "----------", "--------------", "--------------");
}

void decl_db_destroy(decl_db *db)
{
    free(db->data);
    free(db->decl);
    free(db);
}

decl_ref decl_new(decl_db *db, _Tag tag, _Set attrs)
{
    if (db->decl_offset >= db->decl_size) {
        db->decl_size <<= 1;
        db->decl = (decl*)realloc(db->decl, sizeof(decl) * db->decl_size);
    }
    decl_ref d = { db, db->decl_offset++ };
    decl_ptr(d)->_tag = tag;
    decl_ptr(d)->_attrs = attrs;
    return d;
}

const char* decl_name_new(decl_ref d, const char *name)
{
    size_t len = strlen(name) + 1;
    if (len == 1) return "";
    if (d.db->data_offset + len > d.db->data_size) {
        while (d.db->data_offset + len > d.db->data_size) {
            d.db->data_size <<= 1;
        }
        d.db->data = (char*)realloc(d.db->data, d.db->data_size);
    }
    size_t name_offset = d.db->data_offset;
    d.db->data_offset += len;
    d.db->decl[d.decl_idx]._name = name_offset;
    memcpy(d.db->data + name_offset, name, len);
    return d.db->data + name_offset;
}

const char* decl_name(decl_ref d)
{
    return decl_ptr(d)->_name ? d.db->data + decl_ptr(d)->_name : "";
}

decl_ref decl_find_intrinsic(decl_db *db, _Set attrs, size_t width)
{
    for (size_t i = 0; i < db->decl_offset; i++) {
        decl *d = db->decl + i;
        if (d->_tag == _decl_intrinsic &&
            d->_decl_intrinsic._width == width &&
                ((d->_attrs & attrs) == attrs)) {
            return decl_ref { db, i };
        }
    }
    return decl_ref { db, 0 };
}

static int _decl_top_level_fetch(decl_db *db, decl_ref *r, size_t *s,
    int(*decl_lambda)(decl*))
{
    size_t count = 0, limit = s ? *s : 0;
    for (size_t i = 0; i < db->decl_offset; i++) {
        decl *d = db->decl + i;
        if (decl_lambda(d) && decl_is_top(d)) {
            if (count < limit) {
                r[count] = decl_ref { db, i };
            }
            count++;
        }
    }
    if (s) *s = count;
    return 0;
}

int decl_types(decl_db *db, decl_ref *r, size_t *s)
{
    return _decl_top_level_fetch(db, r, s, decl_is_type);
}

int decl_constants(decl_db *db, decl_ref *r, size_t *s)
{
    return _decl_top_level_fetch(db, r, s, decl_is_constant);
}

int decl_variables(decl_db *db, decl_ref *r, size_t *s)
{
    return _decl_top_level_fetch(db, r, s, decl_is_variable);
}

int decl_uniforms(decl_db *db, decl_ref *r, size_t *s)
{
    return _decl_top_level_fetch(db, r, s, decl_is_uniform);
}

int decl_functions(decl_db *db, decl_ref *r, size_t *s)
{
    return _decl_top_level_fetch(db, r, s, decl_is_function);
}

size_t decl_type_width(decl_ref d)
{
    switch (decl_tag(decl_ptr(d))) {
    case _decl_intrinsic: return decl_intrinsic_width(d);
    case _decl_struct: return decl_struct_width(d);
    case _decl_union: return decl_union_width(d);
    }
    return 0;
}

size_t decl_intrinsic_width(decl_ref d)
{
    if (decl_tag(decl_ptr(d)) == _decl_intrinsic) {
        return decl_ptr(d)->_decl_intrinsic._width;
    }
    return 0;
}

size_t decl_struct_width(decl_ref d)
{
    /* todo */
    return 0;
}

size_t decl_union_width(decl_ref d)
{
    /* todo */
    return 0;
}

size_t decl_array_size(decl_ref d)
{
    if (decl_tag(decl_ptr(d)) == _decl_array) {
        return decl_ptr(d)->_decl_array._size;
    }
    return 0;
}

decl_ref decl_typedef_type(decl_ref d)
{
    if (decl_tag(decl_ptr(d)) == _decl_typedef) {
        return decl_ref { d.db, decl_ptr(d)->_decl_typedef._decl};
    }
    return decl_ref { d.db, 0 };
}

decl_ref decl_array_type(decl_ref d)
{
    if (decl_tag(decl_ptr(d)) == _decl_array) {
        return decl_ref { d.db, decl_ptr(d)->_decl_array._decl};
    }
    return decl_ref { d.db, 0 };
}

decl_ref decl_constant_type(decl_ref d)
{
    if (decl_tag(decl_ptr(d)) == _decl_constant) {
        return decl_ref { d.db, decl_ptr(d)->_decl_constant._decl};
    }
    return decl_ref { d.db, 0 };
}

decl_ref decl_variable_type(decl_ref d)
{
    if (decl_tag(decl_ptr(d)) == _decl_variable) {
        return decl_ref { d.db, decl_ptr(d)->_decl_variable._decl};
    }
    return decl_ref { d.db, 0 };
}

decl_ref decl_uniform_type(decl_ref d)
{
    if (decl_tag(decl_ptr(d)) == _decl_uniform) {
        return decl_ref { d.db, decl_ptr(d)->_decl_uniform._decl};
    }
    return decl_ref { d.db, 0 };
}

decl_ref decl_param_type(decl_ref d)
{
    if (decl_tag(decl_ptr(d)) == _decl_param) {
        return decl_ref { d.db, decl_ptr(d)->_decl_param._decl};
    }
    return decl_ref { d.db, 0 };
}

static int _decl_array_fetch(decl_ref d, decl_ref *r, size_t *s, size_t x)
{
    size_t count = 0, limit = s ? *s : 0;
    while (x && decl_is_type(d.db->decl + x)) {
        if (count < limit) {
            r[count] = decl_ref { d.db, x };
        }
        count++;
        x = d.db->decl[x]._next;
    }
    if (s) *s = count;
    return 0;
}

int decl_enum_constants(decl_ref d, decl_ref *r, size_t *s)
{
    if (!decl_is_enum(decl_ptr(d))) return -1;
    return _decl_array_fetch(d, r, s, decl_ptr(d)->_decl_enum._link);
}

int decl_set_constants(decl_ref d, decl_ref *r, size_t *s)
{
    if (!decl_is_set(decl_ptr(d))) return -1;
    return _decl_array_fetch(d, r, s, decl_ptr(d)->_decl_set._link);
}

int decl_struct_types(decl_ref d, decl_ref *r, size_t *s)
{
    if (!decl_is_struct(decl_ptr(d))) return -1;
    return _decl_array_fetch(d, r, s, decl_ptr(d)->_decl_struct._link);
}

int decl_union_types(decl_ref d, decl_ref *r, size_t *s)
{
    if (!decl_is_union(decl_ptr(d))) return -1;
    return _decl_array_fetch(d, r, s, decl_ptr(d)->_decl_union._link);
}

int decl_function_params(decl_ref d, decl_ref *r, size_t *s)
{
    if (!decl_is_function(decl_ptr(d))) return -1;
    return _decl_array_fetch(d, r, s, decl_ptr(d)->_decl_function._link);
}

_Size decl_constant_value(decl_ref d)
{
    if (!decl_is_constant(decl_ptr(d))) return 0;
    return decl_ptr(d)->_decl_constant._value;
}

void * decl_variable_addr(decl_ref d)
{
    if (!decl_is_variable(decl_ptr(d))) return nullptr;
    return (void*)decl_ptr(d)->_decl_variable._addr;
}

void * decl_uniform_addr(decl_ref d)
{
    if (!decl_is_uniform(decl_ptr(d))) return nullptr;
    return (void*)decl_ptr(d)->_decl_uniform._addr;
}

void * decl_function_addr(decl_ref d)
{
    if (!decl_is_function(decl_ptr(d))) return nullptr;
    return (void*)decl_ptr(d)->_decl_function._addr;
}
