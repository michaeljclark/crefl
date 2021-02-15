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

	decl_ref r1 = crefl_new(db, _decl_intrinsic, _top);
	assert(crefl_tag(r1) == _decl_intrinsic);
	assert(crefl_idx(r1) == 1);

	const char *s1 = crefl_name_new(r1, "s1");
	assert(strcmp("s1", s1) == 0);
	assert(strcmp("s1", crefl_name(r1)) == 0);

	decl_ref r2 = crefl_new(db, _decl_struct, _top);
	assert(crefl_tag(r2) == _decl_struct);
	assert(crefl_idx(r2) == 2);

	const char *s2 = crefl_name_new(r2, "s2");
	assert(strcmp("s2", s2) == 0);
	assert(strcmp("s2", crefl_name(r2)) == 0);

	crefl_db_destroy(db);
}

int main()
{
	t1();
}
