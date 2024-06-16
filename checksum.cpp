#include "checksum.hpp"

template <typename T>
static inline T addc_fallback(T a, T b, T cin, T *cout) {
    T s;
    bool c1 = __builtin_add_overflow(a, b, &s);
    bool c2 = __builtin_add_overflow(s, cin, &s);
    *cout = c1 | c2;
    return s;
}

static inline unsigned int addc(unsigned int a, unsigned int b, unsigned int cin, unsigned int *cout) {
#if __has_builtin(__builtin_addc)
    return __builtin_addc(a, b, cin, cout);
#else
    return addc_fallback(a, b, cin, cout);
#endif
}

static inline unsigned long int
addc(unsigned long int a, unsigned long int b, unsigned long int cin, unsigned long int *cout) {
#if __has_builtin(__builtin_addcl)
    return __builtin_addcl(a, b, cin, cout);
#else
    return addc_fallback(a, b, cin, cout);
#endif
}

static inline unsigned long long int
addc(unsigned long long int a, unsigned long long int b, unsigned long long int cin, unsigned long long int *cout) {
#if __has_builtin(__builtin_addcll)
    return __builtin_addcll(a, b, cin, cout);
#else
    return addc_fallback(a, b, cin, cout);
#endif
}

uint64_t checksum_nofold_generic(const uint8_t *b, size_t size, uint64_t initial) {
    uint64_t ac = initial;
    uint64_t carry = 0;

    while (size >= 32) {
        ac = addc(ac, *reinterpret_cast<const uint64_t *>(&b[0]), 0, &carry);
        ac = addc(ac, *reinterpret_cast<const uint64_t *>(&b[8]), carry, &carry);
        ac = addc(ac, *reinterpret_cast<const uint64_t *>(&b[16]), carry, &carry);
        ac = addc(ac, *reinterpret_cast<const uint64_t *>(&b[24]), carry, &carry);
        ac += carry;
        b += 32;
        size -= 32;
    }
    if (size >= 16) {
        ac = addc(ac, *reinterpret_cast<const uint64_t *>(&b[0]), 0, &carry);
        ac = addc(ac, *reinterpret_cast<const uint64_t *>(&b[8]), carry, &carry);
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
        ac = addc(ac, static_cast<uint64_t>(b[0]), 0, &carry);
        ac += carry;
    }

    return ac;
}
