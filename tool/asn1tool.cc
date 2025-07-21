/*
 * asn1tool - tool to dump PEM and DER encoded ASN.1 data.
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

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cctype>
#include <cerrno>

#include <string>
#include <vector>

#include <cinf/util.h>
#include <cinf/base64.h>
#include <cinf/asn1.h>
#include <cinf/oid.h>

static std::string oid_str(const char *data, size_t sz)
{
    std::string s;
    size_t count, len;
    asn1_oid obj = { 0 };

    cinf_buf *buf = cinf_buf_new(sz);
    memcpy(buf->data, data, sz);

    cinf_asn1_ber_oid_read(buf, sz, &obj);

    len = 0;
    cinf_asn1_oid_to_string(NULL, &len, &obj);
    s.resize(len+1);
    cinf_asn1_oid_to_string(s.data(), &len,  &obj);
    s.resize(len);

    cinf_buf_destroy(buf);

    return s;
}

static std::string hex_str(const uint8_t *data, size_t sz)
{
    std::string s;
    char hex[3];
    for (size_t i = 0; i < sz; i++) {
        snprintf(hex, sizeof(hex), "%02hhx", data[i]);
        if (i != 0) s.append(",");
        s.append(hex);
    }
    return s;
}

extern const char* asn1_tag_names[];

static int read_asn1(cinf_buf *buf, size_t offset, size_t limit, int depth)
{
    asn1_hdr hdr;
    std::string indent, undent, oid, desc;
    size_t current;

    cinf_buf_seek(buf, offset);

    if (cinf_asn1_ber_ident_read(buf, &hdr._id) < 0) goto err;
    if (cinf_asn1_ber_length_read(buf, &hdr._length) < 0) goto err;

    indent = std::string(depth, ' ');
    indent += indent;
    undent = std::string(15-depth, ' ');
    undent += undent;

    printf("[%5zu;%-5llu]%s|-%c%-20s",
        cinf_buf_offset(buf), hdr._length,
        indent.c_str(), hdr._id._constructed ? '*' : ' ',
        asn1_tag_name(hdr._id._identifier));

    switch(hdr._id._identifier) {
    case asn1_tag_set:
    case asn1_tag_sequence:
        printf("\n");
        do {
            current = cinf_buf_offset(buf);
            int ret = read_asn1(buf, current, current + hdr._length, depth+1);
            if (ret < 0) return ret;
            offset = cinf_buf_offset(buf);
        } while (offset < limit);
        break;
    case asn1_tag_object_identifier:
        current = cinf_buf_offset(buf);
        oid = oid_str(buf->data + current, hdr._length);
        desc = cinf_asn1_oid_desc(buf->data + current, hdr._length);
        printf("%s%s (%s)\n", undent.c_str(), desc.c_str(), oid.c_str());
        cinf_buf_seek(buf, current + hdr._length);
        break;
    case asn1_tag_real:
    case asn1_tag_integer:
    case asn1_tag_bit_string:
        current = cinf_buf_offset(buf);
        printf("%s{%s}\n", undent.c_str(),
            hex_str((const uint8_t*)buf->data + current, hdr._length).c_str());
        cinf_buf_seek(buf, current + hdr._length);
        break;
    case asn1_tag_utc_time:
    case asn1_tag_utf8_string:
    case asn1_tag_printable_string:
        current = cinf_buf_offset(buf);
        printf("%s\"%s\"\n", undent.c_str(),
            std::string(buf->data + current, hdr._length).c_str());
        cinf_buf_seek(buf, current + hdr._length);
        break;
    default:
        printf("\n");
        /* skip past entries we don't understand */
        current = cinf_buf_offset(buf);
        cinf_buf_seek(buf, current + hdr._length);
        break;
    }

    return 0;
err:
    return -1;
}

static int label_char(char c)
{
    return isalpha(c) || isdigit(c) || isspace(c);
}

static const char *scan_line(const char *p, const char *end, const char **eol)
{
    const char *nl = (const char*)memchr(p, '\n', end - p);
    if (!nl) return NULL;
    *eol = (nl > p && *(nl - 1) == '\r') ? nl - 1 : nl;
    return nl + 1;
}

size_t scan_armor_pem(const char *buf, size_t buf_len,
    const char **label_text, size_t *label_len,
    const char **base64_data, size_t *base64_len)
{
    const char *p = buf, *end = buf + buf_len;
    const char *eol, *next, *base64_start;

    while (p < end) {
        next = scan_line(p, end, &eol);
        if (!next) break;

        if ((size_t)(eol - p) > 11 && strncmp(p, "-----BEGIN ", 11) == 0) {
            const char *ls = p + 11, *le = ls;
            while (le < eol && label_char(*le)) {
                le++;
            }
            if (le + 5 > eol || strncmp(le, "-----", 5) != 0) {
                p = next;
                continue;
            }
            *label_text = ls;
            *label_len = le - ls;
            *base64_data = (char *)next;
            p = next;
            goto found;
        }
        p = next;
    }
    return 0;

found:
    while (p < end) {
        next = scan_line(p, end, &eol);
        if (!next) break;

        if ((size_t)(eol - p) > 9 && strncmp(p, "-----END ", 9) == 0) {
            const char *ls = p + 9, *le = ls;
            while (le < eol && label_char(*le)) {
                le++;
            }
            if (le + 5 > eol || strncmp(le, "-----", 5) != 0) {
                p = next;
                continue;
            }
            size_t found_len = le - ls;
            if (*label_len == found_len &&
                strncmp(*label_text, ls, found_len) == 0) {
                *base64_len = (size_t)(p - *base64_data - 1);
                return (size_t)(next - buf);
            }
        }
        p = next;
    }
    return 0;
}

static void dump_asn1_pem(const char *filename)
{
    std::vector<uint8_t> v;

    if (cinf_read_file(v, filename) != 0) return;
    const char *data_text = (const char*)v.data();
    size_t data_len = v.size();

    for (;;) {
        const char *label_text, *base64_data;
        size_t label_len, base64_len, parse_len;

        parse_len = scan_armor_pem(data_text, data_len,
             &label_text, &label_len, &base64_data, &base64_len);

        if (parse_len == 0) break;

        printf("-----BEGIN %.*s-----\n", (int)label_len, label_text);

        size_t max_len = base64_len / 4 * 3 + 2;
        uint8_t *out = (uint8_t*)malloc(max_len);
        size_t out_len = base64_decode(base64_len, base64_data, max_len, out);

        cinf_buf *buf = cinf_buf_new(v.size());
        memcpy(buf->data, out, out_len);
        if (read_asn1(buf, 0, out_len, 0) < 0) {
            fprintf(stderr, "error: recurse_asn1 returned an error\n");
        }
        cinf_buf_destroy(buf);

        printf("-----END %.*s-----\n", (int)label_len, label_text);

        data_text += parse_len;
        data_len -= parse_len;
    }
}

static void dump_asn1_der(const char *filename)
{
    std::vector<uint8_t> v;
    if (cinf_read_file(v, filename) != 0) return;
    cinf_buf *buf = cinf_buf_new(v.size());
    memcpy(buf->data, v.data(), v.size());
    if (read_asn1(buf, 0, v.size(), 0) < 0) {
        fprintf(stderr, "error: recurse_asn1 returned an error\n");
    }
    cinf_buf_destroy(buf);
}

/* option parsing */

static const char* der_filename = NULL;
static const char* pem_filename = NULL;
static int help_text = 0;

void print_help(int argc, const char **argv)
{
    fprintf(stderr,
        "Usage: %s [options]\n"
        "  -p, --pem <filename.pem>           dump ASN.1 PEM file\n"
        "  -d, --der <filename.der>           dump ASN.1 DER file\n"
        "  -h, --help                         print help\n", argv[0]);
}

bool check_param(bool cond, const char *param)
{
    if (cond) {
        printf("error: %s requires parameter\n", param);
    }
    return (help_text = cond);
}

bool match_opt(const char *arg, const char *opt, const char *longopt)
{
    return strcmp(arg, opt) == 0 || strcmp(arg, longopt) == 0;
}

int main(int argc, const char **argv)
{
    int i = 1;
    while (i < argc) {
        if (match_opt(argv[i], "-p", "--pem")) {
            if (check_param(++i == argc, "--pem")) break;
            pem_filename = argv[i++];
        }
        else if (match_opt(argv[i], "-d", "--der")) {
            if (check_param(++i == argc, "--der")) break;
            der_filename = argv[i++];
        }
        else if (match_opt(argv[i], "-h", "--help")) {
            help_text = true;
            i++;
        }
        else {
            fprintf(stderr, "error: unknown option: %s\n", argv[i]);
            help_text = true;
            break;
        }
    }
    if (!pem_filename && !der_filename) {
        help_text = true;
    }
    if (help_text) {
        print_help(argc, argv);
        exit(1);
    }
    if (pem_filename) {
        dump_asn1_pem(pem_filename);
    }
    if (der_filename) {
        dump_asn1_der(der_filename);
    }

    exit(0);
}
