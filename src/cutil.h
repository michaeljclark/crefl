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

#include <cstdio>
#include <cstdarg>

#include <string>

/*
 * std::string formatting wrappers for vsnprintf
 *
 * create an initial buffer and if the string does not fit,
 * resize the buffer and call again with a new va_list.
 */

static const size_t string_vprintf_initial_capacity = 256;

static std::string string_vprintf(const char* fmt, va_list args)
{
    std::string buf(string_vprintf_initial_capacity, '\0');

    va_list args2;
    va_copy(args2, args);
    int len = vsnprintf(&buf[0], buf.size()-1, fmt, args);
    if ((size_t)len > buf.size()) {
        buf.resize(len+1);
        len = vsnprintf(&buf[0], buf.size(), fmt, args2);
    }
    buf.resize(len);

    return buf;
}

static std::string string_printf(const char* fmt, ...)
{
    std::string buf(string_vprintf_initial_capacity, '\0');

    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(&buf[0], buf.size()-1, fmt, args);
    va_end(args);
    if ((size_t)len > buf.size()) {
        buf.resize(len+1);
        va_list args;
        va_start(args, fmt);
        len = vsnprintf(&buf[0], buf.size(), fmt, args);
        va_end(args);
    }
    buf.resize(len);

    return buf;
}
