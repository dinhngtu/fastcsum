#include <cpuid.h>
#include "cpuid.hpp"

namespace fastcsum {
namespace impl {

bool fastcsum_has_adx() {
    unsigned int eax, ebx, ecx, edx;
    return __get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx) && (ebx & bit_ADX);
}

#if _fastcsum_has_avx2
bool fastcsum_has_avx2() {
    unsigned int eax, ebx, ecx, edx;
    return __get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx) && (ebx & bit_AVX2);
}
#else
bool fastcsum_has_avx2() {
    return false;
}
#endif

} // namespace impl
} // namespace fastcsum
