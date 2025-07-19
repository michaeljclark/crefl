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
    decl_ref *types, *fields;
    size_t ntypes = 0, nfields = 0;

    db = cinf_db_new();
    assert(db != NULL);

    cinf_db_read_file(db, DB_DIR "array-struct-1.h.refl");

    cinf_source_decls(cinf_root(db), NULL, &ntypes);
    types = calloc(ntypes, sizeof(decl_ref));
    assert(types);
    cinf_source_decls(cinf_root(db), types, &ntypes);
    assert(ntypes == 1);
    assert(cinf_is_struct(types[0]));

    cinf_struct_fields(types[0], NULL, &nfields);
    fields = calloc(nfields, sizeof(decl_ref));
    assert(fields);
    cinf_struct_fields(types[0], fields, &nfields);
    assert(nfields == 2);
    assert(cinf_is_field(fields[0]));
    assert(cinf_is_field(fields[1]));
    assert(strcmp(cinf_decl_name(fields[0]), "a") == 0);
    assert(strcmp(cinf_decl_name(fields[1]), "b") == 0);
    assert(cinf_is_array(cinf_field_type(fields[0])));
    assert(cinf_is_array(cinf_field_type(fields[1])));
    assert(cinf_array_count(cinf_field_type(fields[0])) == 5);
    assert(cinf_array_count(cinf_field_type(fields[1])) == 10);
    assert(cinf_type_width(cinf_field_type(fields[0])) == 160);
    assert(cinf_type_width(cinf_field_type(fields[1])) == 320);
    free(fields);

    free(types);
    cinf_db_destroy(db);
}

int main()
{
    t3();
}
