#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "cmodel.h"
#include "cfileio.h"

extern const char __crefl_main_data[];
extern const size_t __crefl_main_size;

static const char* _pad_depth(size_t depth)
{
    static char buf[256];
    memset(buf, ' ', sizeof(buf));
    if ((depth<<2) >= 256) buf[255] = '\0';
    else buf[(depth<<2)] = '\0';
    return buf;
}

static char * _type_name(decl_ref r)
{
    static char buf[256];
    if (crefl_is_none(r)) {
        snprintf(buf, sizeof(buf), "(none)");
    } else if (crefl_is_struct(r) || crefl_is_union(r)) {
        if (crefl_decl_has_name(r)) {
            snprintf(buf, sizeof(buf), "%s %s",
                crefl_tag_name(crefl_decl_tag(r)),
                crefl_decl_name(r));
        } else {
            snprintf(buf, sizeof(buf), "%s /* anonymous id=%u */",
                crefl_tag_name(crefl_decl_tag(r)),
                crefl_decl_idx(r));
        }
    } else {
        if (crefl_decl_has_name(r)) {
            snprintf(buf, sizeof(buf), "%s",
                crefl_decl_name(r));
        } else {
            snprintf(buf, sizeof(buf), "%s /* anonymous id=%u */",
                crefl_tag_name(crefl_decl_tag(r)),
                crefl_decl_idx(r));
        }
    }
    return buf;
}

static void _print(decl_ref r, size_t depth);
static void _print_typedef(decl_ref r, size_t depth);
static void _print_field(decl_ref r, size_t depth);
static void _print_struct(decl_ref r, size_t depth);
static void _print_union(decl_ref r, size_t depth);

static void _print_typedef(decl_ref r, size_t depth)
{
    decl_ref ft = crefl_typedef_type(r);

    printf("%s%s ",
        _pad_depth(depth),
        crefl_tag_name(crefl_decl_tag(r)));

    switch (crefl_decl_tag(ft)) {
    case _decl_struct: _print_struct(ft, depth); break;
    case _decl_union: _print_union(ft, depth); break;
    default:
        printf("%s%s", _pad_depth(depth), _type_name(ft));
        break;
    }

    printf(" /* size=%zu */ %s",
        crefl_type_width(ft),
        crefl_decl_name(r));
}

static void _print_field(decl_ref r, size_t depth)
{
    decl_ref ft = crefl_field_type(r);

    switch (crefl_decl_tag(ft)) {
    case _decl_struct: _print_struct(ft, depth); break;
    case _decl_union: _print_union(ft, depth); break;
    default:
        printf("%s%s", _pad_depth(depth), _type_name(ft));
        break;
    }

    printf(" /* size=%zu */ %s",
        crefl_type_width(r),
        crefl_decl_name(r));
}

static void _print_struct(decl_ref r, size_t depth)
{
    size_t nfields = 0;
    crefl_struct_fields(r, NULL, &nfields);

    printf("%s%s /* size=%zu */%s",
        _pad_depth(depth),
        _type_name(r),
        crefl_type_width(r), nfields > 0 ? " {\n" : "");
    if (nfields == 0) return;

    decl_ref *_fields = calloc(nfields, sizeof(decl_ref));
    assert(_fields);
    crefl_struct_fields(r, _fields, &nfields);

    for (size_t j = 0; j < nfields; j++) {
        _print_field(_fields[j], depth + 1);
        printf(";\n");
    }
    printf("%s}", _pad_depth(depth));
}

static void _print_union(decl_ref r, size_t depth)
{
    size_t nfields = 0;
    crefl_union_fields(r, NULL, &nfields);

    printf("%s%s /* size=%zu */%s",
        _pad_depth(depth),
        _type_name(r),
        crefl_type_width(r), nfields > 0 ? " {\n" : "");
    if (nfields == 0) return;

    decl_ref *_fields = calloc(nfields, sizeof(decl_ref));
    assert(_fields);
    crefl_union_fields(r, _fields, &nfields);

    for (size_t j = 0; j < nfields; j++) {
        _print_field(_fields[j], depth + 1);
        printf(";\n");
    }
    printf("%s}", _pad_depth(depth));
}

static void _print_function(decl_ref r, size_t depth)
{
    size_t nparams = 0;
    crefl_function_params(r, NULL, &nparams);

    decl_ref *_params = calloc(nparams, sizeof(decl_ref));
    assert(_params);
    crefl_function_params(r, _params, &nparams);

    if (nparams > 0) {
        decl_ref pt = crefl_param_type(_params[0]);
        printf("%s%s ", _pad_depth(depth), _type_name(pt));
    } else {
        printf("%s", _pad_depth(depth));
    }
    printf("%s(", _type_name(r));
    for (size_t j = 1; j < nparams; j++) {
        decl_ref pt = crefl_param_type(_params[j]);
        if (j > 1) printf(", ");
        printf("%s %s", crefl_decl_name(pt), crefl_decl_name(_params[j]));
    }
    printf(")");
}

static void _print(decl_ref r, size_t depth)
{
    if (!crefl_decl_has_name(r)) return;
    if (crefl_is_struct(r)) _print_struct(r, depth);
    else if (crefl_is_union(r)) _print_union(r, depth);
    else if (crefl_is_function(r)) _print_function(r, depth);
    else if (crefl_is_typedef(r)) _print_typedef(r, depth);
    else if (crefl_is_field(r)) _print_field(r, depth);
    else return;
    printf(";\n");
}

int main(int argc, const char **argv)
{
    decl_db *db = crefl_db_new();
    crefl_db_read_mem(db, __crefl_main_data, __crefl_main_size);

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

    for (size_t i = 0; i < ntypes; i++) {
        _print(_types[i], 0);
    }

    crefl_db_destroy(db);
}
