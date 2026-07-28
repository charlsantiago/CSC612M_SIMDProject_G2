# ============================================================================
# Makefile - Group 2 SIMD Project (sum of 64-bit integer vector)
# ============================================================================

CC      := gcc
CFLAGS  := -O2 -Wall -Wextra -mavx2
LDFLAGS := -lrt
NASM    := nasm
NASMFLAGS := -f elf64

TARGET  := CSC612M_G2_Sum

OBJS := main.o asmfunc1.o asmfunc2.o asmfunc3.o

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

asmfunc1.o: asmfunc1.asm
	$(NASM) $(NASMFLAGS) asmfunc1.asm -o asmfunc1.o

asmfunc2.o: asmfunc2.asm
	$(NASM) $(NASMFLAGS) asmfunc2.asm -o asmfunc2.o

asmfunc3.o: asmfunc3.asm
	$(NASM) $(NASMFLAGS) asmfunc3.asm -o asmfunc3.o

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
