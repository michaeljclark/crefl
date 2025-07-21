/*
 * <cinf/base64.h>
 *
 * cinf runtime library and compiler plug-in to support reflection in C.
 *
 * Copyright (c) 2020-2025 Michael Clark <michaeljclark@mac.com>
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

/* PUBLIC DOMAIN - Jon Mayo - November 13, 2003 */
/* $Id: base64.h 128 2007-04-20 08:20:40Z orange $ */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* used to encode 3 bytes into 4 base64 digits */
void base64encode(const unsigned char in[3], unsigned char out[4], int count);

/* used to decode 4 base64 digits into 3 bytes */
int base64decode(const char in[4], char out[3]);

/* encode binary data into base64 digits with MIME style === pads */
size_t base64_encode(size_t in_len, const unsigned char *in,
	size_t out_len, char *out);

/* decode base64 digits with MIME style === pads into binary data */
size_t base64_decode(size_t in_len, const char *in,
	size_t out_len, unsigned char *out);

#ifdef __cplusplus
}
#endif
