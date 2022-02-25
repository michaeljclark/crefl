/*
 * <crefl/dump.h>
 *
 * crefl runtime library and compiler plug-in to support reflection in C.
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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

enum crefl_db_dump_fmt
{
    crefl_db_dump_std,
    crefl_db_dump_fqn,
    crefl_db_dump_sum,
    crefl_db_dump_all,
    crefl_db_dump_ext,
    crefl_db_dump_ext_fqn,
    crefl_db_dump_ext_sum,
    crefl_db_dump_ext_all
};

void crefl_db_header_names();
void crefl_db_header_lines();

void crefl_db_set_dump_fmt(enum crefl_db_dump_fmt fmt);

void crefl_db_dump(decl_db *db);
void crefl_db_dump_row(decl_db *db, decl_ref r);
void crefl_db_dump_stats(decl_db *db);

#ifdef __cplusplus
}
#endif
