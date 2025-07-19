/*
 * <cinf/buf.h>
 *
 * cinf runtime library and compiler plug-in to support reflection in C.
 *
 * Copyright (c) 2020-2022 Michael Clark <michaeljclark@mac.com>
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
#include <string.h>

#include <cinf/endian.h>

#ifdef __cplusplus
extern "C" {
#endif

struct cinf_buf;
struct cinf_span;

typedef struct cinf_buf cinf_buf;
typedef struct cinf_span cinf_span;

struct cinf_span
{
    void *data;
    size_t length;
};

struct cinf_buf
{
    char *data;
    size_t data_offset;
    size_t data_size;
};

cinf_buf* cinf_buf_new(size_t size);
void cinf_buf_destroy(cinf_buf* buf);
void cinf_buf_dump(cinf_buf *buf);
int cinf_format_byte(char *buf, size_t buflen, uint8_t c);

static size_t cinf_buf_write_i8(cinf_buf* buf, int8_t num);
static size_t cinf_buf_write_i16(cinf_buf* buf, int16_t num);
static size_t cinf_buf_write_i32(cinf_buf* buf, int32_t num);
static size_t cinf_buf_write_i64(cinf_buf* buf, int64_t num);
static size_t cinf_buf_write_bytes(cinf_buf* buf, const char *s, size_t len);
static size_t cinf_buf_write_bytes_unchecked(cinf_buf* buf, const char *s, size_t len);

static size_t cinf_buf_read_i8(cinf_buf* buf, int8_t *num);
static size_t cinf_buf_read_i16(cinf_buf* buf, int16_t *num);
static size_t cinf_buf_read_i32(cinf_buf* buf, int32_t *num);
static size_t cinf_buf_read_i64(cinf_buf* buf, int64_t *num);
static size_t cinf_buf_read_bytes(cinf_buf* buf, char *s, size_t len);
static size_t cinf_buf_read_bytes_unchecked(cinf_buf* buf, char *s, size_t len);

static void cinf_buf_reset(cinf_buf* buf);
static void cinf_buf_seek(cinf_buf* buf, size_t offset);
static char* cinf_buf_data(cinf_buf *buf);
static size_t cinf_buf_offset(cinf_buf* buf);

/*
 * buffer inline functions
 */

#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64)
#ifndef USE_UNALIGNED_ACCESSES
#define USE_UNALIGNED_ACCESSES 1
#endif
#endif

#if defined (_MSC_VER)
#define USE_CRT_MEMCPY 0
#else
#define USE_CRT_MEMCPY 1
#endif

#define CREFL_FN(Y,X) cinf_ ## Y ## _ ## X

static inline size_t cinf_buf_check_capacity(cinf_buf *buf, size_t len)
{
    return (buf->data_offset + len > buf->data_size) ? -1 : 0;
}

#if USE_UNALIGNED_ACCESSES && !USE_CRT_MEMCPY

#define CREFL_BUF_WRITE_IMPL(suffix,T,swap)                                    \
static inline size_t CREFL_FN(buf_write,suffix)(cinf_buf *buf, T val)           \
{                                                                              \
    if (buf->data_offset + sizeof(T) > buf->data_size) return 0;               \
    T t = swap(val);                                                           \
    *(T*)(buf->data + buf->data_offset) = t;                                   \
    buf->data_offset += sizeof(T);                                             \
    return sizeof(T);                                                          \
}                                                                              \
static inline size_t CREFL_FN(buf_write_unchecked,suffix)(cinf_buf *buf, T val) \
{                                                                              \
    T t = swap(val);                                                           \
    *(T*)(buf->data + buf->data_offset) = t;                                   \
    buf->data_offset += sizeof(T);                                             \
    return sizeof(T);                                                          \
}

#define CREFL_BUF_READ_IMPL(suffix,T,swap)                                     \
static inline size_t CREFL_FN(buf_read,suffix)(cinf_buf *buf, T* val)           \
{                                                                              \
    if (buf->data_offset + sizeof(T) > buf->data_size) return 0;               \
    T t = *(T*)(buf->data + buf->data_offset);                                 \
    *val = swap(t);                                                            \
    buf->data_offset += sizeof(T);                                             \
    return sizeof(T);                                                          \
}                                                                              \
static inline size_t CREFL_FN(buf_read_unchecked,suffix)(cinf_buf *buf, T* val) \
{                                                                              \
    T t = *(T*)(buf->data + buf->data_offset);                                 \
    *val = swap(t);                                                            \
    buf->data_offset += sizeof(T);                                             \
    return sizeof(T);                                                          \
}

#else

#define CREFL_BUF_WRITE_IMPL(suffix,T,swap)                                    \
static inline size_t CREFL_FN(buf_write,suffix)(cinf_buf *buf, T val)           \
{                                                                              \
    if (buf->data_offset + sizeof(T) > buf->data_size) return 0;               \
    T t = swap(val);                                                           \
    memcpy(buf->data + buf->data_offset, &t, sizeof(T));                       \
    buf->data_offset += sizeof(T);                                             \
    return sizeof(T);                                                          \
}                                                                              \
static inline size_t CREFL_FN(buf_write_unchecked,suffix)(cinf_buf *buf, T val) \
{                                                                              \
    T t = swap(val);                                                           \
    memcpy(buf->data + buf->data_offset, &t, sizeof(T));                       \
    buf->data_offset += sizeof(T);                                             \
    return sizeof(T);                                                          \
}

#define CREFL_BUF_READ_IMPL(suffix,T,swap)                                     \
static inline size_t CREFL_FN(buf_read,suffix)(cinf_buf *buf, T* val)           \
{                                                                              \
    if (buf->data_offset + sizeof(T) > buf->data_size) return 0;               \
    T t;                                                                       \
    memcpy(&t, buf->data + buf->data_offset, sizeof(T));                       \
    *val = swap(t);                                                            \
    buf->data_offset += sizeof(T);                                             \
    return sizeof(T);                                                          \
}                                                                              \
static inline size_t CREFL_FN(buf_read_unchecked,suffix)(cinf_buf *buf, T* val) \
{                                                                              \
    T t;                                                                       \
    memcpy(&t, buf->data + buf->data_offset, sizeof(T));                       \
    *val = swap(t);                                                            \
    buf->data_offset += sizeof(T);                                             \
    return sizeof(T);                                                          \
}

#endif

CREFL_BUF_WRITE_IMPL(i16,int16_t,le16)
CREFL_BUF_WRITE_IMPL(i32,int32_t,le32)
CREFL_BUF_WRITE_IMPL(i64,int64_t,le64)

CREFL_BUF_READ_IMPL(i16,int16_t,le16)
CREFL_BUF_READ_IMPL(i32,int32_t,le32)
CREFL_BUF_READ_IMPL(i64,int64_t,le64)

static inline size_t cinf_buf_read_i8(cinf_buf *buf, int8_t* val)
{
    if (buf->data_offset + 1 > buf->data_size) return 0;
    *val = *(int8_t*)(buf->data + buf->data_offset);
    buf->data_offset++;
    return 1;
}

static inline size_t cinf_buf_read_unchecked_i8(cinf_buf *buf, int8_t* val)
{
    *val = *(int8_t*)(buf->data + buf->data_offset);
    buf->data_offset++;
    return 1;
}

static inline size_t cinf_buf_write_i8(cinf_buf *buf, int8_t val)
{
    if (buf->data_offset + 1 > buf->data_size) return 0;
    *(int8_t*)(buf->data + buf->data_offset) = val;
    buf->data_offset++;
    return 1;
}
static inline size_t cinf_buf_write_unchecked_i8(cinf_buf *buf, int8_t val)
{
    *(int8_t*)(buf->data + buf->data_offset) = val;
    buf->data_offset++;
    return 1;
}

static inline size_t cinf_buf_write_bytes(cinf_buf* buf, const char *src, size_t len)
{
    if (buf->data_offset + len > buf->data_size) return 0;
#if USE_CRT_MEMCPY
    memcpy(&buf->data[buf->data_offset], src, len);
#else
    char *dst = &buf->data[buf->data_offset];
    size_t l = len;
    while (l-- > 0) *dst++ = *src++;
#endif
    buf->data_offset += len;
    return len;
}

static inline size_t cinf_buf_read_bytes(cinf_buf* buf, char *dst, size_t len)
{
    if (buf->data_offset + len > buf->data_size) return 0;
#if USE_CRT_MEMCPY
    memcpy(dst, &buf->data[buf->data_offset], len);
#else
    const char *src = &buf->data[buf->data_offset];
    size_t l = len;
    while (l-- > 0) *dst++ = *src++;
#endif
    buf->data_offset += len;
    return len;
}

static inline size_t cinf_buf_write_bytes_unchecked(cinf_buf* buf, const char *src, size_t len)
{
#if USE_CRT_MEMCPY
    memcpy(&buf->data[buf->data_offset], src, len);
#else
    char *dst = &buf->data[buf->data_offset];
    size_t l = len;
    while (l-- > 0) *dst++ = *src++;
#endif
    buf->data_offset += len;
    return len;
}

static inline size_t cinf_buf_read_bytes_unchecked(cinf_buf* buf, char *dst, size_t len)
{
#if USE_CRT_MEMCPY
    memcpy(dst, &buf->data[buf->data_offset], len);
#else
    const char *src = &buf->data[buf->data_offset];
    size_t l = len;
    while (l-- > 0) *dst++ = *src++;
#endif
    buf->data_offset += len;
    return len;
}

static inline void cinf_buf_reset(cinf_buf* buf)
{
    buf->data_offset = 0;
}

static inline void cinf_buf_seek(cinf_buf* buf, size_t offset)
{
    buf->data_offset = offset;
}

static inline char* cinf_buf_data(cinf_buf *buf)
{
    return buf->data;
}

static inline size_t cinf_buf_offset(cinf_buf* buf)
{
    return buf->data_offset;
}

static inline cinf_span cinf_buf_remaining(cinf_buf* buf)
{
    cinf_span s = {
        &buf->data[buf->data_offset], buf->data_size - buf->data_offset
    };
    return s;
}

#ifdef __cplusplus
}
#endif
