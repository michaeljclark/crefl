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

/*
 * internal file io helpers
 */

static size_t crefl_read_file(std::vector<uint8_t> &buf, const char* filename)
{
    FILE *f;
    struct stat statbuf;
    if ((f = fopen(filename, "rb")) == nullptr) {
        fprintf(stderr, "fopen: %s\n", strerror(errno));
        return -1;
    }
    if (fstat(fileno(f), &statbuf) < 0) {
        fprintf(stderr, "fstat: %s\n", strerror(errno));
        return -1;
    }
    buf.resize(statbuf.st_size);
    size_t len = fread(buf.data(), 1, buf.size(), f);
    fclose(f);

    return buf.size() == len ? 0 : -1;
}

static size_t crefl_write_file(std::vector<uint8_t> &buf, const char* filename)
{
    FILE *f;
    if ((f = fopen(filename, "wb")) == nullptr) {
        fprintf(stderr, "fopen: %s\n", strerror(errno));
        return -1;
    }
    size_t len = fwrite(buf.data(), 1, buf.size(), f);
    fclose(f);

    return buf.size() == len ? 0 : -1;
}

/*
 * decl db magic and size
 */

int crefl_db_magic(const void *addr)
{
    return memcmp(addr, decl_db_magic, sizeof(decl_db_magic));
}

size_t crefl_db_size(decl_db *db)
{
    size_t hdr_sz = sizeof(decl_db_hdr);
    size_t decl_sz = sizeof(decl_node) * (db->decl_offset - db->decl_builtin);
    size_t name_sz = db->name_offset - db->name_builtin;
    size_t total_sz = hdr_sz + decl_sz + name_sz;

    return total_sz;
}

/*
 * decl db memory io
 */

int crefl_db_read_mem(decl_db *db, const uint8_t *buf, size_t input_sz)
{
    if (input_sz < sizeof(decl_db_hdr)) {
        fprintf(stderr, "crefl: *** error: header too short\n");
        return -1;
    }
    if (crefl_db_magic(buf) != 0) {
        fprintf(stderr, "crefl: *** error: invalid magic\n");
        return -1;
    }

    decl_db_hdr *hdr = (decl_db_hdr*)&buf[0];
    size_t hdr_sz = sizeof(decl_db_hdr);
    size_t decl_sz = sizeof(decl_node) * hdr->decl_entry_count;
    size_t name_sz = hdr->name_table_size;
    size_t root_idx = hdr->root_element;

    /*
     * for space compactness we elide builtin types from the output.
     * initialize builtin types then check the first element is root.
     * check ensures we don't load a db if the defaults have changed.
     *
     * note: this implies a restriction that the first element is the root
     */
    crefl_db_defaults(db);
    if (db->decl_offset != root_idx || db->decl_builtin != root_idx) {
        fprintf(stderr, "crefl: *** error: incompatible builtin types\n");
        return -1;
    }

    /* resize buffers */
    if (db->decl_size < decl_sz) {
        db->decl_size = decl_sz;
        db->decl = (decl_node*)realloc(db->decl, sizeof(decl_node) * db->decl_size);
    }
    if (db->name_size < name_sz) {
        db->name_size = name_sz;
        db->name = (char*)realloc(db->name, db->name_size);
    }

    /* append decls from temporary buffer */
    memcpy(db->decl + db->decl_offset, &buf[hdr_sz], decl_sz);
    db->decl_offset += hdr->decl_entry_count;

    /* append names from temporary buffer */
    memcpy(db->name + db->name_offset, &buf[hdr_sz + decl_sz], name_sz);
    db->name_offset += hdr->name_table_size;
    db->root_element = hdr->root_element;

    /* verify that node and name links are within bounds. */
    for (decl_id i = 0; i < db->decl_offset; i++) {
        decl_node *d = db->decl + i;
        if (d->_link >= db->decl_offset) {
            fprintf(stderr, "crefl: *** error: decl " fmt_ID
                " link " fmt_ID " out of bounds\n", i, d->_link);
            return -1;
        }
        if (d->_next >= db->decl_offset) {
            fprintf(stderr, "crefl: *** error: decl " fmt_ID
                " next " fmt_ID " out of bounds\n", i, d->_next);
            return -1;
        }
        if (d->_attr >= db->decl_offset) {
            fprintf(stderr, "crefl: *** error: decl " fmt_ID
                " attr " fmt_ID " out of bounds\n", i, d->_attr);
            return -1;
        }
        if (d->_name >= db->name_offset) {
            fprintf(stderr, "crefl: *** error: decl " fmt_ID
                " name " fmt_ID " out of bounds\n", i, d->_name);
            return -1;
        }
    }

    return 0;
}

int crefl_db_write_mem(decl_db *db, uint8_t *buf, size_t output_sz)
{
    size_t hdr_sz = sizeof(decl_db_hdr);
    size_t decl_sz = sizeof(decl_node) * (db->decl_offset - db->decl_builtin);
    size_t name_sz = db->name_offset - db->name_builtin;
    size_t total_sz = hdr_sz + decl_sz + name_sz;

    if (total_sz > output_sz) return -1;

    decl_db_hdr *hdr = (decl_db_hdr*)buf;
    memcpy(hdr->magic, decl_db_magic, sizeof(decl_db_magic));
    hdr->decl_entry_count = db->decl_offset - db->decl_builtin;
    hdr->name_table_size = name_sz;
    hdr->root_element = db->root_element;
    memcpy(&buf[hdr_sz], db->decl + db->decl_builtin, decl_sz);
    memcpy(&buf[hdr_sz + decl_sz], db->name + db->name_builtin, name_sz);

    return 0;
}

/*
 * decl db file io
 */

int crefl_db_read_file(decl_db *db, const char *input_filename)
{
    std::vector<uint8_t> buf;
    int ret = crefl_read_file(buf, input_filename);
    if (ret != 0) return ret;
    return crefl_db_read_mem(db, buf.data(), buf.size());
}

int crefl_db_write_file(decl_db *db, const char *output_filename)
{
    std::vector<uint8_t> buf(crefl_db_size(db));
    int ret = crefl_db_write_mem(db, buf.data(), buf.size());
    if (ret != 0) return ret;
    return crefl_write_file(buf, output_filename);
}
