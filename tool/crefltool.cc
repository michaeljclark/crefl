/*
 * crefltool - tool to dump crefl reflection metadata.
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
#include <cstring>

#include <crefl/model.h>
#include <crefl/dump.h>
#include <crefl/link.h>
#include <crefl/db.h>

#define array_size(arr) ((sizeof(arr)/sizeof(arr[0])))

void do_merge(const char *output, const char **input, size_t n)
{
    decl_db *db_out = crefl_db_new();
    decl_db **db_in = (decl_db**)malloc(sizeof(decl_db*) * n);
    for (size_t i = 0; i < n; i++) {
        db_in[i] = crefl_db_new();
        crefl_db_read_file(db_in[i], input[i]);
    }
    if (crefl_link_merge(db_out, output, db_in, n) < 0) {
        fprintf(stderr, "error: merging input files\n");
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
    fprintf(f, "const unsigned char __crefl_%s_data[] = {\n", name);
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

typedef enum {
    _dump_std,
    _dump_fqn,
    _dump_sum,
    _dump_all,
    _dump_ext,
    _dump_ext_fqn,
    _dump_ext_sum,
    _dump_ext_all,
    _merge,
    _emit,
    _stats
} mode_enum;

struct {
    mode_enum val;
    const char *arg;
} mode_args[] = {
    { _dump_std,      "--dump"         },
    { _dump_fqn,      "--dump-fqn"     },
    { _dump_sum,      "--dump-sum"     },
    { _dump_all,      "--dump-all"     },
    { _dump_ext,      "--dump-ext"     },
    { _dump_ext_fqn,  "--dump-ext-fqn" },
    { _dump_ext_sum,  "--dump-ext-sum" },
    { _dump_ext_all,  "--dump-ext-all" },
    { _merge,         "--merge"        },
    { _emit,          "--emit"         },
    { _stats,         "--stats"        },
};

int main(int argc, const char **argv)
{
    size_t i;
    mode_enum mode;

    if (argc < 3) goto help_exit;
    for (i = 0; i < array_size(mode_args); i++) {
        if (strcmp(argv[1], mode_args[i].arg) == 0) {
            mode = mode_args[i].val; break;
        }
    }
    if (i == array_size(mode_args)) goto help_exit;

    if ( (mode == _merge && argc < 4) ||
         (mode == _emit && argc != 4) ||
         (mode != _merge && mode != _emit && argc != 3) )
    {
        fprintf(stderr, "error: *** unknown command line option\n\n");
        goto help_exit;
    }

    switch (mode) {
        case _dump_std: do_dump(crefl_db_dump_std, argv[2]); break;
        case _dump_fqn: do_dump(crefl_db_dump_fqn, argv[2]); break;
        case _dump_sum: do_dump(crefl_db_dump_sum, argv[2]); break;
        case _dump_all: do_dump(crefl_db_dump_all, argv[2]); break;
        case _dump_ext: do_dump(crefl_db_dump_ext, argv[2]); break;
        case _dump_ext_fqn: do_dump(crefl_db_dump_ext_fqn, argv[2]); break;
        case _dump_ext_sum: do_dump(crefl_db_dump_ext_sum, argv[2]); break;
        case _dump_ext_all: do_dump(crefl_db_dump_ext_all, argv[2]); break;
        case _stats: do_stats(argv[2]); break;
        case _merge: do_merge(argv[2], argv + 3, argc - 3); break;
        case _emit: do_emit(argv[2], argv[3], "main"); break;
    }
    exit(0);

help_exit:
    fprintf(stderr, "usage: %s <command>\n\n"
    "Commands:\n\n"
    "--merge <output> [<input>]+  merge reflection metadata\n"
    "--emit <output> [<input>]    emit reflection metadata\n"
    "--dump <input>               dump main fields in standard 80-col format\n"
    "--dump-fqn <input>           dump main fields plus fqn in standard 103-col format\n"
    "--dump-sum <input>           dump main fields plus sum in standard 137-col format\n"
    "--dump-all <input>           dump main fields plus sum and fqn in standard 160-col format\n"
    "--dump-ext <input>           dump main fields in extended 113-col format\n"
    "--dump-ext-fqn <input>       dump main fields plus fqn in extended 143-col format\n"
    "--dump-ext-sum <input>       dump main fields plus sum in extended 170-col format\n"
    "--dump-ext-all <input>       dump main fields plus sum and fqn in extended 200-col format\n"
    "--stats                      print reflection db statistics\n\n", argv[0]);
    exit(1);
}
