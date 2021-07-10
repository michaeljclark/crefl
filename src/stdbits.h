#pragma once

/*
 * stdbits.h
 *
 * This header defines the following functions:
 *
 *   clz(int)     - count leading zeros
 *   ctz(int)     - count trialing zeros
 *   popcnt(int)  - population count
 *
 * This header can be included from C++ and uses template specialization
 *
 *   GCC -O2 -x c++ -std=c++14
 *   MSVC /O2 /TP /std:c++14
 *
 * This header can be included from C and uses C11 _Generic selection
 *
 *   GCC -O2 -x c -std=c11
 *   MSVC /O2 /TC /std:c11
 */

typedef unsigned int       __bits_u32;
typedef signed int         __bits_s32;
typedef unsigned long long __bits_u64;
typedef signed long long   __bits_s64;

#if defined (_MSC_VER)
#include <intrin.h>
#endif

#define _clz_defined 1
#define _ctz_defined 2
#define _popcnt_defined 4

/*
 * clz, ctz and popcnt mappings to compiler intrinsics
 *
 * notes
 *
 * 1. zero test should be optimized out whem lowering to tzcount and lzcount
 * 2. zero test is required for the older bitscan instructions.
 * 3. popcount intrinsic is missing on i386.
 */
#if defined (__GNUC__)
static inline unsigned clz_u32(__bits_u32 val) { return val == 0 ? 32 : __builtin_clz(val); }
static inline unsigned clz_u64(__bits_u64 val) { return val == 0 ? 64 : __builtin_clzll(val); }
static inline unsigned ctz_u32(__bits_u32 val) { return val == 0 ? 32 : __builtin_ctz(val); }
static inline unsigned ctz_u64(__bits_u64 val) { return val == 0 ? 64 : __builtin_ctzll(val); }
static inline unsigned popcnt_u32(__bits_u32 val) { return val == 0 ? 32 : __builtin_popcount(val); }
static inline unsigned popcnt_u64(__bits_u64 val) { return val == 0 ? 64 : __builtin_popcountll(val); }
#define _bits_defined (_clz_defined | _ctz_defined | _popcnt_defined)
#elif defined (_MSC_VER) && defined (_M_X64)
static inline unsigned clz_u32(__bits_u32 val) { return (int)_lzcnt_u32(val); }
static inline unsigned clz_u64(__bits_u64 val) { return (int)_lzcnt_u64(val); }
static inline unsigned ctz_u32(__bits_u32 val) { return (int)_tzcnt_u32(val); }
static inline unsigned ctz_u64(__bits_u64 val) { return (int)_tzcnt_u64(val); }
static inline unsigned popcnt_u32(__bits_u32 val) { return (int)__popcnt(val); }
static inline unsigned popcnt_u64(__bits_u64 val) { return (int)__popcnt64(val); }
#define _bits_defined (_clz_defined | _ctz_defined | _popcnt_defined)
#elif defined (_MSC_VER) && defined (_M_IX86)
static inline unsigned clz_u32(__bits_u32 val) { unsigned long count; return val == 0 ? 32 : (_BitScanReverse(&count, val) ^ 31); }
static inline unsigned clz_u64(__bits_u64 val) { unsigned long count; return val == 0 ? 64 : (_BitScanReverse64(&count, val) ^ 63); }
static inline unsigned ctz_u32(__bits_u32 val) { unsigned long count; return val == 0 ? 32 :_BitScanForward(&count, val); }
static inline unsigned ctz_u64(__bits_u64 val) { unsigned long count; return val == 0 ? 64 : _BitScanForward64(&count, val); }
/* there is no popcount intrinsic on i386 */
#define _bits_defined (_clz_defined | _ctz_defined)
#else
#define _bits_defined 0
#endif

/*
 * fallback algorithms from stanford bit twiddling hacks
 */

#if (_bits_defined & _popcnt_defined) != _popcnt_defined
static inline unsigned popcnt_u32(__bits_u32 val)
{
    val = (val & 0x55555555) + ((val >> 1) & 0x55555555);
    val = (val & 0x33333333) + ((val >> 2) & 0x33333333);
    val = (val & 0x0F0F0F0F) + ((val >> 4) & 0x0F0F0F0F);
    val = (val & 0x00FF00FF) + ((val >> 8) & 0x00FF00FF);
    val = (val & 0x0000FFFF) + ((val >>16) & 0x0000FFFF);
    return (unsigned)val;
}
static inline unsigned popcnt_u64(__bits_u64 val)
{
    val = (val & 0x5555555555555555ULL) + ((val >>  1) & 0x5555555555555555ULL);
    val = (val & 0x3333333333333333ULL) + ((val >>  2) & 0x3333333333333333ULL);
    val = (val & 0x0F0F0F0F0F0F0F0FULL) + ((val >>  4) & 0x0F0F0F0F0F0F0F0FULL);
    val = (val & 0x00FF00FF00FF00FFULL) + ((val >>  8) & 0x00FF00FF00FF00FFULL);
    val = (val & 0x0000FFFF0000FFFFULL) + ((val >> 16) & 0x0000FFFF0000FFFFULL);
    val = (val & 0x00000000FFFFFFFFULL) + ((val >> 32) & 0x00000000FFFFFFFFULL);
    return (unsigned)val;
}
#endif

#if (_bits_defined & _clz_defined) != _clz_defined
static inline unsigned clz_u32(__bits_u32 x)
{
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >>16);
    return popcnt_u32(~x);
}

static inline unsigned clz_u64(__bits_u64 x)
{
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >>16);
    x = x | (x >>32);
    return popcnt_u64(~x);
}
#endif

#if (_bits_defined & _ctz_defined) != _ctz_defined
static inline unsigned ctz_u32(__bits_u32 v)
{
    unsigned c = 32;
    v &= -(__bits_s32)v;
    if (v) c--;
    if (v & 0x0000FFFF) c -= 16;
    if (v & 0x00FF00FF) c -= 8;
    if (v & 0x0F0F0F0F) c -= 4;
    if (v & 0x33333333) c -= 2;
    if (v & 0x55555555) c -= 1;
    return c;
}

static inline unsigned ctz_u64(__bits_u64 v)
{
    unsigned c = 64;
    v &= -(__bits_s64)v;
    if (v) c--;
    if (v & 0x00000000FFFFFFFFULL) c -= 32;
    if (v & 0x0000FFFF0000FFFFULL) c -= 16;
    if (v & 0x00FF00FF00FF00FFULL) c -= 8;
    if (v & 0x0F0F0F0F0F0F0F0FULL) c -= 4;
    if (v & 0x3333333333333333ULL) c -= 2;
    if (v & 0x5555555555555555ULL) c -= 1;
    return c;
}
#endif

/*
 * C11 generics mapping for clz, ctz and popcnt
 */
#if __STDC_VERSION__ >= 201112L
#define clz(X) _Generic((X), \
	unsigned int: clz_u32, signed int: clz_u32, \
	unsigned long long: clz_u64, signed long long: clz_u64)(X)
#define ctz(X) _Generic((X), \
	unsigned int: ctz_u32, signed int: ctz_u32, \
	unsigned long long: ctz_u64, signed long long: ctz_u64)(X)
#define popcnt(X) _Generic((X), \
	unsigned int: ctz_u32, signed int: popcnt_u32, \
	unsigned long long: ctz_u64, signed long long: popcnt_u64)(X)
#endif

/*
 * C++ template specialization for clz, ctz and popcnt
 */
#if defined (__cplusplus)
template <typename T> inline unsigned clz(T val);
template<> inline unsigned clz(signed int val) { return clz_u32(val); }
template<> inline unsigned clz(unsigned int val) { return clz_u32(val); }
#if defined (_MSC_VER)
template<> inline unsigned clz(signed long val) { return clz_u32(val); }
template<> inline unsigned clz(unsigned long val) { return clz_u32(val); }
#else
template<> inline unsigned clz(signed long val) { return clz_u64(val); }
template<> inline unsigned clz(unsigned long val) { return clz_u64(val); }
#endif
template<> inline unsigned clz(signed long long val) { return clz_u64(val); }
template<> inline unsigned clz(unsigned long long val) { return clz_u64(val); }

template <typename T> inline unsigned ctz(T val);
template<> inline unsigned ctz(signed int val) { return ctz_u32(val); }
template<> inline unsigned ctz(unsigned int val) { return ctz_u32(val); }
#if defined (_MSC_VER)
template<> inline unsigned ctz(signed long val) { return ctz_u32(val); }
template<> inline unsigned ctz(unsigned long val) { return ctz_u32(val); }
#else
template<> inline unsigned ctz(signed long val) { return ctz_u64(val); }
template<> inline unsigned ctz(unsigned long val) { return ctz_u64(val); }
#endif
template<> inline unsigned ctz(signed long long val) { return ctz_u64(val); }
template<> inline unsigned ctz(unsigned long long val) { return ctz_u64(val); }

template <typename T> inline unsigned popcnt(T val);
template<> inline unsigned popcnt(signed int val) { return popcnt_u32(val); }
template<> inline unsigned popcnt(unsigned int val) { return popcnt_u32(val); }
#if defined (_MSC_VER)
template<> inline unsigned popcnt(signed long val) { return popcnt_u32(val); }
template<> inline unsigned popcnt(unsigned long val) { return popcnt_u32(val); }
#else
template<> inline unsigned popcnt(signed long val) { return popcnt_u64(val); }
template<> inline unsigned popcnt(unsigned long val) { return popcnt_u64(val); }
#endif
template<> inline unsigned popcnt(signed long long val) { return popcnt_u64(val); }
template<> inline unsigned popcnt(unsigned long long val) { return popcnt_u64(val); }
#endif
