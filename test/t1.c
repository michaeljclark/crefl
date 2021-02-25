#undef NDEBUG
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#include "cmodel.h"

void t1()
{
	decl_db *db = crefl_db_new();
	assert(db != NULL);

	decl_ref r1 = crefl_decl_new(db, _decl_intrinsic);
	assert(crefl_tag(r1) == _decl_intrinsic);
	assert(crefl_idx(r1) == 1);

	crefl_ptr(r1)->_name = crefl_name_new(db, "s1");
	assert(strcmp("s1", crefl_decl_name(r1)) == 0);

	decl_ref r2 = crefl_decl_new(db, _decl_struct);
	assert(crefl_tag(r2) == _decl_struct);
	assert(crefl_idx(r2) == 2);

	crefl_ptr(r2)->_name = crefl_name_new(db, "s2");
	assert(strcmp("s2", crefl_decl_name(r2)) == 0);

	crefl_db_destroy(db);
}

int main()
{
	t1();
}
