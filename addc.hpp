#pragma once

namespace fastcsum {
namespace impl {

template <typename T>
static inline T addc_fallback(T a, T b, T cin, T *cout) {
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

static inline unsigned int addc(unsigned int a, unsigned int b, unsigned int cin, unsigned int *cout) {
#ifdef _fastcsum_has_addc
    return __builtin_addc(a, b, cin, cout);
#else
    return addc_fallback(a, b, cin, cout);
#endif
}

static inline unsigned long int
addc(unsigned long int a, unsigned long int b, unsigned long int cin, unsigned long int *cout) {
#ifdef _fastcsum_has_addcl
    return __builtin_addcl(a, b, cin, cout);
#else
    return addc_fallback(a, b, cin, cout);
#endif
}

static inline unsigned long long int
addc(unsigned long long int a, unsigned long long int b, unsigned long long int cin, unsigned long long int *cout) {
#ifdef _fastcsum_has_addcll
    return __builtin_addcll(a, b, cin, cout);
#else
    return addc_fallback(a, b, cin, cout);
#endif
}

} // namespace impl
} // namespace fastcsum
