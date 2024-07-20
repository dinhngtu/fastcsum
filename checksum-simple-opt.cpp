#include "fastcsum.hpp"
#include "addc.hpp"

using namespace fastcsum::impl;

namespace fastcsum {

uint64_t fastcsum_nofold_simple_opt(const uint8_t *b, size_t size, uint64_t initial) {
    uint64_t ac = initial;

    while (size >= 4) {
        ac += *reinterpret_cast<const uint32_t *>(&b[0]);
        b += 4;
        size -= 4;
    }
    if (size >= 2) {
        ac += *reinterpret_cast<const uint16_t *>(&b[0]);
        b += 2;
        size -= 2;
    }
    if (size) {
        uint64_t lastbyte = b[0];
        if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
            lastbyte <<= 8;
        ac += lastbyte;
    }

    return ac;
}

} // namespace fastcsum
