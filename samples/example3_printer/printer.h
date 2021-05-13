#pragma once

#include "cmodel.h"

decl_db* crefl_db_internal();
void crefl_print(decl_ref r, void *ptr);
decl_ref _find_type(decl_db *db, const char *name);

#define str(s) #s
#define crefl_type(db,type) _find_type(db,str(type))
