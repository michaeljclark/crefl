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

#include <cstdlib>
#include <cstring>
#include <cinttypes>

#include <string>

#include "stdendian.h"

#include "cmodel.h"
#include "cutil.h"
#include "cbuf.h"

static bool _debug_enabled = false;

#define _debug(...) if (_debug_enabled) { printf(__VA_ARGS__); }
#define _debug_func(fmt,...) \
if (_debug_enabled) { printf("%s: " fmt, __func__, __VA_ARGS__); }

crefl_buf* crefl_buf_new(size_t size)
{
    crefl_buf *buf = (crefl_buf*)malloc(sizeof(crefl_buf));

    buf->data_offset = 0;
    buf->data_size = size;
    buf->data = (char*)malloc(buf->data_size);
    memset(buf->data, 0, buf->data_size);

    return buf;
}

void crefl_buf_destroy(crefl_buf* buf)
{
    free(buf->data);
    free(buf);
}

void crefl_buf_reset(crefl_buf* buf)
{
    buf->data_offset = 0;
}

static std::string _to_binary(uint64_t symbol, size_t bit_width)
{
    static const char* arr[] = { "▄", "▟", "▙", "█" };
    std::string s;
    for (ssize_t i = bit_width-2; i >= 0; i-=2) {
        s.append(arr[(symbol>>i) & 3]);
    }
    return s;
}

void crefl_buf_dump(crefl_buf *buf)
{
    ssize_t stride = 16;
    for (ssize_t i = 0; i < buf->data_offset; i += stride) {
        printf("      ");
        for (ssize_t j = i+stride-1; j >= i; j--) {
            if (j >= buf->data_offset) printf("     ");
            else printf(" 0x%02hhX", buf->data[j]);
        }
        printf("\n");
        printf("%04zX: ", i & 0xffff);
        for (ssize_t j = i+stride-1; j >= i; j--) {
            if (j >= buf->data_offset) printf(" ░░░░");
            else printf(" %s", _to_binary(buf->data[j], 8).c_str());
        }
        printf("\n");
    }
}

template <typename INT, typename SWAP>
static size_t crefl_buf_write_int(crefl_buf *buf, INT num, SWAP f);
template <typename INT, typename SWAP>
static size_t crefl_buf_read_int(crefl_buf *buf, INT *num, SWAP f);

size_t crefl_buf_write_i8(crefl_buf* buf, int8_t num)
{ return crefl_buf_write_int<int8_t>(buf,num,le8); }
size_t crefl_buf_write_i16(crefl_buf* buf, int16_t num)
{ return crefl_buf_write_int<int16_t>(buf,num,le16); }
size_t crefl_buf_write_i32(crefl_buf* buf, int32_t num)
{ return crefl_buf_write_int<int32_t>(buf,num,le32); }
size_t crefl_buf_write_i64(crefl_buf* buf, int64_t num)
{ return crefl_buf_write_int<int64_t>(buf,num,le64); }

size_t crefl_buf_read_i8(crefl_buf* buf, int8_t *num)
{ return crefl_buf_read_int<int8_t>(buf,num,le8); }
size_t crefl_buf_read_i16(crefl_buf* buf, int16_t *num)
{ return crefl_buf_read_int<int16_t>(buf,num,le16); }
size_t crefl_buf_read_i32(crefl_buf* buf, int32_t *num)
{ return crefl_buf_read_int<int32_t>(buf,num,le32); }
size_t crefl_buf_read_i64(crefl_buf* buf, int64_t *num)
{ return crefl_buf_read_int<int64_t>(buf,num,le64); }

template <typename INT, typename SWAP>
static size_t crefl_buf_write_int(crefl_buf *buf, INT num, SWAP f)
{
    if (buf->data_offset + sizeof(num) > buf->data_size) return 0;
    INT t = f(num);
    memcpy(buf->data + buf->data_offset, &t, sizeof(INT));
    _debug_func("bits=%zu, offset=%zu, value=%lld\n",
                sizeof(num)<<3, buf->data_offset, (long long)num);
    buf->data_offset += sizeof(num);
    return sizeof(num);
}

template <typename INT, typename SWAP>
static size_t crefl_buf_read_int(crefl_buf *buf, INT *num, SWAP f)
{
    if (buf->data_offset + sizeof(*num) > buf->data_size) return 0;
    INT t;
    memcpy(&t, buf->data + buf->data_offset, sizeof(INT));
    *num = f(t);
    _debug_func("bits=%zu, offset=%zu, value=%lld\n",
                sizeof(num)<<3, buf->data_offset, (long long)*num);
    buf->data_offset += sizeof(*num);
    return sizeof(*num);
}

size_t crefl_buf_write_bytes(crefl_buf* buf, const char *s, size_t len)
{
    if (buf->data_offset + len > buf->data_size) return 0;
    memcpy(&buf->data[buf->data_offset], s, len);
    _debug_func("length=%zu, offset=%zu\n", len, buf->data_offset);
    buf->data_offset += len;
    return len;
}

size_t crefl_buf_read_bytes(crefl_buf* buf, char *s, size_t len)
{
    if (buf->data_offset + len > buf->data_size) return 0;
    memcpy(s, &buf->data[buf->data_offset], len);
    _debug_func("length=%" PRIu64 ", offset=%zu\n", len, buf->data_offset);
    buf->data_offset += len;
    return len;
}

void crefl_buf_copy_input(crefl_buf* buf, crefl_span input)
{
    size_t copy_len = input.length;
    if (buf->data_offset + copy_len > buf->data_size) {
        copy_len = buf->data_size - buf->data_offset;
    }
    if (input.data != buf->data + buf->data_offset) {
        memmove(buf->data + buf->data_offset, input.data, copy_len);
    }
}

crefl_span crefl_buf_copy_output(crefl_buf* buf)
{
    return crefl_span {
        &buf->data[0], buf->data_offset
    };
}

crefl_span crefl_buf_copy_remaining(crefl_buf* buf)
{
    return crefl_span {
        &buf->data[buf->data_offset], buf->data_size - buf->data_offset
    };
}
