; ============================================================================
; asmfunc3.asm - Group 2 kernel: sum of a 64-bit integer vector
; x86-64 SIMD AVX2 assembly using the YMM register (256-bit / 4 int64 lanes)
; ============================================================================

section .text
global sum_ymm_asm

sum_ymm_asm:
    xor     rax, rax               ; rax = running sum (used for final scalar result)
    xor     rcx, rcx               ; rcx = loop index i
    mov     r8, rsi                ; r8  = n
    and     r8, -4                 ; r8  = n rounded down to multiple of 4

    vpxor   ymm0, ymm0, ymm0       ; ymm0 = {0,0,0,0} vector accumulator

    test    r8, r8
    jz      .reduce                ; fewer than 4 elements -> skip vector loop

.vec_loop:
    vpaddq  ymm0, ymm0, [rdi + rcx*8]   ; ymm0 += B[i:i+3] (4 x int64, unaligned-safe)
    add     rcx, 4
    cmp     rcx, r8
    jl      .vec_loop

.reduce:
    ; Horizontal add of the 4 int64 lanes in ymm0 -> scalar in rax
    vextracti128 xmm1, ymm0, 1     ; xmm1 = high 128 bits (lanes 2,3)
    vpaddq  xmm0, xmm0, xmm1       ; xmm0 = {lane0+lane2, lane1+lane3}
    vpsrldq xmm1, xmm0, 8          ; xmm1 = xmm0 shifted right 8 bytes
    vpaddq  xmm0, xmm0, xmm1       ; xmm0[63:0] = (lane0+lane2) + (lane1+lane3)
    vmovq   rax, xmm0              ; rax = xmm0[63:0]
    vzeroupper                     ; clear upper YMM state (avoid AVX/SSE penalty)

.tail_check:
    cmp     rcx, rsi
    jge     .done

.tail_loop:                        ; boundary element(s): n - r8 is 0, 1, 2, or 3 here
    add     rax, [rdi + rcx*8]
    inc     rcx
    cmp     rcx, rsi
    jl      .tail_loop

.done:
    ret
