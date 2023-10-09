[GLOBAL syscallEntryPoint]
[extern syscalltest]

%macro pushagrd 0
push rbx
push rcx
push rdx
push rsi
push rdi
push rbp
push rsp
push r8
push r9
push r10
push r11
push r12
push r13
push r14
push r15
%endmacro

%macro popagrd 0
pop r15
pop r14
pop r13
pop r12
pop r11
pop r10
pop r9
pop r8
pop rsp
pop rbp
pop rdi
pop rsi
pop rdx
pop rcx
pop rbx
%endmacro

%macro pushacrd 0
mov rcx, cr0
push rcx
mov rcx, cr2
push rcx
mov rcx, cr3
push rcx
mov rcx, cr4
push rcx
%endmacro

%macro popacrd 0
pop rcx
mov cr4, rcx
pop rcx
mov cr3, rcx
pop rcx
mov cr2, rcx
pop rcx
mov cr0, rcx
%endmacro

%define num_syscalls 3

syscallEntryPoint:
    push rbp
    mov rbp, rsp
    pushagrd
    pushacrd
    mov rcx, r10

    cmp rax, num_syscalls
    jge .undefined_syscall

    lea r10, [rel syscall_table]
    lea r10, [r10 + (rax * 8)]
    mov rax, [r10]

    call rax

    popacrd
    popagrd
    pop rbp
    o64 sysret

    .undefined_syscall
    ud2

section .data

extern syscalltest
extern syscalltest2
extern syscalltest3

syscall_table:
    dq syscalltest
    dq syscalltest2
    dq syscalltest3
    dq 0x0000000000000000