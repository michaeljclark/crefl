// See LICENSE.md

#pragma once

#include  <cstddef>
#include  <cstdint>

#if defined (_MSC_VER)
#include <intrin.h>
#endif

/*! clz */
template <typename T>
inline int clz(T val)
{
	const int bits = sizeof(T) << 3;
	unsigned count = 0, found = 0;
	for (int i = bits - 1; i >= 0; --i) {
		count += !(found |= val & T(1)<<i ? 1 : 0);
	}
	return count;
}

/*! ctz */
template <typename T>
inline int ctz(T val)
{
	const int bits = sizeof(T) << 3;
	unsigned count = 0, found = 0;
	for (int i = 0; i < bits; ++i) {
		count += !(found |= val & T(1)<<i ? 1 : 0);
	}
	return count;
}

/* ctz specializations */
#if defined (__GNUC__)
template<> inline int clz(unsigned val) { return __builtin_clz(val); }
template<> inline int clz(unsigned long val) { return __builtin_clzll(val); }
template<> inline int clz(unsigned long long val) { return __builtin_clzll(val); }
template<> inline int ctz(unsigned val) { return __builtin_ctz(val); }
template<> inline int ctz(unsigned long val) { return __builtin_ctzll(val); }
template<> inline int ctz(unsigned long long val) { return __builtin_ctzll(val); }
#endif
#if defined (_MSC_VER)
#if defined (_M_X64)
template<> inline int clz(unsigned val)
{
	return (int)_lzcnt_u32(val);
}
template<> inline int clz(unsigned long long val)
{
	return (int)_lzcnt_u64(val);
}
template<> inline int ctz(unsigned val)
{
	return (int)_tzcnt_u32(val);
}
template<> inline int ctz(unsigned long long val)
{
	return (int)_tzcnt_u64(val);
}
#else
template<> inline int clz(unsigned val)
{
	unsigned long count;
	return _BitScanReverse(&count, val) ^ 31;
}
template<> inline int ctz(unsigned val)
{
	unsigned long count;
	return _BitScanForward(&count, val);
}
#endif
#endif
