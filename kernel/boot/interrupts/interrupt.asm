[extern isrHandler]
[extern irqHandler]

%macro push_general 0
push rax
push rbx
push rcx
push rdx
push rsi
push rdi
%endmacro

%macro pop_general 0
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

irq_common_stub:
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
    call irqHandler

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
    call irqHandler

    pop rax
    pop rax
    mov ds, ax
    mov es, ax
    pop_control
    pop_general
    pop rbp
    add rsp, 0x10
    iretq

global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31

; 0: Divide By Zero Exception
isr0:
    push qword 0
    push qword 0
    jmp isr_common_stub

; 1: Debug Exception
isr1:
    push byte 0
    push byte 1
    jmp isr_common_stub

; 2: Non Maskable Interrupt Exception
isr2:
    push byte 0
    push byte 2
    jmp isr_common_stub

; 3: Int 3 Exception
isr3:
    push byte 0
    push byte 3
    jmp isr_common_stub

; 4: INTO Exception
isr4:
    push byte 0
    push byte 4
    jmp isr_common_stub

; 5: Out of Bounds Exception
isr5:
    push byte 0
    push byte 5
    jmp isr_common_stub

; 6: Invalid Opcode Exception
isr6:
    push byte 0
    push byte 6
    jmp isr_common_stub

; 7: Coprocessor Not Available Exception
isr7:
    push byte 0
    push byte 7
    jmp isr_common_stub

; 8: Double Fault Exception (With Error Code!)
isr8:
    push byte 8
    jmp isr_common_stub

; 9: Coprocessor Segment Overrun Exception
isr9:
    push byte 0
    push byte 9
    jmp isr_common_stub

; 10: Bad TSS Exception (With Error Code!)
isr10:
    push byte 10
    jmp isr_common_stub

; 11: Segment Not Present Exception (With Error Code!)
isr11:
    push byte 11
    jmp isr_common_stub

; 12: Stack Fault Exception (With Error Code!)
isr12:
    push byte 12
    jmp isr_common_stub

; 13: General Protection Fault Exception (With Error Code!)
isr13:
    push byte 13
    jmp isr_common_stub

; 14: Page Fault Exception (With Error Code!)
isr14:
    push byte 14
    jmp isr_common_stub

; 15: Reserved Exception
isr15:
    push byte 0
    push byte 15
    jmp isr_common_stub

; 16: Floating Point Exception
isr16:
    push byte 0
    push byte 16
    jmp isr_common_stub

; 17: Alignment Check Exception
isr17:
    push byte 0
    push byte 17
    jmp isr_common_stub

; 18: Machine Check Exception
isr18:
    push byte 0
    push byte 18
    jmp isr_common_stub

; 19: Reserved
isr19:
    push byte 0
    push byte 19
    jmp isr_common_stub

; 20: Reserved
isr20:
    push byte 0
    push byte 20
    jmp isr_common_stub

; 21: Reserved
isr21:
    push byte 0
    push byte 21
    jmp isr_common_stub

; 22: Reserved
isr22:
    push byte 0
    push byte 22
    jmp isr_common_stub

; 23: Reserved
isr23:
    push byte 0
    push byte 23
    jmp isr_common_stub

; 24: Reserved
isr24:
    push byte 0
    push byte 24
    jmp isr_common_stub

; 25: Reserved
isr25:
    push byte 0
    push byte 25
    jmp isr_common_stub

; 26: Reserved
isr26:
    push byte 0
    push byte 26
    jmp isr_common_stub

; 27: Reserved
isr27:
    push byte 0
    push byte 27
    jmp isr_common_stub

; 28: Reserved
isr28:
    push byte 0
    push byte 28
    jmp isr_common_stub

; 29: Reserved
isr29:
    push byte 0
    push byte 29
    jmp isr_common_stub

; 30: Reserved
isr30:
    push byte 0
    push byte 30
    jmp isr_common_stub

; 31: Reserved
isr31:
    push byte 0
    push byte 31
    jmp isr_common_stub

global irq0
global irq1
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15

irq0:
	push byte 0
	push byte 32
	jmp irq_common_stub

irq1:
	push byte 1
	push byte 33
	jmp irq_common_stub

irq2:
	push byte 2
	push byte 34
	jmp irq_common_stub

irq3:
	push byte 3
	push byte 35
	jmp irq_common_stub

irq4:
	push byte 4
	push byte 36
	jmp irq_common_stub

irq5:
	push byte 5
	push byte 37
	jmp irq_common_stub

irq6:
	push byte 6
	push byte 38
	jmp irq_common_stub

irq7:
	push byte 7
	push byte 39
	jmp irq_common_stub

irq8:
	push byte 8
	push byte 40
	jmp irq_common_stub

irq9:
	push byte 9
	push byte 41
	jmp irq_common_stub

irq10:
	push byte 10
	push byte 42
	jmp irq_common_stub

irq11:
	push byte 11
	push byte 43
	jmp irq_common_stub

irq12:
	push byte 12
	push byte 44
	jmp irq_common_stub

irq13:
	push byte 13
	push byte 45
	jmp irq_common_stub

irq14:
	push byte 14
	push byte 46
	jmp irq_common_stub

irq15:
	push byte 15
	push byte 47
	jmp irq_common_stub