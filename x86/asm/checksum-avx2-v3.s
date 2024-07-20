.intel_syntax noprefix

.LCPI0_0:
        .long   8                               # 0x8
.LCPI0_1:
        .long   4                               # 0x4

.global fastcsum_nofold_avx2_v3
fastcsum_nofold_avx2_v3:
        vpxor   xmm0, xmm0, xmm0
        cmp     rsi, 256
        jb      .LBB0_3
        vpbroadcastd    ymm1, dword ptr [rip + .LCPI0_0] # ymm1 = [8,8,8,8,8,8,8,8]
.LBB0_2:                                # =>This Inner Loop Header: Depth=1
        vmovdqu ymm2, ymmword ptr [rdi]
        vmovdqu ymm3, ymmword ptr [rdi + 64]
        vmovdqu ymm4, ymmword ptr [rdi + 128]
        vmovdqu ymm5, ymmword ptr [rdi + 192]
        vpaddd  ymm6, ymm2, ymmword ptr [rdi + 32]
        vpmaxud ymm2, ymm6, ymm2
        vpcmpeqd        ymm2, ymm6, ymm2
        vpaddd  ymm7, ymm3, ymmword ptr [rdi + 96]
        vpmaxud ymm3, ymm7, ymm3
        vpcmpeqd        ymm3, ymm7, ymm3
        vpaddd  ymm2, ymm3, ymm2
        vpaddd  ymm3, ymm4, ymmword ptr [rdi + 160]
        vpmaxud ymm4, ymm3, ymm4
        vpcmpeqd        ymm4, ymm3, ymm4
        vpaddd  ymm8, ymm5, ymmword ptr [rdi + 224]
        vpmaxud ymm5, ymm8, ymm5
        vpcmpeqd        ymm5, ymm8, ymm5
        vpaddd  ymm4, ymm4, ymm5
        vpaddd  ymm5, ymm7, ymm6
        vpmaxud ymm6, ymm5, ymm6
        vpcmpeqd        ymm6, ymm5, ymm6
        vpaddd  ymm2, ymm2, ymm6
        vpaddd  ymm6, ymm8, ymm3
        vpmaxud ymm3, ymm6, ymm3
        vpcmpeqd        ymm3, ymm6, ymm3
        vpaddd  ymm3, ymm4, ymm3
        vpaddd  ymm2, ymm2, ymm3
        vpaddd  ymm3, ymm6, ymm5
        vpmaxud ymm4, ymm3, ymm5
        vpcmpeqd        ymm4, ymm3, ymm4
        vpaddd  ymm3, ymm3, ymm0
        vpmaxud ymm0, ymm3, ymm0
        vpcmpeqd        ymm0, ymm3, ymm0
        vpaddd  ymm0, ymm0, ymm1
        vpaddd  ymm3, ymm3, ymm4
        vpaddd  ymm2, ymm2, ymm3
        vpaddd  ymm0, ymm2, ymm0
        add     rdi, 256
        add     rsi, -256
        cmp     rsi, 255
        ja      .LBB0_2
.LBB0_3:
        cmp     rsi, 128
        jae     .LBB0_4
        cmp     rsi, 64
        jae     .LBB0_6
.LBB0_7:
        cmp     rsi, 32
        jb      .LBB0_9
.LBB0_8:
        vpaddd  ymm1, ymm0, ymmword ptr [rdi]
        vpmaxud ymm0, ymm1, ymm0
        vpcmpeqd        ymm0, ymm1, ymm0
        vpaddd  ymm0, ymm1, ymm0
        vpcmpeqd        ymm1, ymm1, ymm1
        vpsubd  ymm0, ymm0, ymm1
        add     rdi, 32
        add     rsi, -32
.LBB0_9:
        vmovq   rax, xmm0
        add     rdx, rax
        vpextrq rax, xmm0, 1
        adc     rax, rdx
        vextracti128    xmm0, ymm0, 1
        vmovq   rcx, xmm0
        adc     rcx, rax
        vpextrq rax, xmm0, 1
        adc     rax, rcx
        adc     rax, 0
        cmp     rsi, 16
        jae     .LBB0_10
        cmp     rsi, 8
        jae     .LBB0_12
.LBB0_13:
        cmp     rsi, 4
        jae     .LBB0_14
.LBB0_15:
        cmp     rsi, 2
        jae     .LBB0_16
.LBB0_17:
        test    rsi, rsi
        je      .LBB0_19
.LBB0_18:
        movzx   ecx, byte ptr [rdi]
        add     rax, rcx
        adc     rax, 0
.LBB0_19:
        vzeroupper
        ret
.LBB0_4:
        vmovdqu ymm1, ymmword ptr [rdi]
        vmovdqu ymm2, ymmword ptr [rdi + 64]
        vpaddd  ymm3, ymm1, ymmword ptr [rdi + 32]
        vpmaxud ymm1, ymm3, ymm1
        vpcmpeqd        ymm1, ymm3, ymm1
        vpaddd  ymm4, ymm2, ymmword ptr [rdi + 96]
        vpmaxud ymm2, ymm4, ymm2
        vpcmpeqd        ymm2, ymm4, ymm2
        vpaddd  ymm1, ymm2, ymm1
        vpaddd  ymm2, ymm4, ymm3
        vpmaxud ymm3, ymm2, ymm3
        vpcmpeqd        ymm3, ymm2, ymm3
        vpaddd  ymm2, ymm2, ymm0
        vpmaxud ymm0, ymm2, ymm0
        vpcmpeqd        ymm0, ymm2, ymm0
        vpaddd  ymm1, ymm1, ymm2
        vpaddd  ymm1, ymm1, ymm3
        vpbroadcastd    ymm2, dword ptr [rip + .LCPI0_1] # ymm2 = [4,4,4,4,4,4,4,4]
        vpaddd  ymm0, ymm0, ymm2
        vpaddd  ymm0, ymm1, ymm0
        sub     rdi, -128
        add     rsi, -128
        cmp     rsi, 64
        jb      .LBB0_7
.LBB0_6:
        vpaddd  ymm1, ymm0, ymmword ptr [rdi]
        vpmaxud ymm0, ymm1, ymm0
        vpcmpeqd        ymm0, ymm1, ymm0
        vpaddd  ymm0, ymm1, ymm0
        vpcmpeqd        ymm1, ymm1, ymm1
        vpsubd  ymm0, ymm0, ymm1
        vpaddd  ymm2, ymm0, ymmword ptr [rdi + 32]
        vpmaxud ymm0, ymm2, ymm0
        vpcmpeqd        ymm0, ymm2, ymm0
        vpaddd  ymm0, ymm2, ymm0
        vpsubd  ymm0, ymm0, ymm1
        add     rdi, 64
        add     rsi, -64
        cmp     rsi, 32
        jae     .LBB0_8
        jmp     .LBB0_9
.LBB0_10:
        add     rax, qword ptr [rdi]
        adc     rax, qword ptr [rdi + 8]
        adc     rax, 0
        add     rdi, 16
        add     rsi, -16
        cmp     rsi, 8
        jb      .LBB0_13
.LBB0_12:
        add     rax, qword ptr [rdi]
        adc     rax, 0
        add     rdi, 8
        add     rsi, -8
        cmp     rsi, 4
        jb      .LBB0_15
.LBB0_14:
        mov     ecx, dword ptr [rdi]
        add     rax, rcx
        adc     rax, 0
        add     rdi, 4
        add     rsi, -4
        cmp     rsi, 2
        jb      .LBB0_17
.LBB0_16:
        movzx   ecx, word ptr [rdi]
        add     rax, rcx
        adc     rax, 0
        add     rdi, 2
        add     rsi, -2
        test    rsi, rsi
        jne     .LBB0_18
        jmp     .LBB0_19
