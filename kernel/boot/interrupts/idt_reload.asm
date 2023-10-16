[GLOBAL idtReload]

idtReload:
    push rbp
    mov rbp, rsp
    pushfq
    cli
    lidt [rdi]
    popfq
    pop rbp
    ret
    