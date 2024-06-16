#include <cstdint>
#include <cstddef>
#include <immintrin.h>
#include <boost/endian.hpp>

#include "checksum.hpp"
#include "addc.hpp"

#if !__clang__

uint64_t checksum_nofold_avx2(const uint8_t *b, size_t size, uint64_t initial) {
    uint64_t ac = initial;
    uint64_t carry = 0;
    __m256i vac{};
    __m256i vdt, vsum, vcarry;
    __m256i mask = _mm256_set1_epi32(0x80000000);
    //__m256i cmask = _mm256_set1_epi32(1);
    bool needs_flip = false;

    if (size >= 32) {
        auto off = reinterpret_cast<uintptr_t>(b) & 31;
        if (off) {
            vdt = _mm256_loadu_si256(reinterpret_cast<const __m256i_u *>(b));
            vdt = _mm256_bslli_epi128(vdt, 32 - off);
        }
        needs_flip = off & 1;
    }

    while (size >= 128) {
        vdt = _mm256_load_si256(reinterpret_cast<const __m256i *>(b));
        vsum = _mm256_add_epi32(vac, vdt);
        vcarry = _mm256_cmpgt_epi32(_mm256_xor_si256(mask, vac), _mm256_xor_si256(mask, vsum));
        // vcarry = _mm256_and_si256(vcarry, cmask);
        vcarry = _mm256_srli_epi32(vcarry, 31);
        vac = _mm256_add_epi32(vsum, vcarry);

        vdt = _mm256_load_si256(reinterpret_cast<const __m256i *>(b + 32));
        vsum = _mm256_add_epi32(vac, vdt);
        vcarry = _mm256_cmpgt_epi32(_mm256_xor_si256(mask, vac), _mm256_xor_si256(mask, vsum));
        vcarry = _mm256_srli_epi32(vcarry, 31);
        vac = _mm256_add_epi32(vsum, vcarry);

        vdt = _mm256_load_si256(reinterpret_cast<const __m256i *>(b + 64));
        vsum = _mm256_add_epi32(vac, vdt);
        vcarry = _mm256_cmpgt_epi32(_mm256_xor_si256(mask, vac), _mm256_xor_si256(mask, vsum));
        vcarry = _mm256_srli_epi32(vcarry, 31);
        vac = _mm256_add_epi32(vsum, vcarry);

        vdt = _mm256_load_si256(reinterpret_cast<const __m256i *>(b + 96));
        vsum = _mm256_add_epi32(vac, vdt);
        vcarry = _mm256_cmpgt_epi32(_mm256_xor_si256(mask, vac), _mm256_xor_si256(mask, vsum));
        vcarry = _mm256_srli_epi32(vcarry, 31);
        vac = _mm256_add_epi32(vsum, vcarry);

        b += 128;
        size -= 128;
    }
    if (size >= 64) {
        vdt = _mm256_load_si256(reinterpret_cast<const __m256i *>(b));
        vsum = _mm256_add_epi32(vac, vdt);
        vcarry = _mm256_cmpgt_epi32(_mm256_xor_si256(mask, vac), _mm256_xor_si256(mask, vsum));
        vcarry = _mm256_srli_epi32(vcarry, 31);
        vac = _mm256_add_epi32(vsum, vcarry);

        vdt = _mm256_load_si256(reinterpret_cast<const __m256i *>(b + 32));
        vsum = _mm256_add_epi32(vac, vdt);
        vcarry = _mm256_cmpgt_epi32(_mm256_xor_si256(mask, vac), _mm256_xor_si256(mask, vsum));
        vcarry = _mm256_srli_epi32(vcarry, 31);
        vac = _mm256_add_epi32(vsum, vcarry);

        b += 64;
        size -= 64;
    }
    if (size >= 32) {
        vdt = _mm256_load_si256(reinterpret_cast<const __m256i *>(b));
        vsum = _mm256_add_epi32(vac, vdt);
        vcarry = _mm256_cmpgt_epi32(_mm256_xor_si256(mask, vac), _mm256_xor_si256(mask, vsum));
        vcarry = _mm256_srli_epi32(vcarry, 31);
        vac = _mm256_add_epi32(vsum, vcarry);

        b += 32;
        size -= 32;
    }

    __v4di out[4];
    _mm256_store_si256(&out[0], vac);
    ac = addc(ac, vac[0], 0, &carry);
    ac = addc(ac, vac[1], carry, &carry);
    ac = addc(ac, vac[2], carry, &carry);
    ac = addc(ac, vac[3], carry, &carry);
    ac += carry;

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

    // not sure why flipping is not needed here
    if (needs_flip)
        // boost::endian::endian_reverse_inplace(ac);
        ;

    return ac;
}

#endif
