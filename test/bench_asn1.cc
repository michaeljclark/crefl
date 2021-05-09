#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <chrono>

#include "casn1.h"

using namespace std::chrono;

typedef time_point<high_resolution_clock> time_type;

const char pi_str[] = "3.141592653589793";
const unsigned char pi_asn[] = { 0x09, 0x09, 0x80, 0xD0, 0x03, 0x24, 0x3F, 0x6A, 0x88, 0x85, 0xA3 };

const size_t count = 10000000;

void parse_str()
{
    double f;
    auto st = high_resolution_clock::now();
    for (size_t i = 0; i < count; i++) {
        f = strtod(pi_str, NULL);
    }
    auto et = high_resolution_clock::now();

    double t = (double)duration_cast<nanoseconds>(et - st).count();
    printf("strtod (base 10): %8zu in %.2f sec, %6.2f ns, %8.f req/sec\n",
        count, t / 1e9, t / count, count * (1e9 / t));
    printf("                  IEEE-754.1-DP equivalent %8.3f MiB/sec\n",
        count * (1e9 / t) * 8.0 / (1024*1024));

    assert(abs(f - 3.141592) < 0.0001);
}

void parse_asn1()
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
    printf("ASN.1 DER (Real): %8zu in %.2f sec, %6.2f ns, %8.f req/sec\n",
        count, t / 1e9, t / count, count * (1e9 / t));
    printf("                  IEEE-754.1-DP equivalent %8.3f MiB/sec\n",
        count * (1e9 / t) * 8.0 / (1024*1024));

    assert(abs(f - 3.141592) < 0.0001);
}

int main(int argc, char **argv)
{
    parse_str();
    parse_asn1();
}
