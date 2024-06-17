Fast Internet checksum calculation on x86-64 with Intel ADX
===========================================================

Up to 6x faster than unrolled C++ with the use of ADCX/ADOX instructions.

I recommend **adx-v2** for ADX-capable CPUs and **x64-64b** for most other
x86-64 CPUs.

The generic version emits add-with-carry instructions whenever possible
on clang 10+ and gcc 14+.
Requires a compiler with support for `__builtin_add_overflow`.

CPU feature detection is left as an exercise to the reader.

Original reference:
https://blogs.igalia.com/dpino/2018/06/14/fast-checksum-computation

Benchmarks (1000 iterations)
----------------------------

### g++ 11.4.0 Ubuntu 22.04 (Slow generic implementation)

On AMD Ryzen 7 5700X:

| Data size | Generic            | ADX               |
|-----------|--------------------|-------------------|
| 40        | 3.28 +/- 0.08 ns   | 3.24 +/- 0.02 ns  |
| 576       | 53.76 +/- 0.26 ns  | 9.85 +/- 0.09 ns  |
| 1500      | 146.40 +/- 0.50 ns | 24.39 +/- 0.10 ns |

On Intel Core i5-9400H:

| Data size | Generic            | ADX                |
|-----------|--------------------|--------------------|
| 40        | 23.00 +/- 0.28 ns  | 20.64 +/- 0.33 ns  |
| 576       | 338.85 +/- 3.11 ns | 71.21 +/- 0.92 ns  |
| 1500      | 892.86 +/- 8.39 ns | 171.36 +/- 4.65 ns |

### clang++ 14.0.0 Ubuntu 22.04 (Fast ADC-based generic implementation)

On AMD Ryzen 7 5700X:

| Data size | Generic            | ADX               |
|-----------|--------------------|-------------------|
| 40        | 2.16 +/- 0.02 ns   | 2.81 +/- 0.02 ns  |
| 576       | 14.90 +/- 0.08 ns  | 9.51 +/- 0.08 ns  |
| 1500      | 44.66 +/- 0.14 ns  | 24.60 +/- 0.10 ns |

On Intel Core i5-9400H:

| Data size | Generic            | ADX                |
|-----------|--------------------|--------------------|
| 40        | 17.61 +/- 0.28 ns  | 19.06 +/- 0.63 ns  |
| 576       | 106.92 +/- 3.86 ns | 67.32 +/- 0.68 ns  |
| 1500      | 279.05 +/- 2.90 ns | 169.60 +/- 1.69 ns |

License
-------

MIT.

This repository includes a copy of
[Catch2 v3.6.0](https://github.com/catchorg/Catch2/releases/tag/v3.6.0),
distributed under the Boost Software License, version 1.0.
