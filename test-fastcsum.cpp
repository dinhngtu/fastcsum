#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <climits>
#include <vector>
#include <memory>
#include <random>
#include <arpa/inet.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_get_random_seed.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

#include "fastcsum.h"

#define TEST_CSUM(ref, impl, b, size, initial) \
    do { \
        REQUIRE((ref) == fastcsum_fold_complement(impl((b), (size), (initial)))); \
    } while (0);

// https://stackoverflow.com/a/8845286/8642889
static uint16_t checksum_ref(const uint8_t *buffer, int size) {
    unsigned long cksum = 0;
    while (size > 1) {
        cksum += *(const uint16_t *)buffer;
        buffer += 2;
        size -= 2;
    }
    // https://www.rfc-editor.org/errata/eid3133
    if (size) {
        uint16_t last = 0;
        *(uint8_t *)&last = *(const uint8_t *)buffer;
        cksum += last;
    }

    while (cksum >> 16)
        cksum = (cksum & 0xffff) + (cksum >> 16);

    return (uint16_t)(~cksum);
}

static void fill_random(uint8_t *data, size_t size) {
    std::independent_bits_engine<std::default_random_engine, CHAR_BIT, uint8_t> rnd(Catch::getSeed());
    for (size_t i = 0; i < size; i++)
        data[i] = rnd();
}

static std::vector<uint8_t> create_packet(size_t size) {
    std::independent_bits_engine<std::default_random_engine, CHAR_BIT, uint8_t> rnd(Catch::getSeed());
    std::vector<uint8_t> vec(size);
    fill_random(vec.data(), vec.size());
    return vec;
}

static std::unique_ptr<uint8_t[]> create_packet(size_t align, size_t size) {
    std::independent_bits_engine<std::default_random_engine, CHAR_BIT, uint8_t> rnd(Catch::getSeed());
    auto mem = static_cast<uint8_t *>(aligned_alloc(align, size));
    if (!mem)
        throw std::bad_alloc();
    std::unique_ptr<uint8_t[]> vec(mem);
    fill_random(vec.get(), size);
    return vec;
}

static std::vector<uint8_t> create_packet_carry(size_t size) {
    std::vector<uint8_t> vec(size, 0xff);
    vec[size - 1] = 1;
    return vec;
}

void test_all(uint16_t ref, const uint8_t *buffer, size_t size, uint64_t initial) {
    TEST_CSUM(ref, fastcsum_nofold_generic64, buffer, size, initial);
    TEST_CSUM(ref, fastcsum_nofold_simple, buffer, size, initial);
    TEST_CSUM(ref, fastcsum_nofold_simple2, buffer, size, initial);
#if defined(__x86_64__)
    TEST_CSUM(ref, fastcsum_nofold_x64_128b, buffer, size, initial);
    TEST_CSUM(ref, fastcsum_nofold_x64_64b, buffer, size, initial);
    if (fastcsum_adx_usable()) {
        TEST_CSUM(ref, fastcsum_nofold_adx, buffer, size, initial);
        TEST_CSUM(ref, fastcsum_nofold_adx_v2, buffer, size, initial);
        TEST_CSUM(ref, fastcsum_nofold_adx_align, buffer, size, initial);
        TEST_CSUM(ref, fastcsum_nofold_adx_align2, buffer, size, initial);
    }
    if (fastcsum_avx2_usable()) {
        TEST_CSUM(ref, fastcsum_nofold_avx2, buffer, size, initial);
        TEST_CSUM(ref, fastcsum_nofold_avx2_align, buffer, size, initial);
        TEST_CSUM(ref, fastcsum_nofold_avx2_v2, buffer, size, initial);
        TEST_CSUM(ref, fastcsum_nofold_avx2_256b, buffer, size, initial);
        TEST_CSUM(ref, fastcsum_nofold_avx2_v3, buffer, size, initial);
        TEST_CSUM(ref, fastcsum_nofold_avx2_v4, buffer, size, initial);
        TEST_CSUM(ref, fastcsum_nofold_avx2_v5, buffer, size, initial);
        TEST_CSUM(ref, fastcsum_nofold_avx2_v6, buffer, size, initial);
        TEST_CSUM(ref, fastcsum_nofold_avx2_v7, buffer, size, initial);
    }
#endif
    if (fastcsum_vector_usable()) {
        TEST_CSUM(ref, fastcsum_nofold_simple_opt, buffer, size, initial);
        TEST_CSUM(ref, fastcsum_nofold_vec256, buffer, size, initial);
        TEST_CSUM(ref, fastcsum_nofold_vec256_align, buffer, size, initial);
        TEST_CSUM(ref, fastcsum_nofold_vec256_align_negc, buffer, size, initial);
        TEST_CSUM(ref, fastcsum_nofold_vec128, buffer, size, initial);
        TEST_CSUM(ref, fastcsum_nofold_vec128_align, buffer, size, initial);
    }
}

TEST_CASE("checksum") {
    auto size = GENERATE(Catch::Generators::range(1, 1501));
    auto pkt = create_packet(size);
    auto ref = checksum_ref(pkt.data(), pkt.size());
    test_all(ref, pkt.data(), pkt.size(), 0);
}

TEST_CASE("checksum-zero") {
    auto size = GENERATE(Catch::Generators::range(1, 1501));
    auto pkt = create_packet(size);
    memset(pkt.data(), 0, pkt.size());
    auto ref = checksum_ref(pkt.data(), pkt.size());
    test_all(ref, pkt.data(), pkt.size(), 0);
}

// https://github.com/snabbco/snabb/commit/0068df61213d030ac6064f0d5db8705373e7e3c7
TEST_CASE("checksum-carry") {
    auto size = GENERATE(Catch::Generators::range(1, 1025));
    auto pkt = create_packet_carry(size);
    auto ref = checksum_ref(pkt.data(), pkt.size());
    test_all(ref, pkt.data(), pkt.size(), 0);
}

TEST_CASE("checksum-align") {
    auto pkt = create_packet(1627);
    auto off = GENERATE(range(0, 128));
    auto size = GENERATE(range(1, 1627 - 127));
    auto ref = checksum_ref(&pkt[off], size);
    test_all(ref, &pkt[off], size, 0);
}

TEST_CASE("checksum-align32") {
    auto size = GENERATE(range(32, 64));
    auto off = 31;
    auto pkt = create_packet(32, size);
    auto ref = checksum_ref(pkt.get() + off, size - off);
    test_all(ref, pkt.get() + off, size - off, 0);
}

TEST_CASE("checksum-align16") {
    auto size = GENERATE(range(16, 32));
    auto off = 15;
    auto pkt = create_packet(16, size);
    auto ref = checksum_ref(pkt.get() + off, size - off);
    test_all(ref, pkt.get() + off, size - off, 0);
}

TEST_CASE("checksum-stride1") {
    auto stride = GENERATE(16, 32, 64, 128, 256, 512);
    auto size = stride * 2;
    auto pkt = create_packet(stride, stride * 2);
    memset(&pkt[0], 0, size);
    pkt[0] = 1;
    auto ref = checksum_ref(&pkt[0], stride);
    test_all(ref, &pkt[0], stride, 0);
}

TEST_CASE("checksum-rfc1071") {
    std::array<const uint8_t, 8> pkt{0x00, 0x01, 0xf2, 0x03, 0xf4, 0xf5, 0xf6, 0xf7};
    // fastcsum_fold_complement returns folded, complemented sum in native order
    // whereas 0xddf2 is the folded, non-complemented sum in network order
    uint16_t sum_rfc1071 = 0xddf2;
    uint16_t ref = ntohs(~sum_rfc1071);
    test_all(ref, pkt.data(), pkt.size(), 0);
}

TEST_CASE("bench", "[!benchmark]") {
    auto size = GENERATE(40, 128, 576, 1500, 2048, 4096, 8192, 16384, 32768, 65535);
    auto pkt = create_packet(size);
    BENCHMARK("generic") {
        return fastcsum_fold_complement(fastcsum_nofold_generic64(pkt.data(), pkt.size(), 0));
    };
    BENCHMARK("simple") {
        return fastcsum_fold_complement(fastcsum_nofold_simple(pkt.data(), pkt.size(), 0));
    };
#if defined(__x86_64__)
    BENCHMARK("x64_128b") {
        return fastcsum_fold_complement(fastcsum_nofold_x64_128b(pkt.data(), pkt.size(), 0));
    };
    BENCHMARK("x64_64b") {
        return fastcsum_fold_complement(fastcsum_nofold_x64_128b(pkt.data(), pkt.size(), 0));
    };
    if (fastcsum_adx_usable()) {
        BENCHMARK("adx_v2") {
            return fastcsum_fold_complement(fastcsum_nofold_adx_v2(pkt.data(), pkt.size(), 0));
        };
    }
    if (fastcsum_avx2_usable()) {
        BENCHMARK("avx2_v6") {
            return fastcsum_fold_complement(fastcsum_nofold_avx2_v6(pkt.data(), pkt.size(), 0));
        };
        BENCHMARK("avx2_v7") {
            return fastcsum_fold_complement(fastcsum_nofold_avx2_v7(pkt.data(), pkt.size(), 0));
        };
    }
#endif
    if (fastcsum_vector_usable()) {
        BENCHMARK("simple_opt") {
            return fastcsum_fold_complement(fastcsum_nofold_simple_opt(pkt.data(), pkt.size(), 0));
        };
        BENCHMARK("vec256") {
            return fastcsum_fold_complement(fastcsum_nofold_vec256(pkt.data(), pkt.size(), 0));
        };
        BENCHMARK("vec256_align") {
            return fastcsum_fold_complement(fastcsum_nofold_vec256_align(pkt.data(), pkt.size(), 0));
        };
        BENCHMARK("vec128") {
            return fastcsum_fold_complement(fastcsum_nofold_vec128(pkt.data(), pkt.size(), 0));
        };
        BENCHMARK("vec128_align") {
            return fastcsum_fold_complement(fastcsum_nofold_vec128_align(pkt.data(), pkt.size(), 0));
        };
        BENCHMARK("vec256_align_negc") {
            return fastcsum_fold_complement(fastcsum_nofold_vec256_align_negc(pkt.data(), pkt.size(), 0));
        };
    }
}

TEST_CASE("bench-large", "[!benchmark]") {
    auto size = GENERATE(1500, 2048, 4096, 8192, 16384, 32768, 65535);
    auto pkt = create_packet(size);
    BENCHMARK("generic") {
        return fastcsum_fold_complement(fastcsum_nofold_generic64(pkt.data(), pkt.size(), 0));
    };
    BENCHMARK("simple") {
        return fastcsum_fold_complement(fastcsum_nofold_simple(pkt.data(), pkt.size(), 0));
    };
#if defined(__x86_64__)
    BENCHMARK("x64_128b") {
        return fastcsum_fold_complement(fastcsum_nofold_x64_128b(pkt.data(), pkt.size(), 0));
    };
    if (fastcsum_adx_usable()) {
        BENCHMARK("adx_v2") {
            return fastcsum_fold_complement(fastcsum_nofold_adx_v2(pkt.data(), pkt.size(), 0));
        };
    }
    if (fastcsum_avx2_usable()) {
        BENCHMARK("avx2_v6") {
            return fastcsum_fold_complement(fastcsum_nofold_avx2_v6(pkt.data(), pkt.size(), 0));
        };
        BENCHMARK("avx2_v7") {
            return fastcsum_fold_complement(fastcsum_nofold_avx2_v7(pkt.data(), pkt.size(), 0));
        };
    }
#endif
    if (fastcsum_vector_usable()) {
        BENCHMARK("simple_opt") {
            return fastcsum_fold_complement(fastcsum_nofold_simple_opt(pkt.data(), pkt.size(), 0));
        };
        BENCHMARK("vec256") {
            return fastcsum_fold_complement(fastcsum_nofold_vec256(pkt.data(), pkt.size(), 0));
        };
        BENCHMARK("vec256_align") {
            return fastcsum_fold_complement(fastcsum_nofold_vec256_align(pkt.data(), pkt.size(), 0));
        };
        BENCHMARK("vec128") {
            return fastcsum_fold_complement(fastcsum_nofold_vec128(pkt.data(), pkt.size(), 0));
        };
        BENCHMARK("vec128_align") {
            return fastcsum_fold_complement(fastcsum_nofold_vec128_align(pkt.data(), pkt.size(), 0));
        };
        BENCHMARK("vec256_align_negc") {
            return fastcsum_fold_complement(fastcsum_nofold_vec256_align_negc(pkt.data(), pkt.size(), 0));
        };
    }
}

TEST_CASE("bench-unaligned", "[!benchmark]") {
    auto size = GENERATE(1500, 8192, 65535);
    auto align = GENERATE(16, 32);
    auto off = GENERATE(1, 8, 31);
    auto pkt = create_packet(align, size);
#if defined(__x86_64__)
    BENCHMARK("x64_128b") {
        return fastcsum_fold_complement(fastcsum_nofold_x64_128b(pkt.get() + off, size - off, 0));
    };
    if (fastcsum_adx_usable()) {
        BENCHMARK("adx_v2") {
            return fastcsum_fold_complement(fastcsum_nofold_adx_v2(pkt.get() + off, size - off, 0));
        };
    }
    if (fastcsum_avx2_usable()) {
        BENCHMARK("avx2_v6") {
            return fastcsum_fold_complement(fastcsum_nofold_avx2_v6(pkt.get() + off, size - off, 0));
        };
        BENCHMARK("avx2_v7") {
            return fastcsum_fold_complement(fastcsum_nofold_avx2_v7(pkt.get() + off, size - off, 0));
        };
    }
#endif
    if (fastcsum_vector_usable()) {
        BENCHMARK("simple_opt") {
            return fastcsum_fold_complement(fastcsum_nofold_simple_opt(pkt.get() + off, size - off, 0));
        };
        BENCHMARK("vec256") {
            return fastcsum_fold_complement(fastcsum_nofold_vec256(pkt.get() + off, size - off, 0));
        };
        BENCHMARK("vec256_align") {
            return fastcsum_fold_complement(fastcsum_nofold_vec256_align(pkt.get() + off, size - off, 0));
        };
        BENCHMARK("vec128") {
            return fastcsum_fold_complement(fastcsum_nofold_vec128(pkt.get() + off, size - off, 0));
        };
        BENCHMARK("vec128_align") {
            return fastcsum_fold_complement(fastcsum_nofold_vec128_align(pkt.get() + off, size - off, 0));
        };
        BENCHMARK("vec256_align_negc") {
            return fastcsum_fold_complement(fastcsum_nofold_vec256_align_negc(pkt.get() + off, size - off, 0));
        };
    }
}
