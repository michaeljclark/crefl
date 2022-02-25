#undef NDEBUG
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <wchar.h>

#include <crefl/buf.h>
#include <crefl/asn1.h>

int format_vf8(char *buf, size_t buflen, uint8_t v)
{
    float f;
    char s[16] = { 0 };
    int n = 0;
    crefl_buf *vfbuf = crefl_buf_new(1);
    crefl_buf_write_bytes(vfbuf, (const char*)&v, 1);
    crefl_buf_reset(vfbuf);
    crefl_vf_f32_read(vfbuf, &f);
    crefl_format_byte(s, sizeof(s), v);
    if (isnan(f)) {
        union { float f; unsigned x; } u = { f };
        char x[32];
        snprintf(x, sizeof(x), "%snan(0x%x)", (u.x >> 31) ? "-" : "", v & 0xf);
        n = snprintf(buf, buflen, "%-3d %s %9s", v, s, x);
    }
    else {
        n = snprintf(buf, buflen, "%-3d %s %9.4f", v, s, f);
    }
    crefl_buf_destroy(vfbuf);
    return n;
}

int string_width(const char *buf)
{
    mbstate_t t = { 0 };
    return mbsrtowcs (NULL, &buf, strlen(buf), &t);
}

void print_table(int ncols)
{
    char buf[64];
    int nrows = 128/ncols;
    for (uint8_t c = 0; c < ncols; c++) {
        for (uint8_t r = 0; r < nrows; r++) {
            uint8_t v = c*nrows + r;
            int n = format_vf8(buf, sizeof(buf), v);
	    int w = string_width(buf);
            printf("%s%s%s", buf, "    ",
                   (v % ncols == ncols-1) ? "\n" : "");
        }
    }
}

int main()
{
    print_table(4);
    return 0;
}
