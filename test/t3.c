#undef NDEBUG
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "cmodel.h"
#include "cdump.h"
#include "cfileio.h"

#define DB_DIR "build/tmp/" /* adjacent-decls-1.h.refl */

void t3()
{
    decl_db *db;
    decl_ref *_types, *_fields;
    size_t ntypes = 0, nfields = 0;

    db = crefl_db_new();
    assert(db != NULL);

    crefl_db_read_file(db, DB_DIR "adjacent-decls-1.h.refl");

    crefl_source_decls(crefl_root(db), NULL, &ntypes);
    _types = calloc(ntypes, sizeof(decl_ref));
    assert(_types);
    crefl_source_decls(crefl_root(db), _types, &ntypes);
    assert(ntypes == 2);
    assert(crefl_is_struct(_types[0]));
    assert(crefl_is_struct(_types[1]));

    crefl_struct_fields(_types[0], NULL, &nfields);
    _fields = calloc(nfields, sizeof(decl_ref));
    assert(_fields);
    crefl_struct_fields(_types[0], _fields, &nfields);
    assert(nfields == 1);
    assert(crefl_is_field(_fields[0]));
    assert(crefl_decl_tag(_fields[0]) == _decl_field);
    assert(strcmp(crefl_decl_name(_fields[0]), "a") == 0);
    assert(crefl_type_width(crefl_field_type(_fields[0])) == 32);
    assert((crefl_decl_props(crefl_field_type(_fields[0])) & _decl_sint) == _decl_sint);
    free(_fields);

    crefl_struct_fields(_types[1], NULL, &nfields);
    _fields = calloc(nfields, sizeof(decl_ref));
    assert(_fields);
    crefl_struct_fields(_types[1], _fields, &nfields);
    assert(nfields == 1);
    assert(crefl_is_field(_fields[0]));
    assert(crefl_decl_tag(_fields[0]) == _decl_field);
    assert(strcmp(crefl_decl_name(_fields[0]), "b") == 0);
    assert(crefl_type_width(crefl_field_type(_fields[0])) == 32);
    assert((crefl_decl_props(crefl_field_type(_fields[0])) & _decl_sint) == _decl_sint);
    free(_fields);

    free(_types);
    crefl_db_destroy(db);
}

int main()
{
    t3();
}
