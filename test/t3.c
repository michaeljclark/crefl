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

void t3()
{
    decl_db *db = crefl_db_new();
    assert(db != NULL);

    crefl_db_read_file(db, DB_DIR "adjacent-structs-1.h.refl");

    size_t ntypes = 0;
    crefl_list_types(db, NULL, &ntypes);
    decl_ref *_types = calloc(ntypes, sizeof(decl_ref));
    crefl_list_types(db, _types, &ntypes);

    for (size_t i = 0; i < ntypes; i++) {
        size_t nfields = 0;
        if (crefl_is_struct(_types[i])) {
            crefl_struct_fields(_types[i], NULL, &nfields);
            decl_ref *_fields = calloc(nfields, sizeof(decl_ref));
            crefl_struct_fields(_types[i], _fields, &nfields);
            assert(nfields == 6);
            for (size_t j = 0; j < nfields; j++) {
                assert(crefl_is_field(_fields[j]));
            }
        }
    }

    crefl_db_destroy(db);
}

int main()
{
    t3();
}
