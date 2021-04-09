/*
 * crefltool - tool to dump crefl reflection metadata.
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
#include "clink.h"
#include "cfileio.h"

void do_link(const char *output, const char **input, size_t n)
{
    decl_db *db_out = crefl_db_new();
    decl_db **db_in = (decl_db**)malloc(sizeof(decl_db*) * n);
    for (size_t i = 0; i < n; i++) {
        db_in[i] = crefl_db_new();
        crefl_db_read_file(db_in[i], input[i]);
    }
    if (crefl_link_merge(db_out, output, db_in, n) < 0) {
        fprintf(stderr, "error: linking input files\n");
        exit(1);
    }
    crefl_db_write_file(db_out, output);
    for (size_t i = 0; i < n; i++) {
        crefl_db_destroy(db_in[i]);
    }
    crefl_db_destroy(db_out);
}

void do_emit(const char *output, const char *input, const char *name)
{
    FILE *f;
    decl_db *db;
    uint8_t *buf;
    size_t sz;
    const size_t w = 16;

    db = crefl_db_new();
    crefl_db_read_file(db, input);
    sz = crefl_db_size(db);
    buf = (uint8_t*)malloc(sz);
    if (crefl_db_write_mem(db, buf, sz) < 0 || !(f = fopen(output, "wb"))) {
        free(buf);
        fprintf(stderr, "error: writing db\n");
        exit(1);
    }
    fprintf(f, "#include <stdlib.h>\n");
    fprintf(f, "const char __crefl_%s_data[] = {\n", name);
    for (size_t i = 0; i < sz; i++) {
        fprintf(f, "0x%02hhx", buf[i]);
        if (i != sz -1) fprintf(f, ",");
        if (i % w == w-1 || i == sz - 1) fprintf(f, "\n");
    }
    fprintf(f, "};\n");
    fprintf(f, "const size_t __crefl_%s_size = sizeof(__crefl_%s_data);\n",
        name, name);
    fflush(f);
    fclose(f);
    free(buf);
    crefl_db_destroy(db);
}

void do_dump(crefl_db_dump_fmt fmt, const char *input)
{
    decl_db *db = crefl_db_new();
    crefl_db_read_file(db, input);
    crefl_db_set_dump_fmt(fmt);
    crefl_db_dump(db);
    crefl_db_destroy(db);
}

void do_stats(const char *input)
{
    decl_db *db = crefl_db_new();
    crefl_db_read_file(db, input);
    crefl_db_dump_stats(db);
    crefl_db_destroy(db);
}

int main(int argc, const char **argv)
{
    enum {
        _dump_std, _dump_fqn, _dump_all, _dump_ext, _link, _emit, _stats
    } mode;

    if (argc < 3) goto help_exit;
    else if (strcmp(argv[1], "--dump") == 0) mode = _dump_std;
    else if (strcmp(argv[1], "--dump-fqn") == 0) mode = _dump_fqn;
    else if (strcmp(argv[1], "--dump-all") == 0) mode = _dump_all;
    else if (strcmp(argv[1], "--dump-ext") == 0) mode = _dump_ext;
    else if (strcmp(argv[1], "--link") == 0) mode = _link;
    else if (strcmp(argv[1], "--emit") == 0) mode = _emit;
    else if (strcmp(argv[1], "--stats") == 0) mode = _stats;
    else goto help_exit;

    if ( (mode == _link && argc < 4) ||
         (mode == _emit && argc != 4) ||
         (mode != _link && mode != _emit && argc != 3) )
    {
        fprintf(stderr, "error: *** unknown command line option\n\n");
        goto help_exit;
    }

    switch (mode) {
        case _dump_std: do_dump(crefl_db_dump_std, argv[2]); break;
        case _dump_fqn: do_dump(crefl_db_dump_fqn, argv[2]); break;
        case _dump_ext: do_dump(crefl_db_dump_ext, argv[2]); break;
        case _dump_all: do_dump(crefl_db_dump_all, argv[2]); break;
        case _stats: do_stats(argv[2]); break;
        case _link: do_link(argv[2], argv + 3, argc - 3); break;
        case _emit: do_emit(argv[2], argv[3], "main"); break;
    }
    exit(0);

help_exit:
    fprintf(stderr, "usage: %s <command>\n\n"
    "Commands:\n\n"
    "--link <output> [<input>]+   link reflection metadata\n"
    "--emit <output> [<input>]    emit reflection metadata\n"
    "--dump <input>               dump main fields in 80-col format\n"
    "--dump-fqn <input>           dump main fields plus fqn in 143-col format\n"
    "--dump-all <input>           dump all fields in 160-col format\n"
    "--dump-ext <input>           dump all fields in 200-col format\n"
    "--stats                      print reflection db statistics\n\n", argv[0]);
    exit(1);
}
