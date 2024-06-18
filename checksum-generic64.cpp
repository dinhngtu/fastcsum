#include "fastcsum.hpp"
#include "addc.hpp"

using namespace fastcsum::impl;

namespace fastcsum {

uint64_t fastcsum_nofold_generic64(const uint8_t *b, size_t size, uint64_t initial) {
    uint64_t ac = initial;
    uint64_t carry = 0;

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
        ac = addc(ac, static_cast<uint64_t>(b[0]), 0, &carry);
        ac += carry;
    }

    return ac;
}

} // namespace fastcsum
