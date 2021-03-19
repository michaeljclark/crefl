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

#ifdef __cplusplus
extern "C" {
#endif

struct crefl_buf;
struct crefl_span;

typedef struct crefl_buf crefl_buf;
typedef struct crefl_span crefl_span;

struct crefl_span
{
    void *data;
    size_t length;
};

struct crefl_buf
{
    char *data;
    size_t data_offset;
    size_t data_size;
};

crefl_buf* crefl_buf_new(size_t size);
void crefl_buf_destroy(crefl_buf* buf);
void crefl_buf_dump(crefl_buf *buf);

size_t crefl_buf_write_i8(crefl_buf* buf, int8_t num);
size_t crefl_buf_write_i16(crefl_buf* buf, int16_t num);
size_t crefl_buf_write_i32(crefl_buf* buf, int32_t num);
size_t crefl_buf_write_i64(crefl_buf* buf, int64_t num);
size_t crefl_buf_write_bytes(crefl_buf* buf, const char *s, size_t len);

size_t crefl_buf_read_i8(crefl_buf* buf, int8_t *num);
size_t crefl_buf_read_i16(crefl_buf* buf, int16_t *num);
size_t crefl_buf_read_i32(crefl_buf* buf, int32_t *num);
size_t crefl_buf_read_i64(crefl_buf* buf, int64_t *num);
size_t crefl_buf_read_bytes(crefl_buf* buf, char *s, size_t len);

void crefl_buf_reset(crefl_buf* buf);
void crefl_buf_seek(crefl_buf* buf, size_t offset);
char* crefl_buf_data(crefl_buf *buf);
size_t crefl_buf_offset(crefl_buf* buf);

void crefl_buf_copy_input(crefl_buf* buf, crefl_span input); /* read callback */
crefl_span crefl_buf_copy_output(crefl_buf* buf); /* get write params */
crefl_span crefl_buf_copy_remaining(crefl_buf* buf); /* get read params */

#ifdef __cplusplus
}
#endif
