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
    return 8 - ((clz(tag) - 1) / 7) + 1;
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

out:
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

    llen = 8 - ((clz(tag) - 1) / 7) + 1;
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

size_t crefl_asn1_ident_length(asn1_id _id)
{
    return 1 + ((_id._identifier >= 0x1f) ?
        crefl_asn1_tagnum_length(_id._identifier) : 0);
}

int crefl_asn1_ident_read(crefl_buf *buf, asn1_id *_id)
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
        if (crefl_asn1_tagnum_read(buf, &r._identifier) < 0) {
            goto err;
        }
        if (_id->_identifier < 0x1f) {
            goto err;
        }
    }

    return 0;
err:
    return -1;
}

int crefl_asn1_ident_write(crefl_buf *buf, asn1_id _id)
{
    int8_t b;

    b = ( (_id._class       & 0x02) << 6 ) |
        ( (_id._constructed & 0x01) << 5 ) |
        ( (_id._identifier < 0x1f ? _id._identifier : 0x1f) );

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

size_t crefl_asn1_length_length(u64 length)
{
    return 1 + ((length >= 0x80) ? 8 - (clz(length) / 8) : 0);
}

int crefl_asn1_length_read(crefl_buf *buf, u64 *length)
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

int crefl_asn1_length_write(crefl_buf *buf, u64 length)
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
    b = llen | 0x80;
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
 * ISO/IEC 8825-1:2003 8.3 integer
 *
 * read and write integer
 */

size_t crefl_asn1_boolean_length(bool value)
{
    return 1;
}

int crefl_asn1_boolean_read(crefl_buf *buf, asn1_hdr *_hdr, bool *value)
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

int crefl_asn1_boolean_write(crefl_buf *buf, asn1_hdr *_hdr, bool value)
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

/*
 * ISO/IEC 8825-1:2003 8.3 integer
 *
 * read and write integer
 */

size_t crefl_asn1_integer_length(u64 value)
{
    return 8 - (clz(value) / 8);
}

int crefl_asn1_integer_read(crefl_buf *buf, asn1_hdr *_hdr, u64 *value)
{
    int8_t b;
    size_t len = _hdr->_length;
    u64 v = 0;

    if (len < 1 || len > 8) {
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

int crefl_asn1_integer_write(crefl_buf *buf, asn1_hdr *_hdr, u64 value)
{
    int8_t b;
    size_t len = _hdr->_length;
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
 * read and write tagged integer
 */

int crefl_asn1_tagged_integer_read(crefl_buf *buf, asn1_tag _tag, u64 *value)
{
    asn1_hdr hdr;
    if (crefl_asn1_ident_read(buf, &hdr._id) < 0) goto err;
    if (crefl_asn1_length_read(buf, &hdr._length) < 0) goto err;
    if (crefl_asn1_integer_read(buf, &hdr, value) < 0) goto err;
    return 0;
err:
    return -1;
}

int crefl_asn1_tagged_integer_write(crefl_buf *buf, asn1_tag _tag, u64 value)
{
    asn1_hdr hdr = {
        { asn1_class_universal, 0, _tag }, crefl_asn1_integer_length(value)
    };

    if (crefl_asn1_ident_write(buf, hdr._id) < 0) goto err;
    if (crefl_asn1_length_write(buf, hdr._length) < 0) goto err;
    if (crefl_asn1_integer_write(buf, &hdr, value) < 0) goto err;

    return 0;
err:
    return -1;
}

/*
 * ISO/IEC 8825-1:2003 8.19 object identifier value
 *
 * read and write object identifier value
 */

size_t crefl_asn1_oid_length(u64 *oid, size_t count)
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

int crefl_asn1_oid_read(crefl_buf *buf, asn1_hdr *_hdr, u64 *oid, size_t *count)
{
    size_t len = _hdr->_length;
    size_t offset = crefl_buf_offset(buf);
    size_t n = 0, limit = *count;
    u64 comp;

    while (offset < len) {
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

int crefl_asn1_oid_write(crefl_buf *buf, asn1_hdr *_hdr, u64 *oid, size_t count)
{
    size_t len = _hdr->_length;

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