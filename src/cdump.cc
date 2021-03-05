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

#include "cmodel.h"
#include "cdump.h"

#define array_size(arr) ((sizeof(arr)/sizeof(arr[0])))

struct prop_name
{
    decl_set prop;
    const char *name;
};

static prop_name prop_names[] = {
    { _const, "const" },
    { _volatile, "volatile" },
    { _restrict, "restrict" },
};

static const char * _pretty_props(decl_ref d)
{
    static char buf[1024];

    decl_set props = crefl_decl_props(d);
    size_t l = 0;
    buf[0] = '\0';

    for (size_t i = 0; i < array_size(prop_names); i++) {
        prop_name p = prop_names[i];
        if ((props & p.prop) == p.prop) {
            props &= ~p.prop;
            if (l > 0) {
                strncat(buf, ",", sizeof(buf)-l);
                l++;
            }
            strncat(buf, p.name, sizeof(buf)-l);
            l += strlen(p.name);
        }
    }

    return buf;
}

static const char * _pretty_name(const char* l, decl_db *db, size_t decl_idx)
{
    static char buf[1024];

    decl_ref d = crefl_lookup(db, decl_idx);

    if (strlen(crefl_decl_name(d)) > 0) {
        snprintf(buf, sizeof(buf), "%s=[%s:%u,(\"%s\")]",
            l, crefl_tag_name(crefl_decl_tag(d)), crefl_decl_idx(d), crefl_decl_name(d));
    } else {
        snprintf(buf, sizeof(buf), "%s=[%s:%u,(anonymous)]",
            l, crefl_tag_name(crefl_decl_tag(d)), crefl_decl_idx(d));
    }

    return buf;
}

void crefl_db_header_names()
{
    printf("%-5s %-5s %-10s %-14s %-14s\n",
        "id", "next", "type", "name", "details");
}

void crefl_db_header_lines()
{
    printf("%-5s %-5s %-10s %-14s %-14s\n",
        "-----", "-----", "----------", "--------------", "--------------");
}

void crefl_db_dump_row(decl_db *db, decl_ref r)
{
    char buf[256];
    decl_node *d = crefl_decl_ptr(r);
    switch (crefl_decl_tag(r)) {
    case _decl_intrinsic:
        snprintf(buf, sizeof(buf), "(%s) width=" fmt_SZ,
            _pretty_props(r), d->_width);
        break;
    case _decl_typedef:
        snprintf(buf, sizeof(buf), "%s (%s)",
            _pretty_name("decl", db, d->_link), _pretty_props(r));
        break;
    case _decl_set:
        snprintf(buf, sizeof(buf), "%s (%s)",
            _pretty_name("link", db, d->_link), _pretty_props(r));
        break;
    case _decl_enum:
        snprintf(buf, sizeof(buf), "%s (%s)",
            _pretty_name("link", db, d->_link), _pretty_props(r));
        break;
    case _decl_struct:
        snprintf(buf, sizeof(buf), "%s (%s)",
            _pretty_name("link", db, d->_link), _pretty_props(r));
        break;
    case _decl_union:
        snprintf(buf, sizeof(buf), "%s (%s)",
            _pretty_name("link", db, d->_link), _pretty_props(r));
        break;
    case _decl_field:
        if ((crefl_decl_props(r) & _bitfield) > 0) {
            snprintf(buf, sizeof(buf), "%s (%s) width=" fmt_SZ,
                _pretty_name("decl", db, d->_link), _pretty_props(r), d->_width);
        } else {
            snprintf(buf, sizeof(buf), "%s (%s)",
                _pretty_name("decl", db, d->_link), _pretty_props(r));
        }
        break;
    case _decl_array:
        snprintf(buf, sizeof(buf), "%s (%s) size=" fmt_SZ,
            _pretty_name("decl", db, d->_link), _pretty_props(r), d->_count);
        break;
    case _decl_constant:
        snprintf(buf, sizeof(buf), "%s (%s) value=" fmt_SZ,
            _pretty_name("decl", db, d->_link), _pretty_props(r), d->_value);
        break;
    case _decl_function:
        snprintf(buf, sizeof(buf), "%s (%s) addr=" fmt_AD,
            _pretty_name("link", db, d->_link), _pretty_props(r), d->_addr);
        break;
    case _decl_param:
        snprintf(buf, sizeof(buf), "%s (%s)",
            _pretty_name("decl", db, d->_link), _pretty_props(r));
        break;
    default: buf[0] = '\0'; break;
    }
    printf("%-5u %-5d %-10s %-14s %-14s\n", crefl_decl_idx(r), d->_next,
        crefl_tag_name(crefl_decl_tag(r)),
        strlen(crefl_decl_name(r)) > 0 ? crefl_decl_name(r) : "(anonymous)", buf);
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
