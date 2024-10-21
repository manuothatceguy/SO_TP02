GLOBAL cpuVendor
GLOBAL rtc
GLOBAL kb_getKey

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