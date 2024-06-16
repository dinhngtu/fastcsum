#include <cstdint>
#include <climits>
#include <vector>
#include <random>

#include "catch_amalgamated.hpp"

#include "checksum.hpp"

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
    auto csum = checksum(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum);
    auto csum_x64 = checksum<checksum_raw_nofold_x64>(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum_x64);
    auto csum_x64_64b = checksum<checksum_raw_nofold_x64_64b>(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum_x64_64b);
    auto csum_adx = checksum<checksum_raw_nofold_adx>(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum_adx);
}

// https://github.com/snabbco/snabb/commit/0068df61213d030ac6064f0d5db8705373e7e3c7
TEST_CASE("checksum-carry") {
    auto size = GENERATE(Catch::Generators::range(1, 64));
    auto pkt = create_packet_carry(size);
    auto ref = checksum_ref(reinterpret_cast<uint16_t *>(pkt.data()), pkt.size());
    auto csum = checksum(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum);
    auto csum_x64 = checksum<checksum_raw_nofold_x64>(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum_x64);
    auto csum_x64_64b = checksum<checksum_raw_nofold_x64_64b>(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum_x64_64b);
    auto csum_adx = checksum<checksum_raw_nofold_adx>(pkt.data(), pkt.size(), 0);
    REQUIRE(ref == csum_adx);
}

TEST_CASE("checksum-bench") {
    auto size = GENERATE(40, 576, 1500);
    auto pkt = create_packet(size);
    BENCHMARK("generic") {
        return checksum(pkt.data(), pkt.size(), 0);
    };
    BENCHMARK("x64_128b") {
        return checksum<checksum_raw_nofold_x64>(pkt.data(), pkt.size(), 0);
    };
    BENCHMARK("x64_64b") {
        return checksum<checksum_raw_nofold_x64_64b>(pkt.data(), pkt.size(), 0);
    };
    BENCHMARK("adx") {
        return checksum<checksum_raw_nofold_adx>(pkt.data(), pkt.size(), 0);
    };
}
