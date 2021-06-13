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

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "cmodel.h"
#include "cbuf.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ASN.1 Classes
 */

typedef enum {
	asn1_class_universal        = 0b00,
	asn1_class_application      = 0b01,
	asn1_class_context_specific = 0b10,
	asn1_class_private          = 0b11
} asn1_class;

/*
 * ASN.1 Identifiers
 */

typedef enum {
	asn1_tag_reserved           = 0,
	asn1_tag_boolean            = 1,
	asn1_tag_integer            = 2,
	asn1_tag_bit_string         = 3,
	asn1_tag_octet_string       = 4,
	asn1_tag_null               = 5,
	asn1_tag_object_identifier  = 6,
	asn1_tag_object_descriptor  = 7,
	asn1_tag_external           = 8,
	asn1_tag_real               = 9,
	asn1_tag_enumerated         = 10,
	asn1_tag_embedded_pdv       = 11,
	asn1_tag_relative_oid       = 13,

	asn1_tag_sequence           = 16,
	asn1_tag_set                = 17,

	asn1_tag_utf8_string        = 12,
	asn1_tag_numeric_string     = 18,
	asn1_tag_printable_string   = 19,
	asn1_tag_teletext_string    = 20,
	asn1_tag_ia5_string         = 22,
	asn1_tag_graphic_string     = 25,
	asn1_tag_visible_string     = 26,
	asn1_tag_general_string     = 27,
	asn1_tag_universal_string   = 28,
	asn1_tag_bmp_string         = 30,

	asn1_tag_t61_string         = asn1_tag_teletext_string,
	asn1_tag_iso646_string      = asn1_tag_visible_string,
	asn1_tag_utf32_string       = asn1_tag_universal_string,
	asn1_tag_utf16_string       = asn1_tag_bmp_string,

	asn1_tag_utc_time           = 23,
	asn1_tag_generalized_time   = 24
} asn1_tag;

/*
 * ASN.1 tag types
 */

struct asn1_id;
struct asn1_hdr;
struct asn1_oid;
struct asn1_string;

typedef struct asn1_id asn1_id;
typedef struct asn1_hdr asn1_hdr;
typedef struct asn1_oid asn1_oid;
typedef struct asn1_string asn1_string;

const u8 asn1_charset_numeric_str_chars[] =
	"0123456789 ";
const u8 asn1_charset_printable_str[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789 '()+,-./:=?";

struct asn1_id
{
    u64 _identifier  : 56;
    u64 _constructed : 1;
    u64 _class       : 2;
};

struct asn1_hdr
{
    asn1_id _id;
    u64 _length;
};

enum { asn1_oid_comp_max = 15 };

struct asn1_oid
{
	u64 count;
	u64 oid[asn1_oid_comp_max];
};

struct asn1_string
{
	u64 count;
	u8 *str;
};

/*
 * floating point helpers
 */

float _f32_inf();
float _f32_nan();
float _f32_snan();
double _f64_inf();
double _f64_nan();
double _f64_snan();

/*
 * ASN.1 serialisation and deserialization
 */

struct f32_result { f32 value; s32 error; };
struct f64_result { f64 value; s64 error; };
struct s64_result { s64 value; s64 error; };
struct u64_result { u64 value; s64 error; };

const char* asn1_tag_name(u64 tag);

size_t crefl_asn1_ber_tag_length(u64 len);
int crefl_asn1_ber_tag_read(crefl_buf *buf, u64 *len);
int crefl_asn1_ber_tag_write(crefl_buf *buf, u64 len);

size_t crefl_asn1_ber_ident_length(asn1_id _id);
int crefl_asn1_ber_ident_read(crefl_buf *buf, asn1_id *_id);
int crefl_asn1_ber_ident_write(crefl_buf *buf, asn1_id _id);

size_t crefl_asn1_ber_length_length(u64 length);
int crefl_asn1_ber_length_read(crefl_buf *buf, u64 *length);
int crefl_asn1_ber_length_write(crefl_buf *buf, u64 length);

size_t crefl_asn1_ber_boolean_length(const bool *value);
int crefl_asn1_ber_boolean_read(crefl_buf *buf, size_t len, bool *value);
int crefl_asn1_ber_boolean_write(crefl_buf *buf, size_t len, const bool *value);
int crefl_asn1_der_boolean_read(crefl_buf *buf, asn1_tag _tag, bool *value);
int crefl_asn1_der_boolean_write(crefl_buf *buf, asn1_tag _tag, const bool *value);

size_t crefl_asn1_ber_integer_u64_length(const u64 *value);
int crefl_asn1_ber_integer_u64_read(crefl_buf *buf, size_t len, u64 *value);
int crefl_asn1_ber_integer_u64_write(crefl_buf *buf, size_t len, const u64 *value);
int crefl_asn1_der_integer_u64_read(crefl_buf *buf, asn1_tag _tag, u64 *value);
int crefl_asn1_der_integer_u64_write(crefl_buf *buf, asn1_tag _tag, const u64 *value);

size_t crefl_asn1_ber_integer_u64_length_byval(const u64 value);
struct u64_result crefl_asn1_ber_integer_u64_read_byval(crefl_buf *buf, size_t len);
int crefl_asn1_ber_integer_u64_write_byval(crefl_buf *buf, size_t len, const u64 value);
struct u64_result crefl_asn1_der_integer_u64_read_byval(crefl_buf *buf, asn1_tag _tag);
int crefl_asn1_der_integer_u64_write_byval(crefl_buf *buf, asn1_tag _tag, const u64 value);

size_t crefl_asn1_ber_integer_s64_length(const s64 *value);
int crefl_asn1_ber_integer_s64_read(crefl_buf *buf, size_t len, s64 *value);
int crefl_asn1_ber_integer_s64_write(crefl_buf *buf, size_t len, const s64 *value);
int crefl_asn1_der_integer_s64_read(crefl_buf *buf, asn1_tag _tag, s64 *value);
int crefl_asn1_der_integer_s64_write(crefl_buf *buf, asn1_tag _tag, const s64 *value);

size_t crefl_asn1_ber_integer_s64_length_byval(const s64 value);
struct s64_result crefl_asn1_ber_integer_s64_read_byval(crefl_buf *buf, size_t len);
int crefl_asn1_ber_integer_s64_write_byval(crefl_buf *buf, size_t len, const s64 value);
struct s64_result crefl_asn1_der_integer_s64_read_byval(crefl_buf *buf, asn1_tag _tag);
int crefl_asn1_der_integer_s64_write_byval(crefl_buf *buf, asn1_tag _tag, const s64 value);

size_t crefl_le_ber_integer_u64_length(const u64 *value);
int crefl_le_ber_integer_u64_read(crefl_buf *buf, size_t len, u64 *value);
int crefl_le_ber_integer_u64_write(crefl_buf *buf, size_t len, const u64 *value);

size_t crefl_le_ber_integer_u64_length_byval(const u64 value);
struct u64_result crefl_le_ber_integer_u64_read_byval(crefl_buf *buf, size_t len);
int crefl_le_ber_integer_u64_write_byval(crefl_buf *buf, size_t len, const u64 value);

size_t crefl_le_ber_integer_s64_length(const s64 *value);
int crefl_le_ber_integer_s64_read(crefl_buf *buf, size_t len, s64 *value);
int crefl_le_ber_integer_s64_write(crefl_buf *buf, size_t len, const s64 *value);

size_t crefl_le_ber_integer_s64_length_byval(const s64 *value);
struct s64_result crefl_le_ber_integer_s64_read_byval(crefl_buf *buf, size_t len);
int crefl_le_ber_integer_s64_write_byval(crefl_buf *buf, size_t len, const s64 value);

size_t crefl_asn1_ber_real_f64_length(const double *value);
int crefl_asn1_ber_real_f64_read(crefl_buf *buf, size_t len, double *value);
int crefl_asn1_ber_real_f64_write(crefl_buf *buf, size_t len, const double *value);
int crefl_asn1_der_real_f64_read(crefl_buf *buf, asn1_tag _tag, double *value);
int crefl_asn1_der_real_f64_write(crefl_buf *buf, asn1_tag _tag, const double *value);

size_t crefl_asn1_ber_real_f64_length_byval(const double value);
struct f64_result crefl_asn1_ber_real_f64_read_byval(crefl_buf *buf, size_t len);
int crefl_asn1_ber_real_f64_write_byval(crefl_buf *buf, size_t len, const double value);
struct f64_result crefl_asn1_der_real_f64_read_byval(crefl_buf *buf, asn1_tag _tag);
int crefl_asn1_der_real_f64_write_byval(crefl_buf *buf, asn1_tag _tag, const double value);

int crefl_vf_f64_read(crefl_buf *buf, double *value);
int crefl_vf_f64_write(crefl_buf *buf, const double *value);
struct f64_result crefl_vf_f64_read_byval(crefl_buf *buf);
int crefl_vf_f64_write_byval(crefl_buf *buf, const double value);

int crefl_vf_f32_read(crefl_buf *buf, float *value);
int crefl_vf_f32_write(crefl_buf *buf, const float *value);
struct f32_result crefl_vf_f32_read_byval(crefl_buf *buf);
int crefl_vf_f32_write_byval(crefl_buf *buf, const float value);

int crefl_leb_u64_read(crefl_buf *buf, u64 *value);
int crefl_leb_u64_write(crefl_buf *buf, const u64 *value);
struct u64_result crefl_leb_u64_read_byval(crefl_buf *buf);
int crefl_leb_u64_write_byval(crefl_buf *buf, const u64 value);

int crefl_vlu_u64_read(crefl_buf *buf, u64 *value);
int crefl_vlu_u64_write(crefl_buf *buf, const u64 *value);
struct u64_result crefl_vlu_u64_read_byval(crefl_buf *buf);
int crefl_vlu_u64_write_byval(crefl_buf *buf, const u64 value);

size_t crefl_asn1_ber_oid_length(const asn1_oid *obj);
int crefl_asn1_ber_oid_read(crefl_buf *buf, size_t len, asn1_oid *obj);
int crefl_asn1_ber_oid_write(crefl_buf *buf, size_t len, const asn1_oid *obj);
int crefl_asn1_der_oid_read(crefl_buf *buf, asn1_tag _tag, asn1_oid *obj);
int crefl_asn1_der_oid_write(crefl_buf *buf, asn1_tag _tag, const asn1_oid *obj);

int crefl_asn1_oid_to_string(char *str, size_t *buflen, const asn1_oid *obj);
int crefl_asn1_oid_from_string(asn1_oid *obj, const char *str, size_t buflen);

size_t crefl_asn1_ber_octets_length(const asn1_string *obj);
int crefl_asn1_ber_octets_read(crefl_buf *buf, size_t len, asn1_string *obj);
int crefl_asn1_ber_octets_write(crefl_buf *buf, size_t len, const asn1_string *obj);
int crefl_asn1_der_octets_read(crefl_buf *buf, asn1_tag _tag, asn1_string *obj);
int crefl_asn1_der_octets_write(crefl_buf *buf, asn1_tag _tag, const asn1_string *obj);

size_t crefl_asn1_ber_null_length();
int crefl_asn1_ber_null_read(crefl_buf *buf, size_t len);
int crefl_asn1_ber_null_write(crefl_buf *buf, size_t len);
int crefl_asn1_der_null_read(crefl_buf *buf, asn1_tag _tag);
int crefl_asn1_der_null_write(crefl_buf *buf, asn1_tag _tag);

#ifdef __cplusplus
}
#endif
