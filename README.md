Fast Internet checksum calculation on x86-64 with ADX and/or SIMD
=================================================================

I recommend **adx_v2** for ADX-capable CPUs and **x64_64b** for most other
x86-64 CPUs. Also try out **simple_opt**, **vec256_align** or **vec128**
depending on CPU support.

The generic version emits add-with-carry instructions whenever possible
on clang 10+, gcc 14+ and x86.
Requires a compiler with support for `__builtin_add_overflow`.

Original reference:
https://blogs.igalia.com/dpino/2018/06/14/fast-checksum-computation

License
-------

MIT.
