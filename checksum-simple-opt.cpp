#include "fastcsum.h"
#include "addc.hpp"

extern "C" uint64_t fastcsum_nofold_simple_opt(const uint8_t *b, size_t size, uint64_t initial) {
    uint64_t ac = 0;

    while (size >= 4) {
        ac += *reinterpret_cast<const u32u *>(&b[0]);
        b += 4;
        size -= 4;
    }
    if (size >= 2) {
        ac += *reinterpret_cast<const u16u *>(&b[0]);
        b += 2;
        size -= 2;
    }
    if (size) {
        uint64_t lastbyte = b[0];
        if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
            lastbyte <<= 8;
        ac += lastbyte;
    }

    ac += initial;
    if (ac < initial)
        ac++;
    return ac;
}
