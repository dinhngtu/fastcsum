.intel_syntax noprefix

.global fastcsum_nofold_x64_128b

fastcsum_nofold_x64_128b:
    # rdi: byte ptr
    # rsi: size
    # rdx: initial

    # no stack!

    mov rax, rdx                    # returns primary accumulator (CF)

128:
    cmp rsi, 128
    jb 64f

    add rax, [rdi]
    adc rax, [rdi + 8]
    adc rax, [rdi + 16]
    adc rax, [rdi + 24]
    adc rax, [rdi + 32]
    adc rax, [rdi + 40]
    adc rax, [rdi + 48]
    adc rax, [rdi + 56]
    adc rax, [rdi + 64]
    adc rax, [rdi + 72]
    adc rax, [rdi + 80]
    adc rax, [rdi + 88]
    adc rax, [rdi + 96]
    adc rax, [rdi + 104]
    adc rax, [rdi + 112]
    adc rax, [rdi + 120]
    adc rax, 0

    sub rsi, 128
    add rdi, 128
    jmp 128b

64:
    cmp rsi, 64                     # eight qwords
    jb 32f

    add rax, [rdi]
    adc rax, [rdi + 8]
    adc rax, [rdi + 16]
    adc rax, [rdi + 24]
    adc rax, [rdi + 32]
    adc rax, [rdi + 40]
    adc rax, [rdi + 48]
    adc rax, [rdi + 56]
    adc rax, 0

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

    mov ecx, dword ptr [rdi]
    add rax, rcx
    adc rax, 0

    sub rsi, 4
    add rdi, 4

2:
    cmp rsi, 2                      # one word
    jb 1f

    movzx ecx, word ptr [rdi]
    add rax, rcx
    adc rax, 0

    sub rsi, 2
    add rdi, 2

1:
    cmp rsi, 1                      # last byte
    jb 0f

    movzx ecx, byte ptr [rdi]
    add rax, rcx
    adc rax, 0

0:
    ret
