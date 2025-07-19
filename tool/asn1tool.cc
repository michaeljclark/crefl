#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cerrno>

#include <string>
#include <vector>

#include <cinf/util.h>
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

static void dump_asn1(const char *filename)
{
    std::vector<uint8_t> v;
    if (cinf_read_file(v, filename) != 0) return;
    cinf_buf *buf = cinf_buf_new(v.size());
    memcpy(buf->data, v.data(), v.size());
    if (read_asn1(buf, 0, v.size(), 0) < 0) {
        fprintf(stderr, "error: recurse_asn1 returned an error\n");
    }
}

int main(int argc, const char **argv)
{
    if (argc != 3) goto help_exit;

    if (strcmp(argv[1], "--dump") == 0);
    else goto help_exit;

    dump_asn1(argv[2]);
    exit(0);

help_exit:
    fprintf(stderr, "usage: %s [--dump] <filename.refl>\n", argv[0]);
    exit(1);
}
