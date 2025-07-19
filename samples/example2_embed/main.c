#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <cinf/model.h>
#include <cinf/db.h>

extern const unsigned char __cinf_main_data[];
extern const size_t __cinf_main_size;

static const char* _pad_depth(size_t depth)
{
    static char buf[256];
    memset(buf, ' ', sizeof(buf));
    if ((depth<<2) >= 256) buf[255] = '\0';
    else buf[(depth<<2)] = '\0';
    return buf;
}

static char * _decl_name(decl_ref r)
{
    static char buf[256];
    if (cinf_is_none(r)) {
        snprintf(buf, sizeof(buf), "(none)");
    } else if (cinf_is_struct(r) || cinf_is_union(r)) {
        if (cinf_decl_has_name(r)) {
            snprintf(buf, sizeof(buf), "%s %s",
                cinf_tag_name(cinf_decl_tag(r)), cinf_decl_name(r));
        } else {
            snprintf(buf, sizeof(buf), "%s /* anonymous id=%u */",
                cinf_tag_name(cinf_decl_tag(r)), cinf_decl_idx(r));
        }
    } else {
        if (cinf_decl_has_name(r)) {
            snprintf(buf, sizeof(buf), "%s", cinf_decl_name(r));
        } else {
            snprintf(buf, sizeof(buf), "%s /* anonymous id=%u */",
                cinf_tag_name(cinf_decl_tag(r)), cinf_decl_idx(r));
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
    decl_ref ft = cinf_typedef_type(r);

    printf("%s%s ", _pad_depth(depth), cinf_tag_name(cinf_decl_tag(r)));

    switch (cinf_decl_tag(ft)) {
    case _decl_struct: _print_struct(ft, depth); break;
    case _decl_union: _print_union(ft, depth); break;
    default:
        printf("%s%s", _pad_depth(depth), _decl_name(ft));
        break;
    }

    printf(" /* size=%zu */ %s", cinf_type_width(ft), cinf_decl_name(r));
}

static void _print_field(decl_ref r, size_t depth)
{
    decl_ref ft = cinf_field_type(r);

    switch (cinf_decl_tag(ft)) {
    case _decl_struct: _print_struct(ft, depth); break;
    case _decl_union: _print_union(ft, depth); break;
    default:
        printf("%s%s", _pad_depth(depth), _decl_name(ft));
        break;
    }

    printf(" /* size=%zu */ %s", cinf_type_width(r), cinf_decl_name(r));
}

static void _print_struct(decl_ref r, size_t depth)
{
    size_t nfields = 0;
    cinf_struct_fields(r, NULL, &nfields);

    printf("%s%s /* size=%zu */%s", _pad_depth(depth), _decl_name(r),
        cinf_type_width(r), nfields > 0 ? " {\n" : "");

    if (nfields == 0) return;

    decl_ref *_fields = calloc(nfields, sizeof(decl_ref));
    assert(_fields);
    cinf_struct_fields(r, _fields, &nfields);

    for (size_t j = 0; j < nfields; j++) {
        _print_field(_fields[j], depth + 1);
        printf(";\n");
    }
    printf("%s}", _pad_depth(depth));
    free(_fields);
}

static void _print_union(decl_ref r, size_t depth)
{
    size_t nfields = 0;
    cinf_union_fields(r, NULL, &nfields);

    printf("%s%s /* size=%zu */%s", _pad_depth(depth), _decl_name(r),
        cinf_type_width(r), nfields > 0 ? " {\n" : "");
    if (nfields == 0) return;

    decl_ref *_fields = calloc(nfields, sizeof(decl_ref));
    assert(_fields);
    cinf_union_fields(r, _fields, &nfields);

    for (size_t j = 0; j < nfields; j++) {
        _print_field(_fields[j], depth + 1);
        printf(";\n");
    }
    printf("%s}", _pad_depth(depth));
    free(_fields);
}

static void _print_function(decl_ref r, size_t depth)
{
    size_t nparams = 0;
    cinf_function_parameters(r, NULL, &nparams);

    decl_ref *_params = calloc(nparams, sizeof(decl_ref));
    assert(_params);
    cinf_function_parameters(r, _params, &nparams);

    if (nparams > 0) {
        decl_ref pt = cinf_parameter_type(_params[0]);
        printf("%s%s ", _pad_depth(depth), _decl_name(pt));
    } else {
        printf("%s", _pad_depth(depth));
    }
    printf("%s(", _decl_name(r));
    for (size_t j = 1; j < nparams; j++) {
        decl_ref pt = cinf_parameter_type(_params[j]);
        if (j > 1) printf(", ");
        printf("%s %s", cinf_decl_name(pt), cinf_decl_name(_params[j]));
    }
    printf(")");
    free(_params);
}

static void _print(decl_ref r, size_t depth)
{
    if (!cinf_decl_has_name(r)) return;
    if (cinf_is_struct(r)) _print_struct(r, depth);
    else if (cinf_is_union(r)) _print_union(r, depth);
    else if (cinf_is_function(r)) _print_function(r, depth);
    else if (cinf_is_typedef(r)) _print_typedef(r, depth);
    else if (cinf_is_field(r)) _print_field(r, depth);
    else return;
    printf(";\n");
}

int main(int argc, const char **argv)
{
    decl_db *db = cinf_db_new();
    cinf_db_read_mem(db, __cinf_main_data, __cinf_main_size);

    size_t nsources = 0;
    cinf_archive_sources(cinf_root(db), NULL, &nsources);
    assert(nsources == 1);
    decl_ref *_sources = calloc(nsources, sizeof(decl_ref));
    assert(_sources);
    cinf_archive_sources(cinf_root(db), _sources, &nsources);

    size_t ntypes = 0;
    cinf_source_decls(_sources[0], NULL, &ntypes);
    decl_ref *_types = calloc(ntypes, sizeof(decl_ref));
    assert(_types);
    cinf_source_decls(_sources[0], _types, &ntypes);

    for (size_t i = 0; i < ntypes; i++) {
        _print(_types[i], 0);
    }
    free(_types);
    free(_sources);

    cinf_db_destroy(db);
}
