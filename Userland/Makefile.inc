GCC=x86_64-linux-gnu-gcc
GPP=x86_64-linux-gnu-g++
LD=x86_64-linux-gnu-ld
AR=x86_64-linux-gnu-ar
ASM=nasm

GCCFLAGS=-fno-pie -no-pie -m64 -fno-exceptions -std=c99 -Wall -ffreestanding -nostdlib -fno-common -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -fno-builtin-malloc -fno-builtin-free -fno-builtin-realloc -g 
ARFLAGS=rvs
ASMFLAGS=-felf64
