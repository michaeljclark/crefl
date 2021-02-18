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
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cassert>

#include <vector>

#include <sys/stat.h>

#include "cmodel.h"
#include "cfileio.h"

static size_t read_file(std::vector<uint8_t> &buf, const char* filename)
{
    FILE *f;
    struct stat statbuf;
    if ((f = fopen(filename, "rb")) == nullptr) {
        fprintf(stderr, "fopen: %s\n", strerror(errno));
        exit(1);
    }
    if (fstat(fileno(f), &statbuf) < 0) {
        fprintf(stderr, "fstat: %s\n", strerror(errno));
        exit(1);
    }
    buf.resize(statbuf.st_size);
    size_t len = fread(buf.data(), 1, buf.size(), f);
    assert(buf.size() == len);
    fclose(f);
    return buf.size();
}

/*
 * write file from std::vector using buffered file IO
 */
static size_t write_file(std::vector<uint8_t> &buf, const char* filename)
{
    FILE *f;
    if ((f = fopen(filename, "wb")) == nullptr) {
        fprintf(stderr, "fopen: %s\n", strerror(errno));
        exit(1);
    }
    size_t len = fwrite(buf.data(), 1, buf.size(), f);
    assert(buf.size() == len);
    fclose(f);
    return buf.size();
}

int crefl_check_magic(void *addr)
{
    return memcmp(addr, decl_db_magic, sizeof(decl_db_magic));
}

void crefl_read_db(decl_db *db, const char *input_filename)
{
    std::vector<uint8_t> buf;

    read_file(buf, input_filename);

    if (buf.size() < sizeof(decl_db_hdr)) {
        fprintf(stderr, "crefl: *** rror: header too short\n");
        exit(1);
    }
    if (crefl_check_magic(buf.data()) != 0) {
        fprintf(stderr, "crefl: *** error: invalid magic\n");
        exit(1);
    }

    decl_db_hdr *hdr = (decl_db_hdr*)&buf[0];
    size_t hdr_sz = sizeof(decl_db_hdr);
    size_t decl_sz = sizeof(decl) * hdr->decl_entry_count;
    size_t name_sz = hdr->name_table_size;

    /* resize buffers */
    if (db->decl_size < decl_sz) {
        free(db->decl);
        db->decl_size = hdr->decl_entry_count;
        db->decl = (decl*)malloc(sizeof(decl) * db->decl_size);
    }
    if (db->name_size < name_sz) {
        free(db->name);
        db->name_size = hdr->name_table_size;
        db->name = (char*)malloc(db->name_size);
    }

    /* copy data from temporary buffer */
    db->decl_offset = hdr->decl_entry_count;
    db->name_offset = hdr->name_table_size;
    db->root_element = hdr->root_element;
    memcpy(db->decl, &buf[hdr_sz], decl_sz);
    memcpy(db->name, &buf[hdr_sz + decl_sz], name_sz);
}

void crefl_write_db(decl_db *db, const char *output_filename)
{
    std::vector<uint8_t> buf;

    size_t hdr_sz = sizeof(decl_db_hdr);
    size_t decl_sz = sizeof(decl) * db->decl_offset;
    size_t name_sz = db->name_offset;

    /* stage header and data in temporary buffer */
    buf.resize(hdr_sz + decl_sz + name_sz);
    decl_db_hdr *hdr = (decl_db_hdr*)&buf[0];
    memcpy(hdr->magic, decl_db_magic, sizeof(decl_db_magic));
    hdr->decl_entry_count = db->decl_offset;
    hdr->name_table_size = db->name_offset;
    hdr->root_element = db->root_element;
    memcpy(&buf[hdr_sz], db->decl, decl_sz);
    memcpy(&buf[hdr_sz + decl_sz], db->name, name_sz);

    write_file(buf, output_filename);
}
