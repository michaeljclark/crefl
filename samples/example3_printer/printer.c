#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "cmodel.h"
#include "cfileio.h"
#include "cprinter.h"

#define array_size(a) (sizeof(a)/sizeof(a[0]))

typedef const char* (*print_fn)(char *buf, size_t len, void *ptr);

const char* fmt_void(char *buf, size_t len, void *ptr) { snprintf(buf, len, "%s", ""); }
const char* fmt_u8(char *buf, size_t len, void *ptr) { snprintf(buf, len, "%hhu", *(u8*)ptr); }
const char* fmt_u16(char *buf, size_t len, void *ptr) { snprintf(buf, len, "%hu", *(u16*)ptr); }
const char* fmt_u32(char *buf, size_t len, void *ptr) { snprintf(buf, len, "%u", *(u32*)ptr); }
const char* fmt_u64(char *buf, size_t len, void *ptr) { snprintf(buf, len, "%llu", *(u64*)ptr); }
const char* fmt_s8(char *buf, size_t len, void *ptr) { snprintf(buf, len, "%hhd", *(s8*)ptr); }
const char* fmt_s16(char *buf, size_t len, void *ptr) { snprintf(buf, len, "%hd", *(s16*)ptr); }
const char* fmt_s32(char *buf, size_t len, void *ptr) { snprintf(buf, len, "%d", *(s32*)ptr); }
const char* fmt_s64(char *buf, size_t len, void *ptr) { snprintf(buf, len, "%lld", *(s64*)ptr); }
const char* fmt_f32(char *buf, size_t len, void *ptr) { snprintf(buf, len, "%f", *(f32*)ptr); }
const char* fmt_f64(char *buf, size_t len, void *ptr) { snprintf(buf, len, "%f", *(f64*)ptr); }

typedef struct { decl_set props; size_t width; print_fn fn; } _type_fmt;

static _type_fmt _formats[] = {
    { _decl_void, 0, fmt_void },
    { _decl_uint, 8, fmt_u8 },
    { _decl_uint, 16, fmt_u16 },
    { _decl_uint, 32, fmt_u32 },
    { _decl_uint, 64, fmt_u64 },
    { _decl_sint, 8, fmt_s8 },
    { _decl_sint, 16, fmt_s16 },
    { _decl_sint, 32, fmt_s32 },
    { _decl_sint, 64, fmt_s64 },
    { _decl_float, 32, fmt_f32 },
    { _decl_float, 64, fmt_f64 },
};

static _type_fmt _find_intrinsic_format(decl_ref r)
{
    size_t width = crefl_intrinsic_width(r);
    decl_set props = crefl_decl_props(r);
    for (size_t i = 0; i < array_size(_formats); i++) {
        if ((_formats[i].props & props) == _formats[i].props && _formats[i].width == width) {
            return _formats[i];
        }
    }
    return _formats[0];
}

static void _print_intrinsic(decl_ref r, void *ptr, size_t offset, size_t depth)
{
    char buf[128];
    _type_fmt fmt = _find_intrinsic_format(r);
    fmt.fn(buf, sizeof(buf), (char *)ptr + (offset >> 3));
    printf("%s", buf);
}

static const char* _pad_depth(size_t depth)
{
    static char buf[256];
    memset(buf, ' ', sizeof(buf));
    if ((depth<<2) >= 256) buf[255] = '\0';
    else buf[(depth<<2)] = '\0';
    return buf;
}

void _print_struct(decl_ref r, void *ptr, size_t offset, size_t depth);

static void _print_field(decl_ref r, void *ptr, size_t offset, size_t depth)
{
    decl_ref ft = crefl_field_type(r);

    printf("%s : ", crefl_decl_name(r));

    switch (crefl_decl_tag(ft)) {
    case _decl_struct: _print_struct(ft, ptr, offset, depth); break;
    case _decl_intrinsic: _print_intrinsic(ft, ptr, offset, depth); break;
    default: break;
    }
    printf("; ");
}

void _print_struct(decl_ref r, void *ptr, size_t offset, size_t depth)
{
    size_t nfields = 0;
    crefl_struct_fields_offsets(r, NULL, NULL, &nfields);

    printf("%s%s",
        crefl_decl_name(r),
        nfields > 0 ? " { " : "{}");
    if (nfields == 0) return;

    decl_ref *_fields = calloc(nfields, sizeof(decl_ref));
    size_t *_offsets = calloc(nfields, sizeof(size_t));
    assert(_fields);
    crefl_struct_fields_offsets(r, _fields, _offsets, &nfields);

    for (size_t j = 0; j < nfields - 1; j++) {
        _print_field(_fields[j], ptr, _offsets[j] + offset, depth + 1);
    }
    printf("}");
    free(_fields);
    free(_offsets);
}

decl_ref _find_type(decl_db *db, const char *name)
{
    decl_ref t = { db, 0 };

    size_t nsources = 0;
    crefl_archive_sources(crefl_root(db), NULL, &nsources);
    assert(nsources == 1);
    decl_ref *_sources = calloc(nsources, sizeof(decl_ref));
    assert(_sources);
    crefl_archive_sources(crefl_root(db), _sources, &nsources);

    size_t ntypes = 0;
    crefl_source_decls(_sources[0], NULL, &ntypes);
    decl_ref *_types = calloc(ntypes, sizeof(decl_ref));
    assert(_types);
    crefl_source_decls(_sources[0], _types, &ntypes);

    if (strncmp(name, "struct ", 7) == 0) name +=7;

    for (size_t i = 0; i < ntypes; i++) {
        decl_ref r = _types[i];
        if (crefl_is_struct(r) && strcmp(crefl_decl_name(r), name) == 0) {
            t = r;
            break;
        }
    }

    free(_types);
    free(_sources);

    return t;
}

extern const char __crefl_main_data[];
extern const size_t __crefl_main_size;

decl_db* crefl_db_internal()
{
    decl_db *db = crefl_db_new();
    crefl_db_read_mem(db, __crefl_main_data, __crefl_main_size);
    return db;
}

void crefl_print(decl_ref r, void *ptr)
{
    if (crefl_is_struct(r)) {
        _print_struct(r, ptr, 0, 0);
        printf("\n");
    }
}
