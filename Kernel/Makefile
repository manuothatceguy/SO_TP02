include Makefile.inc

KERNEL=kernel.bin
SOURCES=$(wildcard *.c drivers/*.c interrupts/*.c lib/*.c memoryManagement/*.c process/*.c syscalls/*.c semaphores/*.c ds/*.c pipes/*.c)
SOURCES_ASM=$(wildcard asm/*.asm)
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
LOADERSRC=loader.asm
LOADEROBJECT=$(LOADERSRC:.asm=.o)
STATICLIBS=

all: $(KERNEL)

$(KERNEL): $(LOADEROBJECT) $(OBJECTS) $(STATICLIBS) $(OBJECTS_ASM)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_ASM) $(STATICLIBS)
	$(LD) $(LDFLAGS) -T kernel.ld --oformat=elf64-x86-64 -o kernel.elf $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_ASM) $(STATICLIBS)


%.o: %.c
	$(GCC) $(GCCFLAGS) -I./include -I../SharedLibraries $(MM) -c $< -o $@

%.o : %.asm drivers/keyboardDriver.o drivers/time.o process/scheduler.o 
	$(ASM) $(ASMFLAGS) $< -o $@

$(LOADEROBJECT):
	$(ASM) $(ASMFLAGS) $(LOADERSRC) -o $(LOADEROBJECT)

clean:
	rm -rf asm/*.o *.o *.bin */*.o

.PHONY: all clean
