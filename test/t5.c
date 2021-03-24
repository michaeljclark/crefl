#undef NDEBUG
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "cbuf.h"
#include "casn1.h"

const char* tagnum_fmt  = "\nASN.1 X.690 tagnum(%zu)[0x%zx]\n";
const char* length_fmt  = "\nASN.1 X.690 ber_length(%zu)[0x%zx]\n";
const char* ident_fmt   = "\nASN.1 X.690 ber_ident(%zu)[0x%zx]\n";
const char* ber_int_fmt  = "\nASN.1 X.690 ber_int(%zu)[0x%zx]\n";
const char* ber_bool_fmt = "\nASN.1 X.690 ber_bool(%s)[0x%zx]\n";
const char* ber_oid_fmt  = "\nASN.1 X.690 ber_oid(%s)\n";
const char* der_int_fmt  = "\nASN.1 X.690 der_int(%zu)[0x%zx]\n";
const char* der_bool_fmt = "\nASN.1 X.690 der_bool(%s)[0x%zx]\n";
const char* der_oid_fmt  = "\nASN.1 X.690 der_oid(%s)\n";

struct oid_test
{
    char *str;
    u64 *oid;
    size_t count;
    u8 *der;
    size_t nbytes;
};

struct oid_test oid_tests[] = {
    {
        "1.2",
        (u64[]){ 1,2 }, 2,
        (u8[]){ 0x2a }, 1
    }, {
        "1.2.3",
        (u64[]){ 1,2,3 }, 3,
        (u8[]){ 0x2a,0x03 }, 2
    }, {
        "1.2.840.113549.1.1.11",
        (u64[]){ 1,2,840,113549,1,1,11 }, 7,
        (u8[]){ 0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0b }, 9
    }
};

#define array_size(arr) ((sizeof(arr)/sizeof(arr[0])))

#define FN(Y,X) test ## _ ## Y ## _ ## X
#define U64(X) X ## ll

#define T_TAGNUM(X,num)                                            \
void FN(tagnum,X)()                                                \
{                                                                  \
    u64 num2;                                                      \
    crefl_buf *buf;                                                \
    printf(tagnum_fmt, (size_t)num, (size_t)num);                  \
    assert(buf = crefl_buf_new(1024));                             \
    assert(!crefl_asn1_tagnum_write(buf, U64(num)));               \
    crefl_buf_dump(buf);                                           \
    crefl_buf_reset(buf);                                          \
    assert(!crefl_asn1_tagnum_read(buf, &num2));                   \
    assert(num == num2);                                           \
    assert(crefl_buf_offset(buf) ==                                \
           crefl_asn1_tagnum_length(num));                         \
    crefl_buf_destroy(buf);                                        \
}

T_TAGNUM(1,0)
T_TAGNUM(2,10)
T_TAGNUM(3,128)
T_TAGNUM(4,170)
T_TAGNUM(5,256)
T_TAGNUM(6,43690)
T_TAGNUM(7,65536)
T_TAGNUM(8,11184810)
T_TAGNUM(9,16777216)
T_TAGNUM(10,2863311530)
T_TAGNUM(11,4294967296)
T_TAGNUM(12,733007751850)
T_TAGNUM(13,1099511627776)

#define T_BER_LENGTH(X,num)                                        \
void FN(ber_length,X)()                                            \
{                                                                  \
    u64 num2;                                                      \
    crefl_buf *buf;                                                \
    printf(length_fmt, (size_t)num, (size_t)num);                  \
    assert(buf = crefl_buf_new(1024));                             \
    assert(!crefl_asn1_ber_length_write(buf, U64(num)));           \
    crefl_buf_dump(buf);                                           \
    crefl_buf_reset(buf);                                          \
    assert(!crefl_asn1_ber_length_read(buf, &num2));               \
    assert(num == num2);                                           \
    assert(crefl_buf_offset(buf) ==                                \
           crefl_asn1_ber_length_length(num));                     \
    crefl_buf_destroy(buf);                                        \
}

T_BER_LENGTH(1,0)
T_BER_LENGTH(2,10)
T_BER_LENGTH(3,128)
T_BER_LENGTH(4,170)
T_BER_LENGTH(5,256)
T_BER_LENGTH(6,43690)
T_BER_LENGTH(7,65536)
T_BER_LENGTH(8,11184810)
T_BER_LENGTH(9,16777216)
T_BER_LENGTH(10,2863311530)
T_BER_LENGTH(11,4294967296)
T_BER_LENGTH(12,733007751850)
T_BER_LENGTH(13,1099511627776)

#define T_BER_IDENT(X,num)                                         \
void FN(ber_ident,X)()                                             \
{                                                                  \
    asn1_id _id1 = {                                               \
      U64(num), 0, asn1_class_universal,                           \
    };                                                             \
    asn1_id _id2;                                                  \
    crefl_buf *buf;                                                \
    printf(ident_fmt, (size_t)num, (size_t)num);                   \
    assert(buf = crefl_buf_new(1024));                             \
    assert(!crefl_asn1_ber_ident_write(buf, _id1));                \
    crefl_buf_dump(buf);                                           \
    crefl_buf_reset(buf);                                          \
    assert(!crefl_asn1_ber_ident_read(buf, &_id2));                \
    assert(_id1._class == _id1._class);                            \
    assert(_id1._constructed == _id1._constructed);                \
    assert(_id1._identifier == _id1._identifier);                  \
    assert(crefl_buf_offset(buf) ==                                \
           crefl_asn1_ber_ident_length(_id1));                     \
    crefl_buf_destroy(buf);                                        \
}

T_BER_IDENT(1,0)
T_BER_IDENT(2,10)
T_BER_IDENT(3,128)
T_BER_IDENT(4,170)
T_BER_IDENT(5,256)
T_BER_IDENT(6,43690)
T_BER_IDENT(7,65536)
T_BER_IDENT(8,11184810)
T_BER_IDENT(9,16777216)
T_BER_IDENT(10,2863311530)
T_BER_IDENT(11,4294967296)
T_BER_IDENT(12,733007751850)
T_BER_IDENT(13,1099511627776)

#define T_BER_BOOL(X,num)                                          \
void FN(ber_bool,X)()                                              \
{                                                                  \
    bool num2;                                                     \
    crefl_buf *buf;                                                \
    printf(ber_bool_fmt, num?"true":"false", (size_t)num);         \
    assert(buf = crefl_buf_new(1024));                             \
    size_t len = crefl_asn1_ber_boolean_length(U64(num));          \
    assert(!crefl_asn1_ber_boolean_write(buf, len, (bool)(num)));  \
    crefl_buf_dump(buf);                                           \
    crefl_buf_reset(buf);                                          \
    assert(!crefl_asn1_ber_boolean_read(buf, len, &num2));         \
    assert(num == num2);                                           \
    crefl_buf_destroy(buf);                                        \
}

T_BER_BOOL(1,false)
T_BER_BOOL(2,true)

#define T_BER_INT(X,num)                                           \
void FN(ber_int,X)()                                               \
{                                                                  \
    u64 num2;                                                      \
    crefl_buf *buf;                                                \
    printf(ber_int_fmt, (size_t)num, (size_t)num);                 \
    assert(buf = crefl_buf_new(1024));                             \
    size_t len = crefl_asn1_ber_integer_length(U64(num));          \
    assert(!crefl_asn1_ber_integer_write(buf, len, U64(num)));     \
    crefl_buf_dump(buf);                                           \
    crefl_buf_reset(buf);                                          \
    assert(!crefl_asn1_ber_integer_read(buf, len, &num2));         \
    assert(num == num2);                                           \
    crefl_buf_destroy(buf);                                        \
}

T_BER_INT(1,0)
T_BER_INT(2,10)
T_BER_INT(3,128)
T_BER_INT(4,170)
T_BER_INT(5,256)
T_BER_INT(6,43690)
T_BER_INT(7,65536)
T_BER_INT(8,11184810)
T_BER_INT(9,16777216)
T_BER_INT(10,2863311530)
T_BER_INT(11,4294967296)
T_BER_INT(12,733007751850)
T_BER_INT(13,1099511627776)

#define T_BER_OID(X)                                               \
void FN(ber_oid,X)()                                               \
{                                                                  \
    char *str = oid_tests[X].str;                                  \
    u64 *oid = oid_tests[X].oid;                                   \
    size_t count = oid_tests[X].count;                             \
    u8 *der = oid_tests[X].der;                                    \
    size_t nbytes = oid_tests[X].nbytes;                           \
    u64 oid2[16];                                                  \
    u8 str2[128];                                                  \
    size_t count2;                                                 \
    size_t len = crefl_asn1_ber_oid_length(oid, count);            \
    crefl_buf *buf = crefl_buf_new(1024);                          \
    assert(buf);                                                   \
    assert(!crefl_asn1_ber_oid_write(buf, len, oid, count));       \
    assert(crefl_buf_offset(buf) == nbytes);                       \
    assert(memcmp(crefl_buf_data(buf), der, nbytes) == 0);         \
    printf(ber_oid_fmt, str);                                      \
    crefl_buf_dump(buf);                                           \
    crefl_buf_reset(buf);                                          \
    count2 = 0;                                                    \
    assert(!crefl_asn1_ber_oid_read(buf, len, NULL, &count2));     \
    assert(count == count2);                                       \
    crefl_buf_reset(buf);                                          \
    count2 = array_size(oid2);                                     \
    assert(!crefl_asn1_ber_oid_read(buf, len, oid2, &count2));     \
    assert(memcmp(oid, oid2, sizeof(u64) * count2) == 0);          \
    assert(crefl_asn1_oid_to_string(NULL, 0, oid, count)           \
           == strlen(str));                                        \
    assert(crefl_asn1_oid_to_string(str2, 128, oid, count)         \
           == strlen(str));                                        \
    assert(memcmp(str, str2, strlen(str)) == 0);                   \
    crefl_buf_destroy(buf);                                        \
}

T_BER_OID(0)
T_BER_OID(1)
T_BER_OID(2)

#define T_DER_BOOL(X,num)                                          \
void FN(der_bool,X)()                                              \
{                                                                  \
    bool num2;                                                     \
    crefl_buf *buf;                                                \
    printf(der_bool_fmt, num?"true":"false", (size_t)num);         \
    assert(buf = crefl_buf_new(1024));                             \
    assert(!crefl_asn1_der_boolean_write(buf,                      \
        asn1_tag_boolean, (bool)(num)));                           \
    crefl_buf_dump(buf);                                           \
    crefl_buf_reset(buf);                                          \
    assert(!crefl_asn1_der_boolean_read(buf,                       \
        asn1_tag_boolean, &num2));                                 \
    assert(num == num2);                                           \
    crefl_buf_destroy(buf);                                        \
}

T_DER_BOOL(1,false)
T_DER_BOOL(2,true)

#define T_DER_INT(X,num)                                           \
void FN(der_int,X)()                                               \
{                                                                  \
    u64 num2;                                                      \
    crefl_buf *buf;                                                \
    printf(der_int_fmt, (size_t)num, (size_t)num);                 \
    assert(buf = crefl_buf_new(1024));                             \
    assert(!crefl_asn1_der_integer_write(buf,                      \
        asn1_tag_integer, U64(num)));                              \
    crefl_buf_dump(buf);                                           \
    crefl_buf_reset(buf);                                          \
    assert(!crefl_asn1_der_integer_read(buf,                       \
        asn1_tag_integer, &num2));                                 \
    assert(num == num2);                                           \
    crefl_buf_destroy(buf);                                        \
}

T_DER_INT(1,0)
T_DER_INT(2,10)
T_DER_INT(3,128)
T_DER_INT(4,170)
T_DER_INT(5,256)
T_DER_INT(6,43690)
T_DER_INT(7,65536)
T_DER_INT(8,11184810)
T_DER_INT(9,16777216)
T_DER_INT(10,2863311530)
T_DER_INT(11,4294967296)
T_DER_INT(12,733007751850)
T_DER_INT(13,1099511627776)

#define T_DER_OID(X)                                               \
void FN(der_oid,X)()                                               \
{                                                                  \
    char *str = oid_tests[X].str;                                  \
    u64 *oid = oid_tests[X].oid;                                   \
    size_t count = oid_tests[X].count;                             \
    u8 *der = oid_tests[X].der;                                    \
    size_t nbytes = oid_tests[X].nbytes;                           \
    u64 oid2[16];                                                  \
    u8 str2[128];                                                  \
    size_t count2;                                                 \
    asn1_hdr hdr = {                                               \
        { asn1_tag_object_identifier, 0, asn1_class_universal },   \
        crefl_asn1_ber_oid_length(oid, count)                      \
    };                                                             \
    crefl_buf *buf = crefl_buf_new(1024);                          \
    assert(buf);                                                   \
    assert(!crefl_asn1_der_oid_write(buf,                          \
        asn1_tag_object_identifier, oid, count));                  \
    printf(der_oid_fmt, str);                                      \
    crefl_buf_dump(buf);                                           \
    crefl_buf_reset(buf);                                          \
    count2 = 0;                                                    \
    assert(!crefl_asn1_der_oid_read(buf,                           \
        asn1_tag_object_identifier, NULL, &count2));               \
    assert(count == count2);                                       \
    crefl_buf_reset(buf);                                          \
    count2 = array_size(oid2);                                     \
    assert(!crefl_asn1_der_oid_read(buf,                           \
        asn1_tag_object_identifier, oid2, &count2));               \
    assert(memcmp(oid, oid2, sizeof(u64) * count2) == 0);          \
    crefl_buf_destroy(buf);                                        \
}

T_DER_OID(0)
T_DER_OID(1)
T_DER_OID(2)

int main()
{
    test_tagnum_1();
    test_tagnum_2();
    test_tagnum_3();
    test_tagnum_4();
    test_tagnum_5();
    test_tagnum_6();
    test_tagnum_7();
    test_tagnum_8();
    test_tagnum_9();
    test_tagnum_10();
    test_tagnum_11();
    test_tagnum_12();
    test_tagnum_13();

    test_ber_length_1();
    test_ber_length_2();
    test_ber_length_3();
    test_ber_length_4();
    test_ber_length_5();
    test_ber_length_6();
    test_ber_length_7();
    test_ber_length_8();
    test_ber_length_9();
    test_ber_length_10();
    test_ber_length_11();
    test_ber_length_12();
    test_ber_length_13();

    test_ber_ident_1();
    test_ber_ident_2();
    test_ber_ident_3();
    test_ber_ident_4();
    test_ber_ident_5();
    test_ber_ident_6();
    test_ber_ident_7();
    test_ber_ident_8();
    test_ber_ident_9();
    test_ber_ident_10();
    test_ber_ident_11();
    test_ber_ident_12();
    test_ber_ident_13();

    test_ber_bool_1();
    test_ber_bool_2();

    test_ber_int_1();
    test_ber_int_2();
    test_ber_int_3();
    test_ber_int_4();
    test_ber_int_5();
    test_ber_int_6();
    test_ber_int_7();
    test_ber_int_8();
    test_ber_int_9();
    test_ber_int_10();
    test_ber_int_11();
    test_ber_int_12();
    test_ber_int_13();

    test_ber_oid_0();
    test_ber_oid_1();
    test_ber_oid_2();

    test_der_bool_1();
    test_der_bool_2();

    test_der_int_1();
    test_der_int_2();
    test_der_int_3();
    test_der_int_4();
    test_der_int_5();
    test_der_int_6();
    test_der_int_7();
    test_der_int_8();
    test_der_int_9();
    test_der_int_10();
    test_der_int_11();
    test_der_int_12();
    test_der_int_13();

    test_der_oid_0();
    test_der_oid_1();
    test_der_oid_2();

    printf("\n");
}
