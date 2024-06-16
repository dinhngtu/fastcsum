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
