#include <cstdint>
#include <climits>
#include <vector>
#include <random>
#include <arpa/inet.h>

#include "catch_amalgamated.hpp"

#include "fastcsum.hpp"

using namespace fastcsum;

#define TEST_CSUM(ref, impl, b, size, initial) \
    do { \
        auto ac = impl((b), (size), (initial)); \
        REQUIRE((ref) == fold_complement_checksum(ac)); \
    } while (0);

// https://stackoverflow.com/a/8845286/8642889
static uint16_t checksum_ref(const uint8_t *buffer, int size) {
    unsigned long cksum = 0;
    while (size > 1) {
        cksum += *(const uint16_t *)buffer;
        buffer += 2;
        size -= 2;
    }
    if (size)
        cksum += *(const uint8_t *)buffer;

    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);
    return (uint16_t)(~cksum);
}

static std::vector<uint8_t> create_packet(size_t size) {
    std::independent_bits_engine<std::default_random_engine, CHAR_BIT, uint8_t> rnd(Catch::getSeed());
    std::vector<uint8_t> vec(size);
    for (auto &b : vec)
        b = rnd();
    return vec;
}

static std::vector<uint8_t> create_packet_carry(size_t size) {
    std::vector<uint8_t> vec(size, 0xff);
    vec[size - 1] = 1;
    return vec;
}

TEST_CASE("checksum") {
    auto size = GENERATE(Catch::Generators::range(1, 1501));
    auto pkt = create_packet(size);
    auto ref = checksum_ref(pkt.data(), pkt.size());
    TEST_CSUM(ref, impl::fastcsum_nofold_generic64, pkt.data(), pkt.size(), 0);
    TEST_CSUM(ref, impl::fastcsum_nofold_x64_128b, pkt.data(), pkt.size(), 0);
    TEST_CSUM(ref, impl::fastcsum_nofold_x64_64b, pkt.data(), pkt.size(), 0);
    if (impl::fastcsum_has_adx()) {
        TEST_CSUM(ref, impl::fastcsum_nofold_adx, pkt.data(), pkt.size(), 0);
        TEST_CSUM(ref, impl::fastcsum_nofold_adx_v2, pkt.data(), pkt.size(), 0);
        TEST_CSUM(ref, impl::fastcsum_nofold_adx_align, pkt.data(), pkt.size(), 0);
        TEST_CSUM(ref, impl::fastcsum_nofold_adx_align2, pkt.data(), pkt.size(), 0);
    }
    if (impl::fastcsum_has_avx2())
        TEST_CSUM(ref, impl::fastcsum_nofold_avx2, pkt.data(), pkt.size(), 0);
}

// https://github.com/snabbco/snabb/commit/0068df61213d030ac6064f0d5db8705373e7e3c7
TEST_CASE("checksum-carry") {
    auto size = GENERATE(Catch::Generators::range(1, 64));
    auto pkt = create_packet_carry(size);
    auto ref = checksum_ref(pkt.data(), pkt.size());
    TEST_CSUM(ref, impl::fastcsum_nofold_generic64, pkt.data(), pkt.size(), 0);
    TEST_CSUM(ref, impl::fastcsum_nofold_x64_128b, pkt.data(), pkt.size(), 0);
    TEST_CSUM(ref, impl::fastcsum_nofold_x64_64b, pkt.data(), pkt.size(), 0);
    if (impl::fastcsum_has_adx()) {
        TEST_CSUM(ref, impl::fastcsum_nofold_adx, pkt.data(), pkt.size(), 0);
        TEST_CSUM(ref, impl::fastcsum_nofold_adx_v2, pkt.data(), pkt.size(), 0);
        TEST_CSUM(ref, impl::fastcsum_nofold_adx_align, pkt.data(), pkt.size(), 0);
        TEST_CSUM(ref, impl::fastcsum_nofold_adx_align2, pkt.data(), pkt.size(), 0);
    }
    if (impl::fastcsum_has_avx2())
        TEST_CSUM(ref, impl::fastcsum_nofold_avx2, pkt.data(), pkt.size(), 0);
}

TEST_CASE("checksum-align") {
    auto pkt = create_packet(1627);
    auto off = GENERATE(range(0, 128));
    auto size = GENERATE(range(1, 1627 - 127));
    auto ref = checksum_ref(&pkt[off], size);
    TEST_CSUM(ref, impl::fastcsum_nofold_generic64, &pkt[off], size, 0);
    TEST_CSUM(ref, impl::fastcsum_nofold_x64_128b, &pkt[off], size, 0);
    TEST_CSUM(ref, impl::fastcsum_nofold_x64_64b, &pkt[off], size, 0);
    if (impl::fastcsum_has_adx()) {
        TEST_CSUM(ref, impl::fastcsum_nofold_adx, &pkt[off], size, 0);
        TEST_CSUM(ref, impl::fastcsum_nofold_adx_v2, &pkt[off], size, 0);
        TEST_CSUM(ref, impl::fastcsum_nofold_adx_align, &pkt[off], size, 0);
        TEST_CSUM(ref, impl::fastcsum_nofold_adx_align2, &pkt[off], size, 0);
    }
    if (impl::fastcsum_has_avx2())
        TEST_CSUM(ref, impl::fastcsum_nofold_avx2, &pkt[off], size, 0);
}

TEST_CASE("checksum-rfc1071") {
    std::array<const uint8_t, 8> pkt{0x00, 0x01, 0xf2, 0x03, 0xf4, 0xf5, 0xf6, 0xf7};
    // fold_complement_checksum returns folded, complemented sum in native order
    // whereas 0xddf2 is the folded, non-complemented sum in network order
    uint16_t sum_rfc1071 = 0xddf2;
    uint16_t ref = ntohs(~sum_rfc1071);
    TEST_CSUM(ref, impl::fastcsum_nofold_generic64, pkt.data(), pkt.size(), 0);
    TEST_CSUM(ref, impl::fastcsum_nofold_x64_128b, pkt.data(), pkt.size(), 0);
    TEST_CSUM(ref, impl::fastcsum_nofold_x64_64b, pkt.data(), pkt.size(), 0);
    if (impl::fastcsum_has_adx()) {
        TEST_CSUM(ref, impl::fastcsum_nofold_adx, pkt.data(), pkt.size(), 0);
        TEST_CSUM(ref, impl::fastcsum_nofold_adx_v2, pkt.data(), pkt.size(), 0);
        TEST_CSUM(ref, impl::fastcsum_nofold_adx_align, pkt.data(), pkt.size(), 0);
        TEST_CSUM(ref, impl::fastcsum_nofold_adx_align2, pkt.data(), pkt.size(), 0);
    }
    if (impl::fastcsum_has_avx2())
        TEST_CSUM(ref, impl::fastcsum_nofold_avx2, pkt.data(), pkt.size(), 0);
}

TEST_CASE("checksum-bench") {
    auto size = GENERATE(40, 128, 576, 1500);
    auto pkt = create_packet(size);
    BENCHMARK("generic") {
        return fold_complement_checksum(impl::fastcsum_nofold_generic64(pkt.data(), pkt.size(), 0));
    };
    BENCHMARK("x64_128b") {
        return fold_complement_checksum(impl::fastcsum_nofold_x64_128b(pkt.data(), pkt.size(), 0));
    };
    BENCHMARK("x64_64b") {
        return fold_complement_checksum(impl::fastcsum_nofold_x64_128b(pkt.data(), pkt.size(), 0));
    };
    if (impl::fastcsum_has_adx()) {
        BENCHMARK("adx") {
            return fold_complement_checksum(impl::fastcsum_nofold_adx(pkt.data(), pkt.size(), 0));
        };
        BENCHMARK("adx_v2") {
            return fold_complement_checksum(impl::fastcsum_nofold_adx_v2(pkt.data(), pkt.size(), 0));
        };
        BENCHMARK("adx_align") {
            return fold_complement_checksum(impl::fastcsum_nofold_adx_align(pkt.data(), pkt.size(), 0));
        };
        BENCHMARK("adx_align2") {
            return fold_complement_checksum(impl::fastcsum_nofold_adx_align2(pkt.data(), pkt.size(), 0));
        };
    }
    if (impl::fastcsum_has_avx2()) {
        BENCHMARK("avx2") {
            return fold_complement_checksum(impl::fastcsum_nofold_avx2(pkt.data(), pkt.size(), 0));
        };
    }
}
