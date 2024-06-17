#include <cstdint>
#include <climits>
#include <vector>
#include <random>

#include "catch_amalgamated.hpp"

#include "fastcsum.hpp"

using namespace fastcsum;

static inline uint16_t checksum_generic(const uint8_t *b, size_t size, uint64_t initial) {
    auto ac = impl::fastcsum_nofold_generic(b, size, initial);
    return fold_complement_checksum(ac);
}

static inline uint16_t checksum_x64_128b(const uint8_t *b, size_t size, uint64_t initial) {
    auto ac = impl::fastcsum_nofold_x64_128b(b, size, initial);
    return fold_complement_checksum(ac);
}

static inline uint16_t checksum_x64_64b(const uint8_t *b, size_t size, uint64_t initial) {
    auto ac = impl::fastcsum_nofold_x64_64b(b, size, initial);
    return fold_complement_checksum(ac);
}

static inline uint16_t checksum_adx(const uint8_t *b, size_t size, uint64_t initial) {
    auto ac = impl::fastcsum_nofold_adx(b, size, initial);
    return fold_complement_checksum(ac);
}

static inline uint16_t checksum_adx_v2(const uint8_t *b, size_t size, uint64_t initial) {
    auto ac = impl::fastcsum_nofold_adx_v2(b, size, initial);
    return fold_complement_checksum(ac);
}

static inline uint16_t checksum_adx_align(const uint8_t *b, size_t size, uint64_t initial) {
    auto ac = impl::fastcsum_nofold_adx_align(b, size, initial);
    return fold_complement_checksum(ac);
}

static inline uint16_t checksum_adx_align2(const uint8_t *b, size_t size, uint64_t initial) {
    auto ac = impl::fastcsum_nofold_adx_align2(b, size, initial);
    return fold_complement_checksum(ac);
}

#if _fastcsum_has_avx2
static inline uint16_t checksum_avx2(const uint8_t *b, size_t size, uint64_t initial) {
    auto ac = fastcsum_nofold_avx2(b, size, initial);
    return fold_complement_checksum(ac);
}
#endif

// https://stackoverflow.com/a/8845286/8642889
static uint16_t checksum_ref(const uint16_t *buffer, int size) {
    unsigned long cksum = 0;
    while (size > 1) {
        cksum += *buffer++;
        size -= sizeof(uint16_t);
    }
    if (size)
        cksum += *(const uint8_t *)buffer;

    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);
    return (uint16_t)(~cksum);
}

static std::vector<uint8_t> create_packet(size_t size) {
    std::independent_bits_engine<std::default_random_engine, CHAR_BIT, uint8_t> rnd;
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
    auto ref = checksum_ref(reinterpret_cast<uint16_t *>(pkt.data()), pkt.size());
    auto csum_generic = checksum_generic(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum_generic);
    auto csum_x64_128b = checksum_x64_128b(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum_x64_128b);
    auto csum_x64_64b = checksum_x64_64b(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum_x64_64b);
    auto csum_adx = checksum_adx(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum_adx);
    auto csum_adx_v2 = checksum_adx_v2(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum_adx_v2);
    auto csum_adx_align = checksum_adx_align(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum_adx_align);
    auto csum_adx_align2 = checksum_adx_align2(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum_adx_align2);
#if _fastcsum_has_avx2
    auto csum_avx2 = checksum_avx2(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum_avx2);
#endif
}

// https://github.com/snabbco/snabb/commit/0068df61213d030ac6064f0d5db8705373e7e3c7
TEST_CASE("checksum-carry") {
    auto size = GENERATE(Catch::Generators::range(1, 64));
    auto pkt = create_packet_carry(size);
    auto ref = checksum_ref(reinterpret_cast<uint16_t *>(pkt.data()), pkt.size());
    auto csum_generic = checksum_generic(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum_generic);
    auto csum_x64_128b = checksum_x64_128b(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum_x64_128b);
    auto csum_x64_64b = checksum_x64_64b(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum_x64_64b);
    auto csum_adx = checksum_adx(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum_adx);
    auto csum_adx_v2 = checksum_adx_v2(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum_adx_v2);
    auto csum_adx_align = checksum_adx_align(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum_adx_align);
    auto csum_adx_align2 = checksum_adx_align2(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum_adx_align2);
#if _fastcsum_has_avx2
    auto csum_avx2 = checksum_avx2(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum_avx2);
#endif
}

TEST_CASE("checksum-align") {
    auto pkt = create_packet(1627);
    auto off = GENERATE(range(0, 128));
    auto size = GENERATE(range(1, 1627 - 127));
    auto ref = checksum_ref(reinterpret_cast<uint16_t *>(&pkt[off]), size);
    auto csum_generic = checksum_generic(&pkt[off], size, 0);
    REQUIRE(ref == csum_generic);
    auto csum_x64_128b = checksum_x64_128b(&pkt[off], size, 0);
    REQUIRE(ref == csum_x64_128b);
    auto csum_x64_64b = checksum_x64_64b(&pkt[off], size, 0);
    REQUIRE(ref == csum_x64_64b);
    auto csum_adx = checksum_adx(&pkt[off], size, 0);
    REQUIRE(ref == csum_adx);
    auto csum_adx_v2 = checksum_adx_v2(&pkt[off], size, 0);
    REQUIRE(ref == csum_adx_v2);
    auto csum_adx_align = checksum_adx_align(&pkt[off], size, 0);
    REQUIRE(ref == csum_adx_align);
    auto csum_adx_align2 = checksum_adx_align2(&pkt[off], size, 0);
    REQUIRE(ref == csum_adx_align2);
#if _fastcsum_has_avx2
    auto csum_avx2 = checksum_avx2(&pkt[off], size, 0);
    REQUIRE(ref == csum_avx2);
#endif
}

TEST_CASE("checksum-bench") {
    auto size = GENERATE(40, 128, 576, 1500);
    auto pkt = create_packet(size);
    BENCHMARK("generic") {
        return checksum_generic(pkt.data(), pkt.size(), 0);
    };
    BENCHMARK("x64_128b") {
        return checksum_x64_128b(pkt.data(), pkt.size(), 0);
    };
    BENCHMARK("x64_64b") {
        return checksum_x64_64b(pkt.data(), pkt.size(), 0);
    };
    BENCHMARK("adx") {
        return checksum_adx(pkt.data(), pkt.size(), 0);
    };
    BENCHMARK("adx_v2") {
        return checksum_adx_v2(pkt.data(), pkt.size(), 0);
    };
    BENCHMARK("adx_align") {
        return checksum_adx_align(pkt.data(), pkt.size(), 0);
    };
    BENCHMARK("adx_align2") {
        return checksum_adx_align2(pkt.data(), pkt.size(), 0);
    };
#if _fastcsum_has_avx2
    BENCHMARK("avx2") {
        return checksum_avx2(pkt.data(), pkt.size(), 0);
    };
#endif
}
