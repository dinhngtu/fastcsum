#include <stdexcept>
#include <cpuid.h>
#include <immintrin.h>

#include "fastcsum.hpp"
#include "addc.hpp"

using namespace fastcsum::impl;

namespace fastcsum {

#if FASTCSUM_ENABLE_AVX2

namespace impl {

static inline void addc_epi32(__m256i &s, __m256i &c, __m256i a, __m256i b) {
    __m256i mask = _mm256_set1_epi32(0x80000000);
    s = _mm256_add_epi32(a, b);
    c = _mm256_cmpgt_epi32(_mm256_xor_si256(mask, a), _mm256_xor_si256(mask, s));
    c = _mm256_srli_epi32(c, 31);
}

[[gnu::always_inline]] static inline void addc_minus1_epi32(__m256i &s, __m256i &c, __m256i a, __m256i b) {
    s = _mm256_add_epi32(a, b);
    c = _mm256_max_epu32(s, a);
    c = _mm256_cmpeq_epi32(s, c);
}

static inline uint64_t addc_fold_epi64(__m256i &v, uint64_t initial) {
    unsigned long long ac = initial;
    uint64_t out[4];
    unsigned char c;
    _mm256_storeu_si256(reinterpret_cast<__m256i_u *>(&out[0]), v);
    c = _addcarry_u64(0, ac, static_cast<uint64_t>(out[0]), &ac);
    c = _addcarry_u64(c, ac, static_cast<uint64_t>(out[1]), &ac);
    c = _addcarry_u64(c, ac, static_cast<uint64_t>(out[2]), &ac);
    c = _addcarry_u64(c, ac, static_cast<uint64_t>(out[3]), &ac);
    ac += c;
    return ac;
}

} // namespace impl

uint64_t fastcsum_nofold_avx2(const uint8_t *b, size_t size, uint64_t initial) {
    unsigned long long ac = initial;
    __m256i vac = _mm256_setzero_si256();
    __m256i v, s, c;

    while (size >= 128) {
        v = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        v = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 32));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        v = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 64));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        v = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 96));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        b += 128;
        size -= 128;
    }
    if (size >= 64) {
        v = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        v = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 32));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        b += 64;
        size -= 64;
    }
    if (size >= 32) {
        v = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        b += 32;
        size -= 32;
    }

    ac = addc_fold_epi64(vac, ac);
    ac = csum_31bytes(b, size, ac);

    return ac;
}

uint64_t fastcsum_nofold_avx2_align(const uint8_t *b, size_t size, uint64_t initial) {
    unsigned long long ac = initial;
    __m256i vac = _mm256_setzero_si256();
    __m256i v, s, c;
    bool needs_flip = false;

    if (size >= 32) {
        auto off = reinterpret_cast<uintptr_t>(b) & 31;
        if (off) {
            needs_flip = off & 1;
            auto todo = 32 - off;
            ac = csum_31bytes(b, todo, ac);
            b += todo;
            size -= todo;
            if (needs_flip)
                ac = __builtin_bswap64(ac);
        }
    }

    while (size >= 128) {
        v = _mm256_load_si256(reinterpret_cast<const __m256i *>(b));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        v = _mm256_load_si256(reinterpret_cast<const __m256i *>(b + 32));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        v = _mm256_load_si256(reinterpret_cast<const __m256i *>(b + 64));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        v = _mm256_load_si256(reinterpret_cast<const __m256i *>(b + 96));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        b += 128;
        size -= 128;
    }
    if (size >= 64) {
        v = _mm256_load_si256(reinterpret_cast<const __m256i *>(b));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        v = _mm256_load_si256(reinterpret_cast<const __m256i *>(b + 32));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        b += 64;
        size -= 64;
    }
    if (size >= 32) {
        v = _mm256_load_si256(reinterpret_cast<const __m256i *>(b));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        b += 32;
        size -= 32;
    }

    ac = addc_fold_epi64(vac, ac);
    ac = csum_31bytes(b, size, ac);

    if (needs_flip)
        ac = __builtin_bswap64(ac);

    return ac;
}

uint64_t fastcsum_nofold_avx2_v2(const uint8_t *b, size_t size, uint64_t initial) {
    unsigned long long ac = initial;
    __m256i vac = _mm256_setzero_si256();
    __m256i v, s, c;

    while (size >= 128) {
        auto v1 = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b));
        auto v2 = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 32));
        auto v3 = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 64));
        auto v4 = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 96));

        __m256i s1, c1;
        addc_epi32(s1, c1, v1, v2);

        __m256i s2, c2;
        addc_epi32(s2, c2, v3, v4);

        addc_epi32(s, c, s1, s2);
        c = _mm256_add_epi32(c, c1);
        c = _mm256_add_epi32(c, c2);

        __m256i cc;
        addc_epi32(vac, cc, vac, s);
        c = _mm256_add_epi32(c, cc);
        vac = _mm256_add_epi32(vac, c);

        b += 128;
        size -= 128;
    }
    if (size >= 64) {
        v = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        v = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 32));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        b += 64;
        size -= 64;
    }
    if (size >= 32) {
        v = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        b += 32;
        size -= 32;
    }

    ac = addc_fold_epi64(vac, ac);
    ac = csum_31bytes(b, size, ac);

    return ac;
}

uint64_t fastcsum_nofold_avx2_256b(const uint8_t *b, size_t size, uint64_t initial) {
    unsigned long long ac = initial;
    __m256i vac = _mm256_setzero_si256();
    __m256i v, s, c;

    while (size >= 256) {
        auto v1 = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b));
        auto v2 = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 32));
        auto v3 = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 64));
        auto v4 = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 96));
        auto v5 = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 128));
        auto v6 = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 160));
        auto v7 = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 192));
        auto v8 = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 224));

        __m256i s1, c1;
        addc_epi32(s1, c1, v1, v2);
        __m256i s2, c2;
        addc_epi32(s2, c2, v3, v4);
        __m256i s3, c3;
        addc_epi32(s3, c3, v5, v6);
        __m256i s4, c4;
        addc_epi32(s4, c4, v7, v8);

        __m256i s5, c5;
        addc_epi32(s5, c5, s1, s2);
        __m256i s6, c6;
        addc_epi32(s6, c6, s3, s4);
        addc_epi32(s, c, s5, s6);

        auto c7 = _mm256_add_epi32(c1, c2);
        auto c8 = _mm256_add_epi32(c3, c4);
        c = _mm256_add_epi32(c, _mm256_add_epi32(c5, c7));
        c = _mm256_add_epi32(c, _mm256_add_epi32(c6, c8));

        __m256i cc;
        addc_epi32(vac, cc, vac, s);
        c = _mm256_add_epi32(c, cc);
        vac = _mm256_add_epi32(vac, c);

        b += 256;
        size -= 256;
    }
    if (size >= 128) {
        auto v1 = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b));
        auto v2 = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 32));
        auto v3 = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 64));
        auto v4 = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 96));

        __m256i s1, c1;
        addc_epi32(s1, c1, v1, v2);

        __m256i s2, c2;
        addc_epi32(s2, c2, v3, v4);

        addc_epi32(s, c, s1, s2);
        c = _mm256_add_epi32(c, c1);
        c = _mm256_add_epi32(c, c2);

        __m256i cc;
        addc_epi32(vac, cc, vac, s);
        c = _mm256_add_epi32(c, cc);
        vac = _mm256_add_epi32(vac, c);

        b += 128;
        size -= 128;
    }
    if (size >= 64) {
        v = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        v = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 32));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        b += 64;
        size -= 64;
    }
    if (size >= 32) {
        v = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        b += 32;
        size -= 32;
    }

    ac = addc_fold_epi64(vac, ac);
    ac = csum_31bytes(b, size, ac);

    return ac;
}

uint64_t fastcsum_nofold_avx2_v4(const uint8_t *b, size_t size, uint64_t initial) {
    unsigned long long ac = initial;
    __m256i vac = _mm256_setzero_si256();
    __m256i v, s, c;

    while (size >= 128) {
        v = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        v = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 32));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        v = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 64));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        v = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 96));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        b += 128;
        size -= 128;
    }
    if (size >= 64) {
        v = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        v = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b + 32));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        b += 64;
        size -= 64;
    }
    if (size >= 32) {
        v = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b));
        addc_epi32(s, c, vac, v);
        vac = _mm256_add_epi32(s, c);

        b += 32;
        size -= 32;
    }

    ac = addc_fold_epi64(vac, ac);
    ac = csum_31bytes(b, size, ac);

    return ac;
}

#else

#define fastcsum_no_avx2(f) \
    uint64_t f([[maybe_unused]] const uint8_t *, [[maybe_unused]] size_t, [[maybe_unused]] uint64_t) { \
        throw std::logic_error("fastcsum was not built with AVX2"); \
    }

fastcsum_no_avx2(fastcsum_nofold_avx2);
fastcsum_no_avx2(fastcsum_nofold_avx2_align);
fastcsum_no_avx2(fastcsum_nofold_avx2_v2);
fastcsum_no_avx2(fastcsum_nofold_avx2_256b);
extern "C" fastcsum_no_avx2(fastcsum_nofold_avx2_v3);
fastcsum_no_avx2(fastcsum_nofold_avx2_v4);
extern "C" fastcsum_no_avx2(fastcsum_nofold_avx2_v5);
extern "C" fastcsum_no_avx2(fastcsum_nofold_avx2_v6);

#endif

} // namespace fastcsum
