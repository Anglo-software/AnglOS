%macro push_general 0
push rax
push rbx
push rcx
push rdx
push rsi
push rdi
push r8
push r9
push r10
push r11
push r12
push r13
push r14
push r15
%endmacro

%macro pop_general 0
pop r15
pop r14
pop r13
pop r12
pop r11
pop r10
pop r9
pop r8
pop rdi
pop rsi
pop rdx
pop rcx
pop rbx
pop rax
%endmacro

%macro push_control 0
mov rax, cr0
push rax
mov rax, cr2
push rax
mov rax, cr3
push rax
mov rax, cr4
push rax
%endmacro

%macro pop_control 0
pop rax
mov cr4, rax
pop rax
mov cr3, rax
pop rax
mov cr2, rax
pop rax
mov cr0, rax
%endmacro

save_context:
    push rbp
    mov rbp, rsp

    push rbx
    mov rbx, [gs:0x28]
    mov [rbx + 0x00], rax
    pop rbx

    push rax
    mov rax, [gs:0x20]
    mov [rax + 0x08], rbx
    mov [rax + 0x10], rcx
    mov [rax + 0x18], rdx
    mov [rax + 0x20], rsi
    mov [rax + 0x28], rdi
    mov [rax + 0x30], r8
    mov [rax + 0x38], r9
    mov [rax + 0x40], r10
    mov [rax + 0x48], r11
    mov [rax + 0x50], r12
    mov [rax + 0x58], r13
    mov [rax + 0x60], r14
    mov [rax + 0x68], r15

    pop rax
    pop rbp
    ret

restore_context:
    push rbp
    mov rbp, rsp

    mov rax, [gs:0x28]
    mov rbx, [rax + 0x08]
    mov rcx, [rax + 0x10]
    mov rdx, [rax + 0x18]
    mov rsi, [rax + 0x20]
    mov rdi, [rax + 0x28]
    mov r8,  [rax + 0x30]
    mov r9,  [rax + 0x38]
    mov r10, [rax + 0x40]
    mov r11, [rax + 0x48]
    mov r12, [rax + 0x50]
    mov r13, [rax + 0x58]
    mov r14, [rax + 0x60]
    mov r15, [rax + 0x68]
    mov rax, [rax + 0x00]

    pop rbp
    ret

extern interrupt_handlers

%macro irq_stub 1
irq_stub_%+%1:
    push rbp
    mov rbp, rsp
    push rax
    xor rax, rax
    mov ax, ds
    cmp ax, 0x30
    pop rax
    je .in_kernel

    .in_user:
    swapgs
    call save_context
    
    mov ax, 0x30
    mov ds, ax
    mov es, ax
    mov ss, ax

    lea r15, [rel interrupt_handlers]
    mov r15, [r15 + %1 * 8]
    call r15

    mov ax, 0x3B
    mov ds, ax
    mov es, ax
    call restore_context
    swapgs
    pop rbp
    iretq

    .in_kernel:
    push_general
    push_control
    lea r15, [rel interrupt_handlers]
    mov r15, [r15 + %1 * 8]
    call r15
    pop_control
    pop_general
    pop rbp
    iretq
%endmacro

extern isrHandler

isr_common_stub:
    push rbp
    mov rbp, rsp
    push_general
    push_control
    mov ax, ds
    push rax
    push qword 0

    push rax
    xor eax, eax
    mov ax, ds
    cmp ax, 0x30
    pop rax
    je .in_kernel

    .in_user:
    swapgs
    call save_context

    mov ax, 0x30
    mov ds, ax
    mov es, ax
    mov ss, ax

    lea rdi, [rsp + 0x10]
    call isrHandler

    call restore_context

    pop rax
    pop rax
    mov ds, ax
    mov es, ax
    pop_control
    pop_general
    pop rbp
    add rsp, 0x10
    swapgs
    iretq

    .in_kernel:
    lea rdi, [rsp + 0x10]
    call isrHandler

    pop rax
    pop rax
    mov ds, ax
    mov es, ax
    pop_control
    pop_general
    pop rbp
    add rsp, 0x10
    iretq

%macro isr_err_stub 1
isr_stub_%+%1:
    push %1
    jmp isr_common_stub
%endmacro

%macro isr_no_err_stub 1
isr_stub_%+%1:
    push 0
    push %1
    jmp isr_common_stub
%endmacro

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31
irq_stub        32
irq_stub        33
irq_stub        34
irq_stub        35
irq_stub        36
irq_stub        37
irq_stub        38
irq_stub        39
irq_stub        40
irq_stub        41
irq_stub        42
irq_stub        43
irq_stub        44
irq_stub        45
irq_stub        46
irq_stub        47

global isr_stub_table
isr_stub_table:
%assign i 0 
%rep    32 
    dq isr_stub_%+i
%assign i i+1 
%endrep
%assign i 32
%rep    16
    dq irq_stub_%+i
%assign i i+1
%endrep