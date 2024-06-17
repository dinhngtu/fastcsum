#include <cpuid.h>

namespace fastcsum {
namespace impl {

bool fastcsum_has_adx() {
    unsigned int eax, ebx, ecx, edx;
    return __get_cpuid(7, &eax, &ebx, &ecx, &edx) && !!(ebx & bit_ADX);
}

} // namespace impl
} // namespace fastcsum
