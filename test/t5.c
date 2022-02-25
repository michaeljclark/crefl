#undef NDEBUG
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include <crefl/buf.h>
#include <crefl/asn1.h>

const char* ber_tag_fmt = "\nASN.1 X.690 ber_tag(%zu)[0x%zx]\n";
const char* ber_length_fmt = "\nASN.1 X.690 ber_length(%zu)[0x%zx]\n";
const char* ber_ident_fmt = "\nASN.1 X.690 ber_ident(%zu)[0x%zx]\n";
const char* ber_uint_fmt = "\nASN.1 X.690 ber_uint(%zu)[0x%zx]\n";
const char* ber_sint_fmt = "\nASN.1 X.690 ber_sint(%zd)[0x%zx]\n";
const char* ber_bool_fmt = "\nASN.1 X.690 ber_bool(%s)[0x%zx]\n";
const char* ber_oid_fmt = "\nASN.1 X.690 ber_oid(%s)\n";
const char* ber_real_fmt = "\nASN.1 X.690 ber_real(%.16g)\n";
const char* ber_octets_fmt = "\nASN.1 X.690 ber_octets(\"%s\")\n";
const char* der_uint_fmt = "\nASN.1 X.690 der_uint(%zu)[0x%zx]\n";
const char* der_sint_fmt = "\nASN.1 X.690 der_sint(%zd)[0x%zx]\n";
const char* der_bool_fmt = "\nASN.1 X.690 der_bool(%s)[0x%zx]\n";
const char* der_oid_fmt = "\nASN.1 X.690 der_oid(%s)\n";
const char* der_real_fmt = "\nASN.1 X.690 der_real(%.16g)\n";
const char* der_octets_fmt = "\nASN.1 X.690 der_octets(\"%s\")\n";

struct oid_test
{
    char *str;
    asn1_oid oid;
    u8 *der;
    size_t nbytes;
};

struct oid_test oid_tests[] = {
    {
        "1.2",
        { 2, { 1,2 } },
        (u8[]){ 0x2a }, 1
    }, {
        "1.2.3",
        { 3, { 1,2,3 } },
        (u8[]){ 0x2a,0x03 }, 2
    }, {
        "1.2.840.113549.1.1.11",
        { 7, { 1,2,840,113549,1,1,11 } },
        (u8[]){ 0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0b }, 9
    }
};

#define array_size(arr) ((sizeof(arr)/sizeof(arr[0])))

#define FN(Y,X) test ## _ ## Y ## _ ## X
#define U64(X) X ## llu
#define S64(X) X ## ll

#define T_TAGNUM(X,num)                                            \
void FN(ber_tag,X)()                                               \
{                                                                  \
    u64 num2;                                                      \
    crefl_buf *buf;                                                \
    printf(ber_tag_fmt, (size_t)num, (size_t)num);                 \
    assert((buf = crefl_buf_new(1024)));                           \
    assert(!crefl_asn1_ber_tag_write(buf, U64(num)));              \
    crefl_buf_dump(buf);                                           \
    crefl_buf_reset(buf);                                          \
    assert(!crefl_asn1_ber_tag_read(buf, &num2));                  \
    assert(num == num2);                                           \
    assert(crefl_buf_offset(buf) ==                                \
           crefl_asn1_ber_tag_length(num));                        \
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
    printf(ber_length_fmt, (size_t)num, (size_t)num);              \
    assert((buf = crefl_buf_new(1024)));                           \
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
    printf(ber_ident_fmt, (size_t)num, (size_t)num);               \
    assert((buf = crefl_buf_new(1024)));                           \
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
    bool num1 = (bool)(num), num2;                                 \
    crefl_buf *buf;                                                \
    printf(ber_bool_fmt, num ? "true" : "false", (size_t)num);     \
    assert((buf = crefl_buf_new(1024)));                           \
    size_t len = crefl_asn1_ber_boolean_length(&num1);             \
    assert(!crefl_asn1_ber_boolean_write(buf, len, &num1));        \
    crefl_buf_dump(buf);                                           \
    crefl_buf_reset(buf);                                          \
    assert(!crefl_asn1_ber_boolean_read(buf, len, &num2));         \
    assert(num1 == num2);                                          \
    crefl_buf_destroy(buf);                                        \
}

T_BER_BOOL(1,false)
T_BER_BOOL(2,true)

#define T_BER_UINT(X,num)                                          \
void FN(ber_uint,X)()                                              \
{                                                                  \
    u64 num1 = U64(num), num2;                                     \
    crefl_buf *buf;                                                \
    printf(ber_uint_fmt, (u64)num, (u64)num);                      \
    assert((buf = crefl_buf_new(1024)));                           \
    size_t len = crefl_asn1_ber_integer_u64_length(&num1);         \
    assert(!crefl_asn1_ber_integer_u64_write(buf, len, &num1));    \
    crefl_buf_dump(buf);                                           \
    crefl_buf_reset(buf);                                          \
    assert(!crefl_asn1_ber_integer_u64_read(buf, len, &num2));     \
    assert(num == num2);                                           \
    crefl_buf_destroy(buf);                                        \
}

T_BER_UINT(1,0)
T_BER_UINT(2,10)
T_BER_UINT(3,128)
T_BER_UINT(4,170)
T_BER_UINT(5,256)
T_BER_UINT(6,43690)
T_BER_UINT(7,65536)
T_BER_UINT(8,11184810)
T_BER_UINT(9,16777216)
T_BER_UINT(10,2863311530)
T_BER_UINT(11,4294967296)
T_BER_UINT(12,733007751850)
T_BER_UINT(13,1099511627776)
T_BER_UINT(14,72057594037927935)

#define T_BER_SINT(X,num)                                          \
void FN(ber_sint,X)()                                              \
{                                                                  \
    s64 num1 = S64(num), num2;                                     \
    crefl_buf *buf;                                                \
    printf(ber_sint_fmt, (s64)num, (s64)num);                      \
    assert((buf = crefl_buf_new(1024)));                           \
    size_t len = crefl_asn1_ber_integer_s64_length(&num1);         \
    assert(!crefl_asn1_ber_integer_s64_write(buf, len, &num1));    \
    crefl_buf_dump(buf);                                           \
    crefl_buf_reset(buf);                                          \
    assert(!crefl_asn1_ber_integer_s64_read(buf, len, &num2));     \
    assert(num == num2);                                           \
    crefl_buf_destroy(buf);                                        \
}

T_BER_SINT(1,0)
T_BER_SINT(2,-10)
T_BER_SINT(3,128)
T_BER_SINT(4,-170)
T_BER_SINT(5,256)
T_BER_SINT(6,-43690)
T_BER_SINT(7,65536)
T_BER_SINT(8,-11184810)
T_BER_SINT(9,16777216)
T_BER_SINT(10,-2863311530)
T_BER_SINT(11,4294967296)
T_BER_SINT(12,-733007751850)
T_BER_SINT(13,1099511627776)
T_BER_SINT(14,72057594037927935)

#define T_BER_OID(X)                                               \
void FN(ber_oid,X)()                                               \
{                                                                  \
    char *str = oid_tests[X].str;                                  \
    u8 *der = oid_tests[X].der;                                    \
    size_t nbytes = oid_tests[X].nbytes;                           \
    asn1_oid oid1 = oid_tests[X].oid, oid2 = { 0 };                \
    u8 str2[128];                                                  \
    size_t buflen;                                                 \
    size_t len = crefl_asn1_ber_oid_length(&oid1);                 \
    crefl_buf *buf = crefl_buf_new(1024);                          \
    assert(buf);                                                   \
    assert(!crefl_asn1_ber_oid_write(buf, len, &oid1));            \
    assert(crefl_buf_offset(buf) == nbytes);                       \
    assert(memcmp(crefl_buf_data(buf), der, nbytes) == 0);         \
    printf(ber_oid_fmt, str);                                      \
    crefl_buf_dump(buf);                                           \
    crefl_buf_reset(buf);                                          \
    assert(!crefl_asn1_ber_oid_read(buf, len, &oid2));             \
    assert(oid1.count == oid2.count);                              \
    assert(memcmp(oid1.oid, oid2.oid,                              \
        sizeof(u64) * oid1.count) == 0);                           \
    buflen = 0;                                                    \
    assert(!crefl_asn1_oid_to_string(NULL, &buflen, &oid2));       \
    assert(buflen);                                                \
    buflen = sizeof(str2);                                         \
    assert(!crefl_asn1_oid_to_string((char*)str2, &buflen, &oid2)); \
    assert(memcmp(str, str2, strlen(str)) == 0);                   \
    crefl_buf_destroy(buf);                                        \
}

T_BER_OID(0)
T_BER_OID(1)
T_BER_OID(2)

#define T_BER_REAL(X,num)                                          \
void FN(ber_real,X)()                                              \
{                                                                  \
    double num1 = (double)(num), num2;                             \
    crefl_buf *buf;                                                \
    printf(ber_real_fmt, (double)num);                             \
    assert((buf = crefl_buf_new(1024)));                           \
    size_t len = crefl_asn1_ber_real_f64_length(&num1);            \
    assert(!crefl_asn1_ber_real_f64_write(buf, len, &num1));       \
    crefl_buf_dump(buf);                                           \
    crefl_buf_reset(buf);                                          \
    assert(!crefl_asn1_ber_real_f64_read(buf, len, &num2));        \
    assert(num == num2 || (isnan(num) && isnan(num2)));            \
    crefl_buf_destroy(buf);                                        \
}

T_BER_REAL(1,0.0)
T_BER_REAL(2,0.5)
T_BER_REAL(3,1.0)
T_BER_REAL(4,2.0)
T_BER_REAL(5,1/256.0)
T_BER_REAL(6,_f64_inf())
T_BER_REAL(7,-_f64_inf())
T_BER_REAL(8,-0.0)
T_BER_REAL(9,_f64_nan())
T_BER_REAL(10,3279/65536.0)
T_BER_REAL(11,0.1)
T_BER_REAL(12,2.71828182845904523536028747135266249)
T_BER_REAL(13,3.14159265358979323846264338327950288)
T_BER_REAL(14,1.77777777777777777777)
T_BER_REAL(15,1e307)

#define T_BER_OCTETS(X,str)                                        \
void FN(ber_octets,X)()                                            \
{                                                                  \
    u8 str2[256];                                                  \
    size_t count;                                                  \
    crefl_buf *buf;                                                \
    asn1_string obj1 = { strlen(str), (u8*)str }, obj2 = { 0, (u8*)str2 }; \
    printf(ber_octets_fmt, str);                                   \
    assert((buf = crefl_buf_new(1024)));                           \
    size_t len = crefl_asn1_ber_octets_length(&obj1);              \
    assert(!crefl_asn1_ber_octets_write(buf, len, &obj1));         \
    crefl_buf_dump(buf);                                           \
    crefl_buf_reset(buf);                                          \
    obj2.count = 0;                                                \
    assert(!crefl_asn1_ber_octets_read(buf, len, &obj2));          \
    assert(obj2.count == strlen(str));                             \
    obj2.count = sizeof(str2);                                     \
    crefl_buf_reset(buf);                                          \
    assert(!crefl_asn1_ber_octets_read(buf, len, &obj2));          \
    assert(obj2.count == strlen(str));                             \
    assert(memcmp(str, str2, strlen(str)) == 0);                   \
    crefl_buf_destroy(buf);                                        \
}

T_BER_OCTETS(1,"")
T_BER_OCTETS(2,"hello")

#define T_DER_BOOL(X,num)                                          \
void FN(der_bool,X)()                                              \
{                                                                  \
    bool num1 = (bool)(num), num2;                                 \
    crefl_buf *buf;                                                \
    printf(der_bool_fmt, num ? "true" : "false", (size_t)num);     \
    assert((buf = crefl_buf_new(1024)));                           \
    assert(!crefl_asn1_der_boolean_write(buf,                      \
        asn1_tag_boolean, &num1));                                 \
    crefl_buf_dump(buf);                                           \
    crefl_buf_reset(buf);                                          \
    assert(!crefl_asn1_der_boolean_read(buf,                       \
        asn1_tag_boolean, &num2));                                 \
    assert(num1 == num2);                                          \
    crefl_buf_destroy(buf);                                        \
}

T_DER_BOOL(1,false)
T_DER_BOOL(2,true)

#define T_DER_UINT(X,num)                                          \
void FN(der_uint,X)()                                              \
{                                                                  \
    u64 num1 = U64(num), num2;                                     \
    crefl_buf *buf;                                                \
    printf(der_uint_fmt, (u64)num, (u64)num);                      \
    assert((buf = crefl_buf_new(1024)));                           \
    assert(!crefl_asn1_der_integer_u64_write(buf,                  \
        asn1_tag_integer, &num1));                                 \
    crefl_buf_dump(buf);                                           \
    crefl_buf_reset(buf);                                          \
    assert(!crefl_asn1_der_integer_u64_read(buf,                   \
        asn1_tag_integer, &num2));                                 \
    assert(num == num2);                                           \
    crefl_buf_destroy(buf);                                        \
}

T_DER_UINT(1,0)
T_DER_UINT(2,10)
T_DER_UINT(3,128)
T_DER_UINT(4,170)
T_DER_UINT(5,256)
T_DER_UINT(6,43690)
T_DER_UINT(7,65536)
T_DER_UINT(8,11184810)
T_DER_UINT(9,16777216)
T_DER_UINT(10,2863311530)
T_DER_UINT(11,4294967296)
T_DER_UINT(12,733007751850)
T_DER_UINT(13,1099511627776)
T_DER_UINT(14,72057594037927935)

#define T_DER_SINT(X,num)                                          \
void FN(der_sint,X)()                                              \
{                                                                  \
    s64 num1 = S64(num), num2;                                     \
    crefl_buf *buf;                                                \
    printf(der_uint_fmt, (s64)num, (s64)num);                      \
    assert((buf = crefl_buf_new(1024)));                           \
    assert(!crefl_asn1_der_integer_s64_write(buf,                  \
        asn1_tag_integer, &num1));                                 \
    crefl_buf_dump(buf);                                           \
    crefl_buf_reset(buf);                                          \
    assert(!crefl_asn1_der_integer_s64_read(buf,                   \
        asn1_tag_integer, &num2));                                 \
    assert(num == num2);                                           \
    crefl_buf_destroy(buf);                                        \
}

T_DER_SINT(1,0)
T_DER_SINT(2,10)
T_DER_SINT(3,128)
T_DER_SINT(4,170)
T_DER_SINT(5,256)
T_DER_SINT(6,43690)
T_DER_SINT(7,65536)
T_DER_SINT(8,11184810)
T_DER_SINT(9,16777216)
T_DER_SINT(10,2863311530)
T_DER_SINT(11,4294967296)
T_DER_SINT(12,733007751850)
T_DER_SINT(13,1099511627776)
T_DER_SINT(14,72057594037927935)

#define T_DER_OID(X)                                               \
void FN(der_oid,X)()                                               \
{                                                                  \
    char *str = oid_tests[X].str;                                  \
    u8 *der = oid_tests[X].der;                                    \
    size_t nbytes = oid_tests[X].nbytes;                           \
    asn1_oid oid1 = oid_tests[X].oid, oid2 = { 0 };                \
    asn1_hdr hdr = {                                               \
        { asn1_tag_object_identifier, 0, asn1_class_universal },   \
        crefl_asn1_ber_oid_length(&oid1)                           \
    };                                                             \
    crefl_buf *buf = crefl_buf_new(1024);                          \
    assert(buf);                                                   \
    assert(!crefl_asn1_der_oid_write(buf,                          \
        asn1_tag_object_identifier, &oid1));                       \
    printf(der_oid_fmt, str);                                      \
    crefl_buf_dump(buf);                                           \
    crefl_buf_reset(buf);                                          \
    assert(!crefl_asn1_der_oid_read(buf,                           \
        asn1_tag_object_identifier, &oid2));                       \
    assert(oid1.count == oid2.count);                              \
    assert(memcmp(oid1.oid, oid2.oid,                              \
        sizeof(u64) * oid1.count) ==0 );                           \
    crefl_buf_destroy(buf);                                        \
}

T_DER_OID(0)
T_DER_OID(1)
T_DER_OID(2)

#define T_DER_REAL(X,num)                                          \
void FN(der_real,X)()                                              \
{                                                                  \
    double num1 = (double)num, num2;                               \
    crefl_buf *buf;                                                \
    printf(der_real_fmt, (double)num);                             \
    assert((buf = crefl_buf_new(1024)));                           \
    assert(!crefl_asn1_der_real_f64_write(buf, asn1_tag_real,      \
        &num1));                                                   \
    crefl_buf_dump(buf);                                           \
    crefl_buf_reset(buf);                                          \
    assert(!crefl_asn1_der_real_f64_read(buf, asn1_tag_real,       \
        &num2));                                                   \
    assert(num == num2 || (isnan(num) && isnan(num2)));            \
    crefl_buf_destroy(buf);                                        \
}

T_DER_REAL(1,0.0)
T_DER_REAL(2,0.5)
T_DER_REAL(3,1.0)
T_DER_REAL(4,2.0)
T_DER_REAL(5,1/256.0)
T_DER_REAL(6,_f64_inf())
T_DER_REAL(7,-_f64_inf())
T_DER_REAL(8,-0.0)
T_DER_REAL(9,_f64_nan())
T_DER_REAL(10,3279/65536.0)
T_DER_REAL(11,0.1)
T_DER_REAL(12,2.71828182845904523536028747135266249)
T_DER_REAL(13,3.14159265358979323846264338327950288)
T_DER_REAL(14,1.77777777777777777777)
T_DER_REAL(15,1e307)

#define T_DER_OCTETS(X,str)                                        \
void FN(der_octets,X)()                                            \
{                                                                  \
    u8 str2[256];                                                  \
    size_t count;                                                  \
    crefl_buf *buf;                                                \
    asn1_string obj1 = { strlen(str), (u8*)str }, obj2 = { 0, (u8*)str2 }; \
    printf(der_octets_fmt, str);                                   \
    assert((buf = crefl_buf_new(1024)));                           \
    assert(!crefl_asn1_der_octets_write(buf, asn1_tag_octet_string,\
        &obj1));                                                   \
    crefl_buf_dump(buf);                                           \
    crefl_buf_reset(buf);                                          \
    obj2.count = 0;                                                \
    assert(!crefl_asn1_der_octets_read(buf, asn1_tag_octet_string, \
        &obj2));                                                   \
    assert(obj2.count == strlen(str));                             \
    crefl_buf_reset(buf);                                          \
    count = sizeof(str2);                                          \
    assert(!crefl_asn1_der_octets_read(buf, asn1_tag_octet_string, \
        &obj2));                                                   \
    assert(obj2.count == strlen(str));                             \
    assert(memcmp(str, str2, strlen(str)) == 0);                   \
    crefl_buf_destroy(buf);                                        \
}

T_DER_OCTETS(1,"")
T_DER_OCTETS(2,"hello")

int main()
{
    test_ber_tag_1();
    test_ber_tag_2();
    test_ber_tag_3();
    test_ber_tag_4();
    test_ber_tag_5();
    test_ber_tag_6();
    test_ber_tag_7();
    test_ber_tag_8();
    test_ber_tag_9();
    test_ber_tag_10();
    test_ber_tag_11();
    test_ber_tag_12();
    test_ber_tag_13();

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

    test_ber_uint_1();
    test_ber_uint_2();
    test_ber_uint_3();
    test_ber_uint_4();
    test_ber_uint_5();
    test_ber_uint_6();
    test_ber_uint_7();
    test_ber_uint_8();
    test_ber_uint_9();
    test_ber_uint_10();
    test_ber_uint_11();
    test_ber_uint_12();
    test_ber_uint_13();
    test_ber_uint_14();

    test_ber_sint_1();
    test_ber_sint_2();
    test_ber_sint_3();
    test_ber_sint_4();
    test_ber_sint_5();
    test_ber_sint_6();
    test_ber_sint_7();
    test_ber_sint_8();
    test_ber_sint_9();
    test_ber_sint_10();
    test_ber_sint_11();
    test_ber_sint_12();
    test_ber_sint_13();
    test_ber_sint_14();

    test_ber_oid_0();
    test_ber_oid_1();
    test_ber_oid_2();

    test_ber_octets_1();
    test_ber_octets_2();

    test_ber_real_1();
    test_ber_real_2();
    test_ber_real_3();
    test_ber_real_4();
    test_ber_real_5();
    test_ber_real_6();
    test_ber_real_7();
    test_ber_real_8();
    test_ber_real_9();
    test_ber_real_10();
    test_ber_real_11();
    test_ber_real_12();
    test_ber_real_13();
    test_ber_real_14();
    test_ber_real_15();

    test_der_bool_1();
    test_der_bool_2();

    test_der_uint_1();
    test_der_uint_2();
    test_der_uint_3();
    test_der_uint_4();
    test_der_uint_5();
    test_der_uint_6();
    test_der_uint_7();
    test_der_uint_8();
    test_der_uint_9();
    test_der_uint_10();
    test_der_uint_11();
    test_der_uint_12();
    test_der_uint_13();
    test_der_uint_14();

    test_der_sint_1();
    test_der_sint_2();
    test_der_sint_3();
    test_der_sint_4();
    test_der_sint_5();
    test_der_sint_6();
    test_der_sint_7();
    test_der_sint_8();
    test_der_sint_9();
    test_der_sint_10();
    test_der_sint_11();
    test_der_sint_12();
    test_der_sint_13();
    test_der_sint_14();

    test_der_oid_0();
    test_der_oid_1();
    test_der_oid_2();

    test_der_real_1();
    test_der_real_2();
    test_der_real_3();
    test_der_real_4();
    test_der_real_5();
    test_der_real_6();
    test_der_real_7();
    test_der_real_8();
    test_der_real_9();
    test_der_real_10();
    test_der_real_11();
    test_der_real_12();
    test_der_real_13();
    test_der_real_14();
    test_der_real_15();

    test_der_octets_1();
    test_der_octets_2();

    printf("\n");
}
