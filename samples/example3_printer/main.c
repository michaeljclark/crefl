#include <stdio.h>

#include "printer.h"

struct point { float x; float y; };
struct index { unsigned a, b, c; };
struct moon { struct point p; struct index s; };
struct dune { int order[10]; struct moon *m; };

int main(int argc, const char **argv)
{
    decl_db *db = cinf_db_internal();

    struct moon m = { { 3.0f, 4.0f }, { 1, 2, 3 } };
    cinf_print(cinf_type(db, struct moon), &m);

    struct dune d = { { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 }, &m };
    cinf_print(cinf_type(db, struct dune), &d);

    int x = 7;
    cinf_print(cinf_intrinsic(db, _decl_int, 32), &x);

    cinf_db_destroy(db);
}
