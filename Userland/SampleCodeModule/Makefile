include ../Makefile.inc

MODULE=0000-sampleCodeModule.bin
SOURCES=$(wildcard [^_]*.c)
SOURCES_ASM=$(wildcard asm/*.asm)
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
INCLUDE=./include

all: tests $(MODULE)

$(MODULE): $(OBJECTS) $(OBJECTS_ASM)

	$(GCC) $(GCCFLAGS) -T sampleCodeModule.ld _loader.c $(OBJECTS) $(OBJECTS_ASM) -o ../$(MODULE) -I $(INCLUDE)

tests:
	cd tests && make all

%.o: %.c
	$(GCC) $(GCCFLAGS) -I$(INCLUDE) -c $< -o $@

%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -rf asm/*.o *.o
	cd tests && make clean && cd ..

.PHONY: all clean