# CSC612M - 3rd Term 2025-2026 - SIMD Project (Group 2)
**Hontiveros, Jan Aldo | Santiago, Charl Joseph**
----
**Requirement:** <br>
- The kernel adds the values of 64-bit vector integers.  <br>

**Input:**  <br>
- Scalar variable n (integer) contains the length of the vector; Vector B contains 64-bit integer. <br>

**Process:**
```c
// SUM = B[0] + B[1] +..... +B[n-1]

static void init_vector(int64_t *B, long long n) {
    for (long long i = 0; i < n; i++) {
        B[i] = i + 1; /* documented initialization: B[i] = i+1 */
    }
}
```

**Output:**  <br>
- Store the result in scalar variable SUM. Display the result. <br>

## Files

| File              | Description                                                          |
|-------------------|----------------------------------------------------------------------|
| `main.c`          | C driver: init, timing harness, correctness checks, reporting        |
| `asmfunc1.asm`    | x86-64 assembly, no SIMD (scalar accumulate loop)                    |
| `asmfunc2.asm`    | x86-64 SIMD AVX2 using **XMM** registers (2 × int64 per iteration)   |
| `asmfunc3.asm`    | x86-64 SIMD AVX2 using **YMM** registers (4 × int64 per iteration)   |
| `Makefile`        | Build rules (`nasm` + `gcc`)                                         |
| `CSC612M_G2_Sum`  | Python Notebook that runs the .exe file created by Makefile          |

---


## I. Program Output with Execution Time for All Cases (screenshot)
The **CSC612M_G2_Sum**.exe file created after running the Makefile / manual scripts displays all the output of the four kernels for:
- `asmfunc1.asm (x86-x64)`,
- `asmfunc2.asm (XMM Registers)`,
- `asmfunc3.asm (YMM Registers)`, and
- `main.c (Correctness Check)`

<img src="Screenshots/A. Program Output (All Cases).png" alt="All Cases" width="1200">



## II. Comparative Table of Execution Time and Analysis 
**XMM vs. YMM performance**
- ***Expectation:*** Since the YMM registers processes four 64-bit integers (256 bits) and the XMM register processes two 64-bit integers (128 bits), YMM should be twice as fast as XMM.
- ***Actual:*** YMM is not twice faster than XMM in all vector array sizes. Although, the smaller vector array of 2^20 displays a "larger" speed difference between XMM and YMM than the 2^30 vector size.
- ***Explanation:*** Addition has an extremely low computational intensity. Most modern CPUs have larger cache than a 2^20 vector size (~8MB), allowing the CPU to fetch data almost instantly. A vector array of size 2^30 is already ~8GB, which can't be handled by the CPU cache. It looks to the RAM to transfer chunks of data to the motherboard basically CPU is stalled by the hardware

<img src="Screenshots/B. Comparative Table.png" alt="Comparative Table" width="1000">


## III. Program Output - with Correctness Check (C) (Screenshot)
The code that will verify if the results comming from x86-x64, SIMD XMM, and SIMD YMM are based on this logic:
```c
int64_t sum_c(const int64_t *B, long long n) {
    int64_t sum = 0;
    for (long long i = 0; i < n; i++) {
        sum += B[i];
    }
    return sum;
}
```
<img src="Screenshots/C. Program Output (C).png" alt="" width="700">

## IV. Program Output - x86-x64 including correctness check (Screenshot)
The output derived from the x86-x64 - **sum_scalar_asm** will be compared with the Correctness Check (**sum_c**)
<img src="Screenshots/D. Program Output (x86-x64).png" alt="86-x64 " width="700">

## V. Program Output - SIMD, XMM Register including correctness check (Screenshot)
The output derived from the SIMD-XMM - **sum_xmm_asm** will be compared with the Correctness Check (**sum_c**)
<img src="Screenshots/E. Program Output (SIMD XMM).png" alt="SIMD, XMM Register" width="700">

## VI. Program Output - SIMD, YMM Register including correctness check (Screenshot)
The output derived from the SIMD-YMM - **sum_ymm_asm** will be compared with the Correctness Check (**sum_c**)
<img src="Screenshots/F. Program Output (SIMD YMM).png" alt="SIMD, YMM Register" width="700">

## VII. Boundary Check for SIMD XMM and SIMD YMM (Screenshot)
**Boundary Handling** 
- ***Expectation:*** Regardless if the vector size's divisibility by 4, the whole process should happen exclusively within the XMM and YMM registers
- ***Actual:*** If the size is not divisible by 4, the program will 'crash'
- ***Fix:*** Aside from the vector loop that handles the vector sizes divisibly by 4, we implemented a "cleanup" loop (tail_loop) that handles the 1 to 3 extra elements. It performs the same as the loop for non-SIMD x86-64 (asmfunc1.asm) hence the slower run time.

<img src="Screenshots/G. Boundary Check for SIMD XMM-YMM.png" alt="Boundary Check for SIMD XMM-YMM" width="700">

## VIII. Discuss the problems encountered and solutions made, unique methodology used, AHA moments, etc.
<img src="Screenshots/H. SASM Reference.png" alt="SASM" width="700">


---


## Prerequisites

Before building the project, install the following:

- GCC (GNU C Compiler)
- NASM (Netwide Assembler)
- GNU Make

To do this, link the NASM in the PATH (Linux) by doing:

```bash
export PATH=$PATH:/usr/local/bin
```

## Building the Project

Compile the project using:

```bash
make
```

This generates the executable:

```
CSC612M_G2_Sum
```

---

## Running the Program

Run the executable:

```bash
./CSC612M_G2_Sum
```


## Makefile Commands

| Command | Description |
|---------|-------------|
| `make` | Compiles all source files and links them into `CSC612M_G2_Sum`. |
| `make run` | Builds (if needed) and runs the executable. |
| `make clean` | Removes all object files and the executable. |
| `make clean && make` | Performs a clean rebuild. |

---

## Manual Compilation Commands

| Command | Description |
|---------|-------------|
| `gcc -O2 -Wall -Wextra -mavx2 -c main.c -o main.o` | Compiles the C source file into an object file. |
| `nasm -f elf64 asmfunc1.asm -o asmfunc1.o` | Assembles `asmfunc1.asm`. |
| `nasm -f elf64 asmfunc2.asm -o asmfunc2.o` | Assembles `asmfunc2.asm`. |
| `nasm -f elf64 asmfunc3.asm -o asmfunc3.o` | Assembles `asmfunc3.asm`. |
| `gcc -O2 -Wall -Wextra -mavx2 -o CSC612M_G2_Sum main.o asmfunc1.o asmfunc2.o asmfunc3.o -lrt` | Links all object files into the executable. |
| `./CSC612M_G2_Sum` | Runs the executable manually. |

---

## GCC Command Breakdown

### Compile Command

```bash
gcc -O2 -Wall -Wextra -mavx2 -c main.c -o main.o
```

| Component | Description |
|-----------|-------------|
| `gcc` | GNU C Compiler |
| `-O2` | Enables Level 2 optimization for better performance. |
| `-Wall` | Enables common compiler warnings. |
| `-Wextra` | Enables additional compiler warnings. |
| `-mavx2` | Enables AVX2 instruction support. |
| `-c` | Compiles without linking. |
| `main.c` | Input C source file. |
| `-o main.o` | Specifies the output object file. |

---

### Link Command

```bash
gcc -O2 -Wall -Wextra -mavx2 -o CSC612M_G2_Sum main.o asmfunc1.o asmfunc2.o asmfunc3.o -lrt
```

| Component | Description |
|-----------|-------------|
| `gcc` | GNU C Compiler and linker. |
| `-o CSC612M_G2_Sum` | Names the executable `CSC612M_G2_Sum`. |
| `main.o` | Compiled C object file. |
| `asmfunc1.o` | Compiled assembly object file. |
| `asmfunc2.o` | Compiled assembly object file. |
| `asmfunc3.o` | Compiled assembly object file. |
| `-lrt` | Links the POSIX Real-Time Library. |

---

## NASM Command Breakdown

Example:

```bash
nasm -f elf64 asmfunc1.asm -o asmfunc1.o
```

| Component | Description |
|-----------|-------------|
| `nasm` | Netwide Assembler. |
| `-f elf64` | Produces a 64-bit ELF object file for Linux. |
| `asmfunc1.asm` | Input assembly source file. |
| `-o asmfunc1.o` | Specifies the output object file. |

---

## Build Workflow

```
            main.c
              │
              ▼
            gcc -c
              │
            main.o
              │
              ▼
asmfunc1.asm ─► nasm ─► asmfunc1.o
asmfunc2.asm ─► nasm ─► asmfunc2.o
asmfunc3.asm ─► nasm ─► asmfunc3.o
              │
              ▼
  gcc (Link all object files)
              │
              ▼
      CSC612M_G2_Sum
              │
              ▼
     ./CSC612M_G2_Sum
```

---

## References


| URL | Description |
|-----------|-------------|
| https://www.nasm.us/docs/3.02/ | NASM - The Netwide Assembler Document <br> ***The version running in Linux Server is 2.16.03***  |
| https://makefiletutorial.com/ | Makefile Tutorial |
