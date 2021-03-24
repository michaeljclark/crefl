#undef NDEBUG
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "cbuf.h"
#include "casn1.h"

const char* length_fmt = "\nASN.1 X.690 length(%zu)[0x%zx]\n";
const char* tagnum_fmt = "\nASN.1 X.690 tagnum(%zu)[0x%zx]\n";
const char* ident_fmt  = "\nASN.1 X.690 ident(%zu)[0x%zx]\n";
const char* tagint_fmt = "\nASN.1 X.690 tagint(%zu)[0x%zx]\n";
const char* oid_fmt    = "\nASN.1 X.690 oid(%s)\n";

#define array_size(arr) ((sizeof(arr)/sizeof(arr[0])))

#define FN(Y,X) test ## _ ## Y ## _ ## X
#define U64(X) X ## ll

#define T_LENGTH(X,num)                                    \
void FN(length,X)()                                        \
{                                                          \
    u64 num2;                                              \
    crefl_buf *buf;                                        \
    printf(length_fmt, (size_t)num, (size_t)num);          \
    assert(buf = crefl_buf_new(1024));                     \
    assert(!crefl_asn1_length_write(buf, U64(num)));       \
    crefl_buf_dump(buf);                                   \
    crefl_buf_reset(buf);                                  \
    assert(!crefl_asn1_length_read(buf, &num2));           \
    assert(num == num2);                                   \
    assert(crefl_buf_offset(buf) ==                        \
           crefl_asn1_length_length(num));                 \
    crefl_buf_destroy(buf);                                \
}

T_LENGTH(1,1)
T_LENGTH(2,10)
T_LENGTH(3,128)
T_LENGTH(4,170)
T_LENGTH(5,256)
T_LENGTH(6,43690)
T_LENGTH(7,65536)
T_LENGTH(8,11184810)
T_LENGTH(9,16777216)
T_LENGTH(10,2863311530)
T_LENGTH(11,4294967296)
T_LENGTH(12,733007751850)
T_LENGTH(13,1099511627776)

#define T_TAGNUM(X,num)                                    \
void FN(tagnum,X)()                                        \
{                                                          \
    u64 num2;                                              \
    crefl_buf *buf;                                        \
    printf(tagnum_fmt, (size_t)num, (size_t)num);          \
    assert(buf = crefl_buf_new(1024));                     \
    assert(!crefl_asn1_tagnum_write(buf, U64(num)));       \
    crefl_buf_dump(buf);                                   \
    crefl_buf_reset(buf);                                  \
    assert(!crefl_asn1_tagnum_read(buf, &num2));           \
    assert(num == num2);                                   \
    assert(crefl_buf_offset(buf) ==                        \
           crefl_asn1_tagnum_length(num));                 \
    crefl_buf_destroy(buf);                                \
}

T_TAGNUM(1,1)
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

#define T_IDENT(X,num)                                     \
void FN(ident,X)()                                         \
{                                                          \
    asn1_id _id1 = {                                       \
      U64(num), 0, asn1_class_universal,                   \
    };                                                     \
    asn1_id _id2;                                          \
    crefl_buf *buf;                                        \
    printf(ident_fmt, (size_t)num, (size_t)num);           \
    assert(buf = crefl_buf_new(1024));                     \
    assert(!crefl_asn1_ident_write(buf, _id1));            \
    crefl_buf_dump(buf);                                   \
    crefl_buf_reset(buf);                                  \
    assert(!crefl_asn1_ident_read(buf, &_id2));            \
    assert(_id1._class == _id1._class);                    \
    assert(_id1._constructed == _id1._constructed);        \
    assert(_id1._identifier == _id1._identifier);          \
    assert(crefl_buf_offset(buf) ==                        \
           crefl_asn1_ident_length(_id1));                 \
    crefl_buf_destroy(buf);                                \
}

T_IDENT(1,1)
T_IDENT(2,10)
T_IDENT(3,128)
T_IDENT(4,170)
T_IDENT(5,256)
T_IDENT(6,43690)
T_IDENT(7,65536)
T_IDENT(8,11184810)
T_IDENT(9,16777216)
T_IDENT(10,2863311530)
T_IDENT(11,4294967296)
T_IDENT(12,733007751850)
T_IDENT(13,1099511627776)

#define T_TAGINT(X,num)                                    \
void FN(tagint,X)()                                        \
{                                                          \
    u64 num2;                                              \
    crefl_buf *buf;                                        \
    printf(tagint_fmt, (size_t)num, (size_t)num);          \
    assert(buf = crefl_buf_new(1024));                     \
    assert(!crefl_asn1_tagged_integer_write(buf,           \
        asn1_tag_integer, U64(num)));                      \
    crefl_buf_dump(buf);                                   \
    crefl_buf_reset(buf);                                  \
    assert(!crefl_asn1_tagged_integer_read(buf,            \
        asn1_tag_integer, &num2));                         \
    assert(num == num2);                                   \
    crefl_buf_destroy(buf);                                \
}

T_TAGINT(1,1)
T_TAGINT(2,10)
T_TAGINT(3,128)
T_TAGINT(4,170)
T_TAGINT(5,256)
T_TAGINT(6,43690)
T_TAGINT(7,65536)
T_TAGINT(8,11184810)
T_TAGINT(9,16777216)
T_TAGINT(10,2863311530)
T_TAGINT(11,4294967296)
T_TAGINT(12,733007751850)
T_TAGINT(13,1099511627776)

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

#define T_OID(X)                                                  \
void FN(oid,X)()                                                  \
{                                                                 \
    char *str = oid_tests[X].str;                                 \
    u64 *oid = oid_tests[X].oid;                                  \
    size_t count = oid_tests[X].count;                            \
    u8 *der = oid_tests[X].der;                                   \
    size_t nbytes = oid_tests[X].nbytes;                          \
    u64 oid2[16];                                                 \
    u8 str2[128];                                                 \
    size_t count2;                                                \
    asn1_hdr hdr = {                                              \
        { asn1_tag_object_identifier, 0, asn1_class_universal },  \
        crefl_asn1_oid_length(oid, count)                         \
    };                                                            \
    crefl_buf *buf = crefl_buf_new(1024);                         \
    assert(buf);                                                  \
    assert(!crefl_asn1_oid_write(buf, &hdr, oid, count));         \
    assert(crefl_buf_offset(buf) == nbytes);                      \
    assert(memcmp(crefl_buf_data(buf), der, nbytes) == 0);        \
    printf(oid_fmt, str);                                         \
    crefl_buf_dump(buf);                                          \
    crefl_buf_reset(buf);                                         \
    count2 = 0;                                                   \
    assert(!crefl_asn1_oid_read(buf, &hdr, NULL, &count2));       \
    assert(count == count2);                                      \
    crefl_buf_reset(buf);                                         \
    count2 = array_size(oid2);                                    \
    assert(!crefl_asn1_oid_read(buf, &hdr, oid2, &count2));       \
    assert(memcmp(oid, oid2, sizeof(u64) * count2) == 0);         \
    assert(crefl_asn1_oid_to_string(NULL, 0, oid, count)          \
           == strlen(str));                                       \
    assert(crefl_asn1_oid_to_string(str2, 128, oid, count)        \
           == strlen(str));                                       \
    assert(memcmp(str, str2, strlen(str)) == 0);                  \
    crefl_buf_destroy(buf);                                       \
}

T_OID(0)
T_OID(1)
T_OID(2)

int main()
{
    test_length_1();
    test_length_2();
    test_length_3();
    test_length_4();
    test_length_5();
    test_length_6();
    test_length_7();
    test_length_8();
    test_length_9();
    test_length_10();
    test_length_11();
    test_length_12();
    test_length_13();

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

    test_ident_1();
    test_ident_2();
    test_ident_3();
    test_ident_4();
    test_ident_5();
    test_ident_6();
    test_ident_7();
    test_ident_8();
    test_ident_9();
    test_ident_10();
    test_ident_11();
    test_ident_12();
    test_ident_13();

    test_tagint_1();
    test_tagint_2();
    test_tagint_3();
    test_tagint_4();
    test_tagint_5();
    test_tagint_6();
    test_tagint_7();
    test_tagint_8();
    test_tagint_9();
    test_tagint_10();
    test_tagint_11();
    test_tagint_12();
    test_tagint_13();

    test_oid_0();
    test_oid_1();
    test_oid_2();

    printf("\n");
}
