#undef NDEBUG
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <crefl/asn1.h>

void test_oid(const char *s, const char *exp, const asn1_oid *oid, int result)
{
    u8 buf[128];
    size_t len, slen;
    asn1_oid oid1;

    assert(crefl_asn1_oid_from_string(&oid1, s, strlen(s)) == result);
    assert(oid->count == oid1.count);
    assert(memcmp(oid->oid, oid1.oid, sizeof(u64) * oid->count) == 0);

    slen = 0;
    assert(!crefl_asn1_oid_to_string(NULL, &slen, oid));
    slen = sizeof(buf);
    assert(!crefl_asn1_oid_to_string((char*)buf, &slen, oid));
    assert(slen == strlen(exp));
    assert(memcmp(buf, exp, strlen(exp)) == 0);
}

static const char *test_1_str = "";
static const asn1_oid test_1_oid = { 0, { 0 } };

static const char *test_2_str = "1";
static const asn1_oid test_2_oid = { 1, { 1 } };

static const char *test_3_str = "1.2.3";
static const asn1_oid test_3_oid = { 3, { 1, 2, 3 } };

static const char *test_4_str = "2.99.1x";
static const char *test_4_exp = "2.99";
static const asn1_oid test_4_oid = { 2, { 2, 99 } };

static const char *test_5_str = "2.99..100000";
static const char *test_5_exp = "2.99";
static const asn1_oid test_5_oid = { 2, { 2, 99 } };

static const char *test_6_str = "2.99.100000..";
static const char *test_6_exp = "2.99.100000";
static const asn1_oid test_6_oid = { 3, { 2, 99, 100000 } };

static const char *test_7_str = "2.99.72057594037927935";
static const asn1_oid test_7_oid = { 3, { 2, 99, 72057594037927935ull } };

static const char *test_8_str = "2.99.72057594037927936";
static const char *test_8_exp = "2.99";
static const asn1_oid test_8_oid = { 2, { 2, 99 } };

#define FN(Y,X) test_ ## Y ## _ ## X
#define T_OID(X,Y,Z)                                                    \
void FN(oid,X)() {                                                      \
    test_oid(FN(X,str), FN(X,Y), &FN(X,oid), Z);                        \
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
