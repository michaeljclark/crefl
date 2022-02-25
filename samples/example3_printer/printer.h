#pragma once

#include <crefl/model.h>

decl_db* crefl_db_internal();
void crefl_print(decl_ref r, void *ptr);
decl_ref crefl_type_by_name(decl_db *db, const char *name);

#define str(s) #s
#define crefl_type(db,type) crefl_type_by_name(db,str(type))
