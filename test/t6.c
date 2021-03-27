#undef NDEBUG
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "casn1.h"

#define array_size(arr) ((sizeof(arr)/sizeof(arr[0])))

void test_oid(const char *s, const char *exp, u64 *arr, size_t count, int result)
{
    u8 buf[128];
    u64 c[16];
    size_t len, slen;

    len = 0;
    assert(crefl_asn1_oid_from_string(NULL, &len, s, strlen(s)) == result);
    len = array_size(c);
    assert(crefl_asn1_oid_from_string(c, &len, s, strlen(s)) == result);
    assert(len == count);
    assert(memcmp(arr, c, sizeof(u64) * len) == 0);

    slen = 0;
    assert(!crefl_asn1_oid_to_string(NULL, &slen, c, len));
    slen = sizeof(buf);
    assert(!crefl_asn1_oid_to_string(buf, &slen, c, len));
    assert(slen == strlen(exp));
    assert(memcmp(buf, exp, strlen(exp)) == 0);
}

static const char *test_1_str = "";
static u64 test_1_arr[] = { };

static const char *test_2_str = "1";
static u64 test_2_arr[] = { 1 };

static const char *test_3_str = "1.2.3";
static u64 test_3_arr[] = { 1, 2, 3 };

static const char *test_4_str = "2.99.1x";
static const char *test_4_exp = "2.99";
static u64 test_4_arr[] = { 2, 99 };

static const char *test_5_str = "2.99..100000";
static const char *test_5_exp = "2.99";
static u64 test_5_arr[] = { 2, 99 };

static const char *test_6_str = "2.99.100000..";
static const char *test_6_exp = "2.99.100000";
static u64 test_6_arr[] = { 2, 99, 100000 };

static const char *test_7_str = "2.99.72057594037927935";
static u64 test_7_arr[] = { 2, 99, 72057594037927935ull };

static const char *test_8_str = "2.99.72057594037927936";
static const char *test_8_exp = "2.99";
static u64 test_8_arr[] = { 2, 99 };

#define FN(Y,X) test_ ## Y ## _ ## X
#define T_OID(X,Y,Z)                                                    \
void FN(oid,X)() {                                                      \
    test_oid(FN(X,str), FN(X,Y), FN(X,arr), array_size(FN(X,arr)), Z);  \
}

T_OID(1,str,0)
T_OID(2,str,0)
T_OID(3,str,0)
T_OID(4,exp,-1)
T_OID(5,exp,-1)
T_OID(6,exp,-1)
T_OID(7,str,0)
T_OID(8,exp,-1)

int main()
{
    test_oid_1();
    test_oid_2();
    test_oid_3();
    test_oid_4();
    test_oid_5();
    test_oid_6();
    test_oid_7();
    test_oid_8();
}
