GLOBAL wrapper
GLOBAL processStackFrame
GLOBAL idle

wrapper:
    pop rdi    ; argc
    pop rsi    ; argv
    pop rax    ; función real
    call rax   ; rdi=argc, rsi=argv
    mov rax, 16
    int 80h

processStackFrame: ; rdi process->base, rsi process->rip, rdx argc, rcx argv
    push rbp
    mov rbp, rsp
    mov rsp, rdi
    and rsp, -16; align
    push 0      ; ss
    push rdi    ; stack vacío
    push 0x202  ; rflags
    push 0x8    ; cs
    push rsi    ; rip
    push 0      ; r15
    push 0      ; r14
    push 0      ; r13
    push 0      ; r12
    push 0      ; r11
    push 0      ; r10
    push 0      ; r9
    push 0      ; r8
    push 0      ; rbp
    push 0      ; rbp
    push 0      ; rdx
    push rcx    ; argv
    push rdx    ; argc
    push rsi    ; función real
    push 0      ; rbx
    push 0      ; rax
    mov rax, rsp; para retornar
    mov rsp, rbp
    pop rbp
    ret
