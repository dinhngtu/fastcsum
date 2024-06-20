#include <cpuid.h>

namespace fastcsum {

bool fastcsum_built_with_adx() {
    return true;
}
bool fastcsum_cpu_has_adx() {
    unsigned int eax, ebx, ecx, edx;
    return __get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx) && (ebx & bit_ADX);
}

#if FASTCSUM_ENABLE_AVX2
bool fastcsum_built_with_avx2() {
    return true;
}
#else
bool fastcsum_built_with_avx2() {
    return false;
}
#endif

bool fastcsum_cpu_has_avx2() {
    unsigned int eax, ebx, ecx, edx;
    return __get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx) && (ebx & bit_AVX2);
}

#if FASTCSUM_ENABLE_AVX
bool fastcsum_built_with_avx() {
    return true;
}
#else
bool fastcsum_built_with_avx() {
    return false;
}
#endif

bool fastcsum_cpu_has_avx() {
    unsigned int eax, ebx, ecx, edx;
    return __get_cpuid(1, &eax, &ebx, &ecx, &edx) && (ecx & bit_AVX);
}

#if FASTCSUM_ENABLE_SSE41
bool fastcsum_built_with_sse41() {
    return true;
}
#else
bool fastcsum_built_with_sse41() {
    return false;
}
#endif

bool fastcsum_cpu_has_sse41() {
    unsigned int eax, ebx, ecx, edx;
    return __get_cpuid(1, &eax, &ebx, &ecx, &edx) && (ecx & bit_SSE4_1);
}

} // namespace fastcsum
