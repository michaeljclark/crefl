#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>

#include "cmodel.h"
#include "ctypes.h"

#define array_size(arr) ((sizeof(arr)/sizeof(arr[0])))

/*
 * decl helpers
 */

decl_tag crefl_tag(decl *d) { return d->_tag; }
decl_set crefl_attrs(decl *d) { return d->_attrs; }

int crefl_is_top(decl *d)
{
    return (crefl_attrs(d) & _top) > 0;
}

int crefl_is_type(decl *d)
{
    decl_tag t = crefl_tag(d);
    return t == _decl_typedef || t == _decl_intrinsic ||
           t == _decl_set     || t == _decl_enum      ||
           t == _decl_struct  || t == _decl_union     ||
           t == _decl_field   || t == _decl_array;
}

int crefl_is_typedef(decl *d) { return crefl_tag(d) == _decl_typedef; }
int crefl_is_intrinsic(decl *d) { return crefl_tag(d) == _decl_intrinsic; }
int crefl_is_set(decl *d) { return crefl_tag(d) == _decl_set; }
int crefl_is_enum(decl *d) { return crefl_tag(d) == _decl_enum; }
int crefl_is_struct(decl *d) { return crefl_tag(d) == _decl_struct; }
int crefl_is_union(decl *d) { return crefl_tag(d) == _decl_union; }
int crefl_is_field(decl *d) { return crefl_tag(d) == _decl_field; }
int crefl_is_array(decl *d) { return crefl_tag(d) == _decl_array; }
int crefl_is_constant(decl *d) { return crefl_tag(d) == _decl_constant; }
int crefl_is_variable(decl *d) { return crefl_tag(d) == _decl_variable; }
int crefl_is_uniform(decl *d) { return crefl_tag(d) == _decl_uniform; }
int crefl_is_function(decl *d) { return crefl_tag(d) == _decl_function; }
int crefl_is_param(decl *d) { return crefl_tag(d) == _decl_param; }

/*
 * decl relflection
 */

static const char * crefl_tag_names_arr[] = {
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

const char * crefl_tag_name(decl_tag tag)
{
    if (tag < array_size(crefl_tag_names_arr)) {
        return crefl_tag_names_arr[tag];
    } else {
        return "<unknown>";
    }
}

decl_ref crefl_at(decl_db *db, size_t decl_idx)
{
    return decl_ref { db, decl_idx };
}

decl * crefl_ptr(decl_ref d)
{
    return d.db->decl + d.decl_idx;
}

decl_id crefl_ref_idx(decl_ref d) { return d.decl_idx; }

decl_tag crefl_ref_tag(decl_ref d) { return (d.db->decl + d.decl_idx)->_tag; }
decl_set crefl_ref_attrs(decl_ref d) { return (d.db->decl + d.decl_idx)->_attrs; }

decl_db * crefl_db_new()
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

void crefl_db_defaults(decl_db *db)
{
    const _ctype **d = all_types;
    while (*d != 0) {
        if ((*d)->_tag == _decl_intrinsic) {
            decl_ref r = crefl_new(db, _decl_intrinsic, _top | (*d)->_attrs);
            crefl_name_new(r, (*d)->_name);
            crefl_ptr(r)->_decl_intrinsic._width = (*d)->_width;
        }
        d++;
    }
}

void crefl_db_destroy(decl_db *db)
{
    free(db->data);
    free(db->decl);
    free(db);
}

decl_ref crefl_new(decl_db *db, decl_tag tag, decl_set attrs)
{
    if (db->decl_offset >= db->decl_size) {
        db->decl_size <<= 1;
        db->decl = (decl*)realloc(db->decl, sizeof(decl) * db->decl_size);
    }
    decl_ref d = { db, db->decl_offset++ };
    crefl_ptr(d)->_tag = tag;
    crefl_ptr(d)->_attrs = attrs;
    return d;
}

const char* crefl_name_new(decl_ref d, const char *name)
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

const char* crefl_name(decl_ref d)
{
    return crefl_ptr(d)->_name ? d.db->data + crefl_ptr(d)->_name : "";
}

decl_ref crefl_find_intrinsic(decl_db *db, decl_set attrs, size_t width)
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
        if (decl_lambda(d) && crefl_is_top(d)) {
            if (count < limit) {
                r[count] = decl_ref { db, i };
            }
            count++;
        }
    }
    if (s) *s = count;
    return 0;
}

int crefl_types(decl_db *db, decl_ref *r, size_t *s)
{
    return _decl_top_level_fetch(db, r, s, crefl_is_type);
}

int crefl_constants(decl_db *db, decl_ref *r, size_t *s)
{
    return _decl_top_level_fetch(db, r, s, crefl_is_constant);
}

int crefl_variables(decl_db *db, decl_ref *r, size_t *s)
{
    return _decl_top_level_fetch(db, r, s, crefl_is_variable);
}

int crefl_uniforms(decl_db *db, decl_ref *r, size_t *s)
{
    return _decl_top_level_fetch(db, r, s, crefl_is_uniform);
}

int crefl_functions(decl_db *db, decl_ref *r, size_t *s)
{
    return _decl_top_level_fetch(db, r, s, crefl_is_function);
}

size_t crefl_type_width(decl_ref d)
{
    switch (crefl_tag(crefl_ptr(d))) {
    case _decl_intrinsic: return crefl_intrinsic_width(d);
    case _decl_struct: return crefl_struct_width(d);
    case _decl_union: return crefl_union_width(d);
    }
    return 0;
}

size_t crefl_intrinsic_width(decl_ref d)
{
    if (crefl_tag(crefl_ptr(d)) == _decl_intrinsic) {
        return crefl_ptr(d)->_decl_intrinsic._width;
    }
    return 0;
}

size_t crefl_struct_width(decl_ref d)
{
    /* todo */
    return 0;
}

size_t crefl_union_width(decl_ref d)
{
    /* todo */
    return 0;
}

size_t crefl_array_size(decl_ref d)
{
    if (crefl_tag(crefl_ptr(d)) == _decl_array) {
        return crefl_ptr(d)->_decl_array._size;
    }
    return 0;
}

decl_ref crefl_typedef_type(decl_ref d)
{
    if (crefl_tag(crefl_ptr(d)) == _decl_typedef) {
        return decl_ref { d.db, crefl_ptr(d)->_decl_typedef._decl};
    }
    return decl_ref { d.db, 0 };
}

decl_ref crefl_array_type(decl_ref d)
{
    if (crefl_tag(crefl_ptr(d)) == _decl_array) {
        return decl_ref { d.db, crefl_ptr(d)->_decl_array._decl};
    }
    return decl_ref { d.db, 0 };
}

decl_ref crefl_constant_type(decl_ref d)
{
    if (crefl_tag(crefl_ptr(d)) == _decl_constant) {
        return decl_ref { d.db, crefl_ptr(d)->_decl_constant._decl};
    }
    return decl_ref { d.db, 0 };
}

decl_ref crefl_variable_type(decl_ref d)
{
    if (crefl_tag(crefl_ptr(d)) == _decl_variable) {
        return decl_ref { d.db, crefl_ptr(d)->_decl_variable._decl};
    }
    return decl_ref { d.db, 0 };
}

decl_ref crefl_uniform_type(decl_ref d)
{
    if (crefl_tag(crefl_ptr(d)) == _decl_uniform) {
        return decl_ref { d.db, crefl_ptr(d)->_decl_uniform._decl};
    }
    return decl_ref { d.db, 0 };
}

decl_ref crefl_param_type(decl_ref d)
{
    if (crefl_tag(crefl_ptr(d)) == _decl_param) {
        return decl_ref { d.db, crefl_ptr(d)->_decl_param._decl};
    }
    return decl_ref { d.db, 0 };
}

static int _decl_array_fetch(decl_ref d, decl_ref *r, size_t *s, size_t x)
{
    size_t count = 0, limit = s ? *s : 0;
    while (x && crefl_is_type(d.db->decl + x)) {
        if (count < limit) {
            r[count] = decl_ref { d.db, x };
        }
        count++;
        x = d.db->decl[x]._next;
    }
    if (s) *s = count;
    return 0;
}

int crefl_enum_constants(decl_ref d, decl_ref *r, size_t *s)
{
    if (!crefl_is_enum(crefl_ptr(d))) return -1;
    return _decl_array_fetch(d, r, s, crefl_ptr(d)->_decl_enum._link);
}

int crefl_set_constants(decl_ref d, decl_ref *r, size_t *s)
{
    if (!crefl_is_set(crefl_ptr(d))) return -1;
    return _decl_array_fetch(d, r, s, crefl_ptr(d)->_decl_set._link);
}

int crefl_struct_types(decl_ref d, decl_ref *r, size_t *s)
{
    if (!crefl_is_struct(crefl_ptr(d))) return -1;
    return _decl_array_fetch(d, r, s, crefl_ptr(d)->_decl_struct._link);
}

int crefl_union_types(decl_ref d, decl_ref *r, size_t *s)
{
    if (!crefl_is_union(crefl_ptr(d))) return -1;
    return _decl_array_fetch(d, r, s, crefl_ptr(d)->_decl_union._link);
}

int crefl_function_params(decl_ref d, decl_ref *r, size_t *s)
{
    if (!crefl_is_function(crefl_ptr(d))) return -1;
    return _decl_array_fetch(d, r, s, crefl_ptr(d)->_decl_function._link);
}

decl_sz crefl_constant_value(decl_ref d)
{
    if (!crefl_is_constant(crefl_ptr(d))) return 0;
    return crefl_ptr(d)->_decl_constant._value;
}

void * crefl_variable_addr(decl_ref d)
{
    if (!crefl_is_variable(crefl_ptr(d))) return nullptr;
    return (void*)crefl_ptr(d)->_decl_variable._addr;
}

void * crefl_uniform_addr(decl_ref d)
{
    if (!crefl_is_uniform(crefl_ptr(d))) return nullptr;
    return (void*)crefl_ptr(d)->_decl_uniform._addr;
}

void * crefl_function_addr(decl_ref d)
{
    if (!crefl_is_function(crefl_ptr(d))) return nullptr;
    return (void*)crefl_ptr(d)->_decl_function._addr;
}
