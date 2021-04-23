#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

#include "cmodel.h"
#include "cfileio.h"

extern const char __crefl_main_data[];
extern const size_t __crefl_main_size;

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
        size_t nfields = 0;
        if (crefl_is_struct(_types[i])) {
            printf("%s %s : %zu\n",
                crefl_tag_name(crefl_decl_tag(_types[i])),
                crefl_decl_name(_types[i]),
                crefl_type_width(_types[i]));
            crefl_struct_fields(_types[i], NULL, &nfields);
            decl_ref *_fields = calloc(nfields, sizeof(decl_ref));
            assert(_fields);
            crefl_struct_fields(_types[i], _fields, &nfields);
            for (size_t j = 0; j < nfields; j++) {
                printf("\t%s %s : %zu\n",
                    crefl_tag_name(crefl_decl_tag(_fields[j])),
                    crefl_decl_name(_fields[j]),
                    crefl_type_width(_fields[j]));
            }
        }
    }

    crefl_db_destroy(db);
}
