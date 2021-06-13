#undef NDEBUG
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <random>

#include "casn1.h"

void print_header()
{
    printf("%4s %5s %8s - %-8s %8s %8s %8s\n",
        "A", "B", "x", "y", "size(A)", "size(B)", "p");
}

size_t test_vf64(double f)
{
    double r;
    size_t s;
    crefl_buf *buf = crefl_buf_new(128);
    assert(!crefl_vf_f64_write(buf, &f));
    s = crefl_buf_offset(buf);
    crefl_buf_reset(buf);
    crefl_vf_f64_read(buf, &r);
    assert(isnan(f) ? isnan(r) : f == r);
    crefl_buf_destroy(buf);
    return s;
}

void test_vf64_rand(double x, double y, size_t count)
{
    size_t s = 0;
    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(x,y);
    generator.seed(0);
    for (size_t i = 0; i < count; i++) {
        s += test_vf64(distribution(generator));
    }
    printf("%4s %5s %8.1g - %-8.1g %8zu %8zu %8.3f %%\n",
        "f64", "vf128", x, y, count * 8, s, (((double)s / (double)(count * 8)) - 1.)*100.);
}

size_t test_vf32(float f)
{
    float r;
    size_t s;
    crefl_buf *buf = crefl_buf_new(128);
    assert(!crefl_vf_f32_write(buf, &f));
    s = crefl_buf_offset(buf);
    crefl_buf_reset(buf);
    crefl_vf_f32_read(buf, &r);
    assert(isnan(f) ? isnan(r) : f == r);
    crefl_buf_destroy(buf);
    return s;
}

void test_vf32_rand(float x, float y, size_t count)
{
    size_t s = 0;
    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution(x,y);
    generator.seed(0);
    for (size_t i = 0; i < count; i++) {
        s += test_vf32(distribution(generator));
    }
    printf("%4s %5s %8.1g - %-8.1g %8zu %8zu %8.3f %%\n",
        "f32", "vf128", x, y, count * 4, s, (((double)s / (double)(count * 4)) - 1.)*100.);
}

int main(int argc, const char **argv)
{
    const size_t count = 1000;
    print_header();
    test_vf64_rand(-1,0,count);
    test_vf64_rand(0,1,count);
    test_vf64_rand(-0.5,0.5,count);
    test_vf64_rand(-1,1,count);
    test_vf64_rand(-10,10,count);
    test_vf64_rand(-100,100,count);
    test_vf64_rand(-1000,1000,count);
    test_vf64_rand(-1e38,1e38,count);
    test_vf64_rand(-1e307,1e307,count);
    test_vf32_rand(-1,0,count);
    test_vf32_rand(0,1,count);
    test_vf32_rand(-0.5,0.5,count);
    test_vf32_rand(-1,1,count);
    test_vf32_rand(-10,10,count);
    test_vf32_rand(-100,100,count);
    test_vf32_rand(-1000,1000,count);
    test_vf32_rand(-1e38,1e38,count);
}
