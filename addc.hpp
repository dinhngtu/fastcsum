#pragma once

#include <cstring>

namespace fastcsum {
namespace impl {

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

[[gnu::always_inline]] static inline unsigned int
addc(unsigned int a, unsigned int b, unsigned int cin, unsigned int *cout) {
#ifdef _fastcsum_has_addc
    return __builtin_addc(a, b, cin, cout);
#else
    return addc_fallback(a, b, cin, cout);
#endif
}

[[gnu::always_inline]] static inline unsigned long int
addc(unsigned long int a, unsigned long int b, unsigned long int cin, unsigned long int *cout) {
#ifdef _fastcsum_has_addcl
    return __builtin_addcl(a, b, cin, cout);
#else
    return addc_fallback(a, b, cin, cout);
#endif
}

[[gnu::always_inline]] static inline unsigned long long int
addc(unsigned long long int a, unsigned long long int b, unsigned long long int cin, unsigned long long int *cout) {
#ifdef _fastcsum_has_addcll
    return __builtin_addcll(a, b, cin, cout);
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

static inline uint64_t csum_31bytes(const uint8_t *b, size_t size, uint64_t initial) {
    uint64_t ac = initial;
    uint64_t carry;
    if (size >= 16) {
        uint64_t tmp;
        memcpy(&tmp, &b[0], sizeof(tmp));
        ac = addc(ac, tmp, 0, &carry);
        // add an extra carry add here to help older gcc at the risk of adding an unnecessary adc
        ac += carry;
        memcpy(&tmp, &b[8], sizeof(tmp));
        ac = addc(ac, tmp, 0, &carry);
        ac += carry;
        b += 16;
        size -= 16;
    }
    if (size >= 8) {
        uint64_t tmp;
        memcpy(&tmp, &b[0], sizeof(tmp));
        ac = addc(ac, tmp, 0, &carry);
        ac += carry;
        b += 8;
        size -= 8;
    }
    if (size >= 4) {
        uint32_t tmp;
        memcpy(&tmp, &b[0], sizeof(tmp));
        ac = addc(ac, static_cast<uint64_t>(tmp), 0, &carry);
        ac += carry;
        b += 4;
        size -= 4;
    }
    if (size >= 2) {
        uint16_t tmp;
        memcpy(&tmp, &b[0], sizeof(tmp));
        ac = addc(ac, static_cast<uint64_t>(tmp), 0, &carry);
        ac += carry;
        b += 2;
        size -= 2;
    }
    if (size) {
        ac = addc(ac, static_cast<uint64_t>(b[0]), 0, &carry);
        ac += carry;
    }
    return ac;
}

} // namespace impl
} // namespace fastcsum
