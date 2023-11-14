global threadContextSwitch

threadContextSwitch:
    push rbp
    mov rbp, rsp
    
    mov rax, [rdi + 0x58]
    mov rsp, [rax + 0x10]
    mov rcx, [rax + 0x18]
    mov rax, [rax + 0x28]
    mov r15, [rax + 0x78]
    mov cr3, r15
    mov rbx, [rax + 0x08]
    mov rdx, [rax + 0x18]
    mov rdi, [rax + 0x20]
    mov rsi, [rax + 0x28]
    mov r8,  [rax + 0x30]
    mov r9,  [rax + 0x38]
    mov r10, [rax + 0x40]
    mov r12, [rax + 0x50]
    mov r13, [rax + 0x58]
    mov r14, [rax + 0x60]
    mov r15, [rax + 0x68]
    mov rax, [rax + 0x00]
    mov r11, 0x202
    sti
    o64 sysret
