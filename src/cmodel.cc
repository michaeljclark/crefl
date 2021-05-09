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
#include <cstdlib>
#include <cstring>

#include <string>

#include "stdbits.h"

#include "cmodel.h"
#include "ctypes.h"

#define array_size(arr) ((sizeof(arr)/sizeof(arr[0])))

/*
 * decl helpers
 */

int crefl_is_any(decl_ref d) { return 1; }

int crefl_is_type(decl_ref d)
{
    decl_tag t = crefl_decl_tag(d);
    return t == _decl_typedef || t == _decl_intrinsic ||
           t == _decl_set     || t == _decl_enum      ||
           t == _decl_struct  || t == _decl_union     ||
           t == _decl_array   || t == _decl_pointer;
}

int crefl_is_none      (decl_ref d) { return crefl_decl_tag(d) == _decl_none;      }
int crefl_is_typedef   (decl_ref d) { return crefl_decl_tag(d) == _decl_typedef;   }
int crefl_is_intrinsic (decl_ref d) { return crefl_decl_tag(d) == _decl_intrinsic; }
int crefl_is_set       (decl_ref d) { return crefl_decl_tag(d) == _decl_set;       }
int crefl_is_enum      (decl_ref d) { return crefl_decl_tag(d) == _decl_enum;      }
int crefl_is_struct    (decl_ref d) { return crefl_decl_tag(d) == _decl_struct;    }
int crefl_is_union     (decl_ref d) { return crefl_decl_tag(d) == _decl_union;     }
int crefl_is_field     (decl_ref d) { return crefl_decl_tag(d) == _decl_field;     }
int crefl_is_array     (decl_ref d) { return crefl_decl_tag(d) == _decl_array;     }
int crefl_is_pointer   (decl_ref d) { return crefl_decl_tag(d) == _decl_pointer;   }
int crefl_is_constant  (decl_ref d) { return crefl_decl_tag(d) == _decl_constant;  }
int crefl_is_function  (decl_ref d) { return crefl_decl_tag(d) == _decl_function;  }
int crefl_is_param     (decl_ref d) { return crefl_decl_tag(d) == _decl_param;     }
int crefl_is_attribute (decl_ref d) { return crefl_decl_tag(d) == _decl_attribute; }
int crefl_is_value     (decl_ref d) { return crefl_decl_tag(d) == _decl_value;     }
int crefl_is_archive   (decl_ref d) { return crefl_decl_tag(d) == _decl_archive;   }
int crefl_is_source    (decl_ref d) { return crefl_decl_tag(d) == _decl_source;    }
int crefl_is_alias     (decl_ref d) { return crefl_decl_tag(d) == _decl_alias;     }

/*
 * decl accessors
 */

decl_ref crefl_decl_void(decl_ref d) { return decl_ref { d.db, 0 }; }
decl_node * crefl_decl_ptr(decl_ref d) { return d.db->decl + d.decl_idx; }
decl_tag crefl_decl_tag(decl_ref d) { return (d.db->decl + d.decl_idx)->_tag; }
decl_set crefl_decl_props(decl_ref d) { return (d.db->decl + d.decl_idx)->_props; }
decl_id crefl_decl_idx(decl_ref d) { return d.decl_idx; }
decl_ref crefl_decl_next(decl_ref d) { return decl_ref { d.db, (d.db->decl + d.decl_idx)->_next }; }
decl_ref crefl_decl_link(decl_ref d) { return decl_ref { d.db, (d.db->decl + d.decl_idx)->_link }; }
decl_ref crefl_decl_attr(decl_ref d) { return decl_ref { d.db, (d.db->decl + d.decl_idx)->_attr }; }
decl_sz crefl_decl_qty(decl_ref d) { return (d.db->decl + d.decl_idx)->_quantity; }
decl_ref crefl_lookup(decl_db *db, size_t decl_idx) { return decl_ref { db, decl_idx }; }

/*
 * decl relflection
 */

static const char * crefl_tag_names_arr[] = {
    "none",
    "intrinsic",
    "typedef",
    "set",
    "enum",
    "struct",
    "union",
    "field",
    "array",
    "pointer",
    "constant",
    "function",
    "param",
    "attribute",
    "value",
    "archive",
    "source",
    "alias",
};

const char * crefl_tag_name(decl_tag tag)
{
    if (tag < array_size(crefl_tag_names_arr)) {
        return crefl_tag_names_arr[tag];
    } else {
        return "<unknown>";
    }
}

decl_db * crefl_db_new()
{
    decl_db *db = (decl_db*)malloc(sizeof(decl_db));

    db->name_offset = 1; /* offset 0 holds empty string */
    db->name_builtin = 1;
    db->name_size = 32;
    db->name = (char*)malloc(db->name_size);
    memset(db->name, 0, db->name_size);

    db->decl_offset = 1; /* offset 0 slot is empty */
    db->decl_builtin = 1;
    db->decl_size = 32;
    db->decl = (decl_node*)malloc(sizeof(decl_node) * db->decl_size);
    memset(db->decl, 0, sizeof(decl_node) * db->decl_size);

    db->root_element = 0;

    return db;
}

void crefl_db_defaults(decl_db *db)
{
    const _ctype **d = all_types;
    while (*d != 0) {
        if ((*d)->_tag == _decl_intrinsic) {
            decl_ref r = crefl_decl_new(db, _decl_intrinsic);
            crefl_decl_ptr(r)->_name = crefl_name_new(db, (*d)->_name);
            crefl_decl_ptr(r)->_props = (*d)->_props;
            crefl_decl_ptr(r)->_width = (*d)->_width;
        }
        d++;
    }
    /* save builtin offsets */
    db->name_builtin = db->name_offset;
    db->decl_builtin = db->decl_offset;
}

void crefl_db_destroy(decl_db *db)
{
    free(db->name);
    free(db->decl);
    free(db);
}

decl_ref crefl_decl_new(decl_db *db, decl_tag tag)
{
    if (db->decl_offset >= db->decl_size) {
        db->decl_size <<= 1;
        db->decl = (decl_node*)realloc(db->decl, sizeof(decl_node) * db->decl_size);
    }
    decl_ref d = { db, db->decl_offset++ };
    memset(crefl_decl_ptr(d), 0, sizeof(decl_node));
    crefl_decl_ptr(d)->_tag = tag;
    return d;
}

decl_id crefl_name_new(decl_db *db, const char *name)
{
    size_t len = strlen(name) + 1;
    if (len == 1) return 0;
    if (db->name_offset + len > db->name_size) {
        while (db->name_offset + len > db->name_size) {
            db->name_size <<= 1;
        }
        db->name = (char*)realloc(db->name, db->name_size);
    }
    size_t name_offset = db->name_offset;
    db->name_offset += len;
    memcpy(db->name + name_offset, name, len);
    return name_offset;
}

const char* crefl_decl_name(decl_ref d)
{
    return d.db->name + crefl_decl_ptr(d)->_name;
}

int crefl_decl_has_name(decl_ref d)
{
    return crefl_decl_ptr(d)->_name != 0;
}

decl_ref crefl_root(decl_db *db)
{
    return decl_ref { db, db->root_element };
}

decl_ref crefl_intrinsic(decl_db *db, decl_set props, size_t width)
{
    for (size_t i = 0; i < db->decl_offset; i++) {
        decl_ref d = crefl_lookup(db, i);
        if (crefl_decl_tag(d) == _decl_intrinsic &&
            crefl_decl_qty(d) == width &&
                ((crefl_decl_props(d) & props) == props)) {
            return decl_ref { db, i };
        }
    }
    return decl_ref { db, 0 };
}

static int _decl_array_fetch(decl_db *db, decl_ref *r, size_t *s, decl_ref d,
    int(*decl_lambda)(decl_ref))
{
    size_t count = 0, limit = s ? *s : 0;
    while (crefl_decl_idx(d))  {
        if (decl_lambda(d)) {
            if (r && count < limit) {
                r[count] = d;
            }
            count++;
        }
        d = crefl_decl_next(d);
    }
    if (s) *s = count;
    return 0;
}

size_t crefl_type_width(decl_ref d)
{
    switch (crefl_decl_tag(d)) {
    case _decl_intrinsic: return crefl_intrinsic_width(d);
    case _decl_struct: return crefl_struct_width(d);
    case _decl_union: return crefl_union_width(d);
    case _decl_field: return crefl_type_width(crefl_decl_link(d));
    case _decl_array: return crefl_type_width(crefl_array_type(d))
        * crefl_array_count(d);
    case _decl_pointer: return crefl_pointer_width(d);
    }
    return 0;
}

size_t crefl_intrinsic_width(decl_ref d)
{
    if (crefl_decl_tag(d) == _decl_intrinsic) {
        return crefl_decl_qty(d);
    }
    return 0;
}

static inline size_t _pad_align(intptr_t offset, intptr_t width, size_t count, decl_set props)
{
    const intptr_t maxalign = 9; /* 128 bits */

    intptr_t n = 63 - clz(width), addend;

    if (n > maxalign) n = maxalign;

    if ((props & _decl_pad_byte)) {
        offset = (offset + 7) & ~7;
        addend = (width + 7) & ~7;
    }
    else if ((props & _decl_pad_pow2)) {
        offset  = (offset + ((1<<n)-1)) & ~((1<<n)-1);
        addend = (width + ((1<<n)-1)) & ~((1<<n)-1);
    }
    else {
        addend = width;
    }

    return offset + addend * count;
}

size_t crefl_struct_width(decl_ref d)
{
    size_t offset = 0;

    if (crefl_decl_tag(d) != _decl_struct) return 0;

    decl_ref dx = crefl_lookup(d.db, crefl_decl_ptr(d)->_link);
    while (crefl_decl_idx(dx)) {
        switch (crefl_decl_tag(dx)) {
        decl_ref t;
        case _decl_field:
            t = crefl_field_type(dx);
            switch (crefl_decl_tag(t)) {
            case _decl_array:
                offset = _pad_align(offset, crefl_type_width(crefl_array_type(t)),
                    crefl_decl_qty(t), crefl_decl_props(crefl_array_type(t)));
                break;
            case _decl_pointer:
                offset = _pad_align(offset, crefl_type_width(t), 1, _decl_pad_pow2);
                break;
            case _decl_struct:
                offset = _pad_align(offset, crefl_type_width(t), 1, _decl_pad_pow2);
                break;
            case _decl_union:
            case _decl_intrinsic:
                offset = _pad_align(offset, crefl_type_width(t), 1, crefl_decl_props(t));
                break;
            }
            break;
        case _decl_intrinsic:
        case _decl_struct:
        case _decl_union:
        default:
            /* type definitions don't add any width to a struct or union */
            break;
        }
        dx = crefl_decl_next(dx);
    }

    return offset;
}

size_t crefl_union_width(decl_ref d)
{
    decl_ref t;
    size_t offset = 0, width;

    if (crefl_decl_tag(d) != _decl_union) return 0;

    decl_ref dx = crefl_lookup(d.db, crefl_decl_ptr(d)->_link);
    while (crefl_decl_idx(dx)) {
        switch (crefl_decl_tag(dx)) {
        case _decl_field:
            t = crefl_field_type(dx);
            switch (crefl_decl_tag(t)) {
            case _decl_array:
                width = _pad_align(0, crefl_type_width(crefl_array_type(t)),
                    crefl_decl_qty(t), crefl_decl_props(crefl_array_type(t)));
                break;
            case _decl_pointer:
                width = _pad_align(0, crefl_type_width(t), 1, _decl_pad_pow2);
                break;
            case _decl_struct:
                width = _pad_align(0, crefl_type_width(t), 1, _decl_pad_pow2);
                break;
            case _decl_union:
            case _decl_intrinsic:
                width = _pad_align(0, crefl_type_width(t), 1, crefl_decl_props(t));
                break;
            }
            if (width > offset) offset = width;
            break;
        case _decl_intrinsic:
        case _decl_struct:
        case _decl_union:
        default:
            /* type definitions don't add any width to a struct or union */
            break;
        }

        dx = crefl_decl_next(dx);
    }

    return offset;
}

size_t crefl_array_count(decl_ref d)
{
    return crefl_is_array(d) ? crefl_decl_qty(d) : 0;
}

size_t crefl_pointer_width(decl_ref d)
{
    return crefl_is_pointer(d) ? crefl_decl_qty(d) : 0;
}

decl_ref crefl_typedef_type(decl_ref d)
{
    return crefl_is_typedef(d) ? crefl_decl_link(d) : crefl_decl_void(d);
}

decl_ref crefl_field_type(decl_ref d)
{
    return crefl_is_field(d) ? crefl_decl_link(d) : crefl_decl_void(d);
}

decl_ref crefl_array_type(decl_ref d)
{
    return crefl_is_array(d) ? crefl_decl_link(d) : crefl_decl_void(d);
}

decl_ref crefl_pointer_type(decl_ref d)
{
    return crefl_is_pointer(d) ? crefl_decl_link(d) : crefl_decl_void(d);
}

decl_ref crefl_constant_type(decl_ref d)
{
    return crefl_is_constant(d) ? crefl_decl_link(d) : crefl_decl_void(d);
}

decl_ref crefl_param_type(decl_ref d)
{
    return crefl_is_param(d) ? crefl_decl_link(d) : crefl_decl_void(d);
}

int crefl_enum_constants(decl_ref d, decl_ref *r, size_t *s)
{
    if (!crefl_is_enum(d)) return -1;
    return _decl_array_fetch(d.db, r, s, crefl_decl_link(d), crefl_is_constant);
}

int crefl_set_constants(decl_ref d, decl_ref *r, size_t *s)
{
    if (!crefl_is_set(d)) return -1;
    return _decl_array_fetch(d.db, r, s, crefl_decl_link(d), crefl_is_constant);
}

int crefl_struct_fields(decl_ref d, decl_ref *r, size_t *s)
{
    if (!crefl_is_struct(d)) return -1;
    return _decl_array_fetch(d.db, r, s, crefl_decl_link(d), crefl_is_field);
}

int crefl_union_fields(decl_ref d, decl_ref *r, size_t *s)
{
    if (!crefl_is_union(d)) return -1;
    return _decl_array_fetch(d.db, r, s, crefl_decl_link(d), crefl_is_field);
}

int crefl_function_params(decl_ref d, decl_ref *r, size_t *s)
{
    if (!crefl_is_function(d)) return -1;
    return _decl_array_fetch(d.db, r, s, crefl_decl_link(d), crefl_is_param);
}

int crefl_source_decls(decl_ref d, decl_ref *r, size_t *s)
{
    if (!crefl_is_source(d)) return -1;
    return _decl_array_fetch(d.db, r, s, crefl_decl_link(d), crefl_is_any);
}

int crefl_source_types(decl_ref d, decl_ref *r, size_t *s)
{
    if (!crefl_is_source(d)) return -1;
    return _decl_array_fetch(d.db, r, s, crefl_decl_link(d), crefl_is_type);
}

int crefl_source_fields(decl_ref d, decl_ref *r, size_t *s)
{
    if (!crefl_is_source(d)) return -1;
    return _decl_array_fetch(d.db, r, s, crefl_decl_link(d), crefl_is_field);
}

int crefl_source_functions(decl_ref d, decl_ref *r, size_t *s)
{
    if (!crefl_is_source(d)) return -1;
    return _decl_array_fetch(d.db, r, s, crefl_decl_link(d), crefl_is_function);
}

int crefl_archive_sources(decl_ref d, decl_ref *r, size_t *s)
{
    if (!crefl_is_archive(d)) return -1;
    return _decl_array_fetch(d.db, r, s, crefl_decl_link(d), crefl_is_source);
}

decl_raw crefl_constant_value(decl_ref d)
{
    if (!crefl_is_constant(d)) return decl_raw { 0 };
    return decl_raw { crefl_decl_ptr(d)->_value };
}

void * crefl_function_addr(decl_ref d)
{
    if (!crefl_is_function(d)) return nullptr;
    return (void*)crefl_decl_ptr(d)->_addr;
}
