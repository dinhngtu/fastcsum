#include <cstdlib>

#include "fastcsum.hpp"
#include "vectorized.hpp"

namespace fastcsum {
namespace impl {

#if _fastcsum_has_avx2

static inline void addc_minus1_vec8(__v8su &s, __v8su &c, __v8su a, __v8su b) {
    s = a + b;
    c = (__v8su)_mm256_max_epu32((__m256i)s, (__m256i)a);
    c = s == c;
}

static inline uint64_t addc_fold_vec8(__v8su &v, uint64_t initial) {
    unsigned long long ac = initial;
    unsigned char c;
    __v4du d = (__v4du)v;
    c = _addcarry_u64(0, ac, static_cast<uint64_t>(d[0]), &ac);
    c = _addcarry_u64(c, ac, static_cast<uint64_t>(d[1]), &ac);
    c = _addcarry_u64(c, ac, static_cast<uint64_t>(d[2]), &ac);
    c = _addcarry_u64(c, ac, static_cast<uint64_t>(d[3]), &ac);
    ac += c;
    return ac;
}

uint64_t fastcsum_nofold_vec256(const uint8_t *b, size_t size, uint64_t initial) {
    unsigned long long ac = initial;
    __v8su vac{};

    while (size >= 256) {
        __v8su v1, c1;
        v1 = (__v8su)_mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b));
        addc_minus1_vec8(v1, c1, v1, (__v8su)_mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 32)));

        __v8su v2, c2;
        v2 = (__v8su)_mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 64));
        addc_minus1_vec8(v2, c2, v2, (__v8su)_mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 96)));

        __v8su v3, c3;
        v3 = (__v8su)_mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 128));
        addc_minus1_vec8(v3, c3, v3, (__v8su)_mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 160)));

        __v8su v4, c4;
        v4 = (__v8su)_mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 192));
        addc_minus1_vec8(v4, c4, v4, (__v8su)_mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 224)));

        __v8su v5, c5;
        addc_minus1_vec8(v5, c5, v1, v2);
        __v8su v6, c6;
        addc_minus1_vec8(v6, c6, v3, v4);

        __v8su v7, c7;
        addc_minus1_vec8(v7, c7, v5, v6);
        __v8su c;
        addc_minus1_vec8(vac, c, vac, v7);
        vac += c + c1 + c2 + c3 + c4 + c5 + c6 + c7;
        vac += 8;

        b += 256;
        size -= 256;
    }
    if (size >= 128) {
        __v8su v1, c1;
        v1 = (__v8su)_mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b));
        addc_minus1_vec8(v1, c1, v1, (__v8su)_mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 32)));

        __v8su v2, c2;
        v2 = (__v8su)_mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 64));
        addc_minus1_vec8(v2, c2, v2, (__v8su)_mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 96)));

        __v8su v5, c5;
        addc_minus1_vec8(v5, c5, v1, v2);

        __v8su c;
        addc_minus1_vec8(vac, c, vac, v5);
        vac += c + c1 + c2 + c5;
        vac += 4;

        b += 128;
        size -= 128;
    }
    if (size >= 64) {
        __v8su v1, c1;
        v1 = (__v8su)_mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b));
        addc_minus1_vec8(v1, c1, v1, (__v8su)_mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 32)));

        __v8su c;
        addc_minus1_vec8(vac, c, vac, v1);
        vac += c + c1;
        vac += 2;

        b += 64;
        size -= 64;
    }
    if (size >= 32) {
        __v8su c;
        addc_minus1_vec8(vac, c, vac, (__v8su)_mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b)));
        vac += c;
        vac += 1;

        b += 32;
        size -= 32;
    }

    ac = addc_fold_vec8(vac, ac);
    ac = csum_31bytes(b, size, ac);

    return ac;
}

#else

uint64_t fastcsum_nofold_vec256(
    [[maybe_unused]] const uint8_t *b,
    [[maybe_unused]] size_t size,
    [[maybe_unused]] uint64_t initial) {
    abort();
}

#endif

} // namespace impl
} // namespace fastcsum
