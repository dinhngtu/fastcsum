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

.data
.Lalignmask:
.balign 16
    # no swap
    .quad 0x0706050403020100
    .quad 0x0f0e0d0c0b0a0908
    .quad 0xffffffffffffffff
    .quad 0xffffffffffffffff
    # swap
    .quad 0x08090a0b0c0d0e0f
    .quad 0x0001020304050607
    .quad 0xffffffffffffffff
    .quad 0xffffffffffffffff

.text

.global fastcsum_nofold_avx2_v8

fastcsum_nofold_avx2_v8:
    # rdi: byte ptr
    # rsi: size
    # rdx: initial

    # no stack!

    mov rax, rdx                    # accumulator
    xor ecx, ecx                    # ecx is align scratch/flip
    vpxor ymm0, ymm0, ymm0          # ymm0 is vector accumulator

    # alignment
    cmp rsi, 64
    jb 32f

    mov ecx, edi
    and ecx, 31                     # ecx is now masked offset
    jz 256f
    mov edx, 32
    sub edx, ecx                    # edx is now number of bytes to advance

    sub rsi, rdx                    # reduce size by advance count ahead of time

.Lalign16:
    cmp edx, 16                     # two qwords
    jb .Lswap

    add rax, [rdi]
    adc rax, [rdi + 8]
    adc rax, 0

    add rdi, 16
    sub edx, 16
    jz 256f

.Lswap:
    # begin of swapped bytes section
    and ecx, 1                      # ecx is now flip
    jz .Lalign15
    bswap rax

.Lalign15:
    vpcmpeqd ymm3, ymm3, ymm3
    lea r8, [rip + .Lalignmask]
    mov r9d, ecx
    shl r9d, 5
    vmovdqa ymm1, ymmword ptr [r8 + r9]
    vmovd xmm2, edx
    vpbroadcastb ymm2, xmm2
    vpcmpgtb ymm2, ymm2, ymm1
    vpxor ymm2, ymm3, ymm2
    vpor ymm1, ymm1, ymm2           # trim shuffle mask in ymm1 to edx

    vmovdqu ymm0, ymmword ptr [rdi]
    vpshufb ymm0, ymm0, ymm1

    add rdi, rdx

.balign 16
256:
    cmp rsi, 256
    jb 128f

    vmovdqa ymm2, ymmword ptr [rdi]
    xvpadcdm ymm1, ymm2, ymm2, (ymmword ptr [rdi + 32])

    vmovdqa ymm4, ymmword ptr [rdi + 64]
    xvpadcdm ymm3, ymm4, ymm4, (ymmword ptr [rdi + 96])

    vmovdqa ymm6, ymmword ptr [rdi + 128]
    xvpadcdm ymm5, ymm6, ymm6, (ymmword ptr [rdi + 160])

    vmovdqa ymm8, ymmword ptr [rdi + 192]
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
    mov edx, 8
    vmovd xmm1, edx
    vpbroadcastd ymm1, xmm1
    vpaddd ymm0, ymm0, ymm1

    sub rsi, 256
    add rdi, 256
    jmp 256b

128:
    cmp rsi, 128
    jb 64f

    vmovdqa ymm1, ymmword ptr [rdi]
    vmovdqa ymm5, ymmword ptr [rdi + 32]
    vmovdqa ymm3, ymmword ptr [rdi + 64]
    vmovdqa ymm7, ymmword ptr [rdi + 96]

    # first reduction
    xvpadcd ymm1, ymm2, ymm1, ymm5
    xvpadcd ymm3, ymm4, ymm3, ymm7

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
    mov edx, 4
    vmovd xmm1, edx
    vpbroadcastd ymm1, xmm1
    vpaddd ymm0, ymm0, ymm1

    sub rsi, 128
    add rdi, 128

64:
    cmp rsi, 64                     # eight qwords
    jb 32f

    vmovdqa ymm1, ymmword ptr [rdi]
    vmovdqa ymm3, ymmword ptr [rdi + 32]
    # reduction
    xvpadcd ymm1, ymm2, ymm1, ymm3
    xvpadcd ymm0, ymm5, ymm0, ymm1
    # carry reduction
    vpaddd ymm0, ymm0, ymm2
    vpaddd ymm0, ymm0, ymm5
    # finalize
    vpcmpeqd ymm1, ymm1, ymm1
    vpsubd ymm0, ymm0, ymm1
    vpsubd ymm0, ymm0, ymm1

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
