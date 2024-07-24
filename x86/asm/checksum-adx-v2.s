.intel_syntax noprefix

.global fastcsum_nofold_adx_v2

fastcsum_nofold_adx_v2:
    # rdi: byte ptr
    # rsi: size
    # rdx: initial

    # no stack!

    mov rax, rdx                    # returns primary accumulator (CF)
    xor rdx, rdx                    # rdx is scratch

128:
    cmp rsi, 128
    jb 64f

    xor ecx, ecx                    # rcx is second accumulator (OF)
                                    # clear CF/OF to prepare carry chains
    adcx rax, [rdi]
    mov rcx, [rdi + 8]
    adcx rax, [rdi + 16]
    adox rcx, [rdi + 24]
    adcx rax, [rdi + 32]
    adox rcx, [rdi + 40]
    adcx rax, [rdi + 48]
    adox rcx, [rdi + 56]
    adcx rax, [rdi + 64]
    adox rcx, [rdi + 72]
    adcx rax, [rdi + 80]
    adox rcx, [rdi + 88]
    adcx rax, [rdi + 96]
    adox rcx, [rdi + 104]
    adcx rax, [rdi + 112]
    adox rcx, [rdi + 120]
    adox rax, rdx                   # use rdx as zero
    adc rax, rcx
    adc rax, 0

    sub rsi, 128
    add rdi, 128
    jmp 128b

64:
    cmp rsi, 64                     # loop eight qwords at a time
    jb 32f

    xor ecx, ecx
    adcx rax, [rdi]
    mov rcx, [rdi + 8]
    adcx rax, [rdi + 16]
    adox rcx, [rdi + 24]
    adcx rax, [rdi + 32]
    adox rcx, [rdi + 40]
    adcx rax, [rdi + 48]
    adox rcx, [rdi + 56]
    adox rax, rdx
    adc rax, rcx
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

    # we start using rdx later so don't "adcx/adox reg, rdx" any more
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

    mov edx, dword ptr [rdi]
    add rax, rdx
    adc rax, 0

    sub rsi, 4
    add rdi, 4

2:
    cmp rsi, 2                      # one word
    jb 1f

    movzx edx, word ptr [rdi]
    add rax, rdx
    adc rax, 0

    sub rsi, 2
    add rdi, 2

1:
    cmp rsi, 1                      # last byte
    jb 0f

    movzx edx, byte ptr [rdi]
    add rax, rdx
    adc rax, 0

0:
    ret
