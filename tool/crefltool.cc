#include <cstdio>

#include "cmodel.h"
#include "cfileio.h"

int main(int argc, const char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <filename.refl>", argv[0]);
    }

    decl_db *db = crefl_db_new();
    crefl_read_db(db, argv[1]);
    crefl_db_dump(db);
    crefl_db_destroy(db);
}
