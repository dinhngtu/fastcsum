#pragma once

#include <cstdint>
#include <cstddef>

#include "cpuid.hpp"

#if _fastcsum_has_avx2

#include <immintrin.h>

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

#endif
