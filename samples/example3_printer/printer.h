#pragma once

#include <cinf/model.h>

decl_db* cinf_db_internal();
void cinf_print(decl_ref r, void *ptr);
decl_ref cinf_type_by_name(decl_db *db, const char *name);

#define str(s) #s
#define cinf_type(db,type) cinf_type_by_name(db,str(type))
