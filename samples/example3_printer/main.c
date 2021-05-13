#include <stdio.h>

#include "printer.h"

struct point { float x; float y; };
struct index { unsigned a, b, c; };
struct moon { struct point p; struct index s; };

int main(int argc, const char **argv)
{
    decl_db *db = crefl_db_internal();

    struct moon m = { { 3.0f, 4.0f }, { 1, 2, 3 } };
    crefl_print(crefl_type(db, struct moon), &m);

    crefl_db_destroy(db);
}
