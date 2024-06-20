#pragma once

#include <cstdint>
#include <cstddef>

#define fastcsum_feature_helper(feat) \
    static inline bool fastcsum_##feat##_usable() { \
        return fastcsum_built_with_##feat() && fastcsum_cpu_has_##feat(); \
    }

namespace fastcsum {

uint64_t fastcsum_nofold_generic64(const uint8_t *b, size_t size, uint64_t initial);
extern "C" uint64_t fastcsum_nofold_x64_128b(const uint8_t *ptr, size_t size, uint64_t initial);
extern "C" uint64_t fastcsum_nofold_x64_64b(const uint8_t *ptr, size_t size, uint64_t initial);

bool fastcsum_built_with_adx();
bool fastcsum_cpu_has_adx();
static inline bool fastcsum_adx_usable() {
    return fastcsum_built_with_adx() && fastcsum_cpu_has_adx();
}

extern "C" uint64_t fastcsum_nofold_adx(const uint8_t *ptr, size_t size, uint64_t initial);
extern "C" uint64_t fastcsum_nofold_adx_v2(const uint8_t *ptr, size_t size, uint64_t initial);
extern "C" uint64_t fastcsum_nofold_adx_align(const uint8_t *ptr, size_t size, uint64_t initial);
extern "C" uint64_t fastcsum_nofold_adx_align2(const uint8_t *ptr, size_t size, uint64_t initial);

bool fastcsum_built_with_avx2();
bool fastcsum_cpu_has_avx2();
fastcsum_feature_helper(avx2);

bool fastcsum_built_with_avx();
bool fastcsum_cpu_has_avx();
fastcsum_feature_helper(avx);

bool fastcsum_built_with_sse41();
bool fastcsum_cpu_has_sse41();
fastcsum_feature_helper(sse41);

uint64_t fastcsum_nofold_avx2(const uint8_t *ptr, size_t size, uint64_t initial);
uint64_t fastcsum_nofold_avx2_align(const uint8_t *ptr, size_t size, uint64_t initial);
uint64_t fastcsum_nofold_avx2_v2(const uint8_t *ptr, size_t size, uint64_t initial);
uint64_t fastcsum_nofold_avx2_256b(const uint8_t *ptr, size_t size, uint64_t initial);
// `avx2_v3` is a plain assembly version converted from `avx2_256b` compiled with Clang.
extern "C" uint64_t fastcsum_nofold_avx2_v3(const uint8_t *ptr, size_t size, uint64_t initial);
uint64_t fastcsum_nofold_avx2_v4(const uint8_t *ptr, size_t size, uint64_t initial);
extern "C" uint64_t fastcsum_nofold_avx2_v5(const uint8_t *ptr, size_t size, uint64_t initial);
extern "C" uint64_t fastcsum_nofold_avx2_v6(const uint8_t *ptr, size_t size, uint64_t initial);

uint64_t fastcsum_nofold_vec256(const uint8_t *ptr, size_t size, uint64_t initial);

uint64_t fastcsum_nofold_vec128(const uint8_t *ptr, size_t size, uint64_t initial);

// returns folded, complemented checksum in native byte order
[[gnu::always_inline]] static inline uint16_t fold_complement_checksum64(uint64_t initial) {
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
