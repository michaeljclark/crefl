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
#include <sstream>
#include <functional>

#include "cutil.h"
#include "cmodel.h"
#include "clink.h"
#include "cdump.h"
#include "cfileio.h"

static decl_index *ld;

#define array_size(arr) ((sizeof(arr)/sizeof(arr[0])))

typedef std::string (*format_fn)(const struct crefl_field*, void *obj);
static std::string _field_id(const struct crefl_field *f, void *obj);
static std::string _field_str(const struct crefl_field *f, void *obj);

struct crefl_field
{
    const char *name;
    size_t width, offset;
    format_fn format;
};

struct crefl_db_row
{
    decl_id id, attr, next, link;
    std::string type, name, props, detail, hash, fqn;
};

struct crefl_prop
{
    decl_set prop;
    const char *name;
};

static crefl_prop prop_names[] = {
    { _decl_const,     "const"      },
    { _decl_volatile,  "volatile"   },
    { _decl_restrict,  "restrict"   },
    { _decl_in,        "in"         },
    { _decl_out,       "out"        }
};

#define _FIELD(x) offsetof(crefl_db_row,x)

static const crefl_field f_id =     { "id",     5,  _FIELD(id),     _field_id  };
static const crefl_field f_attr =   { "attr",   5,  _FIELD(attr),   _field_id  };
static const crefl_field f_next =   { "next",   5,  _FIELD(next),   _field_id  };
static const crefl_field f_link =   { "link",   5,  _FIELD(link),   _field_id  };
static const crefl_field f_type =   { "type",   10, _FIELD(type),   _field_str };
static const crefl_field f_name =   { "name",   18, _FIELD(name),   _field_str };
static const crefl_field f_props =  { "props",  18, _FIELD(props),  _field_str };
static const crefl_field f_detail = { "detail", 24, _FIELD(detail), _field_str };
static const crefl_field f_hash =   { "hash",   56, _FIELD(hash),   _field_str };
static const crefl_field f_fqn =    { "fqn",    20, _FIELD(fqn),    _field_str };

static const crefl_field * fields_std[] = {
    &f_id, &f_attr, &f_next, &f_link, &f_type, &f_name, &f_props, &f_detail,
    0
};

static const crefl_field * fields_all[] = {
    &f_id, &f_attr, &f_next, &f_link, &f_type, &f_name, &f_props, &f_detail,
    &f_hash, &f_fqn, 0
};

static const crefl_field ** fields = fields_std;

static std::string _link(decl_ref d)
{
    decl_ref lr = crefl_lookup(d.db, crefl_decl_ptr(d)->_link);
    const char *name = crefl_decl_name(lr);
    return string_printf("%s(\"%s\")", crefl_tag_name(crefl_decl_tag(lr)),
        strlen(name) ? name : "anonymous");
}

static std::string _props(decl_ref d, const char *fmt, ...)
{
    std::string buf;

    decl_set props = crefl_decl_props(d);
    for (size_t i = 0; i < array_size(prop_names); i++) {
        crefl_prop p = prop_names[i];
        if ((props & p.prop) == p.prop) {
            props &= ~p.prop;
            if (buf.size() > 0) {
                buf.append(",");
            }
            buf.append(p.name);
        }
    }

    if (strlen(fmt) > 0)
    {
        va_list args;
        va_start(args, fmt);
        if (buf.size() > 0) {
            buf.append(",");
        }
        buf.append(string_vprintf(fmt, args));
        va_end(args);
    }

    return buf;
}

static std::string _field_id(const crefl_field *f, void *obj)
{
    return std::to_string(*reinterpret_cast<decl_id*>((char*)obj + f->offset));
}

static std::string _field_str(const crefl_field *f, void *obj)
{
    return *reinterpret_cast<std::string*>((char*)obj + f->offset);
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

static std::string _pad_str(std::string s, const size_t w, char pad = ' ')
{
    return s.append(s.size() < w ? w - s.size() : 0, pad);
}

static std::string _fqn(decl_ref r, decl_entry_ref er)
{
    std::stringstream s;
    s << crefl_tag_name(crefl_decl_tag(r)) << " " << crefl_entry_fqn(er);
    return s.str();
}

crefl_db_row crefl_db_get_row(decl_db *db, decl_ref r)
{
    decl_id tag = crefl_decl_tag(r);
    decl_entry_ref er = crefl_entry_ref(ld, r);
    decl_entry *ent = crefl_entry_ptr(er);
    decl_node *d = crefl_decl_ptr(r);

    std::string props;
    switch (crefl_decl_tag(r)) {
    case _decl_typedef:
    case _decl_struct:
    case _decl_union:
    case _decl_param:
    case _decl_attribute: props = _props(r, ""); break;
    case _decl_set:
    case _decl_enum:
    case _decl_pointer:
    case _decl_intrinsic: props = _props(r, "width=" fmt_SZ, d->_width); break;
    case _decl_array:     props = _props(r, "size=" fmt_SZ, d->_count);  break;
    case _decl_constant:
    case _decl_value:     props = _props(r, "value=" fmt_SZ, d->_value); break;
    case _decl_function:  props = _props(r, "addr=" fmt_AD, d->_addr);   break;
    case _decl_field:     props = (crefl_decl_props(r) & _decl_bitfield) ?
                         _props(r, "width=" fmt_SZ, d->_width) : _props(r, "");
    default: break;
    }

    return crefl_db_row {
        crefl_decl_idx(r), d->_attr, d->_next, d->_link, crefl_tag_name(tag),
        crefl_decl_ptr(r)->_name ? crefl_decl_name(r) : "(anonymous)", props,
        _link(r), _hex_str(ent->hash.sum, sizeof(ent->hash.sum)), _fqn(r, er)
    };
}

static std::string crefl_field_iter(const crefl_field ** i,
    std::function<std::string(const crefl_field*)> f)
{
    std::string s;
    while (*i) s.append(std::string(s.size() > 0 ? " " : "") + f(*i++));
    return s;
}

static void _header_names(const crefl_field ** fields)
{
    printf("%s\n", crefl_field_iter(fields,
        [](auto f) { return _pad_str(f->name, f->width); }).c_str());
}

static void _header_lines(const crefl_field ** fields)
{
    printf("%s\n", crefl_field_iter(fields,
        [](auto f) { return _pad_str("", f->width, '-'); }).c_str());
}

static void _row(const crefl_field ** fields, decl_db *db, decl_ref r)
{
    crefl_db_row row = crefl_db_get_row(db, r);
    printf("%s\n", crefl_field_iter(fields,
        [&](auto f) { return _pad_str(f->format(f, &row), f->width); }).c_str());
}

void crefl_db_header_names() { _header_names(fields); }
void crefl_db_header_lines() { _header_lines(fields); }
void crefl_db_dump_row(decl_db *db, decl_ref r) { _row(fields, db, r); }

void crefl_db_dump(decl_db *db)
{
    ld = crefl_index_new();
    crefl_index_scan(ld, db);

    crefl_db_header_names();
    crefl_db_header_lines();
    for (size_t i = db->root_element; i < db->decl_offset; i++) {
        crefl_db_dump_row(db, crefl_lookup(db, i));
    }
    crefl_db_header_lines();

    crefl_index_destroy(ld);
}

void crefl_db_set_dump_fmt(enum crefl_db_dump_fmt fmt)
{
    switch (fmt) {
    case crefl_db_dump_std: fields = fields_std;
    case crefl_db_dump_all: fields = fields_all;
    }
}

void crefl_db_dump_stats(decl_db *db)
{
    size_t decl_builtin = db->decl_builtin;
    size_t decl_user = db->decl_offset - db->decl_builtin;
    size_t decl_total = db->decl_offset;

    size_t name_builtin = db->name_builtin;
    size_t name_user = db->name_offset - db->name_builtin;
    size_t name_total = db->name_offset;

    printf(
        "decl.builtin %zu bytes (%zu records)\n"
        "decl.user    %zu bytes (%zu records)\n"
        "name.builtin %zu bytes\n"
        "name.user    %zu bytes\n"
        "file.size    %zu bytes\n",
        sizeof(decl_node) * decl_builtin, decl_builtin,
        sizeof(decl_node) * decl_user,    decl_user,
        name_builtin,
        name_user,
        sizeof(decl_db_hdr) + sizeof(decl_node) * decl_user + name_user
    );
}
