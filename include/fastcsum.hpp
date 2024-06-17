#pragma once

#include <cstdint>
#include <cstddef>

namespace fastcsum {

namespace impl {

uint64_t fastcsum_nofold_generic64(const uint8_t *b, size_t size, uint64_t initial);
extern "C" uint64_t fastcsum_nofold_x64_128b(const uint8_t *ptr, size_t size, uint64_t initial);
extern "C" uint64_t fastcsum_nofold_x64_64b(const uint8_t *ptr, size_t size, uint64_t initial);

bool fastcsum_has_adx();
extern "C" uint64_t fastcsum_nofold_adx(const uint8_t *ptr, size_t size, uint64_t initial);
extern "C" uint64_t fastcsum_nofold_adx_v2(const uint8_t *ptr, size_t size, uint64_t initial);
extern "C" uint64_t fastcsum_nofold_adx_align(const uint8_t *ptr, size_t size, uint64_t initial);
extern "C" uint64_t fastcsum_nofold_adx_align2(const uint8_t *ptr, size_t size, uint64_t initial);

bool fastcsum_has_avx2();
uint64_t fastcsum_nofold_avx2(const uint8_t *ptr, size_t size, uint64_t initial);
uint64_t fastcsum_nofold_avx2_align(const uint8_t *ptr, size_t size, uint64_t initial);

} // namespace impl

// returns folded, complemented checksum in native byte order
static inline uint16_t fold_complement_checksum(uint64_t initial) {
    uint32_t ac32;
    bool c1 = __builtin_add_overflow(
        static_cast<uint32_t>(initial >> 32),
        static_cast<uint32_t>(initial & 0xffffffff),
        &ac32);
    ac32 += c1;

    uint16_t ac16;
    bool c2 = __builtin_add_overflow(static_cast<uint16_t>(ac32 >> 16), static_cast<uint16_t>(ac32 & 0xffff), &ac16);
    ac16 += c2;

    return ~ac16;
}

} // namespace fastcsum
