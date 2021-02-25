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

static const char * _pretty_name(const char* l, decl_db *db, size_t decl_idx)
{
    static char buf[1024];

    decl_ref d = crefl_lookup(db, decl_idx);

    if (strlen(crefl_name(d)) > 0) {
        snprintf(buf, sizeof(buf), "%s=[%s:%u,(\"%s\")]",
            l, crefl_tag_name(crefl_tag(d)), crefl_idx(d), crefl_name(d));
    } else {
        snprintf(buf, sizeof(buf), "%s=[%s:%u,(anonymous)]",
            l, crefl_tag_name(crefl_tag(d)), crefl_idx(d));
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
    decl *d = crefl_ptr(r);
    switch (crefl_tag(r)) {
    case _decl_typedef:
        snprintf(buf, sizeof(buf), "%s",
            _pretty_name("decl", db, d->_decl_typedef._decl));
        break;
    case _decl_intrinsic:
        snprintf(buf, sizeof(buf), "width=" fmt_SZ,
            d->_decl_intrinsic._width);
        break;
    case _decl_set:
        snprintf(buf, sizeof(buf), "%s",
            _pretty_name("link", db, d->_decl_set._link));
        break;
    case _decl_enum:
        snprintf(buf, sizeof(buf), "%s",
            _pretty_name("link", db, d->_decl_enum._link));
        break;
    case _decl_struct:
        snprintf(buf, sizeof(buf), "%s",
            _pretty_name("link", db, d->_decl_struct._link));
        break;
    case _decl_union:
        snprintf(buf, sizeof(buf), "%s",
            _pretty_name("link", db, d->_decl_union._link));
        break;
    case _decl_field:
        if ((crefl_attrs(r) & _bitfield) > 0) {
            snprintf(buf, sizeof(buf), "%s width=" fmt_SZ,
                _pretty_name("decl", db, d->_decl_field._decl),
                d->_decl_field._width);
        } else {
            snprintf(buf, sizeof(buf), "%s",
                _pretty_name("decl", db, d->_decl_field._decl));
        }
        break;
    case _decl_array:
        snprintf(buf, sizeof(buf), "%s size=" fmt_SZ,
            _pretty_name("decl", db, d->_decl_array._decl),
            d->_decl_array._size);
        break;
    case _decl_constant:
        snprintf(buf, sizeof(buf), "%s value=" fmt_SZ,
            _pretty_name("decl", db, d->_decl_constant._decl),
            d->_decl_constant._value);
        break;
    case _decl_function:
        snprintf(buf, sizeof(buf), "%s addr=" fmt_AD,
            _pretty_name("link", db, d->_decl_function._link),
            d->_decl_function._addr);
        break;
    case _decl_param:
        snprintf(buf, sizeof(buf), "%s",
            _pretty_name("decl", db, d->_decl_param._decl));
        break;
    default: buf[0] = '\0'; break;
    }
    printf("%-5u %-5d %-10s %-14s %-14s\n", crefl_idx(r), d->_next,
        crefl_tag_name(crefl_tag(r)),
        strlen(crefl_name(r)) > 0 ? crefl_name(r) : "(anonymous)", buf);
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
