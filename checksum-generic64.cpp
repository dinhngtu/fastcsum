#include "fastcsum.h"
#include "addc.hpp"

extern "C" uint64_t fastcsum_nofold_generic64(const uint8_t *b, size_t size, uint64_t initial) {
    uint64_t ac = initial;
    uint64_t carry = 0;

    while (size >= 32) {
        ac = addc(ac, *reinterpret_cast<const u64u *>(&b[0]), 0, &carry);
        ac = addc(ac, *reinterpret_cast<const u64u *>(&b[8]), carry, &carry);
        ac = addc(ac, *reinterpret_cast<const u64u *>(&b[16]), carry, &carry);
        ac = addc(ac, *reinterpret_cast<const u64u *>(&b[24]), carry, &carry);
        ac += carry;
        b += 32;
        size -= 32;
    }
    if (size >= 16) {
        ac = addc(ac, *reinterpret_cast<const u64u *>(&b[0]), 0, &carry);
        ac = addc(ac, *reinterpret_cast<const u64u *>(&b[8]), carry, &carry);
        ac += carry;
        b += 16;
        size -= 16;
    }
    if (size >= 8) {
        ac = addc(ac, *reinterpret_cast<const u64u *>(&b[0]), 0, &carry);
        ac += carry;
        b += 8;
        size -= 8;
    }
    if (size >= 4) {
        ac = addc(ac, static_cast<uint64_t>(*reinterpret_cast<const u32u *>(&b[0])), 0, &carry);
        ac += carry;
        b += 4;
        size -= 4;
    }
    if (size >= 2) {
        ac = addc(ac, static_cast<uint64_t>(*reinterpret_cast<const u16u *>(&b[0])), 0, &carry);
        ac += carry;
        b += 2;
        size -= 2;
    }
    if (size) {
        uint64_t lastbyte = b[0];
        if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
            lastbyte <<= 8;
        ac = addc(ac, lastbyte, 0, &carry);
        ac += carry;
    }

    return ac;
}

extern "C" uint64_t fastcsum_nofold_generic64_align(const uint8_t *b, size_t size, uint64_t initial) {
    uint64_t ac = initial;
    uint64_t carry = 0;

    bool flip = false;
    if (size >= 8) {
        auto align = reinterpret_cast<uintptr_t>(b) & 7;
        if (align) {
            flip = align & 1;
            if (flip)
                ac = __builtin_bswap64(ac);
            auto tmp = *reinterpret_cast<const u64u *>(&b[0]);
            if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
                tmp >>= align * 8;
            else
                tmp <<= align * 8;
            ac = addc(ac, tmp, 0, &carry);
            ac += carry;
            b += 8 - align;
            size -= 8 - align;
        }
    }

    while (size >= 32) {
        ac = addc(ac, *reinterpret_cast<const uint64_t *>(&b[0]), 0, &carry);
        ac = addc(ac, *reinterpret_cast<const uint64_t *>(&b[8]), carry, &carry);
        ac = addc(ac, *reinterpret_cast<const uint64_t *>(&b[16]), carry, &carry);
        ac = addc(ac, *reinterpret_cast<const uint64_t *>(&b[24]), carry, &carry);
        ac += carry;
        b += 32;
        size -= 32;
    }
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
        uint64_t lastbyte = b[0];
        if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
            lastbyte <<= 8;
        ac = addc(ac, lastbyte, 0, &carry);
        ac += carry;
    }
    if (flip)
        ac = __builtin_bswap64(ac);

    return ac;
}
