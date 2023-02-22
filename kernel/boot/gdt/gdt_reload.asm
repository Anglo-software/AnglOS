[GLOBAL gdt_reload]

gdt_reload:
    lgdt  [rdi]
    push 0x28                 ; Push code segment to stack, 0x08 is a stand-in for your code segment
    lea rax, [rel .reload_CS] ; Load address of .reload_CS into RAX
    push rax                  ; Push this value to the stack
    retfq                     ; Perform a far return, RETFQ or LRETQ depending on syntax
.reload_CS:
    ; Reload data segment registers
    mov   ax, 0x30 ; 0x10 is a stand-in for your data segment
    mov   ds, ax
    mov   es, ax
    mov   fs, ax
    mov   gs, ax
    mov   ss, ax
    ret
    