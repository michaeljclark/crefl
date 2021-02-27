#undef NDEBUG
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#include "cmodel.h"

/* crefl_list_{decls,types,fields,functions} */

#define array_size(a) (sizeof(a)/sizeof(a[0]))
static decl_ref r[16];

void t2_decls()
{
	decl_db *db = crefl_db_new();
	assert(db != NULL);

	decl_ref r1 = crefl_decl_new(db, _decl_intrinsic);
	db->root_element = crefl_decl_idx(r1);
	decl_ref r2 = crefl_decl_new(db, _decl_struct);
	crefl_decl_ptr(r1)->_next = crefl_decl_idx(r2);
	decl_ref r3 = crefl_decl_new(db, _decl_enum);
	crefl_decl_ptr(r2)->_next = crefl_decl_idx(r3);
	decl_ref r4 = crefl_decl_new(db, _decl_field);
	crefl_decl_ptr(r3)->_next = crefl_decl_idx(r4);
	decl_ref r5 = crefl_decl_new(db, _decl_function);
	crefl_decl_ptr(r4)->_next = crefl_decl_idx(r5);

	size_t s = array_size(r);
	crefl_list_decls(db, r, &s);
	assert(s == 5);
	assert(crefl_decl_tag(r[0]) == _decl_intrinsic);
	assert(crefl_decl_tag(r[1]) == _decl_struct);
	assert(crefl_decl_tag(r[2]) == _decl_enum);
	assert(crefl_decl_tag(r[3]) == _decl_field);
	assert(crefl_decl_tag(r[4]) == _decl_function);

	crefl_db_destroy(db);
}

void t2_types()
{
	decl_db *db = crefl_db_new();
	assert(db != NULL);

	decl_ref r1 = crefl_decl_new(db, _decl_intrinsic);
	db->root_element = crefl_decl_idx(r1);
	decl_ref r2 = crefl_decl_new(db, _decl_struct);
	crefl_decl_ptr(r1)->_next = crefl_decl_idx(r2);
	decl_ref r3 = crefl_decl_new(db, _decl_enum);
	crefl_decl_ptr(r2)->_next = crefl_decl_idx(r3);

	size_t s = array_size(r);
	crefl_list_types(db, r, &s);
	assert(s == 3);
	assert(crefl_decl_tag(r[0]) == _decl_intrinsic);
	assert(crefl_decl_tag(r[1]) == _decl_struct);
	assert(crefl_decl_tag(r[2]) == _decl_enum);

	crefl_db_destroy(db);
}

void t2_fields()
{
	decl_db *db = crefl_db_new();
	assert(db != NULL);

	decl_ref r1 = crefl_decl_new(db, _decl_field);
	db->root_element = crefl_decl_idx(r1);

	size_t s = array_size(r);
	crefl_list_fields(db, r, &s);
	assert(s == 1);
	assert(crefl_decl_tag(r[0]) == _decl_field);

	crefl_db_destroy(db);
}

void t2_functions()
{
	decl_db *db = crefl_db_new();
	assert(db != NULL);

	decl_ref r1 = crefl_decl_new(db, _decl_function);
	db->root_element = crefl_decl_idx(r1);
	decl_ref r2 = crefl_decl_new(db, _decl_function);
	crefl_decl_ptr(r1)->_next = crefl_decl_idx(r2);

	size_t s = array_size(r);
	crefl_list_functions(db, r, &s);
	assert(s == 2);
	assert(crefl_decl_tag(r[0]) == _decl_function);
	assert(crefl_decl_tag(r[1]) == _decl_function);

	crefl_db_destroy(db);
}

int main()
{
	t2_decls();
	t2_types();
	t2_fields();
	t2_functions();
}
