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

    decl_ref lr = crefl_lookup(d.db, crefl_decl_ptr(d)->_link);

    if (strlen(crefl_decl_name(lr)) > 0) {
        snprintf(buf, sizeof(buf), "%s(\"%s\")",
            crefl_tag_name(crefl_decl_tag(lr)), crefl_decl_name(lr));
    }
    else {
        snprintf(buf, sizeof(buf), "%s",
            crefl_tag_name(crefl_decl_tag(lr)));
    }

    size_t len = strlen(buf);

    decl_set props = crefl_decl_props(d);

    for (size_t i = 0; i < array_size(prop_names); i++) {
        prop_name p = prop_names[i];
        if ((props & p.prop) == p.prop) {
            props &= ~p.prop;
            strncat(buf, ",", sizeof(buf)-len);
            len++;
            strncat(buf, p.name, sizeof(buf)-len);
            len += strlen(p.name);
        }
    }

    return buf;
}

void crefl_db_header_names()
{
    printf("%-5s %-5s %-5s %-5s %-10s %-18s %-26s\n",
        "id", "attr", "next", "link", "type", "name", "properties");
}

void crefl_db_header_lines()
{
    printf("%-5s %-5s %-5s %-5s %-10s %-18s %-26s\n",
        "-----", "-----", "-----", "-----", "----------",
        "------------------", "--------------------------");
}

void crefl_db_dump_row(decl_db *db, decl_ref r)
{
    char buf[256];
    decl_node *d = crefl_decl_ptr(r);
    switch (crefl_decl_tag(r)) {
    case _decl_typedef:
    case _decl_set:
    case _decl_enum:
    case _decl_struct:
    case _decl_union:
    case _decl_pointer:
    case _decl_param:
    case _decl_attribute:
        snprintf(buf, sizeof(buf), "%s", _pretty_props(r));
        break;
    case _decl_field:
        if ((crefl_decl_props(r) & _bitfield) > 0) {
            snprintf(buf, sizeof(buf), "%s width=" fmt_SZ,
                _pretty_props(r), d->_width);
        } else {
            snprintf(buf, sizeof(buf), "%s",
                _pretty_props(r));
        }
        break;
    case _decl_intrinsic:
        snprintf(buf, sizeof(buf), "%s width=" fmt_SZ,
            _pretty_props(r), d->_width);
        break;
    case _decl_array:
        snprintf(buf, sizeof(buf), "%s size=" fmt_SZ,
            _pretty_props(r), d->_count);
        break;
    case _decl_constant:
    case _decl_value:
        snprintf(buf, sizeof(buf), "%s value=" fmt_SZ,
            _pretty_props(r), d->_value);
        break;
    case _decl_function:
        snprintf(buf, sizeof(buf), "%s addr=" fmt_AD,
            _pretty_props(r), d->_addr);
        break;
    default: buf[0] = '\0'; break;
    }
    printf("%-5u %-5d %-5d %-5d %-10s %-18s %-26s\n",
        crefl_decl_idx(r), d->_attr, d->_next, d->_link,
        crefl_tag_name(crefl_decl_tag(r)),
        strlen(crefl_decl_name(r)) ? crefl_decl_name(r) : "(anonymous)", buf);
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
