#undef NDEBUG
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#include "cmodel.h"

/*
int decl_types(decl_db *db, decl_ref *r, size_t *s);
int decl_constants(decl_db *db, decl_ref *r, size_t *s);
int decl_variables(decl_db *db, decl_ref *r, size_t *s);
int decl_uniforms(decl_db *db, decl_ref *r, size_t *s);
int decl_functions(decl_db *db, decl_ref *r, size_t *s);
*/

#define array_size(a) (sizeof(a)/sizeof(a[0]))
static decl_ref r[16];

void t2_types()
{
	decl_db *db = decl_db_new();
	assert(db != NULL);

	decl_ref r1 = decl_new(db, _decl_intrinsic, _top);
	decl_ref r2 = decl_new(db, _decl_struct, _top);
	decl_ref r3 = decl_new(db, _decl_enum, _top);

	size_t s = array_size(r);
	decl_types(db, r, &s);
	assert(s == 3);

	decl_db_destroy(db);
}

void t2_constants()
{
	decl_db *db = decl_db_new();
	assert(db != NULL);

	decl_ref r1 = decl_new(db, _decl_constant, _top);
	decl_ref r2 = decl_new(db, _decl_constant, _top);

	size_t s = array_size(r);
	decl_constants(db, r, &s);
	assert(s == 2);

	decl_db_destroy(db);
}

void t2_variables()
{
	decl_db *db = decl_db_new();
	assert(db != NULL);

	decl_ref r1 = decl_new(db, _decl_variable, _top);
	decl_ref r2 = decl_new(db, _decl_variable, _top);

	size_t s = array_size(r);
	decl_variables(db, r, &s);
	assert(s == 2);

	decl_db_destroy(db);
}

void t2_uniforms()
{
	decl_db *db = decl_db_new();
	assert(db != NULL);

	decl_ref r1 = decl_new(db, _decl_uniform, _top);
	decl_ref r2 = decl_new(db, _decl_uniform, _top);

	size_t s = array_size(r);
	decl_uniforms(db, r, &s);
	assert(s == 2);

	decl_db_destroy(db);
}

void t2_functions()
{
	decl_db *db = decl_db_new();
	assert(db != NULL);

	decl_ref r1 = decl_new(db, _decl_function, _top);
	decl_ref r2 = decl_new(db, _decl_function, _top);

	size_t s = array_size(r);
	decl_functions(db, r, &s);
	assert(s == 2);

	decl_db_destroy(db);
}

int main()
{
	t2_types();
	t2_constants();
	t2_variables();
	t2_uniforms();
	t2_functions();
}
