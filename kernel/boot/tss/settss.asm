[GLOBAL setTSS]

setTSS:
    mov ax, 0x48
    ltr ax
    ret