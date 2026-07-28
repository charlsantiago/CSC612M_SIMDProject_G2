; ============================================================================
; asmfunc1.asm - Group 2 kernel: sum of a 64-bit integer vector
; Plain x86-64 assembly (NO SIMD instructions)
; ============================================================================

section .text
global sum_scalar_asm

sum_scalar_asm:
    xor     rax, rax           ; sum = 0
    xor     rcx, rcx           ; i = 0

    cmp    rsi, 0              ; n == 0 ?
    je      .done

.loop:
    add     rax, [rdi + rcx*8] ; sum += B[i]
    inc     rcx                ; i++
    cmp     rcx, rsi
    jl      .loop

.done:
    ret
