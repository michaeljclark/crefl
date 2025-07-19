#undef NDEBUG
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include <cinf/asn1.h>

const char* leb_fmt = "\nLEB out(%llu) in(%llu)\n";

const float pi_f32 = 3.141592f;
const double pi_f64 = 3.141592653589793;
const unsigned char pi_asn[] = { 0x80, 0xD0, 0x03, 0x24, 0x3F, 0x6A, 0x88, 0x85, 0xA3 };

void test_leb(u64 val)
{
    u64 val2;
    cinf_buf *buf = cinf_buf_new(128);
    cinf_leb_u64_write(buf, &val);
    cinf_buf_reset(buf);
    cinf_leb_u64_read(buf, &val2);
    printf(leb_fmt, val, val2);
    cinf_buf_dump(buf);
    cinf_buf_destroy(buf);
}

void test_leb_misc()
{
    test_leb(32);
    test_leb(4096);
    test_leb(524288);
    test_leb(67108864);
    test_leb(8589934592);
    test_leb(1099511627776);
    test_leb(140737488355328);
    test_leb(18014398509481984);
}

int main(int argc, const char **argv)
{
    test_leb_misc();
}
