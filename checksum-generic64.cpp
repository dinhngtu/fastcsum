#include "fastcsum.hpp"
#include "addc.hpp"

using namespace fastcsum::impl;

namespace fastcsum {

uint64_t fastcsum_nofold_generic64(const uint8_t *b, size_t size, uint64_t initial) {
    uint64_t ac = initial;
    uint64_t carry = 0;

    while (size >= 32) {
        uint64_t tmp;
        memcpy(&tmp, &b[0], sizeof(tmp));
        ac = addc(ac, tmp, 0, &carry);
        memcpy(&tmp, &b[8], sizeof(tmp));
        ac = addc(ac, tmp, carry, &carry);
        memcpy(&tmp, &b[16], sizeof(tmp));
        ac = addc(ac, tmp, carry, &carry);
        memcpy(&tmp, &b[24], sizeof(tmp));
        ac = addc(ac, tmp, carry, &carry);
        ac += carry;
        b += 32;
        size -= 32;
    }
    if (size >= 16) {
        uint64_t tmp;
        memcpy(&tmp, &b[0], sizeof(tmp));
        ac = addc(ac, tmp, 0, &carry);
        memcpy(&tmp, &b[8], sizeof(tmp));
        ac = addc(ac, tmp, carry, &carry);
        ac += carry;
        b += 16;
        size -= 16;
    }
    if (size >= 8) {
        uint64_t tmp;
        memcpy(&tmp, &b[0], sizeof(tmp));
        ac = addc(ac, tmp, 0, &carry);
        ac += carry;
        b += 8;
        size -= 8;
    }
    if (size >= 4) {
        uint32_t tmp;
        memcpy(&tmp, &b[0], sizeof(tmp));
        ac = addc(ac, static_cast<uint64_t>(tmp), 0, &carry);
        ac += carry;
        b += 4;
        size -= 4;
    }
    if (size >= 2) {
        uint16_t tmp;
        memcpy(&tmp, &b[0], sizeof(tmp));
        ac = addc(ac, static_cast<uint64_t>(tmp), 0, &carry);
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
