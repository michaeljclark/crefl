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
    size_t data_sz = hdr->symbol_table_size;

    /* resize buffers */
    if (db->decl_size < decl_sz) {
        free(db->decl);
        db->decl_size = hdr->decl_entry_count;
        db->decl = (decl*)malloc(sizeof(decl) * db->decl_size);
    }
    if (db->data_size < data_sz) {
        free(db->data);
        db->data_size = hdr->symbol_table_size;
        db->data = (char*)malloc(db->data_size);
    }

    /* copy data from temporary buffer */
    db->decl_offset = hdr->decl_entry_count;
    db->data_offset = hdr->symbol_table_size;
    memcpy(db->decl, &buf[hdr_sz], decl_sz);
    memcpy(db->data, &buf[hdr_sz + decl_sz], data_sz);
}

void crefl_write_db(decl_db *db, const char *output_filename)
{
    std::vector<uint8_t> buf;

    size_t hdr_sz = sizeof(decl_db_hdr);
    size_t decl_sz = sizeof(decl) * db->decl_offset;
    size_t data_sz = db->data_offset;

    /* stage header and data in temporary buffer */
    buf.resize(hdr_sz + decl_sz + data_sz);
    decl_db_hdr *hdr = (decl_db_hdr*)&buf[0];
    memcpy(hdr->magic, decl_db_magic, sizeof(decl_db_magic));
    hdr->decl_entry_count = db->decl_offset;
    hdr->symbol_table_size = db->data_offset;
    memcpy(&buf[hdr_sz], db->decl, decl_sz);
    memcpy(&buf[hdr_sz + decl_sz], db->data, data_sz);

    write_file(buf, output_filename);
}
