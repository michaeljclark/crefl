#include <stdio.h>

#include "printer.h"

struct point { float x; float y; };
struct index { unsigned a, b, c; };
struct moon { struct point p; struct index s; };

struct dune { int order[10]; struct moon *m; };

int main(int argc, const char **argv)
{
    decl_db *db = crefl_db_internal();

    struct moon m = { { 3.0f, 4.0f }, { 1, 2, 3 } };
    crefl_print(crefl_type(db, struct moon), &m);

    struct dune d = { { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 }, &m };
    crefl_print(crefl_type(db, struct dune), &d);

    int x = 7;
    crefl_print(crefl_intrinsic(db, _decl_sint, 32), &x);

    crefl_db_destroy(db);
}
