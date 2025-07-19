#undef NDEBUG
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#include <cinf/model.h>

/* cinf_db_new, cinf_decl_new, cinf_name_new, cinf_db_destroy */

void t1()
{
	decl_db *db = cinf_db_new();
	assert(db != NULL);

	decl_ref r1 = cinf_decl_new(db, decl_intrinsic);
	assert(cinf_decl_tag(r1) == decl_intrinsic);
	assert(cinf_decl_idx(r1) == 1);

	cinf_decl_ptr(r1)->name = cinf_name_new(db, "s1");
	assert(strcmp("s1", cinf_decl_name(r1)) == 0);

	decl_ref r2 = cinf_decl_new(db, decl_struct);
	assert(cinf_decl_tag(r2) == decl_struct);
	assert(cinf_decl_idx(r2) == 2);

	cinf_decl_ptr(r2)->name = cinf_name_new(db, "s2");
	assert(strcmp("s2", cinf_decl_name(r2)) == 0);

	cinf_db_destroy(db);
}

int main()
{
	t1();
}
