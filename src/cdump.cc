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

#include "cutil.h"
#include "cmodel.h"
#include "cdump.h"
#include "cfileio.h"

#define array_size(arr) ((sizeof(arr)/sizeof(arr[0])))

struct prop_name
{
    decl_set prop;
    const char *name;
};

static prop_name prop_names[] = {
    { _const,     "const"      },
    { _volatile,  "volatile"   },
    { _restrict,  "restrict"   },
    { _in,        "in"         },
    { _out,       "out"        }
};

static std::string _link(decl_ref d)
{
    std::string buf;

    decl_ref lr = crefl_lookup(d.db, crefl_decl_ptr(d)->_link);

    if (strlen(crefl_decl_name(lr)) > 0) {
        buf = string_printf("%s(\"%s\")",
            crefl_tag_name(crefl_decl_tag(lr)), crefl_decl_name(lr));
    }
    else {
        buf = string_printf("%s(anonymous)",
            crefl_tag_name(crefl_decl_tag(lr)));
    }

    return buf;
}

static std::string _props(decl_ref d, const char *fmt, ...)
{
    std::string buf;

    decl_set props = crefl_decl_props(d);
    for (size_t i = 0; i < array_size(prop_names); i++) {
        prop_name p = prop_names[i];
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

void crefl_db_header_names()
{
    printf("%-5s %-5s %-5s %-5s %-10s %-18s %-18s %-24s\n",
        "id", "attr", "next", "link", "type", "name", "props", "link-detail");
}

void crefl_db_header_lines()
{
    printf("%-5s %-5s %-5s %-5s %-10s %-18s %-18s %-24s\n",
        "-----", "-----", "-----", "-----", "----------",
        "------------------", "------------------", "------------------------");
}

void crefl_db_dump_row(decl_db *db, decl_ref r)
{
    std::string name, link, props;

    decl_node *d = crefl_decl_ptr(r);
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
    case _decl_field:
        if ((crefl_decl_props(r) & _bitfield) > 0) {
            props = _props(r, "width=" fmt_SZ, d->_width);
        } else {
            props = _props(r, "");
        }
        break;
    default: break;
    }
    link = _link(r);
    name = crefl_decl_ptr(r)->_name > 0 ? crefl_decl_name(r) : "(anonymous)";

    printf("%-5u %-5d %-5d %-5d %-10s %-18s %-18s %-24s\n",
        crefl_decl_idx(r), d->_attr, d->_next, d->_link,
        crefl_tag_name(crefl_decl_tag(r)),
        name.c_str(), props.c_str(), link.c_str());
}

void crefl_db_dump(decl_db *db)
{
    crefl_db_header_names();
    crefl_db_header_lines();

    for (size_t i = db->root_element; i < db->decl_offset; i++) {
        decl_ref r = crefl_lookup(db, i);
        crefl_db_dump_row(db, r);
    }

    crefl_db_header_lines();
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
