GLOBAL cpuVendor
GLOBAL rtc
GLOBAL kb_getKey
GLOBAL getRegisters

section .text
	
cpuVendor:
	push rbp
	mov rbp, rsp

	push rbx

	mov rax, 0
	cpuid


	mov [rdi], ebx
	mov [rdi + 4], edx
	mov [rdi + 8], ecx

	mov byte [rdi+13], 0

	mov rax, rdi

	pop rbx

	mov rsp, rbp
	pop rbp
	ret

; RTC

rtc:
	push rbp
	mov rbp, rsp 
	mov al, dil  
	out 70h, al
	in al, 71h   
	movzx rax, al
	mov rsp, rbp
	pop rbp
	ret

kb_getKey:
    push rbp
    mov rbp, rsp

.wait_key:
    in al, 64h        
    and al, 1         
    jz .wait_key       

    in al, 60h
	movzx rax, al      
    mov rsp, rbp
    pop rbp
    ret

getRip:
	pop rax
	push rax
	ret

getRegisters:
	call getRip
	mov [regs], rax 
	pop rax
	mov [regs + 8], rax
	mov [regs + 16], rbx
	mov [regs + 24], rcx 
	mov [regs + 32], rdx
	mov [regs + 40], rsi
	mov [regs + 48], rdi 
	mov [regs + 56], rbp
	mov [regs + 64], rsp 
	mov [regs + 72], r8 
	mov [regs + 80], r9 
	mov [regs + 88], r10
	mov [regs + 96], r11
	mov [regs + 104], r12
	mov [regs + 112], r13 
	mov [regs + 120], r14
	mov [regs + 128], r15
	pushfq
	pop rax
	mov [regs + 136], rax 
	mov rax, regs
	ret

section .bss
regs resb 144 ; 18 x 8