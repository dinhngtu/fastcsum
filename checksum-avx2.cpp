#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cpuid.h>
#include <immintrin.h>

#include "fastcsum.hpp"

#if __AVX2__
#define _fastcsum_has_avx2 1
#else
#undef _fastcsum_has_avx2
#endif

namespace fastcsum {
namespace impl {

#if _fastcsum_has_avx2

bool fastcsum_has_avx2() {
    unsigned int eax, ebx, ecx, edx;
    return __get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx) && (ebx & bit_AVX2);
}

static uint64_t csum_31bytes(const uint8_t *b, size_t size, uint64_t initial) {
    unsigned long long ac = initial;
    unsigned char carry;
    if (size >= 16) {
        carry = _addcarry_u64(0, ac, *reinterpret_cast<const uint64_t *>(&b[0]), &ac);
        carry = _addcarry_u64(carry, ac, *reinterpret_cast<const uint64_t *>(&b[8]), &ac);
        ac += carry;
        b += 16;
        size -= 16;
    }
    if (size >= 8) {
        carry = _addcarry_u64(0, ac, *reinterpret_cast<const uint64_t *>(&b[0]), &ac);
        ac += carry;
        b += 8;
        size -= 8;
    }
    if (size >= 4) {
        carry = _addcarry_u64(0, ac, static_cast<uint64_t>(*reinterpret_cast<const uint32_t *>(&b[0])), &ac);
        ac += carry;
        b += 4;
        size -= 4;
    }
    if (size >= 2) {
        carry = _addcarry_u64(0, ac, static_cast<uint64_t>(*reinterpret_cast<const uint16_t *>(&b[0])), &ac);
        ac += carry;
        b += 2;
        size -= 2;
    }
    if (size) {
        carry = _addcarry_u64(0, ac, static_cast<uint64_t>(b[0]), &ac);
        ac += carry;
    }
    return ac;
}

static inline void addc_epi32(__m256i &s, __m256i &c, __m256i a, __m256i b) {
    __m256i mask = _mm256_set1_epi32(0x80000000);
    s = _mm256_add_epi32(a, b);
    c = _mm256_cmpgt_epi32(_mm256_xor_si256(mask, a), _mm256_xor_si256(mask, s));
    c = _mm256_srli_epi32(c, 31);
}

// arithmetically, c is carry minus 1 (all 1 if no overflow, all 0 if overflow)
// therefore s+carry = s+c+1
static inline void addc_minus1_epu32(__m256i &s, __m256i &c, __m256i a, __m256i b) {
    s = _mm256_add_epi32(a, b);
    auto cc = _mm256_max_epu32(s, a);
    c = _mm256_cmpeq_epi32(s, cc);
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

bool fastcsum_has_avx2() {
    return false;
}

uint64_t fastcsum_nofold_avx2(
    [[maybe_unused]] const uint8_t *b,
    [[maybe_unused]] size_t size,
    [[maybe_unused]] uint64_t initial) {
    abort();
}

uint64_t fastcsum_nofold_avx2_align(
    [[maybe_unused]] const uint8_t *b,
    [[maybe_unused]] size_t size,
    [[maybe_unused]] uint64_t initial) {
    abort();
}

uint64_t fastcsum_nofold_avx2_v2(
    [[maybe_unused]] const uint8_t *b,
    [[maybe_unused]] size_t size,
    [[maybe_unused]] uint64_t initial) {
    abort();
}

uint64_t fastcsum_nofold_avx2_256b(
    [[maybe_unused]] const uint8_t *b,
    [[maybe_unused]] size_t size,
    [[maybe_unused]] uint64_t initial) {
    abort();
}

uint64_t fastcsum_nofold_avx2_v3(
    [[maybe_unused]] const uint8_t *b,
    [[maybe_unused]] size_t size,
    [[maybe_unused]] uint64_t initial) {
    abort();
}

uint64_t fastcsum_nofold_avx2_v4(
    [[maybe_unused]] const uint8_t *b,
    [[maybe_unused]] size_t size,
    [[maybe_unused]] uint64_t initial) {
    abort();
}

uint64_t fastcsum_nofold_avx2_v5(
    [[maybe_unused]] const uint8_t *b,
    [[maybe_unused]] size_t size,
    [[maybe_unused]] uint64_t initial) {
    abort();
}

#endif

} // namespace impl
} // namespace fastcsum
