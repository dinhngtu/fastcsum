#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FASTCSUM_DECLARE_FEATURE_HELPERS(feat) \
    bool fastcsum_built_with_##feat(); \
    bool fastcsum_cpu_has_##feat(); \
    static inline bool fastcsum_##feat##_usable() { \
        return fastcsum_built_with_##feat() && fastcsum_cpu_has_##feat(); \
    }

// Unrolled 32 bytes/loop add-with-carry implementation.
uint64_t fastcsum_nofold_generic64(const uint8_t *b, size_t size, uint64_t initial);

// Unrolled 32 bytes/loop qword-aligned add-with-carry implementation.
uint64_t fastcsum_nofold_generic64_align(const uint8_t *b, size_t size, uint64_t initial);

// Adds 4 bytes per loop in a 64-bit register.
uint64_t fastcsum_nofold_simple(const uint8_t *b, size_t size, uint64_t initial);

// Non-unrolled version of fastcsum_nofold_generic64.
uint64_t fastcsum_nofold_simple2(const uint8_t *b, size_t size, uint64_t initial);

// Dword-aligned simple version.
uint64_t fastcsum_nofold_simple_align(const uint8_t *b, size_t size, uint64_t initial);

// 64 bytes/loop assembly implementation.
uint64_t fastcsum_nofold_x64_128b(const uint8_t *ptr, size_t size, uint64_t initial);

// 32 bytes/loop assembly implementation.
uint64_t fastcsum_nofold_x64_64b(const uint8_t *ptr, size_t size, uint64_t initial);

FASTCSUM_DECLARE_FEATURE_HELPERS(adx);

// Dual-carry ADX-based implementation.
__attribute__((deprecated)) uint64_t fastcsum_nofold_adx(const uint8_t *ptr, size_t size, uint64_t initial);

// Dual-carry ADX-based implementation.
uint64_t fastcsum_nofold_adx_v2(const uint8_t *ptr, size_t size, uint64_t initial);

// Dual-carry ADX-based implementation with load alignment.
__attribute__((deprecated)) uint64_t fastcsum_nofold_adx_align(const uint8_t *ptr, size_t size, uint64_t initial);

// Dual-carry ADX-based implementation with load alignment.
__attribute__((deprecated)) uint64_t fastcsum_nofold_adx_align2(const uint8_t *ptr, size_t size, uint64_t initial);

FASTCSUM_DECLARE_FEATURE_HELPERS(avx2);
FASTCSUM_DECLARE_FEATURE_HELPERS(avx);
FASTCSUM_DECLARE_FEATURE_HELPERS(sse41);

// 128 bytes/loop intrinsic-based AVX2 implementation.
__attribute__((deprecated)) uint64_t fastcsum_nofold_avx2(const uint8_t *ptr, size_t size, uint64_t initial);

// 128 bytes/loop intrinsic-based AVX2 implementation with load alignment.
__attribute__((deprecated)) uint64_t fastcsum_nofold_avx2_align(const uint8_t *ptr, size_t size, uint64_t initial);

// 128 bytes/loop intrinsic-based AVX2 implementation with parallel addition.
__attribute__((deprecated)) uint64_t fastcsum_nofold_avx2_v2(const uint8_t *ptr, size_t size, uint64_t initial);

// 256 bytes/loop intrinsic-based AVX2 implementation with parallel addition.
__attribute__((deprecated)) uint64_t fastcsum_nofold_avx2_256b(const uint8_t *ptr, size_t size, uint64_t initial);

// Plain assembly version converted from `avx2_256b` compiled with Clang.
__attribute__((deprecated)) uint64_t fastcsum_nofold_avx2_v3(const uint8_t *ptr, size_t size, uint64_t initial);

// Same as `fastcsum_nofold_avx2`.
__attribute__((deprecated)) uint64_t fastcsum_nofold_avx2_v4(const uint8_t *ptr, size_t size, uint64_t initial);

// Pure assembly version of `avx2_256b`.
__attribute__((deprecated)) uint64_t fastcsum_nofold_avx2_v5(const uint8_t *ptr, size_t size, uint64_t initial);

// `avx2_v5` with load optimization.
__attribute__((deprecated)) uint64_t fastcsum_nofold_avx2_v6(const uint8_t *ptr, size_t size, uint64_t initial);

// 256 bytes/loop plain assembly version with parallel addition and load alignment.
uint64_t fastcsum_nofold_avx2_v7(const uint8_t *ptr, size_t size, uint64_t initial);

bool fastcsum_vector_usable();

// Same as `simple` but with -O3 auto vectorization.
uint64_t fastcsum_nofold_simple_opt(const uint8_t *b, size_t size, uint64_t initial);

// 256 bytes/loop 32-byte vector-based version with parallel addition.
uint64_t fastcsum_nofold_vec256(const uint8_t *ptr, size_t size, uint64_t initial);

// 256 bytes/loop 32-byte vector-based version with parallel addition and load alignment.
uint64_t fastcsum_nofold_vec256_align(const uint8_t *ptr, size_t size, uint64_t initial);

// 128 bytes/loop 16-byte vector-based version with parallel addition.
uint64_t fastcsum_nofold_vec128(const uint8_t *ptr, size_t size, uint64_t initial);

// 128 bytes/loop 16-byte vector-based version with parallel addition and load alignment.
uint64_t fastcsum_nofold_vec128_align(const uint8_t *ptr, size_t size, uint64_t initial);

/*
 * Returns folded, complemented checksum in native byte order.
 * Note that initial, partial and final checksum values must all be loaded and stored in **native** order.
 * See explanation below.
 */
__attribute__((always_inline)) static inline uint16_t fastcsum_fold_complement(uint64_t initial) {
    uint32_t ac32;
    bool c1 = __builtin_add_overflow((uint32_t)(initial >> 32), (uint32_t)(initial & 0xffffffff), &ac32);
    ac32 += c1;

    uint16_t ac16;
    bool c2 = __builtin_add_overflow((uint16_t)(ac32 >> 16), (uint16_t)(ac32 & 0xffff), &ac16);
    ac16 += c2;

    return ~ac16;
}

/*
 * The reason why checksums must be loaded/stored in native order is that fastcsum_nofold calculates the 1's complement
 * sum using native byte order.
 * With this ordering, the resulting checksum has a flipped arithmetic ``order'', but the
 * resulting byte representations are the same.
 *
 * For example, consider the 4-byte input: 00 12 34 00
 * BE checksum = 0x0012 + 0x3400 = 0x3412
 * LE checksum = 0x1200 + 0x0034 = 0x1234
 * However, byte_repr_BE(0x3412) == 34 12 == byte_repr_LE(0x1234).
 */

#ifdef __cplusplus
}
#endif
