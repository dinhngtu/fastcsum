Fast Internet checksum calculation on x86-64 with ADX and/or SIMD
=================================================================

I recommend **adx-v2** for ADX-capable CPUs and **x64-64b** for most other
x86-64 CPUs. If processing big packets (>1500 bytes), try **vec256_align** or
**vec128** depending on CPU support.

The generic version emits add-with-carry instructions whenever possible
on clang 10+, gcc 14+ and x86.
Requires a compiler with support for `__builtin_add_overflow`.

Original reference:
https://blogs.igalia.com/dpino/2018/06/14/fast-checksum-computation

License
-------

MIT.

This repository includes a copy of
[Catch2 v3.6.0](https://github.com/catchorg/Catch2/releases/tag/v3.6.0),
distributed under the Boost Software License, version 1.0.
