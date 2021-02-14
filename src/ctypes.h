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

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _ctype _ctype;
struct _ctype
{
    int _tag;
    int _width;
    int _attrs;
    const char* _name;
    const struct _ctype **_elements;
};

extern const _ctype _cvoid;
extern const _ctype _cptr32;
extern const _ctype _cptr64;
extern const _ctype _cbool;
extern const _ctype _uint1;
extern const _ctype _sint1;
extern const _ctype _uint8;
extern const _ctype _sint8;
extern const _ctype _uint16;
extern const _ctype _sint16;
extern const _ctype _uint32;
extern const _ctype _sint32;
extern const _ctype _uint64;
extern const _ctype _sint64;
extern const _ctype _uint128;
extern const _ctype _sint128;
extern const _ctype _float16;
extern const _ctype _float32;
extern const _ctype _float64;
extern const _ctype _float128;

extern const _ctype *all_types[];

#ifdef __cplusplus
}
#endif
