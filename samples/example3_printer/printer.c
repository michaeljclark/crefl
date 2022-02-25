#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <crefl/model.h>
#include <crefl/db.h>
#include "printer.h"

#define array_size(a) (sizeof(a)/sizeof(a[0]))
#define ptr_offset(ptr,offset) ((u8*)ptr + (offset >> 3))

typedef void (*print_fn)(char *buf, size_t len, void *ptr);

void fmt_void(char *buf, size_t len, void *ptr) { snprintf(buf, len, "%s", ""); }
void fmt_u8(char *buf, size_t len, void *ptr) { snprintf(buf, len, "%hhu", *(u8*)ptr); }
void fmt_u16(char *buf, size_t len, void *ptr) { snprintf(buf, len, "%hu", *(u16*)ptr); }
void fmt_u32(char *buf, size_t len, void *ptr) { snprintf(buf, len, "%u", *(u32*)ptr); }
void fmt_u64(char *buf, size_t len, void *ptr) { snprintf(buf, len, "%llu", *(u64*)ptr); }
void fmt_s8(char *buf, size_t len, void *ptr) { snprintf(buf, len, "%hhd", *(s8*)ptr); }
void fmt_s16(char *buf, size_t len, void *ptr) { snprintf(buf, len, "%hd", *(s16*)ptr); }
void fmt_s32(char *buf, size_t len, void *ptr) { snprintf(buf, len, "%d", *(s32*)ptr); }
void fmt_s64(char *buf, size_t len, void *ptr) { snprintf(buf, len, "%lld", *(s64*)ptr); }
void fmt_f32(char *buf, size_t len, void *ptr) { snprintf(buf, len, "%f", *(f32*)ptr); }
void fmt_f64(char *buf, size_t len, void *ptr) { snprintf(buf, len, "%f", *(f64*)ptr); }

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
    for (size_t i = 0; i < array_size(_formats); i++)
        if ((_formats[i].props & props) == _formats[i].props &&
            _formats[i].width == width) return _formats[i];
    return _formats[0];
}

static const char* _pad_depth(size_t depth)
{
    static char buf[256];
    memset(buf, ' ', sizeof(buf));
    if ((depth<<2) >= sizeof(buf)) {
        buf[sizeof(buf)-1] = '\0';
    } else {
        buf[(depth<<2)] = '\0';
    }
    return buf;
}

static void _print_type(decl_ref r, void *ptr, size_t offset, size_t depth);

static void _print_array(decl_ref r, void *ptr, size_t offset, size_t depth)
{
    size_t qty = 1, width;
    do  {
        qty *= crefl_array_count(r);
        r = crefl_array_type(r);
    } while (crefl_is_array(r));
    width = crefl_type_width(r);

    printf("[ ");
    for (size_t i = 0; i < qty; i++) {
        if (i > 0) printf(", ");
        _print_type(r, ptr, offset + width * i, depth + 1);
    }
    printf(" ]");
}

static void _print_pointer(decl_ref r, void *ptr, size_t offset, size_t depth)
{
    char buf[128];
    snprintf(buf, sizeof(buf), "(%s) 0x%016llx", crefl_decl_name(r),
        *(u64*)ptr_offset(ptr, offset));
    printf("%s", buf);
}

static void _print_intrinsic(decl_ref r, void *ptr, size_t offset, size_t depth)
{
    char buf[128];
    _type_fmt fmt = _find_intrinsic_format(r);
    fmt.fn(buf, sizeof(buf), ptr_offset(ptr, offset));
    printf("%s", buf);
}

static void _print_field(decl_ref r, void *ptr, size_t offset, size_t depth)
{
    decl_ref ft = crefl_field_type(r);

    printf("%s : ", crefl_decl_name(r));
    _print_type(ft, ptr, offset, depth);
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

void _print_union(decl_ref r, void *ptr, size_t offset, size_t depth)
{
    size_t nfields = 0;
    crefl_union_fields(r, NULL, &nfields);

    printf("%s%s",
        crefl_decl_name(r),
        nfields > 0 ? " { " : "{}");
    if (nfields == 0) return;

    decl_ref *_fields = calloc(nfields, sizeof(decl_ref));
    assert(_fields);
    crefl_union_fields(r, _fields, &nfields);

    for (size_t j = 0; j < nfields; j++) {
        _print_field(_fields[j], ptr, offset, depth + 1);
    }
    printf("}");
    free(_fields);
}

static void _print_type(decl_ref r, void *ptr, size_t offset, size_t depth)
{
    switch (crefl_decl_tag(r)) {
    case _decl_struct: _print_struct(r, ptr, offset, depth); break;
    case _decl_union: _print_union(r, ptr, offset, depth); break;
    case _decl_array: _print_array(r, ptr, offset, depth); break;
    case _decl_pointer: _print_pointer(r, ptr, offset, depth); break;
    case _decl_intrinsic: _print_intrinsic(r, ptr, offset, depth); break;
    default: break;
    }
}

decl_ref crefl_type_by_name(decl_db *db, const char *name)
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

    /* fix me - currently we can only find struct */
    if (strncmp(name, "struct ", 7) == 0) {
        name +=7;
        for (size_t i = 0; i < ntypes; i++) {
            decl_ref r = _types[i];
            if (crefl_is_struct(r) && strcmp(crefl_decl_name(r), name) == 0) {
                t = r;
                break;
            }
        }
    }

    free(_types);
    free(_sources);

    return t;
}

extern const unsigned char __crefl_main_data[];
extern const size_t __crefl_main_size;

decl_db* crefl_db_internal()
{
    decl_db *db = crefl_db_new();
    crefl_db_read_mem(db, __crefl_main_data, __crefl_main_size);
    return db;
}

void crefl_print(decl_ref r, void *ptr)
{
    _print_type(r, ptr, 0, 0);
    puts("");
}
