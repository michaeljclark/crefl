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
#include <cstdlib>
#include <cstdint>
#include <cstring>

#include <string>

#include <cinf/bits.h>
#include <cinf/model.h>
#include <cinf/types.h>

#define array_size(arr) ((sizeof(arr)/sizeof(arr[0])))

/*
 * decl helpers
 */

int cinf_is_any(decl_ref d) { return 1; }

int cinf_is_type(decl_ref d)
{
    decl_tag t = cinf_decl_tag(d);
    return t == decl_typedef || t == decl_intrinsic ||
           t == decl_enum    || t == decl_struct    ||
           t == decl_union   || t == decl_array     ||
           t == decl_pointer || t == decl_qualifier;
}

int cinf_is_none      (decl_ref d) { return cinf_decl_tag(d) == decl_none;      }
int cinf_is_typedef   (decl_ref d) { return cinf_decl_tag(d) == decl_typedef;   }
int cinf_is_intrinsic (decl_ref d) { return cinf_decl_tag(d) == decl_intrinsic; }
int cinf_is_enum      (decl_ref d) { return cinf_decl_tag(d) == decl_enum;      }
int cinf_is_struct    (decl_ref d) { return cinf_decl_tag(d) == decl_struct;    }
int cinf_is_union     (decl_ref d) { return cinf_decl_tag(d) == decl_union;     }
int cinf_is_field     (decl_ref d) { return cinf_decl_tag(d) == decl_field;     }
int cinf_is_array     (decl_ref d) { return cinf_decl_tag(d) == decl_array;     }
int cinf_is_pointer   (decl_ref d) { return cinf_decl_tag(d) == decl_pointer;   }
int cinf_is_constant  (decl_ref d) { return cinf_decl_tag(d) == decl_constant;  }
int cinf_is_function  (decl_ref d) { return cinf_decl_tag(d) == decl_function;  }
int cinf_is_parameter (decl_ref d) { return cinf_decl_tag(d) == decl_parameter; }
int cinf_is_qualifier (decl_ref d) { return cinf_decl_tag(d) == decl_qualifier; }
int cinf_is_attribute (decl_ref d) { return cinf_decl_tag(d) == decl_attribute; }
int cinf_is_value     (decl_ref d) { return cinf_decl_tag(d) == decl_value;     }
int cinf_is_archive   (decl_ref d) { return cinf_decl_tag(d) == decl_archive;   }
int cinf_is_source    (decl_ref d) { return cinf_decl_tag(d) == decl_source;    }
int cinf_is_alias     (decl_ref d) { return cinf_decl_tag(d) == decl_alias;     }

/*
 * decl accessors
 */

decl_ref cinf_decl_void(decl_ref d) { return decl_ref { d.db, 0 }; }
decl_node * cinf_decl_ptr(decl_ref d) { return d.db->decl + d.decl_idx; }
decl_tag cinf_decl_tag(decl_ref d) { return (d.db->decl + d.decl_idx)->tag; }
decl_set cinf_decl_props(decl_ref d) { return (d.db->decl + d.decl_idx)->props; }
decl_id cinf_decl_idx(decl_ref d) { return d.decl_idx; }
decl_ref cinf_decl_next(decl_ref d) { return decl_ref { d.db, (d.db->decl + d.decl_idx)->next }; }
decl_ref cinf_decl_link(decl_ref d) { return decl_ref { d.db, (d.db->decl + d.decl_idx)->link }; }
decl_ref cinf_decl_attr(decl_ref d) { return decl_ref { d.db, (d.db->decl + d.decl_idx)->attr }; }
decl_sz cinf_decl_qty(decl_ref d) { return (d.db->decl + d.decl_idx)->quantity; }
decl_ref cinf_lookup(decl_db *db, size_t decl_idx) { return decl_ref { db, decl_idx }; }

/*
 * decl relflection
 */

static const char * cinf_tag_names_arr[] = {
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
    "parameter",
    "qualifier",
    "attribute",
    "value",
    "archive",
    "source",
    "alias",
};

const char * cinf_tag_name(decl_tag tag)
{
    if (tag < array_size(cinf_tag_names_arr)) {
        return cinf_tag_names_arr[tag];
    } else {
        return "<unknown>";
    }
}

decl_db * cinf_db_new()
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

void cinf_db_defaults(decl_db *db)
{
    const decl_type **d = all_types;
    while (*d != 0) {
        if ((*d)->tag == decl_intrinsic) {
            decl_ref r = cinf_decl_new(db, decl_intrinsic);
            cinf_decl_ptr(r)->name = cinf_name_new(db, (*d)->name);
            cinf_decl_ptr(r)->props = (*d)->props;
            cinf_decl_ptr(r)->width = (*d)->width;
        }
        d++;
    }
    /* save builtin offsets */
    db->name_builtin = db->name_offset;
    db->decl_builtin = db->decl_offset;
}

void cinf_db_destroy(decl_db *db)
{
    free(db->name);
    free(db->decl);
    free(db);
}

decl_ref cinf_decl_new(decl_db *db, decl_tag tag)
{
    if (db->decl_offset >= db->decl_size) {
        db->decl_size <<= 1;
        db->decl = (decl_node*)realloc(db->decl, sizeof(decl_node) * db->decl_size);
    }
    decl_ref d = { db, db->decl_offset++ };
    memset(cinf_decl_ptr(d), 0, sizeof(decl_node));
    cinf_decl_ptr(d)->tag = tag;
    return d;
}

decl_id cinf_name_new(decl_db *db, const char *name)
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

const char* cinf_decl_name(decl_ref d)
{
    return d.db->name + cinf_decl_ptr(d)->name;
}

int cinf_decl_has_name(decl_ref d)
{
    return cinf_decl_ptr(d)->name != 0;
}

decl_ref cinf_root(decl_db *db)
{
    return decl_ref { db, db->root_element };
}

decl_ref cinf_intrinsic(decl_db *db, decl_set props, size_t width)
{
    for (size_t i = 0; i < db->decl_offset; i++) {
        decl_ref d = cinf_lookup(db, i);
        if (cinf_is_intrinsic(d) &&
            cinf_decl_qty(d) == width &&
                ((cinf_decl_props(d) & props) == props)) {
            return decl_ref { db, i };
        }
    }
    return decl_ref { db, 0 };
}

static int decl_array_fetch(decl_db *db, decl_ref *r, size_t *s, decl_ref d,
    int(*decl_lambda)(decl_ref))
{
    size_t count = 0, limit = s ? *s : 0;
    while (cinf_decl_idx(d))  {
        if (decl_lambda(d)) {
            if (r && count < limit) {
                r[count] = d;
            }
            count++;
        }
        d = cinf_decl_next(d);
    }
    if (s) *s = count;
    return 0;
}

/*
 * structure alignment rules
 *
 * - handles nearest power of two alignment for 1,2,4,8,16 bytes
 * - handles trailing padding based on largest alignment
 * - [not yet supported] packing and alignment attributes
 */

static size_t cinf_align(size_t offset, size_t n)
{
    return (offset + ((1llu<<n)-1llu)) & ~((1llu<<n)-1llu);
}

struct cinf_alignment { size_t align; size_t size; };

static cinf_alignment cinf_pad_align(size_t width, size_t count, decl_set props)
{
    const intptr_t maxalign = 9; /* 128 bits */

    size_t n;

    if ((props & decl_pad_byte)) {
        n = 8;
    }
    else if ((props & decl_pad_pow2)) {
        n = 63 - clz(width);
        if (n > maxalign) n = maxalign;
    } else {
        n = 0;
    }

    return { n, cinf_align(width, n) * count };
}

static cinf_alignment _type_pad(decl_ref d);

static cinf_alignment _field_pad(decl_ref d)
{
    return cinf_is_field(d) ?
        _type_pad(cinf_decl_link(d)) : cinf_alignment { 0 };
}

static cinf_alignment _intrinsic_pad(decl_ref d)
{
    return cinf_is_intrinsic(d) ?
        cinf_pad_align(cinf_decl_qty(d), 1, cinf_decl_props(d)) : cinf_alignment { 0 };
}

static cinf_alignment _pointer_pad(decl_ref d)
{
    return cinf_is_pointer(d) ?
        cinf_pad_align(cinf_decl_qty(d), 1, decl_pad_pow2) : cinf_alignment { 0 };
}

static cinf_alignment _array_pad(decl_ref d)
{
    size_t qty = 1;
    cinf_alignment pad;

    if (!cinf_is_array(d)) return cinf_alignment { 0 };

    do  {
        qty *= cinf_array_count(d);
        d = cinf_array_type(d);
    } while (cinf_is_array(d));

    pad = _type_pad(d);
    pad.size *= qty;

    return pad;
}

static cinf_alignment _struct_pad(decl_ref d)
{
    cinf_alignment max = { 0 };
    size_t offset = 0;

    if (!cinf_is_struct(d)) return cinf_alignment { 0 };

    d = cinf_decl_link(d);
    while (cinf_decl_idx(d)) {
        if (cinf_is_field(d)) {
            cinf_alignment pad = _type_pad(cinf_field_type(d));
            if (pad.align > max.align) max.align = pad.align;
            if (pad.size > max.size) max.size = pad.size;
            offset = cinf_align(offset, pad.align) + pad.size;
        }
        d = cinf_decl_next(d);
    }

    return cinf_alignment { max.align, cinf_align(offset, max.align) };
}

static cinf_alignment _union_pad(decl_ref d)
{
    cinf_alignment max = { 0 };

    if (!cinf_is_union(d)) return cinf_alignment { 0 };

    d = cinf_decl_link(d);
    while (cinf_decl_idx(d)) {
        if (cinf_is_field(d)) {
            cinf_alignment pad = _type_pad(cinf_field_type(d));
            if (pad.align > max.align) max.align = pad.align;
            if (pad.size > max.size) max.size = pad.size;
        }
        d = cinf_decl_next(d);
    }

    return max;
}

static cinf_alignment _type_pad(decl_ref d)
{
    switch (cinf_decl_tag(d)) {
    case decl_intrinsic: return _intrinsic_pad(d);
    case decl_struct: return _struct_pad(d);
    case decl_union: return _union_pad(d);
    case decl_field: return _field_pad(d);
    case decl_array: return _array_pad(d);
    case decl_pointer: return _pointer_pad(d);
    }
    return cinf_alignment { 0 };
}

int cinf_struct_fields_offsets(decl_ref d, decl_ref *r, size_t *o, size_t *s)
{
    size_t count = 0, offset = 0, limit = s ? *s : 0;
    cinf_alignment max = { 0 };

    if (!cinf_is_struct(d)) return -1;

    d = cinf_decl_link(d);
    while (cinf_decl_idx(d))  {
        if (cinf_is_field(d)) {
            cinf_alignment pad = _type_pad(cinf_field_type(d));
            if (pad.align > max.align) max.align = pad.align;
            if (pad.size > max.size) max.size = pad.size;
            offset = cinf_align(offset, pad.align);
            if (count < limit) {
                if (r) r[count] = d;
                if (o) o[count] = offset;
            }
            offset += pad.size;
            ++count;
        }
        d = cinf_decl_next(d);
    }
    if (count < limit) {
        if (r) r[count] = cinf_decl_void(d);
        if (o) o[count] = cinf_align(offset, max.align);
    }
    if (count > 0) ++count;
    if (s) *s = count;

    return 0;
}

size_t cinf_type_align(decl_ref d) { return _type_pad(d).align; }
size_t cinf_field_align(decl_ref d) { return _field_pad(d).align; }
size_t cinf_intrinsic_align(decl_ref d) { return _intrinsic_pad(d).align; }
size_t cinf_pointer_align(decl_ref d) { return _pointer_pad(d).align; }
size_t cinf_array_align(decl_ref d) { return _array_pad(d).align; }
size_t cinf_struct_align(decl_ref d) { return _struct_pad(d).align; }
size_t cinf_union_align(decl_ref d) { return _union_pad(d).align; }

size_t cinf_type_width(decl_ref d) { return _type_pad(d).size; }
size_t cinf_field_width(decl_ref d) { return _field_pad(d).size; }
size_t cinf_intrinsic_width(decl_ref d) { return _intrinsic_pad(d).size; }
size_t cinf_pointer_width(decl_ref d) { return _pointer_pad(d).size; }
size_t cinf_array_width(decl_ref d) { return _array_pad(d).size; }
size_t cinf_struct_width(decl_ref d) { return _struct_pad(d).size; }
size_t cinf_union_width(decl_ref d) { return _union_pad(d).size; }

size_t cinf_array_count(decl_ref d)
{
    return cinf_is_array(d) ? cinf_decl_qty(d) : 0;
}

decl_ref cinf_typedef_type(decl_ref d)
{
    return cinf_is_typedef(d) ? cinf_decl_link(d) : cinf_decl_void(d);
}

decl_ref cinf_field_type(decl_ref d)
{
    return cinf_is_field(d) ? cinf_decl_link(d) : cinf_decl_void(d);
}

decl_ref cinf_array_type(decl_ref d)
{
    return cinf_is_array(d) ? cinf_decl_link(d) : cinf_decl_void(d);
}

decl_ref cinf_pointer_type(decl_ref d)
{
    return cinf_is_pointer(d) ? cinf_decl_link(d) : cinf_decl_void(d);
}

decl_ref cinf_constant_type(decl_ref d)
{
    return cinf_is_constant(d) ? cinf_decl_link(d) : cinf_decl_void(d);
}

decl_ref cinf_parameter_type(decl_ref d)
{
    return cinf_is_parameter(d) ? cinf_decl_link(d) : cinf_decl_void(d);
}

int cinf_enum_constants(decl_ref d, decl_ref *r, size_t *s)
{
    if (!cinf_is_enum(d)) return -1;
    return decl_array_fetch(d.db, r, s, cinf_decl_link(d), cinf_is_constant);
}

int cinf_struct_fields(decl_ref d, decl_ref *r, size_t *s)
{
    if (!cinf_is_struct(d)) return -1;
    return decl_array_fetch(d.db, r, s, cinf_decl_link(d), cinf_is_field);
}

int cinf_union_fields(decl_ref d, decl_ref *r, size_t *s)
{
    if (!cinf_is_union(d)) return -1;
    return decl_array_fetch(d.db, r, s, cinf_decl_link(d), cinf_is_field);
}

int cinf_function_parameters(decl_ref d, decl_ref *r, size_t *s)
{
    if (!cinf_is_function(d)) return -1;
    return decl_array_fetch(d.db, r, s, cinf_decl_link(d), cinf_is_parameter);
}

int cinf_source_decls(decl_ref d, decl_ref *r, size_t *s)
{
    if (!cinf_is_source(d)) return -1;
    return decl_array_fetch(d.db, r, s, cinf_decl_link(d), cinf_is_any);
}

int cinf_source_types(decl_ref d, decl_ref *r, size_t *s)
{
    if (!cinf_is_source(d)) return -1;
    return decl_array_fetch(d.db, r, s, cinf_decl_link(d), cinf_is_type);
}

int cinf_source_fields(decl_ref d, decl_ref *r, size_t *s)
{
    if (!cinf_is_source(d)) return -1;
    return decl_array_fetch(d.db, r, s, cinf_decl_link(d), cinf_is_field);
}

int cinf_source_functions(decl_ref d, decl_ref *r, size_t *s)
{
    if (!cinf_is_source(d)) return -1;
    return decl_array_fetch(d.db, r, s, cinf_decl_link(d), cinf_is_function);
}

int cinf_archive_sources(decl_ref d, decl_ref *r, size_t *s)
{
    if (!cinf_is_archive(d)) return -1;
    return decl_array_fetch(d.db, r, s, cinf_decl_link(d), cinf_is_source);
}

decl_raw cinf_constant_value(decl_ref d)
{
    if (!cinf_is_constant(d)) return decl_raw { 0 };
    return decl_raw { cinf_decl_ptr(d)->value };
}

void * cinf_function_addr(decl_ref d)
{
    if (!cinf_is_function(d)) return nullptr;
    return (void*)cinf_decl_ptr(d)->addr;
}
