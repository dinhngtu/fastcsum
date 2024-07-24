#pragma once

#include <climits>

#if defined(__i386__) || defined(__x86_64__)
#include <x86intrin.h>
#endif

template <typename T>
[[gnu::always_inline]] static inline T addc_fallback(T a, T b, T cin, T *cout) {
    T s;
    bool c1 = __builtin_add_overflow(a, b, &s);
    bool c2 = __builtin_add_overflow(s, cin, &s);
    *cout = c1 | c2;
    return s;
}

#ifdef _fastcsum_has_addc
#undef _fastcsum_has_addc
#endif
#ifdef _fastcsum_has_addcl
#undef _fastcsum_has_addcl
#endif
#ifdef _fastcsum_has_addcll
#undef _fastcsum_has_addcll
#endif

#ifdef __has_builtin
#if __has_builtin(__builtin_addc)
#define _fastcsum_has_addc 1
#endif
#if __has_builtin(__builtin_addcl)
#define _fastcsum_has_addcl 1
#endif
#if __has_builtin(__builtin_addcll)
#define _fastcsum_has_addcll 1
#endif
#endif

#ifdef _fastcsum_has_addcarry_u32
#undef _fastcsum_has_addcarry_u32
#endif
#ifdef _fastcsum_has_addcarry_u64
#undef _fastcsum_has_addcarry_u64
#endif

#if defined(__i386__) || defined(__x86_64__)
#define _fastcsum_has_addcarry_u32 1
#endif
#if defined(__x86_64__)
#define _fastcsum_has_addcarry_u64 1
#endif

[[gnu::always_inline]] static inline unsigned int
addc(unsigned int a, unsigned int b, unsigned int cin, unsigned int *cout) {
#ifdef _fastcsum_has_addc
    return __builtin_addc(a, b, cin, cout);
#elif defined(_fastcsum_has_addcarry_u32)
    unsigned int s;
    *cout = _addcarry_u32(cin, a, b, &s);
    return s;
#else
    return addc_fallback(a, b, cin, cout);
#endif
}

[[gnu::always_inline]] static inline unsigned long
addc(unsigned long a, unsigned long b, unsigned long cin, unsigned long *cout) {
#ifdef _fastcsum_has_addcl
    return __builtin_addcl(a, b, cin, cout);
#elif defined(_fastcsum_has_addcarry_u32) && __SIZEOF_LONG__ == __SIZEOF_INT__
    unsigned int s;
    *cout = _addcarry_u32(cin, a, b, &s);
    return s;
#elif defined(_fastcsum_has_addcarry_u64) && __SIZEOF_LONG__ == __SIZEOF_LONG_LONG__
    unsigned long long s;
    *cout = _addcarry_u64(cin, a, b, &s);
    return s;
#else
    return addc_fallback(a, b, cin, cout);
#endif
}

[[gnu::always_inline]] static inline unsigned long long
addc(unsigned long long a, unsigned long long b, unsigned long long cin, unsigned long long *cout) {
#ifdef _fastcsum_has_addcll
    return __builtin_addcll(a, b, cin, cout);
#elif defined(_fastcsum_has_addcarry_u64)
    unsigned long long s;
    *cout = _addcarry_u64(cin, a, b, &s);
    return s;
#else
    return addc_fallback(a, b, cin, cout);
#endif
}

// arithmetically, c is carry minus 1 (all 1 if no overflow, all 0 if overflow)
// therefore s+carry = s+c+1
template <typename T>
[[gnu::always_inline]] static inline void addc_minus1_vec(T &s, T &c, T a, T b) {
    s = a + b;
    c = s > a ? s : a;
    c = s == c;
}

// arithmetically, c is -carry: 0 if no overflow, all 1 (=-1) if overflow
// therefore s+carry = s-c
template <typename T>
[[gnu::always_inline]] static inline void addc_negc_vec(T &s, T &c, T a, T b) {
    s = a + b;
    c = s < a;
}

static inline uint64_t csum_31bytes(const uint8_t *b, size_t size, uint64_t initial) {
    uint64_t ac = initial;
    uint64_t carry;
    if (size >= 16) {
        ac = addc(ac, *reinterpret_cast<const uint64_t *>(&b[0]), 0, &carry);
        // add an extra carry add here to help older gcc at the risk of adding an unnecessary adc
        ac += carry;
        ac = addc(ac, *reinterpret_cast<const uint64_t *>(&b[8]), 0, &carry);
        ac += carry;
        b += 16;
        size -= 16;
    }
    if (size >= 8) {
        ac = addc(ac, *reinterpret_cast<const uint64_t *>(&b[0]), 0, &carry);
        ac += carry;
        b += 8;
        size -= 8;
    }
    if (size >= 4) {
        ac = addc(ac, static_cast<uint64_t>(*reinterpret_cast<const uint32_t *>(&b[0])), 0, &carry);
        ac += carry;
        b += 4;
        size -= 4;
    }
    if (size >= 2) {
        ac = addc(ac, static_cast<uint64_t>(*reinterpret_cast<const uint16_t *>(&b[0])), 0, &carry);
        ac += carry;
        b += 2;
        size -= 2;
    }
    if (size) {
        uint64_t lastbyte = b[0];
        if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
            lastbyte <<= 8;
        ac = addc(ac, lastbyte, 0, &carry);
        ac += carry;
    }
    return ac;
}
