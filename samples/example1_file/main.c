#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

#include <crefl/model.h>
#include <crefl/db.h>

int main(int argc, const char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "error: usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    decl_db *db = crefl_db_new();
    crefl_db_read_file(db, argv[1]);

    size_t ntypes = 0;
    crefl_source_decls(crefl_root(db), NULL, &ntypes);
    decl_ref *_types = calloc(ntypes, sizeof(decl_ref));
    assert(_types);
    crefl_source_decls(crefl_root(db), _types, &ntypes);

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
