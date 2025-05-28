GLOBAL wrapper
GLOBAL processStackFrame
GLOBAL jump_to_user_mode

EXTERN user_stack_top

; rdi = puntero a función en espacio usuario
; rsi = argv (pasamos como rdi en modo usuario)
jump_to_user_mode:
    ; Configurar segmentos para user mode (0x23 y 0x1B deben estar en GDT)
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; preparar stack de usuario
    mov rax, [user_stack_top]  ; stack superior del usuario
    and rax, ~0xF              ; alinear a 16 bytes

    ; preparar retorno con iretq
    push 0x23                  ; SS (user data segment)
    push rax                   ; RSP (user stack pointer)
    pushfq                     ; RFLAGS
    push 0x1B                  ; CS (user code segment)
    push rdi                   ; RIP (función de usuario)

    ; cargar argv en rdi
    mov rdi, rsi

    iretq

prepare_user_stack:
    ; No necesitamos hacer nada aquí ya que el stack de usuario
    ; ya está configurado en createProcess
    ret

wrapper:
    ; rdi = función de usuario
    ; rsi = argv
    ; argc ya viene en rdx (lo pasó processStackFrame)

    push rdi            ; guardamos dirección de función de usuario
    push rsi            ; guardamos argv

    call prepare_user_stack

    ; recuperar argumentos para jump_to_user_mode
    pop rsi             ; argv
    pop rdi             ; función de usuario

    call jump_to_user_mode

.hang:
    hlt
    jmp .hang

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
