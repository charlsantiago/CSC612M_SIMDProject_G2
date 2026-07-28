; ============================================================================
; asmfunc2.asm - Group 2 kernel: sum of a 64-bit integer vector
; x86-64 SIMD AVX2 assembly using the XMM register (128-bit / 2 int64 lanes)
; ============================================================================

section .text
global sum_xmm_asm

sum_xmm_asm:
    xor     rax, rax               ; rax = running sum (used for final scalar result)
    xor     rcx, rcx               ; rcx = loop index i
    mov     r8, rsi                ; r8  = n
    and     r8, -2                 ; r8  = n rounded down to multiple of 2

    vpxor   xmm0, xmm0, xmm0       ; xmm0 = {0, 0} vector accumulator

    cmp     r8, 0
    jz      .reduce                ; fewer than 2 elements -> skip vector loop

.vec_loop:
    vpaddq  xmm0, xmm0, [rdi + rcx*8]   ; xmm0 += B[i:i+1] (2 x int64, unaligned-safe)
    add     rcx, 2
    cmp     rcx, r8
    jl      .vec_loop

.reduce:
    ; Horizontal add of the 2 int64 lanes in xmm0 -> scalar in rax
    vpsrldq xmm1, xmm0, 8           ; xmm1 = xmm0 shifted right 8 bytes (high lane -> low)
    vpaddq  xmm0, xmm0, xmm1        ; xmm0[63:0] = lane0 + lane1
    vmovq   rax, xmm0               ; rax = xmm0[63:0]

.tail_check:
    cmp     rcx, rsi
    jge     .done

.tail_loop:                        ; boundary element(s): n - r8 is 0 or 1 here
    add     rax, [rdi + rcx*8]
    inc     rcx
    cmp     rcx, rsi
    jl      .tail_loop

.done:
    ret
