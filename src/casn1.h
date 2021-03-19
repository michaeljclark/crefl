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

typedef struct asn1_id asn1_id;
typedef struct asn1_hdr asn1_hdr;

const u8 asn1_charset_numeric_str_chars[] =
	"0123456789 ";
const u8 asn1_charset_printable_str[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789 '()+,-./:=?";

struct asn1_id
{
    u8 _class;
    u8 _constructed;
    u64 _identifier;
};

struct asn1_hdr
{
    asn1_id _id;
    u64 _length;
};

/*
 * ASN.1 serialisation and deserialization
 */

const char* asn1_tag_name(u64 tag);

size_t crefl_asn1_tagnum_length(u64 len);
int crefl_asn1_tagnum_read(crefl_buf *buf, u64 *len);
int crefl_asn1_tagnum_write(crefl_buf *buf, u64 len);

size_t crefl_asn1_ident_length(asn1_id _id);
int crefl_asn1_ident_read(crefl_buf *buf, asn1_id *_id);
int crefl_asn1_ident_write(crefl_buf *buf, asn1_id _id);

size_t crefl_asn1_length_length(u64 length);
int crefl_asn1_length_read(crefl_buf *buf, u64 *length);
int crefl_asn1_length_write(crefl_buf *buf, u64 length);

size_t crefl_asn1_boolean_length(bool value);
int crefl_asn1_boolean_read(crefl_buf *buf, asn1_hdr *_hdr, bool *value);
int crefl_asn1_boolean_write(crefl_buf *buf, asn1_hdr *_hdr, bool value);

size_t crefl_asn1_integer_length(u64 value);
int crefl_asn1_integer_read(crefl_buf *buf, asn1_hdr *_hdr, u64 *value);
int crefl_asn1_integer_write(crefl_buf *buf, asn1_hdr *_hdr, u64 value);

int crefl_asn1_tagged_integer_read(crefl_buf *buf, asn1_tag _tag, u64 *value);
int crefl_asn1_tagged_integer_write(crefl_buf *buf, asn1_tag _tag, u64 value);

size_t crefl_asn1_oid_length(u64 *oid, size_t count);
int crefl_asn1_oid_read(crefl_buf *buf, asn1_hdr *_hdr, u64 *oid, size_t *count);
int crefl_asn1_oid_write(crefl_buf *buf, asn1_hdr *_hdr, u64 *oid, size_t count);
size_t crefl_asn1_oid_to_string(char *buf, size_t buflen, u64 *oid, size_t count);

#ifdef __cplusplus
}
#endif
