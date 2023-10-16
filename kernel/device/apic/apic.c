#include "apic.h"
#include "boot/interrupts/isr.h"
#include "device/8259/pic.h"
#include "device/io.h"
#include "mm/paging/paging.h"

extern void* lapic_base;
extern void* ioapic_base;

static madt_t* madt_addr;
static uint64_t lapic_addr;
static uint64_t ioapic_addr;

static void apicSetBase(uintptr_t apic) {
    uint32_t edx = 0;
    uint32_t eax = (apic & 0xfffff0000) | IA32_APIC_BASE_MSR_ENABLE;
    edx          = (apic >> 32) & 0x0f;
    wrmsr(IA32_APIC_BASE_MSR, eax, edx);
}

static uintptr_t apicGetBase() {
    uint32_t eax, edx;
    rdmsr(IA32_APIC_BASE_MSR, &eax, &edx);
    return (uint64_t)(eax & 0xfffff000) | ((uint64_t)(edx & 0x0f) << 32);
}

static void apicEnable() {
    apicSetBase(apicGetBase());
    lapicWriteReg(APIC_SPURIOUS, lapicReadReg(APIC_SPURIOUS) | APIC_SW_ENABLE);
}

static void irqSpuriousHandler(registers_t* registers) {}

uint32_t lapicReadReg(uint32_t reg) { return *(uint32_t*)(lapic_addr + reg); }

void lapicWriteReg(uint32_t reg, uint32_t data) {
    uint32_t* ptr = (uint32_t*)(lapic_addr + reg);
    *ptr          = data;
}

uint32_t ioapicReadReg(uint32_t reg) {
    uint32_t volatile* ioapic = (uint32_t volatile*)ioapic_addr;
    ioapic[0]                 = (reg & 0xFF);
    return ioapic[4];
}

void ioapicWriteReg(uint32_t reg, uint32_t data) {
    uint32_t volatile* ioapic = (uint32_t volatile*)ioapic_addr;
    ioapic[0]                 = (reg & 0xFF);
    ioapic[4]                 = data;
}

void initAPIC() {
    madt_addr     = (madt_t*)acpiFindSDT("APIC");
    lapic_addr    = (uint64_t)madt_addr->lapic_addr;
    uint8_t* ptr  = (uint8_t*)madt_addr + sizeof(madt_t);
    uint8_t* ptr2 = (uint8_t*)madt_addr + madt_addr->header.length;
    for (; ptr < ptr2; ptr += ptr[1]) {
        switch (ptr[0]) {
        case 1:
            ioapic_addr = (uint64_t)((madt_entry_type_1_t*)ptr)->ioapic_address;
            break;
        case 5:
            lapic_addr = (uint64_t)((madt_entry_type_5_t*)ptr)->lapic_address;
            break;
        }
    }
    lapic_addr  = (uint64_t)videntity(lapic_base, (void*)lapic_addr, 1,
                                      PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE |
                                          PAGE_FLAG_CACHEDBLE);
    ioapic_addr = (uint64_t)videntity(ioapic_base, (void*)ioapic_addr, 1,
                                      PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE |
                                          PAGE_FLAG_CACHEDBLE);
    picDisable();
    apicEnable();
    isrRegisterHandler(IRQ7, irqSpuriousHandler);
}

void initAPAPIC() { apicEnable(); }

void ioapicReadRedir(uint8_t irq, ioapic_redirection_t* entry) {
    uint64_t* tmp = (uint64_t*)entry;
    *tmp          = (uint64_t)ioapicReadReg(0x10 + irq) |
           (uint64_t)(ioapicReadReg(0x10 + irq + 1) << 32);
}

void ioapicWriteRedir(uint8_t irq, ioapic_redirection_t* entry) {
    uint64_t tmp = *(uint64_t*)entry;
    ioapicWriteReg(0x10 + irq * 2, (uint32_t)(tmp & 0xFFFFFFFF));
    ioapicWriteReg(0x10 + irq * 2 + 1, (uint32_t)((tmp >> 32) & 0xFFFFFFFF));
}

uint64_t apicGetCpuID() {
    if (lapic_addr) {
        return (uint64_t)lapicReadReg(APIC_APICID) >> 24;
    }
    return (uint16_t)-1;
}

void apicSendEOI() { lapicWriteReg(APIC_EOI, 0); }