#include "tss.h"

extern void setTSS();

uint32_t tss_cpu_0[] = {
    0x00000000, // 00 Reserved
    0x00000000, // 01 RSP0 Low
    0x00000000, // 02 RSP0 High
    0x00000000, // 03 RSP1 Low
    0x00000000, // 04 RSP1 High
    0x00000000, // 05 RSP2 Low
    0x00000000, // 06 RSP2 High
    0x00000000, // 07 Reserved
    0x00000000, // 08 Reserved
    0x00000000, // 09 IST1 Low
    0x00000000, // 10 IST1 High
    0x00000000, // 11 IST2 Low
    0x00000000, // 12 IST2 High
    0x00000000, // 13 IST3 Low
    0x00000000, // 14 IST3 High
    0x00000000, // 15 IST4 Low
    0x00000000, // 16 IST4 High
    0x00000000, // 17 IST5 Low
    0x00000000, // 18 IST5 High
    0x00000000, // 19 IST6 Low
    0x00000000, // 20 IST6 High
    0x00000000, // 21 IST7 Low
    0x00000000, // 22 IST7 High
    0x00000000, // 23 Reserved
    0x00000000, // 24 Reserved
    0x00000000, // 25 IOPB 2 bytes | Reserved 2 bytes
};

void init_tss() {
    tss_cpu_0[25] = sizeof(tss_cpu_0) << 16;
    setTSS();
}