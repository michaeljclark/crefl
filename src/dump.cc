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
#include <cerrno>

#include <string>
#include <functional>

#include <cinf/util.h>
#include <cinf/model.h>
#include <cinf/link.h>
#include <cinf/dump.h>
#include <cinf/db.h>

static decl_index *ld;

#define array_size(arr) ((sizeof(arr)/sizeof(arr[0])))

typedef std::string (*format_fn)(const struct cinf_field*, void *obj);
static std::string field_id(const struct cinf_field *f, void *obj);
static std::string field_str(const struct cinf_field *f, void *obj);

struct cinf_field
{
    const char *name;
    size_t width, offset;
    format_fn format;
};

struct cinf_db_row
{
    decl_id id, attr, next, link;
    std::string type, name, props, detail, hash, fqn;
};

struct cinf_prop
{
    decl_set prop;
    const char *name;
};

static cinf_prop prop_names[] = {
    /* cvr-qualifiers */
    { decl_const,     "const"      },
    { decl_volatile,  "volatile"   },
    { decl_restrict,  "restrict"   },
    /* interface qualifiers */
    { decl_static,    "static"     },
    { decl_extern_c,  "extern_c"   },
    { decl_inline,    "inline"     },
    { decl_noreturn,  "noreturn"   },
    /* binding */
    { decl_local,     "local"      },
    { decl_global,    "global"     },
    { decl_weak,      "weak"       },
    /* visibility */
    { decl_default,   "default"    },
    { decl_hidden,    "hidden"     },
    /* param */
    { decl_in,        "in"         },
    { decl_out,       "out"        },
    /* variable-length-array */
    { decl_vla,       "vla"        }
};

#define _FIELD(x) offsetof(cinf_db_row,x)

static const cinf_field f_id =     { "id",     5,  _FIELD(id),     field_id  };
static const cinf_field f_attr =   { "attr",   5,  _FIELD(attr),   field_id  };
static const cinf_field f_next =   { "next",   5,  _FIELD(next),   field_id  };
static const cinf_field f_link =   { "link",   5,  _FIELD(link),   field_id  };
static const cinf_field f_type =   { "type",   10, _FIELD(type),   field_str };
static const cinf_field f_name =   { "name",   15, _FIELD(name),   field_str };
static const cinf_field f_props =  { "props",  15, _FIELD(props),  field_str };
static const cinf_field f_detail = { "detail", 20, _FIELD(detail), field_str };
static const cinf_field f_hash =   { "hash",   57, _FIELD(hash),   field_str };
static const cinf_field f_fqn =    { "fqn",    23, _FIELD(fqn),    field_str };

static const cinf_field fx_name =   { "name",   28, _FIELD(name),   field_str };
static const cinf_field fx_props =  { "props",  25, _FIELD(props),  field_str };
static const cinf_field fx_detail = { "detail", 30, _FIELD(detail), field_str };
static const cinf_field fx_fqn =    { "fqn",    30, _FIELD(fqn),    field_str };

static const cinf_field * fields_std[] = {
    &f_id, &f_attr, &f_next, &f_link, &f_type, &f_name, &f_props, &f_detail,
    0
};

static const cinf_field * fields_fqn[] = {
    &f_id, &f_attr, &f_next, &f_link, &f_type, &f_name, &f_props, &f_detail,
    &f_fqn, 0
};

static const cinf_field * fields_sum[] = {
    &f_id, &f_attr, &f_next, &f_link, &f_type, &f_name, &f_props, &f_detail,
    &f_hash, 0
};

static const cinf_field * fields_all[] = {
    &f_id, &f_attr, &f_next, &f_link, &f_type, &f_name, &f_props, &f_detail,
    &f_hash, &f_fqn, 0
};

static const cinf_field * fields_ext[] = {
    &f_id, &f_attr, &f_next, &f_link, &f_type, &fx_name, &fx_props, &fx_detail,
    0
};

static const cinf_field * fields_ext_fqn[] = {
    &f_id, &f_attr, &f_next, &f_link, &f_type, &fx_name, &fx_props, &fx_detail,
    &fx_fqn, 0
};

static const cinf_field * fields_ext_sum[] = {
    &f_id, &f_attr, &f_next, &f_link, &f_type, &fx_name, &fx_props, &fx_detail,
    &f_hash, 0
};

static const cinf_field * fields_ext_all[] = {
    &f_id, &f_attr, &f_next, &f_link, &f_type, &fx_name, &fx_props, &fx_detail,
    &f_hash, &fx_fqn, 0
};

static const cinf_field ** fields = fields_std;

static std::string ref_link(decl_ref d)
{
    decl_ref lr = cinf_lookup(d.db, cinf_decl_ptr(d)->link);
    const char *name = cinf_decl_name(lr);
    return string_printf("%s(\"%s\")", cinf_tag_name(cinf_decl_tag(lr)),
        strlen(name) ? name : "anonymous");
}

static std::string ref_props(decl_ref d, const char *fmt, ...)
{
    std::string buf;

    decl_set props = cinf_decl_props(d);
    for (size_t i = 0; i < array_size(prop_names); i++) {
        cinf_prop p = prop_names[i];
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

static std::string field_id(const cinf_field *f, void *obj)
{
    return std::to_string(*reinterpret_cast<decl_id*>((char*)obj + f->offset));
}

static std::string field_str(const cinf_field *f, void *obj)
{
    return *reinterpret_cast<std::string*>((char*)obj + f->offset);
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

static std::string pad_str(std::string s, const size_t w, char pad = ' ')
{
    return s.size() < w ? s.append(w - s.size(), pad) : s.substr(0, w - 1) + "…";
}

static std::string ref_fqn(decl_ref r, decl_entry_ref er)
{
    std::string s;
    s.append(cinf_tag_name(cinf_decl_tag(r)));
    s.append(" ");
    s.append(cinf_entry_fqn(er));
    return s;
}

cinf_db_row cinf_db_get_row(decl_db *db, decl_ref r)
{
    decl_id tag = cinf_decl_tag(r);
    decl_entry_ref er = cinf_entry_ref(ld, r);
    decl_entry *ent = cinf_entry_ptr(er);
    decl_node *d = cinf_decl_ptr(r);

    std::string fqn;
    if (cinf_is_alias(r)) {
        decl_ref a = cinf_decl_link(r);
        er = cinf_entry_ref(ld, a);
        fqn = ref_fqn(a, er);
    } else {
        fqn = ref_fqn(r, er);
    }

    std::string props;
    switch (tag) {
    case decl_archive:
    case decl_source:
    case decl_alias:
    case decl_typedef:
    case decl_struct:
    case decl_union:
    case decl_parameter:
    case decl_qualifier:
    case decl_attribute: props = ref_props(r, ""); break;
    case decl_enum:
    case decl_pointer:
    case decl_intrinsic: props = ref_props(r, "width=" fmt_SZ, d->width); break;
    case decl_array:     props = ref_props(r, "size=" fmt_SZ, d->count);  break;
    case decl_constant:
    case decl_value:     props = ref_props(r, "value=" fmt_SZ, d->value); break;
    case decl_function:  props = ref_props(r, "addr=" fmt_AD, d->addr);   break;
    case decl_field:     props = (cinf_decl_props(r) & decl_bitfield) ?
                         ref_props(r, "width=" fmt_SZ, d->width) : ref_props(r, "");
    default: break;
    }

    return cinf_db_row {
        cinf_decl_idx(r), d->attr, d->next, d->link, cinf_tag_name(tag),
        cinf_decl_has_name(r) ? cinf_decl_name(r) : "(anonymous)", props,
        ref_link(r), hex_str(ent->hash.sum, sizeof(ent->hash.sum)), fqn
    };
}

static std::string cinf_field_iter(const cinf_field ** i,
    std::function<std::string(const cinf_field*)> f)
{
    std::string s;
    while (*i) s.append(f(*i++));
    return s;
}

static void header_names(const cinf_field ** fields)
{
    printf("%s\n", cinf_field_iter(fields,
        [](auto f) { return pad_str(f->name, f->width); }).c_str());
}

static void header_lines(const cinf_field ** fields)
{
    printf("%s\n", cinf_field_iter(fields,
        [](auto f) { return pad_str("", f->width, '-'); }).c_str());
}

static void dump_row(const cinf_field ** fields, decl_db *db, decl_ref r)
{
    cinf_db_row row = cinf_db_get_row(db, r);
    printf("%s\n", cinf_field_iter(fields,
        [&](auto f) { return pad_str(f->format(f, &row), f->width); }).c_str());
}

void cinf_dbheader_names() { header_names(fields); }
void cinf_dbheader_lines() { header_lines(fields); }
void cinf_db_dump_row(decl_db *db, decl_ref r) { dump_row(fields, db, r); }

void cinf_db_dump(decl_db *db)
{
    ld = cinf_index_new();
    cinf_index_scan(ld, db);

    cinf_dbheader_names();
    cinf_dbheader_lines();
    for (size_t i = db->root_element; i < db->decl_offset; i++) {
        cinf_db_dump_row(db, cinf_lookup(db, i));
    }
    cinf_dbheader_lines();

    cinf_index_destroy(ld);
}

void cinf_db_set_dump_fmt(enum cinf_db_dump_fmt fmt)
{
    switch (fmt) {
    case cinf_db_dump_std: fields = fields_std; break;
    case cinf_db_dump_fqn: fields = fields_fqn; break;
    case cinf_db_dump_sum: fields = fields_sum; break;
    case cinf_db_dump_all: fields = fields_all; break;
    case cinf_db_dump_ext: fields = fields_ext; break;
    case cinf_db_dump_ext_fqn: fields = fields_ext_fqn; break;
    case cinf_db_dump_ext_sum: fields = fields_ext_sum; break;
    case cinf_db_dump_ext_all: fields = fields_ext_all; break;
    }
}

void cinf_db_dump_stats(decl_db *db)
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
