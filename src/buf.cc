/*
 * crefl runtime library and compiler plug-in to support reflection in C.
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
#include <cstring>

#include <crefl/buf.h>

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

void crefl_buf_dump(crefl_buf *buf)
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
            const char arr[4][4] = { "▄", "▟", "▙", "█" };
            printf(" ");
            for (intptr_t i = 6; i >= 0; i-=2) {
                if (j < buf->data_offset) {
                    printf("%s", arr[(buf->data[j]>>i) & 3]);
                } else {
                    printf("░");
                }
            }
        }
        printf("\n");
    }
}
