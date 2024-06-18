#pragma once

#if __AVX2__
#define _fastcsum_has_avx2 1
#else
#undef _fastcsum_has_avx2
#endif
