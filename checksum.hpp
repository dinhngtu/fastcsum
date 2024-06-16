#pragma once

#include <cstdint>
#include <cstddef>

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

uint64_t checksum_nofold_generic(const uint8_t *b, size_t size, uint64_t initial);
static inline uint16_t checksum_generic(const uint8_t *b, size_t size, uint64_t initial) {
    auto ac = checksum_nofold_generic(b, size, initial);
    return fold_complement_checksum(ac);
}

extern "C" uint64_t checksum_raw_nofold_x64_128b(const uint8_t *ptr, size_t size, uint64_t initial);
static inline uint16_t checksum_x64_128b(const uint8_t *b, size_t size, uint64_t initial) {
    auto ac = checksum_raw_nofold_x64_128b(b, size, initial);
    return fold_complement_checksum(ac);
}

extern "C" uint64_t checksum_raw_nofold_x64_64b(const uint8_t *ptr, size_t size, uint64_t initial);
static inline uint16_t checksum_x64_64b(const uint8_t *b, size_t size, uint64_t initial) {
    auto ac = checksum_raw_nofold_x64_64b(b, size, initial);
    return fold_complement_checksum(ac);
}

extern "C" uint64_t checksum_raw_nofold_adx(const uint8_t *ptr, size_t size, uint64_t initial);
static inline uint16_t checksum_adx(const uint8_t *b, size_t size, uint64_t initial) {
    auto ac = checksum_raw_nofold_adx(b, size, initial);
    return fold_complement_checksum(ac);
}

extern "C" uint64_t checksum_raw_nofold_adx_v2(const uint8_t *ptr, size_t size, uint64_t initial);
static inline uint16_t checksum_adx_v2(const uint8_t *b, size_t size, uint64_t initial) {
    auto ac = checksum_raw_nofold_adx_v2(b, size, initial);
    return fold_complement_checksum(ac);
}

extern "C" uint64_t checksum_raw_nofold_adx_align(const uint8_t *ptr, size_t size, uint64_t initial);
static inline uint16_t checksum_adx_align(const uint8_t *b, size_t size, uint64_t initial) {
    auto ac = checksum_raw_nofold_adx_align(b, size, initial);
    return fold_complement_checksum(ac);
}

extern "C" uint64_t checksum_raw_nofold_adx_align2(const uint8_t *ptr, size_t size, uint64_t initial);
static inline uint16_t checksum_adx_align2(const uint8_t *b, size_t size, uint64_t initial) {
    auto ac = checksum_raw_nofold_adx_align2(b, size, initial);
    return fold_complement_checksum(ac);
}

#if !__clang__
uint64_t checksum_nofold_avx2(const uint8_t *ptr, size_t size, uint64_t initial);
static inline uint16_t checksum_avx2(const uint8_t *b, size_t size, uint64_t initial) {
    auto ac = checksum_nofold_avx2(b, size, initial);
    return fold_complement_checksum(ac);
}
#endif
