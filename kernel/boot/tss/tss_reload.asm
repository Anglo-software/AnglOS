[GLOBAL tssReload]

tssReload:
    push rbp
    mov rbp, rsp
    pushfq
    cli
    ltr di
    popfq
    pop rbp
    ret