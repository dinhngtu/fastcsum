#include "fastcsum.h"
#include "addc.hpp"

extern "C" uint64_t fastcsum_nofold_simple(const uint8_t *b, size_t size, uint64_t initial) {
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

extern "C" uint64_t fastcsum_nofold_simple2(const uint8_t *b, size_t size, uint64_t initial) {
    uint64_t ac = initial;

    while (size >= 8) {
        auto val = *reinterpret_cast<const uint64_t *>(&b[0]);
        ac += val;
        if (ac < val)
            ac++;
        b += 8;
        size -= 8;
    }
    if (size >= 4) {
        auto val = *reinterpret_cast<const uint32_t *>(&b[0]);
        ac += val;
        if (ac < val)
            ac++;
        b += 4;
        size -= 4;
    }
    if (size >= 2) {
        auto val = *reinterpret_cast<const uint16_t *>(&b[0]);
        ac += val;
        if (ac < val)
            ac++;
        b += 2;
        size -= 2;
    }
    if (size) {
        uint64_t val = b[0];
        if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
            val <<= 8;
        ac += val;
        if (ac < val)
            ac++;
    }

    return ac;
}
