#undef NDEBUG
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <cinf/model.h>
#include <cinf/dump.h>
#include <cinf/db.h>

#define DB_DIR "build/tmp/" /* array-struct-1.h.refl */

void t3()
{
    decl_db *db;
    decl_ref *_types, *_fields;
    size_t ntypes = 0, nfields = 0;

    db = cinf_db_new();
    assert(db != NULL);

    cinf_db_read_file(db, DB_DIR "array-struct-1.h.refl");

    cinf_source_decls(cinf_root(db), NULL, &ntypes);
    _types = calloc(ntypes, sizeof(decl_ref));
    assert(_types);
    cinf_source_decls(cinf_root(db), _types, &ntypes);
    assert(ntypes == 1);
    assert(cinf_is_struct(_types[0]));

    cinf_struct_fields(_types[0], NULL, &nfields);
    _fields = calloc(nfields, sizeof(decl_ref));
    assert(_fields);
    cinf_struct_fields(_types[0], _fields, &nfields);
    assert(nfields == 2);
    assert(cinf_is_field(_fields[0]));
    assert(cinf_is_field(_fields[1]));
    assert(strcmp(cinf_decl_name(_fields[0]), "a") == 0);
    assert(strcmp(cinf_decl_name(_fields[1]), "b") == 0);
    assert(cinf_is_array(cinf_field_type(_fields[0])));
    assert(cinf_is_array(cinf_field_type(_fields[1])));
    assert(cinf_array_count(cinf_field_type(_fields[0])) == 5);
    assert(cinf_array_count(cinf_field_type(_fields[1])) == 10);
    assert(cinf_type_width(cinf_field_type(_fields[0])) == 160);
    assert(cinf_type_width(cinf_field_type(_fields[1])) == 320);
    free(_fields);

    free(_types);
    cinf_db_destroy(db);
}

int main()
{
    t3();
}
