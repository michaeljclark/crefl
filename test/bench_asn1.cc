#undef NDEBUG
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <chrono>

#include "casn1.h"

using namespace std::chrono;

typedef time_point<high_resolution_clock> time_type;

const char pi_str[] = "3.141592653589793";
const unsigned char pi_asn[] = { 0x09, 0x09, 0x80, 0xD0, 0x03, 0x24, 0x3F, 0x6A, 0x88, 0x85, 0xA3 };

const unsigned long long i13 = 72057594037927935;
const char i13_str[] = "72057594037927935";
const unsigned char i13_asn[] = { 0x02, 0x08, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

const size_t count = 10000000;

static const char* format_unit(size_t count)
{
    static char buf[32];
    if (count % 1000000000 == 0) {
        snprintf(buf, sizeof(buf), "%zuG", count / 1000000000);
    } else if (count % 1000000 == 0) {
        snprintf(buf, sizeof(buf), "%zuM", count / 1000000);
    } else if (count % 1000 == 0) {
        snprintf(buf, sizeof(buf), "%zuK", count / 1000);
    } else {
        snprintf(buf, sizeof(buf), "%zu", count);
    }
    return buf;
}

static const char* format_comma(size_t count)
{
    static char buf[32];
    char buf1[32];

    snprintf(buf1, sizeof(buf1), "%zu", count);

    size_t l = strlen(buf1), i = 0, j = 0;
    for (; i < l; i++, j++) {
        buf[j] = buf1[i];
        if ((l-i-1) % 3 == 0 && i != l -1) {
            buf[++j] = ',';
        }
    }
    buf[j] == '\0';

    return buf;
}

static void bench_ascii_strtod()
{
    double f;
    auto st = high_resolution_clock::now();
    for (size_t i = 0; i < count; i++) {
        f = strtod(pi_str, NULL);
    }
    auto et = high_resolution_clock::now();

    double t = (double)duration_cast<nanoseconds>(et - st).count();
    printf("f64: strtod (base10)   : %s ops in %.2f sec, %6.2f ns, %10s ops/sec, %7.3f MiB/sec\n",
        format_unit(count), t / 1e9, t / count, format_comma(count * (1e9 / t)),
        count * (1e9 / t) * 8.0 / (1024*1024));

    assert(abs(f - 3.141592) < 0.0001);
}

static void bench_asn1_real()
{
    double f;
    crefl_buf *buf = crefl_buf_new(128);
    crefl_buf_write_bytes(buf, (const char*)pi_asn, sizeof(pi_asn));

    auto st = high_resolution_clock::now();
    for (size_t i = 0; i < count; i++) {
        crefl_buf_reset(buf);
        crefl_asn1_der_real_f64_read(buf, asn1_tag_real, &f);
    }
    auto et = high_resolution_clock::now();
    crefl_buf_destroy(buf);

    double t = (double)duration_cast<nanoseconds>(et - st).count();
    printf("f64: ASN.1 DER (Real)  : %s ops in %.2f sec, %6.2f ns, %10s ops/sec, %7.3f MiB/sec\n",
        format_unit(count), t / 1e9, t / count, format_comma(count * (1e9 / t)),
        count * (1e9 / t) * 8.0 / (1024*1024));

    assert(abs(f - 3.141592) < 0.0001);
}

static void bench_ascii_strtoull()
{
    unsigned long long d;
    auto st = high_resolution_clock::now();
    for (size_t i = 0; i < count; i++) {
        d = strtoull(i13_str, NULL, 10);
    }
    auto et = high_resolution_clock::now();

    double t = (double)duration_cast<nanoseconds>(et - st).count();
    printf("u64: strtoull (base10) : %s ops in %.2f sec, %6.2f ns, %10s ops/sec, %7.3f MiB/sec\n",
        format_unit(count), t / 1e9, t / count, format_comma(count * (1e9 / t)),
        count * (1e9 / t) * 8.0 / (1024*1024));

    assert(d == i13);
}

static void bench_asn1_integer()
{
    unsigned long long d;
    crefl_buf *buf = crefl_buf_new(128);
    crefl_buf_write_bytes(buf, (const char*)i13_asn, sizeof(i13_asn));

    auto st = high_resolution_clock::now();
    for (size_t i = 0; i < count; i++) {
        crefl_buf_reset(buf);
        crefl_asn1_der_integer_u64_read(buf, asn1_tag_integer, &d);
    }
    auto et = high_resolution_clock::now();
    crefl_buf_destroy(buf);

    double t = (double)duration_cast<nanoseconds>(et - st).count();
    printf("u64: ASN.1 DER (Int)   : %s ops in %.2f sec, %6.2f ns, %10s ops/sec, %7.3f MiB/sec\n",
        format_unit(count), t / 1e9, t / count, format_comma(count * (1e9 / t)),
        count * (1e9 / t) * 8.0 / (1024*1024));

    assert(d == i13);
}

int main(int argc, char **argv)
{
    bench_ascii_strtod();
    bench_asn1_real();
    bench_ascii_strtoull();
    bench_asn1_integer();
}
