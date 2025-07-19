#undef NDEBUG
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#include <cinf/model.h>

/* cinf_list_{decls,types,fields,functions} */

#define array_size(a) (sizeof(a)/sizeof(a[0]))
static decl_ref r[16];

void t2_decls()
{
	decl_db *db = cinf_db_new();
	assert(db != NULL);

	decl_ref src = cinf_decl_new(db, _decl_source);
	db->root_element = cinf_decl_idx(src);

	decl_ref r1 = cinf_decl_new(db, _decl_intrinsic);
	cinf_decl_ptr(src)->_link = cinf_decl_idx(r1);
	decl_ref r2 = cinf_decl_new(db, _decl_struct);
	cinf_decl_ptr(r1)->_next = cinf_decl_idx(r2);
	decl_ref r3 = cinf_decl_new(db, _decl_enum);
	cinf_decl_ptr(r2)->_next = cinf_decl_idx(r3);
	decl_ref r4 = cinf_decl_new(db, _decl_field);
	cinf_decl_ptr(r3)->_next = cinf_decl_idx(r4);
	decl_ref r5 = cinf_decl_new(db, _decl_function);
	cinf_decl_ptr(r4)->_next = cinf_decl_idx(r5);

	size_t s = array_size(r);
	cinf_source_decls(cinf_root(db), r, &s);
	assert(s == 5);
	assert(cinf_decl_tag(r[0]) == _decl_intrinsic);
	assert(cinf_decl_tag(r[1]) == _decl_struct);
	assert(cinf_decl_tag(r[2]) == _decl_enum);
	assert(cinf_decl_tag(r[3]) == _decl_field);
	assert(cinf_decl_tag(r[4]) == _decl_function);

	cinf_db_destroy(db);
}

void t2_types()
{
	decl_db *db = cinf_db_new();
	assert(db != NULL);

	decl_ref src = cinf_decl_new(db, _decl_source);
	db->root_element = cinf_decl_idx(src);

	decl_ref r1 = cinf_decl_new(db, _decl_intrinsic);
	cinf_decl_ptr(src)->_link = cinf_decl_idx(r1);
	decl_ref r2 = cinf_decl_new(db, _decl_struct);
	cinf_decl_ptr(r1)->_next = cinf_decl_idx(r2);
	decl_ref r3 = cinf_decl_new(db, _decl_enum);
	cinf_decl_ptr(r2)->_next = cinf_decl_idx(r3);

	size_t s = array_size(r);
	cinf_source_types(cinf_root(db), r, &s);
	assert(s == 3);
	assert(cinf_decl_tag(r[0]) == _decl_intrinsic);
	assert(cinf_decl_tag(r[1]) == _decl_struct);
	assert(cinf_decl_tag(r[2]) == _decl_enum);

	cinf_db_destroy(db);
}

void t2_fields()
{
	decl_db *db = cinf_db_new();
	assert(db != NULL);

	decl_ref src = cinf_decl_new(db, _decl_source);
	db->root_element = cinf_decl_idx(src);

	decl_ref r1 = cinf_decl_new(db, _decl_field);
	cinf_decl_ptr(src)->_link = cinf_decl_idx(r1);

	size_t s = array_size(r);
	cinf_source_fields(cinf_root(db), r, &s);
	assert(s == 1);
	assert(cinf_decl_tag(r[0]) == _decl_field);

	cinf_db_destroy(db);
}

void t2_functions()
{
	decl_db *db = cinf_db_new();
	assert(db != NULL);

	decl_ref src = cinf_decl_new(db, _decl_source);
	db->root_element = cinf_decl_idx(src);

	decl_ref r1 = cinf_decl_new(db, _decl_function);
	cinf_decl_ptr(src)->_link = cinf_decl_idx(r1);
	decl_ref r2 = cinf_decl_new(db, _decl_function);
	cinf_decl_ptr(r1)->_next = cinf_decl_idx(r2);

	size_t s = array_size(r);
	cinf_source_functions(cinf_root(db), r, &s);
	assert(s == 2);
	assert(cinf_decl_tag(r[0]) == _decl_function);
	assert(cinf_decl_tag(r[1]) == _decl_function);

	cinf_db_destroy(db);
}

int main()
{
	t2_decls();
	t2_types();
	t2_fields();
	t2_functions();
}
