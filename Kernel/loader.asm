global loader
global init
extern main
extern initializeKernelBinary
extern getStackBase

loader:
	call initializeKernelBinary	; Set up the kernel binary, and get thet stack address
	mov rsp, rax				; Set up the stack with the returned address
init:
	call getStackBase
	mov rsp, rax
	call main
hang:
	cli
	hlt	; halt machine should kernel return
	jmp hang

	
section .rodata
userland equ 0x400000