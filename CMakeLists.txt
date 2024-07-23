cmake_minimum_required(VERSION 3.13)

project(fastcsum C CXX ASM)

set(CMAKE_CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

include(CheckCXXSourceCompiles)
check_cxx_source_compiles("
#include <cstdint>
#include <climits>
int main() {
    uint64_t a = UINT64_MAX, b = 1, s;
    uint64_t c = __builtin_add_overflow(a, b, &c);
    return 0;
}
    " HAVE_BUILTIN_ADD_OVERFLOW)
if(NOT HAVE_BUILTIN_ADD_OVERFLOW)
    message(FATAL_ERROR "compiler does not support __builtin_add_overflow")
endif()

add_library(fastcsum STATIC)

target_include_directories(fastcsum
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}
)

target_sources(fastcsum
    PUBLIC
        include/fastcsum.h
    PRIVATE
        addc.hpp
        checksum-generic64.cpp
        checksum-simple.cpp
        checksum-simple-opt.cpp
        cpuid.cpp
        checksum-vec256.cpp
        checksum-vec128.cpp
)
target_compile_options(fastcsum
    PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Werror=shadow -Werror=return-type -fno-strict-aliasing>
)
set_source_files_properties(checksum-simple-opt.cpp PROPERTIES COMPILE_OPTIONS "-O3")

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")

option(ENABLE_AVX2 "enable AVX2")
option(ENABLE_AVX "enable AVX")
option(ENABLE_SSE41 "enable SSE 4.1")

set(ARCH "" CACHE STRING "-march")
if (NOT (ARCH STREQUAL ""))
    target_compile_options(fastcsum PRIVATE "-march=${ARCH}")
endif()
set(TUNE "" CACHE STRING "-mtune")
if (NOT (TUNE STREQUAL ""))
    target_compile_options(fastcsum PRIVATE "-mtune=${TUNE}")
endif()

target_sources(fastcsum
    PRIVATE
        x86/asm/checksum-x64-128b.s
        x86/asm/checksum-x64-64b.s
        x86/asm/checksum-adx.s
        x86/asm/checksum-adx-v2.s
        x86/asm/checksum-adx-align.s
        x86/asm/checksum-adx-align2.s
        x86/checksum-avx2.cpp
)

if (ENABLE_AVX2)
    target_sources(fastcsum
        PRIVATE
            x86/asm/checksum-avx2-v3.s
            x86/asm/checksum-avx2-v5.s
            x86/asm/checksum-avx2-v6.s
            x86/asm/checksum-avx2-v7.s
    )
    target_compile_definitions(fastcsum PRIVATE FASTCSUM_ENABLE_AVX2)
    set_source_files_properties(
        x86/checksum-avx2.cpp
        checksum-vec256.cpp
        checksum-vec128.cpp
        checksum-simple-opt.cpp
        PROPERTIES COMPILE_OPTIONS "-mavx2"
    )
endif()
if (ENABLE_AVX)
    target_compile_definitions(fastcsum PRIVATE FASTCSUM_ENABLE_AVX)
    set_source_files_properties(
        checksum-vec256.cpp
        checksum-vec128.cpp
        checksum-simple-opt.cpp
        PROPERTIES COMPILE_OPTIONS "-mavx"
    )
endif()
if (ENABLE_SSE41)
    target_compile_definitions(fastcsum PRIVATE FASTCSUM_ENABLE_SSE41)
    set_source_files_properties(
        checksum-vec256.cpp
        checksum-vec128.cpp
        checksum-simple-opt.cpp
        PROPERTIES COMPILE_OPTIONS "-msse4.1"
    )
endif()

endif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")

add_executable(fastcsum-version fastcsum-version.cpp)
target_link_libraries(fastcsum-version PRIVATE fastcsum)

find_package(Catch2 3 REQUIRED)
add_executable(test-fastcsum test-fastcsum.cpp)
target_compile_options(test-fastcsum PRIVATE "-Wno-deprecated-declarations")
target_link_libraries(test-fastcsum PRIVATE fastcsum Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(test-fastcsum)
