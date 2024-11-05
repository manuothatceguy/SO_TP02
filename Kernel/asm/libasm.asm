GLOBAL cpuVendor
GLOBAL kb_getKey
GLOBAL getRegisters
EXTERN rip_aux
GLOBAL outb
GLOBAL inb
GLOBAL regs
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



getRegisters: ; Deja el vector de registros en RAX. 
	mov [regs], rax
	mov [regs + 8*1], rbx
	mov [regs + 8*2], rcx
	mov [regs + 8*3], rdx 
	mov [regs + 8*4], rsi
	mov [regs + 8*5], rdi
	mov [regs + 8*6], rbp
	mov [regs + 8*7], rsp
	mov [regs + 8*8], r8
	mov [regs + 8*9], r9 
	mov [regs + 8*10], r10 
	mov [regs + 8*11], r11
	mov [regs + 8*12], r12
	mov [regs + 8*13], r13
	mov [regs + 8*14], r14
	mov [regs + 8*15], r15
	pushfq
	pop rax
	push rax
	popfq
	mov [regs + 8*16], rax ;rflags
	mov rax, [rip_aux] ;rip
	mov qword[regs + 8*17], rax ;rip
	mov rax, regs
	ret

outb:
    push rbp              
    mov rbp, rsp           
    mov dx, di             
    mov al, sil            
    out dx, al
    mov rsp, rbp           
    pop rbp                
    ret                    

inb:
	push rbp
	mov rbp, rsp
    mov dx, di             
    in  al, dx             
    movzx eax, al
	mov rsp, rbp
	pop rbp        
    ret                    

section .bss
regs resq 18 ; 18 x 8 bytes 