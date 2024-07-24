.intel_syntax noprefix

# no cobbling, except S=A
.macro xvpadcd s, c, a, b
    vpaddd \s, \a, \b
    vpmaxud \c, \s, \b
    vpcmpeqd \c, \s, \c
.endm

# no cobbling, except C=A; B can be memory operand
.macro xvpadcdm s, c, a, b
    vpaddd \s, \a, \b
    vpmaxud \c, \s, \a
    vpcmpeqd \c, \s, \c
.endm

.eight:
    .long 8
.four:
    .long 4
.two:
    .long 2

.global fastcsum_nofold_avx2_v7

fastcsum_nofold_avx2_v7:
    # rdi: byte ptr
    # rsi: size
    # rdx: initial

    # no stack!

    mov rax, rdx                    # accumulator
    xor ecx, ecx                    # ecx is align scratch/flip
    xor r9, r9                      # r9 is scratch
    vpxor ymm0, ymm0, ymm0          # ymm0 is vector accumulator

    vpbroadcastd ymm10, dword ptr [rip + .eight]

    # alignment
    cmp rsi, 64
    jb 32f

    mov ecx, edi
    and ecx, 31                     # ecx is now masked offset
    jz 256f
    mov edx, 32
    sub edx, ecx                    # edx is now number of bytes to advance
    and ecx, 1                      # ecx is now flip

    sub rsi, rdx

.Lalign16:
    cmp edx, 16                     # two qwords
    jb .Lalign8

    add rax, [rdi]
    adc rax, [rdi + 8]
    adc rax, 0

    sub edx, 16
    add rdi, 16

.Lalign8:
    cmp edx, 8                      # one qword
    jb .Lalign4

    add rax, [rdi]
    adc rax, 0

    sub edx, 8
    add rdi, 8

.Lalign4:
    cmp edx, 4                      # one dword
    jb .Lalign2

    mov r9d, dword ptr [rdi]
    add rax, r9
    adc rax, 0

    sub edx, 4
    add rdi, 4

.Lalign2:
    cmp edx, 2                      # one word
    jb .Lalign1

    movzx r9, word ptr [rdi]
    add rax, r9
    adc rax, 0

    sub edx, 2
    add rdi, 2

.Lalign1:
    cmp edx, 1                      # last byte
    jb 256f

    movzx r9, byte ptr [rdi]
    add rax, r9
    adc rax, 0

    add rdi, 1
    bswap rax

256:
    cmp rsi, 256
    jb 128f

    vmovdqa ymm2, ymmword ptr [rdi]
    vmovdqa ymm4, ymmword ptr [rdi + 64]
    vmovdqa ymm6, ymmword ptr [rdi + 128]
    vmovdqa ymm8, ymmword ptr [rdi + 192]

    xvpadcdm ymm1, ymm2, ymm2, (ymmword ptr [rdi + 32])
    xvpadcdm ymm3, ymm4, ymm4, (ymmword ptr [rdi + 96])
    xvpadcdm ymm5, ymm6, ymm6, (ymmword ptr [rdi + 160])
    xvpadcdm ymm7, ymm8, ymm8, (ymmword ptr [rdi + 224])

    # first carry reduction
    vpaddd ymm2, ymm2, ymm4
    vpaddd ymm6, ymm6, ymm8
    # remaining: ymm1/3/5/7 (sums), ymm2/6 (carries)

    # second reduction
    xvpadcd ymm1, ymm4, ymm1, ymm3
    xvpadcd ymm5, ymm8, ymm5, ymm7
    # ymm1/5 are now sum of all loaded values
    # remaining carries: ymm2/4/6/8

    # third reduction
    xvpadcd ymm1, ymm3, ymm1, ymm5
    # remaining: ymm1 (sum), ymm2/3/4/6/8 (carries)

    # second carry reduction
    vpaddd ymm2, ymm2, ymm4
    vpaddd ymm6, ymm6, ymm8

    # accumulate
    xvpadcd ymm0, ymm5, ymm0, ymm1
    vpaddd ymm2, ymm2, ymm6
    vpaddd ymm0, ymm0, ymm3
    vpaddd ymm0, ymm0, ymm5
    vpaddd ymm0, ymm0, ymm2

    # finalize
    vpaddd ymm0, ymm0, ymm10

    sub rsi, 256
    add rdi, 256
    jmp 256b

128:
    cmp rsi, 128
    jb 64f

    vpbroadcastd ymm11, dword ptr [rip + .four]

    vmovdqa ymm2, ymmword ptr [rdi]
    vmovdqa ymm4, ymmword ptr [rdi + 64]

    # first reduction
    xvpadcdm ymm1, ymm2, ymm2, (ymmword ptr [rdi + 32])
    xvpadcdm ymm3, ymm4, ymm4, (ymmword ptr [rdi + 96])

    # first carry reduction
    vpaddd ymm2, ymm2, ymm4
    # remaining: ymm1/3 (sums), ymm2 (carry)

    # second reduction
    xvpadcd ymm1, ymm4, ymm1, ymm3
    # remaining carries: ymm2/4

    # accumulate
    xvpadcd ymm0, ymm5, ymm0, ymm1
    vpaddd ymm0, ymm0, ymm2
    vpaddd ymm0, ymm0, ymm4
    vpaddd ymm0, ymm0, ymm5

    # finalize
    vpaddd ymm0, ymm0, ymm11

    sub rsi, 128
    add rdi, 128

64:
    cmp rsi, 64                     # eight qwords
    jb 32f

    vpbroadcastd ymm12, dword ptr [rip + .two]

    vmovdqa ymm2, ymmword ptr [rdi]
    # reduction
    xvpadcdm ymm1, ymm2, ymm2, (ymmword ptr [rdi + 32])
    xvpadcd ymm0, ymm5, ymm0, ymm1
    # carry reduction
    vpaddd ymm0, ymm0, ymm2
    vpaddd ymm0, ymm0, ymm5
    # finalize
    vpaddd ymm0, ymm0, ymm12

    sub rsi, 64
    add rdi, 64

32:
    cmp rsi, 32                     # four qwords
    jb 16f

    add rax, [rdi]
    adc rax, [rdi + 8]
    adc rax, [rdi + 16]
    adc rax, [rdi + 24]
    adc rax, 0

    sub rsi, 32
    add rdi, 32

16:
    cmp rsi, 16                     # two qwords
    jb 8f

    add rax, [rdi]
    adc rax, [rdi + 8]
    adc rax, 0

    sub rsi, 16
    add rdi, 16

8:
    cmp rsi, 8                      # one qword
    jb 4f

    add rax, [rdi]
    adc rax, 0

    sub rsi, 8
    add rdi, 8

4:
    cmp rsi, 4                      # one dword
    jb 2f

    mov r9d, dword ptr [rdi]
    add rax, r9
    adc rax, 0

    sub rsi, 4
    add rdi, 4

2:
    cmp rsi, 2                      # one word
    jb 1f

    movzx r9, word ptr [rdi]
    add rax, r9
    adc rax, 0

    sub rsi, 2
    add rdi, 2

1:
    cmp rsi, 1                      # last byte
    jb 0f

    movzx r9, byte ptr [rdi]
    add rax, r9
    adc rax, 0

0:
    vmovq rdx, xmm0
    add rax, rdx
    vpextrq rdx, xmm0, 1
    adc rax, rdx
    vextracti128 xmm0, ymm0, 1
    vmovq rdx, xmm0
    adc rax, rdx
    vpextrq rdx, xmm0, 1
    adc rax, rdx
    adc rax, 0

    test ecx, 1
    jz .Ldone
    bswap rax

.Ldone:
    vzeroupper
    ret
