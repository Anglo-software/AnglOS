[GLOBAL syscallEntryPoint]
[extern syscalltest]

%define num_syscalls 4

extern sys_halt
extern sys_print
extern sys_getc
extern sys_exit

syscall_table:
    dq sys_halt
    dq sys_print
    dq sys_getc
    dq sys_exit
    dq 0x0000000000000000

syscall_user_to_kernel:
    push rbp
    mov rbp, rsp

    mov rax, [gs:0x28]
    mov [rax + 0x00], rax

    xor rax, rax
    mov ax, 0x30
    mov ds, ax
    mov es, ax

    mov rax, [gs:0x28]
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

    pop rbp
    ret

syscall_kernel_to_user:
    push rbp
    mov rbp, rsp

    push rax
    push rdx
    
    mov rax, [gs:0x28]
    mov rax, [rax + 0x78]
    mov cr3, rax

    xor eax, eax
    mov ax, 0x3B
    mov ds, ax
    mov es, ax

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

    pop rdx
    pop rax

    pop rbp
    ret

syscallEntryPoint:
    swapgs
    mov QWORD [gs:0x00], 1
    mov [gs:0x10], rsp
    mov rsp, [gs:0x08]
    push rbp
    mov rbp, rsp

    cmp rax, num_syscalls
    jge .undefined_syscall

    push rax
    call syscall_user_to_kernel
    pop rax

    mov rcx, r10

    lea r10, [rel syscall_table]
    lea r10, [r10 + (rax * 8)]
    mov rax, [r10]

    xor r10, r10

    call rax

    call syscall_kernel_to_user
    pop rbp
    mov rsp, [gs:0x10]
    mov QWORD [gs:0x00], 0
    swapgs
    o64 sysret

    .undefined_syscall:
    ud2

section .data