#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

#include <cinf/model.h>
#include <cinf/db.h>

int main(int argc, const char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "error: usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    decl_db *db = cinf_db_new();
    cinf_db_read_file(db, argv[1]);

    size_t ntypes = 0;
    cinf_source_decls(cinf_root(db), NULL, &ntypes);
    decl_ref *_types = calloc(ntypes, sizeof(decl_ref));
    assert(_types);
    cinf_source_decls(cinf_root(db), _types, &ntypes);

    for (size_t i = 0; i < ntypes; i++) {
        size_t nfields = 0;
        if (cinf_is_struct(_types[i])) {
            printf("%s %s : %zu\n",
                cinf_tag_name(cinf_decl_tag(_types[i])),
                cinf_decl_name(_types[i]),
                cinf_type_width(_types[i]));
            cinf_struct_fields(_types[i], NULL, &nfields);
            decl_ref *_fields = calloc(nfields, sizeof(decl_ref));
            assert(_fields);
            cinf_struct_fields(_types[i], _fields, &nfields);
            for (size_t j = 0; j < nfields; j++) {
                printf("\t%s %s : %zu\n",
                    cinf_tag_name(cinf_decl_tag(_fields[j])),
                    cinf_decl_name(_fields[j]),
                    cinf_type_width(_fields[j]));
            }
        }
    }

    cinf_db_destroy(db);
}
