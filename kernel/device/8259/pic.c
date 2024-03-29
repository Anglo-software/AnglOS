#include "pic.h"
#include "../io.h"

void picMaskIRQ(uint8_t irq) {
    uint16_t port;
    uint8_t masks;

    if (irq < 8) {
        port = PIC_MASTER_DATA;
    }
    else {
        port = PIC_SLAVE_DATA;
        irq -= 8;
    }

    masks = inb(port);
    masks |= (1 << irq);
    outb(port, masks);
}

void picUnmaskIRQ(uint8_t irq) {
    uint16_t port;
    uint8_t masks;

    if (irq < 8) {
        port = PIC_MASTER_DATA;
    }
    else {
        port = PIC_SLAVE_DATA;
        irq -= 8;
    }

    masks = inb(port);
    masks &= ~(1 << irq);
    outb(port, masks);
}

void picRemapOffsets(uint8_t offset) {
    uint8_t master_mask, slave_mask;

    master_mask = inb(PIC_MASTER_DATA);
    slave_mask  = inb(PIC_SLAVE_DATA);

    outb(PIC_MASTER_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    outb(PIC_SLAVE_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);

    outb(PIC_MASTER_DATA, offset);
    outb(PIC_SLAVE_DATA, offset + 0x08);

    outb(PIC_MASTER_DATA, 0x04);
    outb(PIC_SLAVE_DATA, 0x02);

    outb(PIC_MASTER_DATA, PIC_ICW4_8086);
    outb(PIC_SLAVE_DATA, PIC_ICW4_8086);

    outb(PIC_MASTER_DATA, master_mask);
    outb(PIC_SLAVE_DATA, slave_mask);
}

void picSendEOI(uint8_t irq) {
    if (irq >= 8)
        outb(PIC_SLAVE_COMMAND, PIC_EOI);
    outb(PIC_MASTER_COMMAND, PIC_EOI);
}

void picDisable() {
    picRemapOffsets(0x20);
    for (uint8_t irq = 0; irq < 16; irq++)
        picMaskIRQ(irq);
}
