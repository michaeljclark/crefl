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
const char* ident_fmt =  "\nASN.1 X.690 ident(%zu)[0x%zx]\n";
const char* tagint_fmt = "\nASN.1 X.690 tagint(%zu)[0x%zx]\n";

#define FN(Y,X) test ## _ ## Y ## _ ## X
#define U64(X) X ## ll

#define T_LENGTH(X,num)                                    \
void FN(length,X)()                                        \
{                                                          \
    u64 num2;                                              \
    crefl_buf *buf = crefl_buf_new(1024);                  \
    assert(buf);                                           \
    crefl_asn1_length_write(buf, U64(num));                \
    printf(length_fmt, (size_t)num, (size_t)num);          \
    crefl_buf_dump(buf);                                   \
    crefl_buf_reset(buf);                                  \
    crefl_asn1_length_read(buf, &num2);                    \
    assert(num == num2);                                   \
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
    crefl_buf *buf = crefl_buf_new(1024);                  \
    assert(buf);                                           \
    crefl_asn1_tagnum_write(buf, U64(num));                \
    printf(tagnum_fmt, (size_t)num, (size_t)num);          \
    crefl_buf_dump(buf);                                   \
    crefl_buf_reset(buf);                                  \
    crefl_asn1_tagnum_read(buf, &num2);                    \
    assert(num == num2);                                   \
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
      asn1_class_universal, 0, U64(num)                    \
    };                                                     \
    asn1_id _id2;                                          \
    crefl_buf *buf = crefl_buf_new(1024);                  \
    assert(buf);                                           \
    crefl_asn1_ident_write(buf, _id1);                     \
    printf(ident_fmt, (size_t)num, (size_t)num);           \
    crefl_buf_dump(buf);                                   \
    crefl_buf_reset(buf);                                  \
    crefl_asn1_ident_read(buf, &_id2);                     \
    assert(_id1._class == _id1._class);                    \
    assert(_id1._constructed == _id1._constructed);        \
    assert(_id1._identifier == _id1._identifier);          \
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
    crefl_buf *buf = crefl_buf_new(1024);                  \
    assert(buf);                                           \
    crefl_asn1_tagged_integer_write(buf,                   \
        asn1_tag_integer, U64(num));                       \
    printf(tagint_fmt, (size_t)num, (size_t)num);          \
    crefl_buf_dump(buf);                                   \
    crefl_buf_reset(buf);                                  \
    crefl_asn1_tagged_integer_read(buf,                    \
        asn1_tag_integer, &num2);                          \
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

    printf("\n");
}
