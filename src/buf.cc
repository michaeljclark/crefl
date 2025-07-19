/*
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

#include <cstdio>
#include <cstdlib>

#include <cinf/buf.h>

cinf_buf* cinf_buf_new(size_t size)
{
    cinf_buf *buf = (cinf_buf*)malloc(sizeof(cinf_buf));

    buf->data_offset = 0;
    buf->data_size = size;
    buf->data = (char*)malloc(buf->data_size);
    memset(buf->data, 0, buf->data_size);

    return buf;
}

void cinf_buf_destroy(cinf_buf* buf)
{
    free(buf->data);
    free(buf);
}

int cinf_format_byte(char *buf, size_t buflen, uint8_t c)
{
    static const char arr[4][4] = { "▄", "▟", "▙", "█" };

    int n = 0;
    for (intptr_t i = 6; i >= 0; i-=2) {
        if (n < buflen) {
            int ret = snprintf(buf+n, buflen-n, "%s", arr[(c>>i) & 3]);
            n += ret > 0 ? ret : 0;
        }
    }
    return n;
}

void cinf_buf_dump(cinf_buf *buf)
{
    intptr_t stride = 16;
    for (intptr_t i = 0; i < buf->data_offset; i += stride) {
        printf("      ");
        for (intptr_t j = i+stride-1; j >= i; j--) {
            if (j >= buf->data_offset) printf("     ");
            else printf(" 0x%02hhX", buf->data[j]);
        }
        printf("\n%04zX: ", i & 0xffff);
        for (intptr_t j = i+stride-1; j >= i; j--) {
            printf(" ");
            if (j < buf->data_offset) {
                char s[16];
                cinf_format_byte(s, sizeof(s), buf->data[j]);
                printf("%s", s);
            }
            else {
                printf("▢▢▢▢");
            }
        }
        printf("\n");
    }
}
