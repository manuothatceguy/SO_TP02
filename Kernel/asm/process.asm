GLOBAL wrapper
GLOBAL processStackFrame

wrapper:
    call r8
    mov rax, 23
    int 80h

.hang:
    hlt
    jmp .hang

processStackFrame: ; rdi process->base, rsi process->fnptr, rdx argc, rcx argv
    push rbp
    mov rbp, rsp
    mov rsp, rdi
    and rsp, -16; align
    push 0      ; ss
    push rdi    ; stack vacío
    push 0x202  ; rflags
    push 0x8    ; cs
    push wrapper    ; rip
    push 0      ; r15
    push 0      ; r14
    push 0      ; r13
    push 0      ; r12
    push 0      ; r11
    push 0      ; r10
    push 0      ; r9
    push rsi    ; r8
    push 0      ; rbp
    push rdx    ; rdi
    push rcx    ; rsi
    push 0      ; rdx
    push 0      ; rcx
    push 0      ; rbx
    push 0      ; rax
    mov rax, rsp; para retornar
    mov rsp, rbp
    pop rbp
    ret
