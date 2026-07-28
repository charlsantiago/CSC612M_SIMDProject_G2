#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* ---- kernel prototypes -------------------------------------------------- */
int64_t sum_c(const int64_t *B, long long n);

extern int64_t sum_scalar_asm(const int64_t *B, long long n); /* asmfunc1.asm */
extern int64_t sum_xmm_asm(const int64_t *B, long long n);    /* asmfunc2.asm */
extern int64_t sum_ymm_asm(const int64_t *B, long long n);    /* asmfunc3.asm */

typedef int64_t (*kernel_fn)(const int64_t *, long long);

#define NUM_RUNS 30

/* Vector sizes required by the project spec: 2^20, 2^26, 2^30 */
static const long long TEST_SIZES[] = { 
    1LL << 20, 
    1LL << 26, 
    1LL << 30 
};
static const char *TEST_SIZE_LABELS[] = { 
    "SIZE n = 2^20", 
    "SIZE n = 2^26", 
    "SIZE n = 2^30" 
};

#define NUM_TEST_SIZES (int)(sizeof(TEST_SIZES) / sizeof(TEST_SIZES[0]))

/* ---- Boundary Sizes  ----------------------------------------------------
 * deliberately NOT multiples of 4 (and one not even a multiple of 2) so the SIMD kernels are forced to exercise their scalar "tail" cleanup path. 
 * 1003 % 4 = 3, 
 * 1003 % 2 = 1 (odd -> XMM tail of 1, YMM tail of 3). 
 * 1002 % 4 = 2, 1002 % 2 = 0 (even -> XMM tail of 0, YMM tail of 2). 
*/
static const long long BOUNDARY_SIZES[] = { 1002, 1003 };
#define NUM_BOUNDARY_SIZES (int)(sizeof(BOUNDARY_SIZES) / sizeof(BOUNDARY_SIZES[0]))


/* ---- plain C kernel (correctness "answer key") -------------------------- */
int64_t sum_c(const int64_t *B, long long n) {
    int64_t sum = 0;
    for (long long i = 0; i < n; i++) {
        sum += B[i];
    }
    return sum;
}


/* ---- helpers ------------------------------------------------------------- */
static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1.0e6;
}


/* Runs `fn` NUM_RUNS times over B[0..n-1], timing the kernel call only.
 * Returns the average time in milliseconds and writes the (identical,
 * deterministic) kernel result to *out_sum. */
static double time_kernel(kernel_fn fn, const int64_t *B, long long n, int64_t *out_sum) {
    int64_t sum = 0;
    double total_ms = 0.0;

    for (int r = 0; r < NUM_RUNS; r++) {
        double t0 = now_ms();
        sum = fn(B, n);
        double t1 = now_ms();
        total_ms += (t1 - t0);
    }

    *out_sum = sum;
    return total_ms / NUM_RUNS;
}

        
static int64_t *alloc_vector(long long n) {
    void *ptr = NULL;
    /* 32-byte alignment for good AVX2 access patterns (loads are still
     * unaligned-safe via vpaddq, but alignment helps performance). */
    if (posix_memalign(&ptr, 32, (size_t)n * sizeof(int64_t)) != 0) {
        fprintf(stderr, "ERROR: failed to allocate vector of %lld int64 elements\n", n);
        exit(EXIT_FAILURE);
    }
    return (int64_t *)ptr;
}

        
static void init_vector(int64_t *B, long long n) {
    for (long long i = 0; i < n; i++) {
        B[i] = i + 1; /* documented initialization: B[i] = i+1 */
    }
}


static const char *check(int64_t reference, int64_t value) {
    return (reference == value) ? "MATCH" : "MISMATCH";
}


/* Runs and reports all four kernels for one vector size. */
static void run_case(const char *label, long long n) {
    printf("\n==============================================================\n");
    printf(" %s : n = %lld elements (%.3f MB)\n", label, n,
           (double)n * sizeof(int64_t) / (1024.0 * 1024.0));
    printf("==============================================================\n");

    int64_t *B = alloc_vector(n);
    init_vector(B, n);

    int64_t sum_ref, s_c, s_asm, s_xmm, s_ymm;
    double  t_c, t_asm, t_xmm, t_ymm;

    t_c   = time_kernel(sum_c,          B, n, &s_c);
    sum_ref = s_c; /* C version is the correctness answer key */

    t_asm = time_kernel(sum_scalar_asm, B, n, &s_asm);
    t_xmm = time_kernel(sum_xmm_asm,    B, n, &s_xmm);
    t_ymm = time_kernel(sum_ymm_asm,    B, n, &s_ymm);

    printf("\n[Correctness]  (C kernel result is the answer key)\n");
    printf("  C        : SUM = %20lld\n", (long long)s_c);
    printf("  x86-64   : SUM = %20lld   [%s]\n", (long long)s_asm, check(sum_ref, s_asm));
    printf("  SIMD XMM : SUM = %20lld   [%s]\n", (long long)s_xmm, check(sum_ref, s_xmm));
    printf("  SIMD YMM : SUM = %20lld   [%s]\n", (long long)s_ymm, check(sum_ref, s_ymm));

    printf("\n[Timing]  (average over %d runs, kernel call only)\n", NUM_RUNS);
    printf("  C        : %12.6f ms\n", t_c);
    printf("  x86-64   : %12.6f ms   (%.2fx vs C)\n", t_asm, t_c / t_asm);
    printf("  SIMD XMM : %12.6f ms   (%.2fx vs C)\n", t_xmm, t_c / t_xmm);
    printf("  SIMD YMM : %12.6f ms   (%.2fx vs C)\n", t_ymm, t_c / t_ymm);

    free(B);
}


int main(void) {
    printf("Group 2 - SIMD Project: Sum of 64-bit Integer Vector\n");
    printf("Data init: B[i] = i + 1, for i = 0 .. n-1\n");
    printf("Runs per kernel per size: %d (average reported)\n", NUM_RUNS);

    printf("\n#################### MAIN PERFORMANCE RUNS ####################\n");
    for (int i = 0; i < NUM_TEST_SIZES; i++) {
        run_case(TEST_SIZE_LABELS[i], TEST_SIZES[i]);
    }

    printf("\n#################### BOUNDARY HANDLING CHECKS ####################\n");
    printf("(vector sizes are NOT multiples of 4, forcing the SIMD scalar\n");
    printf(" 'tail' cleanup path in asmfunc2.asm / asmfunc3.asm to run)\n");
    for (int i = 0; i < NUM_BOUNDARY_SIZES; i++) {
        long long n = BOUNDARY_SIZES[i];
        char label[64];
        snprintf(label, sizeof(label), "BOUNDARY n = %lld", n);
        run_case(label, n);
    }

    printf("\nAll cases complete.\n");
    return 0;
}
