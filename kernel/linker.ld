OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(_start)

PHDRS
{
    text      PT_LOAD  FLAGS((1 << 0) | (1 << 2)) ; /* Execute + Read */
    userentry PT_LOAD  FLAGS((1 << 0) | (1 << 2)) ; /* Execute + Read */
    syscall   PT_LOAD  FLAGS((1 << 0) | (1 << 2)) ; /* Execute + Read */
    rodata    PT_LOAD  FLAGS((1 << 2)) ;            /* Read only */
    data      PT_LOAD  FLAGS((1 << 1) | (1 << 2)) ; /* Write + Read */
}
 
SECTIONS
{
    . = 0xffffffff80000000;

    MAXPAGESIZE = 0x1000;
 
    .text : {
        *(.text .text.*)
    } :text
 
    . = ALIGN(MAXPAGESIZE);

    .userentry : {
        *(.userentry .userentry.*)
    } :userentry

    . = ALIGN(MAXPAGESIZE);

    .syscall : {
        *(.syscall .syscall.*)
    } :syscall

    . = ALIGN(MAXPAGESIZE);
 
    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    . = ALIGN(MAXPAGESIZE);
 
    .data : {
        *(.data .data.*)
    } :data
 
    .bss : {
        *(COMMON)
        *(.bss .bss.*)
    } :data

    /DISCARD/ : {
        *(.eh_frame)
        *(.note .note.*)
    }
}