#include "fastcsum.hpp"
#include "addc.hpp"

using namespace fastcsum::impl;

namespace fastcsum {

namespace impl {

using u32x4 [[gnu::vector_size(16)]] = uint32_t;
using u32x4u [[gnu::vector_size(16), gnu::aligned(1), gnu::may_alias]] = uint32_t;
using u64x2 [[gnu::vector_size(16)]] = uint64_t;

[[gnu::always_inline]] static inline uint64_t addc_fold_vec4(u32x4 &v, uint64_t initial) {
    uint64_t ac = initial;
    uint64_t c;
    u64x2 d = (u64x2)v;
    ac = addc(ac, static_cast<uint64_t>(d[0]), 0, &c);
    ac = addc(ac, static_cast<uint64_t>(d[1]), c, &c);
    ac += c;
    return ac;
}

} // namespace impl

uint64_t fastcsum_nofold_vec128(const uint8_t *b, size_t size, uint64_t initial) {
    unsigned long long ac = initial;
    u32x4 vac{};

    while (size >= 128) {
        u32x4 v1, c1;
        v1 = (u32x4) * (u32x4u *)(b);
        addc_minus1_vec(v1, c1, v1, (u32x4) * (u32x4u *)(b + 16));

        u32x4 v2, c2;
        v2 = (u32x4) * (u32x4u *)(b + 32);
        addc_minus1_vec(v2, c2, v2, (u32x4) * (u32x4u *)(b + 48));

        u32x4 v3, c3;
        v3 = (u32x4) * (u32x4u *)(b + 64);
        addc_minus1_vec(v3, c3, v3, (u32x4) * (u32x4u *)(b + 80));

        u32x4 v4, c4;
        v4 = (u32x4) * (u32x4u *)(b + 96);
        addc_minus1_vec(v4, c4, v4, (u32x4) * (u32x4u *)(b + 112));

        u32x4 v5, c5;
        addc_minus1_vec(v5, c5, v1, v2);
        u32x4 v6, c6;
        addc_minus1_vec(v6, c6, v3, v4);

        u32x4 v7, c7;
        addc_minus1_vec(v7, c7, v5, v6);
        u32x4 c;
        addc_minus1_vec(vac, c, vac, v7);
        vac += c + c1 + c2 + c3 + c4 + c5 + c6 + c7;
        vac += 8;

        b += 128;
        size -= 128;
    }
    if (size >= 64) {
        u32x4 v1, c1;
        v1 = (u32x4) * (u32x4u *)(b);
        addc_minus1_vec(v1, c1, v1, (u32x4) * (u32x4u *)(b + 16));

        u32x4 v2, c2;
        v2 = (u32x4) * (u32x4u *)(b + 32);
        addc_minus1_vec(v2, c2, v2, (u32x4) * (u32x4u *)(b + 48));

        u32x4 v5, c5;
        addc_minus1_vec(v5, c5, v1, v2);

        u32x4 c;
        addc_minus1_vec(vac, c, vac, v5);
        vac += c + c1 + c2 + c5;
        vac += 4;

        b += 64;
        size -= 64;
    }
    if (size >= 32) {
        u32x4 v1, c1;
        v1 = (u32x4) * (u32x4u *)(b);
        addc_minus1_vec(v1, c1, v1, (u32x4) * (u32x4u *)(b + 16));

        u32x4 c;
        addc_minus1_vec(vac, c, vac, v1);
        vac += c + c1;
        vac += 2;

        b += 32;
        size -= 32;
    }

    ac = addc_fold_vec4(vac, ac);
    ac = csum_31bytes(b, size, ac);

    return ac;
}

uint64_t fastcsum_nofold_vec128_align(const uint8_t *b, size_t size, uint64_t initial) {
    unsigned long long ac = initial;
    u32x4 vac{};

    bool flip = false;
    if (size >= 16) {
        auto align = reinterpret_cast<uintptr_t>(b) & 15;
        if (align) {
            auto toadvance = 16 - align;
            flip = align & 1;
            ac = csum_31bytes(b, toadvance, ac);
            b += toadvance;
            size -= toadvance;
            if (flip)
                ac = __builtin_bswap64(ac);
        }
    }

    while (size >= 128) {
        u32x4 v1, c1;
        v1 = *(u32x4 *)(b);
        addc_minus1_vec(v1, c1, v1, *(u32x4 *)(b + 16));

        u32x4 v2, c2;
        v2 = *(u32x4 *)(b + 32);
        addc_minus1_vec(v2, c2, v2, *(u32x4 *)(b + 48));

        u32x4 v3, c3;
        v3 = *(u32x4 *)(b + 64);
        addc_minus1_vec(v3, c3, v3, *(u32x4 *)(b + 80));

        u32x4 v4, c4;
        v4 = *(u32x4 *)(b + 96);
        addc_minus1_vec(v4, c4, v4, *(u32x4 *)(b + 112));

        u32x4 v5, c5;
        addc_minus1_vec(v5, c5, v1, v2);
        u32x4 v6, c6;
        addc_minus1_vec(v6, c6, v3, v4);

        u32x4 v7, c7;
        addc_minus1_vec(v7, c7, v5, v6);
        u32x4 c;
        addc_minus1_vec(vac, c, vac, v7);
        vac += c + c1 + c2 + c3 + c4 + c5 + c6 + c7;
        vac += 8;

        b += 128;
        size -= 128;
    }
    if (size >= 64) {
        u32x4 v1, c1;
        v1 = *(u32x4 *)(b);
        addc_minus1_vec(v1, c1, v1, *(u32x4 *)(b + 16));

        u32x4 v2, c2;
        v2 = *(u32x4 *)(b + 32);
        addc_minus1_vec(v2, c2, v2, *(u32x4 *)(b + 48));

        u32x4 v5, c5;
        addc_minus1_vec(v5, c5, v1, v2);

        u32x4 c;
        addc_minus1_vec(vac, c, vac, v5);
        vac += c + c1 + c2 + c5;
        vac += 4;

        b += 64;
        size -= 64;
    }
    if (size >= 32) {
        u32x4 v1, c1;
        v1 = *(u32x4 *)(b);
        addc_minus1_vec(v1, c1, v1, *(u32x4 *)(b + 16));

        u32x4 c;
        addc_minus1_vec(vac, c, vac, v1);
        vac += c + c1;
        vac += 2;

        b += 32;
        size -= 32;
    }

    ac = addc_fold_vec4(vac, ac);
    ac = csum_31bytes(b, size, ac);
    if (flip)
        ac = __builtin_bswap64(ac);

    return ac;
}

} // namespace fastcsum
