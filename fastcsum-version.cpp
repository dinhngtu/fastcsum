#include <cstdio>
#include "fastcsum.hpp"

using namespace fastcsum;

int main() {
    printf("built with adx   : %d\n", fastcsum_built_with_adx());
    printf("cpu has adx      : %d\n", fastcsum_cpu_has_adx());

    printf("built with avx2  : %d\n", fastcsum_built_with_avx2());
    printf("cpu has avx2:    : %d\n", fastcsum_cpu_has_avx2());

    printf("built with avx   : %d\n", fastcsum_built_with_avx());
    printf("cpu has avx      : %d\n", fastcsum_cpu_has_avx());

    printf("built with sse41 : %d\n", fastcsum_built_with_sse41());
    printf("cpu has sse41    : %d\n", fastcsum_cpu_has_sse41());
    return 0;
}
