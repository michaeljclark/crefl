#undef NDEBUG
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#include "cmodel.h"

void t1()
{
	decl_db *db = decl_db_new();
	assert(db != NULL);

	decl_ref r1 = decl_new(db, _decl_intrinsic, _top);
	assert(decl_tag(decl_ptr(r1)) == _decl_intrinsic);
	assert(decl_ref_tag(r1) == _decl_intrinsic);
	assert(decl_ref_idx(r1) == 1);

	const char *s1 = decl_name_new(r1, "s1");
	assert(strcmp("s1", s1) == 0);
	assert(strcmp("s1", decl_name(r1)) == 0);

	decl_ref r2 = decl_new(db, _decl_struct, _top);
	assert(decl_tag(decl_ptr(r2)) == _decl_struct);
	assert(decl_ref_tag(r2) == _decl_struct);
	assert(decl_ref_idx(r2) == 2);

	const char *s2 = decl_name_new(r2, "s2");
	assert(strcmp("s2", s2) == 0);
	assert(strcmp("s2", decl_name(r2)) == 0);

	decl_db_destroy(db);
}

int main()
{
	t1();
}
