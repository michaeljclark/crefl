#undef NDEBUG
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "cmodel.h"
#include "cdump.h"
#include "cfileio.h"

#define DB_DIR "build/tmp/"

/* adjacent-structs-1.h.refl */

void t3()
{
    decl_db *db;
    decl_ref *_types, *_fields;
    size_t ntypes = 0, nfields = 0;

    db = crefl_db_new();
    assert(db != NULL);

    crefl_db_read_file(db, DB_DIR "adjacent-structs-1.h.refl");

    crefl_list_types(db, NULL, &ntypes);
    _types = calloc(ntypes, sizeof(decl_ref));
    assert(_types);
    crefl_list_types(db, _types, &ntypes);
    assert(ntypes == 2);
    assert(crefl_is_struct(_types[0]));
    assert(crefl_is_struct(_types[1]));

    crefl_struct_fields(_types[0], NULL, &nfields);
    _fields = calloc(nfields, sizeof(decl_ref));
    assert(_fields);
    crefl_struct_fields(_types[0], _fields, &nfields);
    assert(nfields == 6);
    assert(crefl_decl_tag(_fields[0]) == _decl_field);
    assert(crefl_decl_tag(_fields[1]) == _decl_field);
    assert(crefl_decl_tag(_fields[2]) == _decl_field);
    assert(crefl_decl_tag(_fields[3]) == _decl_field);
    assert(crefl_decl_tag(_fields[4]) == _decl_field);
    assert(crefl_decl_tag(_fields[5]) == _decl_field);
    assert(strcmp(crefl_decl_name(_fields[0]), "a") == 0);
    assert(strcmp(crefl_decl_name(_fields[1]), "b") == 0);
    assert(strcmp(crefl_decl_name(_fields[2]), "bu") == 0);
    assert(strcmp(crefl_decl_name(_fields[3]), "h") == 0);
    assert(strcmp(crefl_decl_name(_fields[4]), "hu") == 0);
    assert(strcmp(crefl_decl_name(_fields[5]), "w") == 0);
    free(_fields);

    crefl_struct_fields(_types[1], NULL, &nfields);
    _fields = calloc(nfields, sizeof(decl_ref));
    assert(_fields);
    crefl_struct_fields(_types[1], _fields, &nfields);
    assert(nfields == 6);
    assert(crefl_decl_tag(_fields[0]) == _decl_field);
    assert(crefl_decl_tag(_fields[1]) == _decl_field);
    assert(crefl_decl_tag(_fields[2]) == _decl_field);
    assert(crefl_decl_tag(_fields[3]) == _decl_field);
    assert(crefl_decl_tag(_fields[4]) == _decl_field);
    assert(crefl_decl_tag(_fields[5]) == _decl_field);
    assert(strcmp(crefl_decl_name(_fields[0]), "wu") == 0);
    assert(strcmp(crefl_decl_name(_fields[1]), "d") == 0);
    assert(strcmp(crefl_decl_name(_fields[2]), "du") == 0);
    assert(strcmp(crefl_decl_name(_fields[3]), "f") == 0);
    assert(strcmp(crefl_decl_name(_fields[4]), "g") == 0);
    assert(strcmp(crefl_decl_name(_fields[5]), "p") == 0);
    free(_fields);

    free(_types);
    crefl_db_destroy(db);
}

int main()
{
    t3();
}
