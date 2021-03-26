/*
 * crefl runtime library and compiler plug-in to support reflection in C.
 *
 * Copyright (c) 2020 Michael Clark <michaeljclark@mac.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>

#include "stdendian.h"

#include "cmodel.h"
#include "cbuf.h"
#include "cbits.h"
#include "casn1.h"

/*
 * ASN.1 tag names
 */

const char* asn1_tag_names[32] = {
    /*  0 */ "reserved",
    /*  1 */ "boolean",
    /*  2 */ "integer",
    /*  3 */ "bit_string",
    /*  4 */ "octet_string",
    /*  5 */ "null",
    /*  6 */ "object_identifier",
    /*  7 */ "object_descriptor",
    /*  8 */ "external",
    /*  9 */ "real",
    /* 10 */ "enumerated",
    /* 11 */ "embedded_pdv",
    /* 12 */ "utf8_string",
    /* 13 */ "relative_oid",
    /* 14 */ "reserved_14",
    /* 15 */ "reserved_15",
    /* 16 */ "sequence",
    /* 17 */ "set",
    /* 18 */ "numeric_string",
    /* 19 */ "printable_string",
    /* 20 */ "t61_string",
    /* 21 */ "reserved_21",
    /* 22 */ "ia5_string",
    /* 23 */ "utc_time",
    /* 24 */ "generalized_time",
    /* 25 */ "graphic_string",
    /* 26 */ "iso646_string",
    /* 27 */ "general_string",
    /* 28 */ "utf32_string",
    /* 29 */ "reserved_29",
    /* 30 */ "utf16_string",
    /* 31 */ "reserved_31",
};

const char* asn1_tag_name(u64 tag)
{
    return (tag < 32) ? asn1_tag_names[tag] : "<unknown>";
}

/*
 * structure of ASN.1 tagged data
 *
 * |------------------------|
 * |   identifier octets    |
 * |------------------------|
 * |     length octets      |
 * |------------------------|
 * |    contents octets     |
 * |------------------------|
 * | end-of-contents octets |
 * |------------------------|
 */

/*
 * ISO/IEC 8825-1:2003 8.1.2.4.2 subsequent octets
 *
 * read and write 56-bit identifier high tag
 * called by ident_read | ident_write if low tag == 0b11111
 */

size_t crefl_asn1_tagnum_length(u64 tag)
{
    return tag == 0 ? 1 : 8 - ((clz(tag) - 1) / 7) + 1;
}

int crefl_asn1_tagnum_read(crefl_buf *buf, u64 *tag)
{
    int8_t b;
    size_t w = 0;
    u64 l = 0;

    do {
        if (crefl_buf_read_i8(buf, &b) != 1) {
            goto err;
        }
        l <<= 7;
        l |= (uint8_t)b & 0x7f;
        w += 7;
    } while ((b & 0x80) && w < 56);

    if (w > 56) {
        goto err;
    }

    *tag = l;
    return 0;
err:
    *tag = 0;
    return -1;
}

int crefl_asn1_tagnum_write(crefl_buf *buf, u64 tag)
{
    int8_t b;
    size_t llen;
    u64 l = 0;

    if (tag >= (1ull << 56)) {
        goto err;
    }

    llen = crefl_asn1_tagnum_length(tag);
    l = tag << (64 - llen * 7);
    for (size_t i = 0; i < llen; i++) {
        b = ((l >> 57) & 0x7f);
        b |= (i != llen - 1) << 7;
        l <<= 7;
        if (crefl_buf_write_i8(buf, b) != 1) {
            goto err;
        }
    }

    return 0;
err:
    return -1;
}

/*
 * ISO/IEC 8825-1:2003 8.1.2 identifier
 *
 * read and write identifier
 */

size_t crefl_asn1_ber_ident_length(asn1_id _id)
{
    return 1 + ((_id._identifier >= 0x1f) ?
        crefl_asn1_tagnum_length(_id._identifier) : 0);
}

int crefl_asn1_ber_ident_read(crefl_buf *buf, asn1_id *_id)
{
    int8_t b;
    asn1_id r = { 0 };

    if (crefl_buf_read_i8(buf, &b) != 1) {
        goto err;
    }

    _id->_class =       (b >> 6) & 0x02;
    _id->_constructed = (b >> 5) & 0x01;
    _id->_identifier =   b       & 0x1f;

    if (_id->_identifier == 0x1f) {
        u64 tagnum;
        if (crefl_asn1_tagnum_read(buf, &tagnum) < 0) {
            goto err;
        }
        if (tagnum < 0x1f) {
            goto err;
        }
        _id->_identifier = tagnum;
    }

    return 0;
err:
    return -1;
}

int crefl_asn1_ber_ident_write(crefl_buf *buf, asn1_id _id)
{
    int8_t b;

    b = ( (u8)(_id._class       & 0x02) << 6 ) |
        ( (u8)(_id._constructed & 0x01) << 5 ) |
        ( (u8)(_id._identifier < 0x1f ? _id._identifier : 0x1f) );

    if (crefl_buf_write_i8(buf, b) != 1) {
        goto err;
    }

    if (_id._identifier >= 0x1f) {
        if (crefl_asn1_tagnum_write(buf, _id._identifier) < 0) {
            goto err;
        }
    }

    return 0;
err:
    return -1;
}

/*
 * ISO/IEC 8825-1:2003 8.1.3 length
 *
 * read and write 64-bit length
 */

size_t crefl_asn1_ber_length_length(u64 length)
{
    return 1 + ((length >= 0x80) ? 8 - (clz(length) / 8) : 0);
}

int crefl_asn1_ber_length_read(crefl_buf *buf, u64 *length)
{
    int8_t b;
    size_t llen = 0;
    u64 l = 0;

    if (crefl_buf_read_i8(buf, &b) != 1) {
        goto err;
    }
    if ((b & 0x80) == 0) {
        l = b & 0x7f;
        goto out;
    }
    llen = b & 0x7f;
    if (llen == 0) {
        // indefinate form not supported
        goto err;
    }
    else if (llen > 8) {
        goto err;
    }
    for (size_t i = 0; i < llen; i++) {
        if (crefl_buf_read_i8(buf, &b) != 1) {
            goto err;
        }
        l <<= 8;
        l |= (uint8_t)b;
    }

out:
    *length = l;
    return 0;
err:
    *length = 0;
    return -1;
}

int crefl_asn1_ber_length_write(crefl_buf *buf, u64 length)
{
    int8_t b;
    size_t llen;
    u64 l = 0;

    if (length <= 0x7f) {
        b = (int8_t)length;
        if (crefl_buf_write_i8(buf, b) != 1) {
            goto err;
        }
        return 0;
    }
    // indefinate form not supported

    llen = 8 - (clz(length) / 8);
    b = (u8)llen | (u8)0x80;
    if (crefl_buf_write_i8(buf, b) != 1) {
        goto err;
    }

    l = length << (64 - llen * 8);
    for (size_t i = 0; i < llen; i++) {
        b = (l >> 56) & 0xff;
        l <<= 8;
        if (crefl_buf_write_i8(buf, b) != 1) {
            goto err;
        }
    }

    return 0;
err:
    return -1;
}

/*
 * ISO/IEC 8825-1:2003 8.2 boolean
 *
 * read and write boolean
 */

size_t crefl_asn1_ber_boolean_length(bool value)
{
    return 1;
}

int crefl_asn1_ber_boolean_read(crefl_buf *buf, size_t len, bool *value)
{
    int8_t b;

    if (crefl_buf_read_i8(buf, &b) != 1) {
        goto err;
    }

    *value = b;
    return 0;
err:
    *value = 0;
    return -1;
}

int crefl_asn1_ber_boolean_write(crefl_buf *buf, size_t len, bool value)
{
    int8_t b;

    b = value;
    if (crefl_buf_write_i8(buf, b) != 1) {
        goto err;
    }

    return 0;
err:
    return -1;
}

int crefl_asn1_der_boolean_read(crefl_buf *buf, asn1_tag _tag, bool *value)
{
    asn1_hdr hdr;
    if (crefl_asn1_ber_ident_read(buf, &hdr._id) < 0) goto err;
    if (crefl_asn1_ber_length_read(buf, &hdr._length) < 0) goto err;
    if (crefl_asn1_ber_boolean_read(buf, hdr._length, value) < 0) goto err;
    return 0;
err:
    return -1;
}

int crefl_asn1_der_boolean_write(crefl_buf *buf, asn1_tag _tag, bool value)
{
    asn1_hdr hdr = {
        { (u64)_tag, 0, asn1_class_universal }, crefl_asn1_ber_boolean_length(value)
    };

    if (crefl_asn1_ber_ident_write(buf, hdr._id) < 0) goto err;
    if (crefl_asn1_ber_length_write(buf, hdr._length) < 0) goto err;
    if (crefl_asn1_ber_boolean_write(buf, hdr._length, value) < 0) goto err;

    return 0;
err:
    return -1;
}

/*
 * ISO/IEC 8825-1:2003 8.3 integer
 *
 * read and write integer
 */

size_t crefl_asn1_ber_integer_u64_length(u64 value)
{
    return value == 0 ? 1 : 8 - (clz(value) / 8);
}

int crefl_asn1_ber_integer_u64_read(crefl_buf *buf, size_t len, u64 *value)
{
    int8_t b;
    u64 v = 0;

    if (len > 8) {
        goto err;
    }
    for (size_t i = 0; i < len; i++) {
        if (crefl_buf_read_i8(buf, &b) != 1) {
            goto err;
        }
        v <<= 8;
        v |= (uint8_t)b;
    }

    *value = v;
    return 0;
err:
    *value = 0;
    return -1;
}

int crefl_asn1_ber_integer_u64_write(crefl_buf *buf, size_t len, u64 value)
{
    int8_t b;
    u64 v = 0;

    if (len < 1 || len > 8) {
        goto err;
    }
    v = value << (64 - len * 8);
    for (size_t i = 0; i < len; i++) {
        b = (v >> 56) & 0xff;
        v <<= 8;
        if (crefl_buf_write_i8(buf, b) != 1) {
            goto err;
        }
    }

    return 0;
err:
    return -1;
}

/*
 * ASN.1 does not distinguish between signed and unsigned integers.
 * signed number deserialization requires that values are sign-extended.
 * negative values are complemented and 1-bit is reserved for the sign.
 *
 * - 0x000000000000007f -> 0x7f
 * - 0x0000000000000080 -> 0x0080
 * - 0xffffffffffffff80 -> 0x80
 * - 0xffffffffffffff7f -> 0xff7f
 */
size_t crefl_asn1_ber_integer_s64_length(s64 value)
{
    return value == 0 ? 1 : 8 - ((clz(value < 0 ? ~value : value)-1) / 8);
}

static s64 _sign_extend_s64(s64 x, size_t y) { return ((s64)(x << y)) >> y; }

int crefl_asn1_ber_integer_s64_read(crefl_buf *buf, size_t len, s64 *value)
{
    int ret = crefl_asn1_ber_integer_u64_read(buf, len, (u64*)value);
    if (ret == 0) {
        *value = _sign_extend_s64(*value, 64-(len << 3));
    }
    return ret;
}

int crefl_asn1_ber_integer_s64_write(crefl_buf *buf, size_t len, s64 value)
{
    return crefl_asn1_ber_integer_u64_write(buf, len, (u64)value);
}

/*
 * read and write tagged integer
 */

int crefl_asn1_der_integer_u64_read(crefl_buf *buf, asn1_tag _tag, u64 *value)
{
    asn1_hdr hdr;
    if (crefl_asn1_ber_ident_read(buf, &hdr._id) < 0) goto err;
    if (crefl_asn1_ber_length_read(buf, &hdr._length) < 0) goto err;
    if (crefl_asn1_ber_integer_u64_read(buf, hdr._length, value) < 0) goto err;
    return 0;
err:
    return -1;
}

int crefl_asn1_der_integer_u64_write(crefl_buf *buf, asn1_tag _tag, u64 value)
{
    asn1_hdr hdr = {
        { (u64)_tag, 0, asn1_class_universal }, crefl_asn1_ber_integer_u64_length(value)
    };

    if (crefl_asn1_ber_ident_write(buf, hdr._id) < 0) goto err;
    if (crefl_asn1_ber_length_write(buf, hdr._length) < 0) goto err;
    if (crefl_asn1_ber_integer_u64_write(buf, hdr._length, value) < 0) goto err;

    return 0;
err:
    return -1;
}

int crefl_asn1_der_integer_s64_read(crefl_buf *buf, asn1_tag _tag, s64 *value)
{
    asn1_hdr hdr;
    if (crefl_asn1_ber_ident_read(buf, &hdr._id) < 0) goto err;
    if (crefl_asn1_ber_length_read(buf, &hdr._length) < 0) goto err;
    if (crefl_asn1_ber_integer_s64_read(buf, hdr._length, value) < 0) goto err;
    return 0;
err:
    return -1;
}

int crefl_asn1_der_integer_s64_write(crefl_buf *buf, asn1_tag _tag, s64 value)
{
    asn1_hdr hdr = {
        { (u64)_tag, 0, asn1_class_universal }, crefl_asn1_ber_integer_s64_length(value)
    };

    if (crefl_asn1_ber_ident_write(buf, hdr._id) < 0) goto err;
    if (crefl_asn1_ber_length_write(buf, hdr._length) < 0) goto err;
    if (crefl_asn1_ber_integer_s64_write(buf, hdr._length, value) < 0) goto err;

    return 0;
err:
    return -1;
}

/*
 * IEEE 754 encoding and decoding functions
 */

union f32_bits { f32 f; u32 u; };
union f64_bits { f64 f; u64 u; };

struct f32_struct { u32 mant : 23; u32 exp : 8;  u32 sign : 1; };
struct f64_struct { u64 mant : 52; u64 exp : 11; u64 sign : 1; };

enum : u32 {
    f32_exp_size = 8,
    f32_mant_size = 23,

    f32_mant_shift = 0,
    f32_exp_shift = f32_mant_size,
    f32_sign_shift = f32_mant_size + f32_exp_size,

    f32_mant_mask = (1 << f32_mant_size) - 1,
    f32_exp_mask = (1 << f32_exp_size) - 1,
    f32_sign_mask = 1,

    f32_mant_prefix = (1ull << f32_mant_size),
    f32_exp_bias = (1 << (f32_exp_size-1)) - 1
};

enum : u64 {
    f64_exp_size = 11,
    f64_mant_size = 52,

    f64_mant_shift = 0,
    f64_exp_shift = f64_mant_size,
    f64_sign_shift = f64_mant_size + f64_exp_size,

    f64_mant_mask = (1ull << f64_mant_size) - 1,
    f64_exp_mask = (1ull << f64_exp_size) - 1,
    f64_sign_mask = 1ull,

    f64_mant_prefix = (1ull << f64_mant_size),
    f64_exp_bias = (1 << (f64_exp_size-1)) - 1
};

static u32 f32_mant_dec(f32 x) { return (f32_bits{x}.u >> f32_mant_shift) & f32_mant_mask; }
static u32 f32_exp_dec(f32 x) { return (f32_bits{x}.u >> f32_exp_shift) & f32_exp_mask; }
static u32 f32_sign_dec(f32 x) { return (f32_bits{x}.u >> f32_sign_shift) & f32_sign_mask; }
static u32 f32_mant_enc(u32 v) { return ((v & f32_mant_mask) << f32_mant_shift); }
static u32 f32_exp_enc(u32 v) { return ((v & f32_exp_mask) << f32_exp_shift); }
static u32 f32_sign_enc(u32 v) { return ((v & f32_sign_mask) << f32_sign_shift); }
static int f32_is_zero(f32 x) { return f32_exp_dec(x) == 0 && f32_mant_dec(x) == 0; }
static int f32_is_inf(f32 x) { return f32_exp_dec(x) == f32_exp_mask && f32_mant_dec(x) == 0; }
static int f32_is_nan(f32 x) { return f32_exp_dec(x) == f32_exp_mask && f32_mant_dec(x) != 0; }
static int f32_is_denorm(f32 x) { return f32_exp_dec(x) == 0 && f32_mant_dec(x) != 0; }

static f32_struct f32_unpack_float(f32 x)
{
    return { f32_mant_dec(x), f32_exp_dec(x), f32_sign_dec(x) };
}

static f32 f32_pack_float(f32_struct s)
{
    union { u32 u; f32 f; };
    u = f32_mant_enc(s.mant) | f32_exp_enc(s.exp) | f32_sign_enc(s.sign);
    return f;
}

static u64 f64_mant_dec(f64 x) { return (f64_bits{x}.u >> f64_mant_shift) & f64_mant_mask; }
static u64 f64_exp_dec(f64 x) { return (f64_bits{x}.u >> f64_exp_shift) & f64_exp_mask; }
static u64 f64_sign_dec(f64 x) { return (f64_bits{x}.u >> f64_sign_shift) & f64_sign_mask; }
static u64 f64_mant_enc(u64 v) { return ((v & f64_mant_mask) << f64_mant_shift); }
static u64 f64_exp_enc(u64 v) { return ((v & f64_exp_mask) << f64_exp_shift); }
static u64 f64_sign_enc(u64 v) { return ((v & f64_sign_mask) << f64_sign_shift); }
static int f64_is_zero(f64 x) { return f64_exp_dec(x) == 0 && f64_mant_dec(x) == 0; }
static int f64_is_inf(f64 x) { return f64_exp_dec(x) == f64_exp_mask && f64_mant_dec(x) == 0; }
static int f64_is_nan(f64 x) { return f64_exp_dec(x) == f64_exp_mask && f64_mant_dec(x) != 0; }
static int f64_is_denorm(f64 x) { return f64_exp_dec(x) == 0 && f64_mant_dec(x) != 0; }

static f64_struct f64_unpack_float(f64 x)
{
    return { f64_mant_dec(x), f64_exp_dec(x), f64_sign_dec(x) };
}

static f64 f64_pack_float(f64_struct s)
{
    union { f64 f; u64 u; };
    u = f64_mant_enc(s.mant) | f64_exp_enc(s.exp) | f64_sign_enc(s.sign);
    return f;
}

float _f32_nan() { return std::numeric_limits<float>::quiet_NaN(); }
float _f32_pos_inf() { return std::numeric_limits<float>::infinity(); }
float _f32_neg_inf() { return -std::numeric_limits<float>::infinity(); }
double _f64_nan() { return std::numeric_limits<double>::quiet_NaN(); }
double _f64_pos_inf() { return std::numeric_limits<double>::infinity(); }
double _f64_neg_inf() { return -std::numeric_limits<double>::infinity(); }

/*
 * ISO/IEC 8825-1:2003 8.5 real
 *
 * read and write real
 *
 * ASN.1 REAL encoding bits from first content byte
 *
 * - 8.5.6 encoding-format
 *   - [8]   0b'1 = binary
 *   - [8:7] 0b'00 = decimal
 *   - [8:7] 0b'01 = special real value
 * - 8.5.7.1 sign (if bit 8 = 0b1)
 *   - [7]   sign S
 * - 8.5.7.2 base (if bit 8 = 0b1)
 *   - [6:5] 0b'00 = base 2
 *   - [6:5] 0b'01 = base 8
 *   - [6:5] 0b'10 = base 16
 *   - [6:5] 0b'11 = reserved
 * - 8.5.7.3 scale-factor (if bit 8 = 0b1)
 *   - [4:3] unsigned scale factor F
 * - 8.5.7.4 exponent-format (if bit 8 = 0b1)
 *   - [2:1] 0b'00 = 1-byte signed exponent in 2nd octet
 *   - [2:1] 0b'01 = 2-byte signed exponent in 2nd to 3rd octet
 *   - [2:1] 0b'10 = 3-byte signed exponent in 2nd to 4rd octet
 *   - [2:1] 0b'11 = 2nd octet contains number of exponent octets
 * - 8.5.8 decimal encoding (if bit 8:7 = 0b00)
 *   - [8:1] 0b'00000001 ISO 6093 NR1 form
 *   - [8:1] 0b'00000010 ISO 6093 NR2 form
 *   - [8:1] 0b'00000011 ISO 6093 NR3 form
 * - 8.5.9 special real value (if bit 8:7 = 0b01)
 *   - [8:1] 0b'01000000 Value is PLUS-INFINITY
 *   - [8:1] 0b'01000001 Value is MINUS-INFINITY
 *   - [8:1] 0b'01000010 Value is NOT-A-NUMBER
 *   - [8:1] 0b'01000011 Value is minus zero
 *
 *  binary encoding: M = S × N × 2^F x (2,8,16)^E
 *  decimal encoding: /[0-9]+\.[0-9]+([eE][+-]?[0-9]+)?/
 */

enum {
    _real_fmt_shift         = 6,
    _real_fmt_mask          = 0b11,
    _real_base_shift        = 4,
    _real_base_mask         = 0b11,
    _real_scale_shift       = 2,
    _real_scale_mask        = 0b11,
    _real_exp_shift         = 0,
    _real_exp_mask          = 0b11,
};

enum _real_fmt {
    _real_fmt_decimal       = 0b00,
    _real_fmt_special       = 0b01,
    _real_fmt_binary_pos    = 0b10,
    _real_fmt_binary_neg    = 0b11
};

enum _real_base {
    _real_base_2            = 0b00,
    _real_base_8            = 0b01,
    _real_base_16           = 0b10
};

enum _real_exp {
    _real_exp_1             = 0b00,
    _real_exp_2             = 0b01,
    _real_exp_3             = 0b10,
    _real_exp_n             = 0b11
};

enum _real_decimal_nr {
    _real_decimal_nr_1      = 0b00000001,
    _real_decimal_nr_2      = 0b00000010,
    _real_decimal_nr_3      = 0b00000011,
};

enum _real_special {
    _real_special_pos_inf   = 0b01000000,
    _real_special_neg_inf   = 0b01000001,
    _real_special_neg_zero  = 0b01000010,
    _real_special_nan       = 0b01000011,
};

static _real_fmt _asn1_real_format(u8 x)
{
    return (_real_fmt)((x >> _real_fmt_shift) & _real_fmt_mask);
}

static _real_exp _asn1_real_exp(u8 x)
{
    return (_real_exp)((x >> _real_exp_shift) & _real_exp_mask);
}

static u8 _asn1_real_binary_full(bool sign, _real_exp exp, _real_base base, u8 scale)
{
    return 0x80 | ((u8)sign<<6)  | (((u8)base)<<4) | ((scale&3)<<2) | (u8)exp;
}

static u8 _asn1_real_binary(bool sign, _real_exp exponent)
{
    return _asn1_real_binary_full(sign, exponent, _real_base_2, 0);
}

/*
 * f64_asn1_data contains fraction, signed exponent, their
 * encoded lengths and flags for sign, infinity, nan and zero.
 */
struct f64_asn1_data
{
    s64 frac, sexp;
    size_t frac_len, exp_len;
    bool sign : 1, inf : 1, nan : 1, zero : 1;
};

/*
 * IEEE 754 exponent is relative to the msb of the mantissa
 * ASN.1 exponent is relative to the lsb of the mantissa
 *
 * right-justify the fraction with the least significant set in bit 1
 * then add the IEEE 754 implied leading digit 0b1.xxx prefix
 */
static f64_asn1_data f64_asn1_data_get(double value)
{
    s64 frac, sexp;
    size_t frac_tz, frac_lz;

    sexp = (s64)f64_exp_dec(value);
    frac = (s64)f64_mant_dec(value) + (-(s64)sexp & f64_mant_prefix);
    frac_tz = ctz(frac);
    frac_lz = clz(frac);
    frac >>= frac_tz;

    if (sexp > 0) {
        sexp += frac_lz + frac_tz - 63 - f64_exp_bias;
    }

    return f64_asn1_data {
        frac, sexp,
        crefl_asn1_ber_integer_u64_length(frac),
        crefl_asn1_ber_integer_s64_length(sexp),
        !!f64_sign_dec(value), !!f64_is_inf(value),
        !!f64_is_nan(value), !!f64_is_zero(value)
    };
}

size_t crefl_asn1_ber_real_f64_length(double value)
{
    f64_asn1_data d = f64_asn1_data_get(value);

    if (d.zero) {
        return d.sign ? 1 : 3;
    } else if (d.inf || d.nan) {
        return 1;
    } else {
        return 1 + d.exp_len + d.frac_len;
    }
}

int crefl_asn1_ber_real_f64_read(crefl_buf *buf, size_t len, double *value)
{
    int8_t b;
    double v = 0;
    _real_fmt fmt;
    _real_exp exp_mode;
    size_t frac_len;
    size_t exp_len;
    u64 frac;
    s64 sexp;
    u64 fexp;
    bool sign;
    size_t frac_lz;

    if (crefl_buf_read_i8(buf, &b) != 1) {
        goto err;
    }
    fmt = _asn1_real_format(b);
    switch (b) {
    case _real_special_pos_inf:  *value = std::numeric_limits<f64>::infinity();  return 0;
    case _real_special_neg_inf:  *value = -std::numeric_limits<f64>::infinity(); return 0;
    case _real_special_neg_zero: *value = -0.0;     return 0;
    case _real_special_nan:      *value = std::numeric_limits<f64>::quiet_NaN();  return 0;
    default: break;
    }
    switch (fmt) {
    case _real_fmt_binary_pos: sign = false; break;
    case _real_fmt_binary_neg: sign = true; break;
    default: return -1;
    }
    exp_mode = _asn1_real_exp(b);
    switch(exp_mode) {
    case _real_exp_1: exp_len = 1; break;
    case _real_exp_2: exp_len = 2; break;
    default: return -1;
    }
    frac_len = len - exp_len - 1;

    if (crefl_asn1_ber_integer_s64_read(buf, exp_len, &sexp) < 0) {
        goto err;
    }
    if (crefl_asn1_ber_integer_u64_read(buf, frac_len, &frac) < 0) {
        goto err;
    }
    frac_lz = clz(frac);

    /*
     * IEEE 754 exponent is relative to the msb of the mantissa
     * ASN.1 exponent is relative to the lsb of the mantissa
     *
     * left-justify the fraction with the most significant set in bit 52
     * shift 1-bit further to crop off the IEEE 754 implied digit 0b1.xxx
     */
    if (frac == 1 && sexp == 0) {
        frac = 0;
        fexp = f64_exp_bias;
    } else if (frac == 0 && sexp == 0) {
        fexp = 0;
    } else {
        frac = (frac << (frac_lz + 1)) >> (64 - f64_mant_size);
        fexp = f64_exp_bias + 63 + sexp - frac_lz;
    }
    if (fexp > f64_exp_mask || frac > f64_mant_mask) {
        return -1;
    }
    v = f64_pack_float(f64_struct{frac, fexp, sign});

    *value = v;
    return 0;
err:
    *value = 0;
    return -1;
}

int crefl_asn1_ber_real_f64_write(crefl_buf *buf, size_t len, double value)
{
    f64_asn1_data d = f64_asn1_data_get(value);

    int8_t b;

    if (d.zero && d.sign) {
        b = _real_special_neg_zero;
    }
    else if (d.inf) {
        b = d.sign ? _real_special_neg_inf : _real_special_pos_inf;
    }
    else if (d.nan) {
        b = _real_special_nan;
    } else {
        _real_exp exp_code;
        switch(d.exp_len) {
        case 1: exp_code = _real_exp_1; break;
        case 2: exp_code = _real_exp_2; break;
        default: return -1;
        }
        b = _asn1_real_binary(d.sign, exp_code);
    }
    if (crefl_buf_write_i8(buf, b) != 1) {
        goto err;
    }
    if ((d.zero && d.sign) || d.inf || d.nan) {
        return 0;
    }
    if (crefl_asn1_ber_integer_s64_write(buf, d.exp_len, d.sexp) < 0) {
        goto err;
    }
    if (crefl_asn1_ber_integer_u64_write(buf, d.frac_len, d.frac) < 0) {
        goto err;
    }

    return 0;
err:
    return -1;
}

int crefl_asn1_der_real_f64_read(crefl_buf *buf, asn1_tag _tag, double *value)
{
    asn1_hdr hdr;
    if (crefl_asn1_ber_ident_read(buf, &hdr._id) < 0) goto err;
    if (crefl_asn1_ber_length_read(buf, &hdr._length) < 0) goto err;
    if (crefl_asn1_ber_real_f64_read(buf, hdr._length, value) < 0) goto err;
    return 0;
err:
    return -1;
}

int crefl_asn1_der_real_f64_write(crefl_buf *buf, asn1_tag _tag, double value)
{
    asn1_hdr hdr = {
        { (u64)_tag, 0, asn1_class_universal }, crefl_asn1_ber_real_f64_length(value)
    };

    if (crefl_asn1_ber_ident_write(buf, hdr._id) < 0) goto err;
    if (crefl_asn1_ber_length_write(buf, hdr._length) < 0) goto err;
    if (crefl_asn1_ber_real_f64_write(buf, hdr._length, value) < 0) goto err;

    return 0;
err:
    return -1;
}

/*
 * ISO/IEC 8825-1:2003 8.19 object identifier value
 *
 * read and write object identifier value
 */

size_t crefl_asn1_ber_oid_length(u64 *oid, size_t count)
{
    size_t length = 0;
    for (size_t i = 0; i < count; i++) {
        /*
         * 8.19.4 rule where first two components are combined -> (X*40) + Y
         */
        if (i == 0 && count > 1) {
            length = crefl_asn1_tagnum_length(oid[0] * 40 + oid[1]);
            i++;
        } else {
            length += crefl_asn1_tagnum_length(oid[i]);
        }
    }
    return length;
}

int crefl_asn1_ber_oid_read(crefl_buf *buf, size_t len, u64 *oid, size_t *count)
{
    size_t start = crefl_buf_offset(buf), offset = start;
    size_t n = 0, limit = *count;
    u64 comp;

    while ((offset - start) < len) {
        if (crefl_asn1_tagnum_read(buf, &comp) < 0) goto err;
        /*
         * 8.19.4 rule where first two components are combined -> (X*40) + Y
         */
        if (n == 0 && comp > 40) {
            if (n < limit && oid) oid[n] = comp/40;
            n++;
            if (n < limit && oid) oid[n] = comp%40;
            n++;
        }
        else {
            if (n < limit && oid) oid[n] = comp;
            n++;
        }
        offset = crefl_buf_offset(buf);
    }
    *count = n;
    return 0;
err:
    *count = 0;
    return -1;
}

int crefl_asn1_ber_oid_write(crefl_buf *buf, size_t len, u64 *oid, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        /*
         * 8.19.4 rule where first two components are combined -> (X*40) + Y
         */
        if (i == 0 && count > 1) {
            if (crefl_asn1_tagnum_write(buf, oid[0] * 40 + oid[1]) < 0) goto err;
            i++;
        } else {
            if (crefl_asn1_tagnum_write(buf, oid[i]) < 0) goto err;
        }
    }

    return 0;
err:
    return -1;
}

size_t crefl_asn1_oid_to_string(char *buf, size_t buflen, u64 *oid, size_t count)
{
    if (buf && buflen) {
        buf[0] = '\0';
    }
    size_t offset = 0;
    for (size_t i = 0; i < count; i ++) {
        offset += snprintf(
            (buflen ? buf + offset : NULL),
            (buflen ? buflen - offset : 0),
            (i == 0 ? "%lld" : ".%lld"),
            oid[i]
        );
    }
    return offset;
}

int crefl_asn1_der_oid_read(crefl_buf *buf, asn1_tag _tag, u64 *oid, size_t *count)
{
    asn1_hdr hdr;
    if (crefl_asn1_ber_ident_read(buf, &hdr._id) < 0) goto err;
    if (crefl_asn1_ber_length_read(buf, &hdr._length) < 0) goto err;
    if (crefl_asn1_ber_oid_read(buf, hdr._length, oid, count) < 0) goto err;
    return 0;
err:
    return -1;
}

int crefl_asn1_der_oid_write(crefl_buf *buf, asn1_tag _tag, u64 *oid, size_t count)
{
    asn1_hdr hdr = {
        { (u64)_tag, 0, asn1_class_universal }, crefl_asn1_ber_oid_length(oid, count)
    };

    if (crefl_asn1_ber_ident_write(buf, hdr._id) < 0) goto err;
    if (crefl_asn1_ber_length_write(buf, hdr._length) < 0) goto err;
    if (crefl_asn1_ber_oid_write(buf, hdr._length, oid, count) < 0) goto err;

    return 0;
err:
    return -1;
}

/*
 * ISO/IEC 8825-1:2003 8.7 octet string
 *
 * read and write octet string
 *
 * the two lengths arrangement is due to the pattern that BER functions
 * take the length of the object from the header and the *count parameter
 * is the user buffer size. the read and write routines will seek length
 * bytes in the buffer so we are aligned to the next object but will copy
 * min(length, *count), and for read, place that value in *count.
 *
 * count is not redundant, because this mechanism allows the user to read
 * a truncated value, while advancing the buffer based on the object length.
 * if the string buffer passed to the read function is NULL, then the length
 * will be returned with no copy taking place. the is relevant for the DER
 * interface where the user does not know the length in advance.
 */

size_t crefl_asn1_ber_octets_length(u8 *str, size_t count)
{
    return count;
}

int crefl_asn1_ber_octets_read(crefl_buf *buf, size_t len, u8 *str, size_t *count)
{
    size_t copy_count = len > *count ? *count : len;

    crefl_span span = crefl_buf_remaining(buf);
    if (span.length < copy_count) {
        return -1;
    }

    if (str) {
        memcpy(str, span.data, copy_count);
    }
    crefl_buf_seek(buf, crefl_buf_offset(buf) + len);
    *count = len;

    return 0;
}

int crefl_asn1_ber_octets_write(crefl_buf *buf, size_t len, u8 *str, size_t count)
{
    size_t copy_count = len > count ? count : len;

    crefl_span span = crefl_buf_remaining(buf);
    if (span.length < copy_count) {
        return -1;
    }

    memcpy(span.data, str, copy_count);
    crefl_buf_seek(buf, crefl_buf_offset(buf) + len);

    return 0;
}

int crefl_asn1_der_octets_read(crefl_buf *buf, asn1_tag _tag, u8 *str, size_t *count)
{
    asn1_hdr hdr;
    if (crefl_asn1_ber_ident_read(buf, &hdr._id) < 0) goto err;
    if (crefl_asn1_ber_length_read(buf, &hdr._length) < 0) goto err;
    return crefl_asn1_ber_octets_read(buf, hdr._length, str, count);
err:
    return -1;
}

int crefl_asn1_der_octets_write(crefl_buf *buf, asn1_tag _tag, u8 *str, size_t count)
{
    asn1_hdr hdr = {
        { (u64)_tag, 0, asn1_class_universal }, crefl_asn1_ber_octets_length(str, count)
    };

    if (crefl_asn1_ber_ident_write(buf, hdr._id) < 0) goto err;
    if (crefl_asn1_ber_length_write(buf, hdr._length) < 0) goto err;
    return crefl_asn1_ber_octets_write(buf, hdr._length, str, count);
err:
    return -1;
}

/*
 * ISO/IEC 8825-1:2003 8.8 null
 *
 * read and write null
 *
 */

size_t crefl_asn1_ber_null_length()
{
    return 0;
}

int crefl_asn1_ber_null_read(crefl_buf *buf, size_t len)
{
    return len == 0 ? 0 : -1;
}

int crefl_asn1_ber_null_write(crefl_buf *buf, size_t len)
{
    return len == 0 ? 0 : -1;
}

int crefl_asn1_der_null_read(crefl_buf *buf, asn1_tag _tag)
{
    asn1_hdr hdr;
    if (crefl_asn1_ber_ident_read(buf, &hdr._id) < 0) return -1;
    if (crefl_asn1_ber_length_read(buf, &hdr._length) < 0) return -1;
    return crefl_asn1_ber_null_read(buf, hdr._length);
}

int crefl_asn1_der_null_write(crefl_buf *buf, asn1_tag _tag)
{
    asn1_hdr hdr = {
        { (u64)_tag, 0, asn1_class_universal }, crefl_asn1_ber_null_length()
    };

    if (crefl_asn1_ber_ident_write(buf, hdr._id) < 0) return -1;
    if (crefl_asn1_ber_length_write(buf, hdr._length) < 0) return -1;
    return crefl_asn1_ber_null_write(buf, hdr._length);
}
