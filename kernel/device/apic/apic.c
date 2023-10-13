#include "apic.h"
#include "device/io.h"
#include "device/8259/pic.h"
#include "boot/interrupts/isr.h"
#include "mm/paging/paging.h"

extern void* lapic_base;
extern void* ioapic_base;

static madt_t* madt_addr;
static uint64_t lapic_addr;
static uint64_t ioapic_addr;

static void cpu_set_apic_base(uintptr_t apic) {
    uint32_t edx = 0;
    uint32_t eax = (apic & 0xfffff0000) | IA32_APIC_BASE_MSR_ENABLE;
    edx = (apic >> 32) & 0x0f;
    wrmsr(IA32_APIC_BASE_MSR, eax, edx);
}

static uintptr_t cpu_get_apic_base() {
    uint32_t eax, edx;
    rdmsr(IA32_APIC_BASE_MSR, &eax, &edx);
    return (eax & 0xfffff000) | ((edx & 0x0f) << 32);
}
 
static void apic_enable() {
    cpu_set_apic_base(cpu_get_apic_base());
    write_lapic_reg(APIC_SPURIOUS, read_lapic_reg(APIC_SPURIOUS) | APIC_SW_ENABLE);
}

static void irq_spurious_handler(registers_t* registers) {
}

uint32_t read_lapic_reg(uint32_t reg) {
    return *(uint32_t*)(lapic_addr + reg);
}

void write_lapic_reg(uint32_t reg, uint32_t data) {
    uint32_t* ptr = (uint32_t*)(lapic_addr + reg);
    *ptr = data;
}

uint32_t read_ioapic_reg(uint32_t reg) {
    uint32_t volatile* ioapic = (uint32_t volatile*)ioapic_addr;
    ioapic[0] = (reg & 0xFF);
    return ioapic[4];
}

void write_ioapic_reg(uint32_t reg, uint32_t data) {
    uint32_t volatile* ioapic = (uint32_t volatile*)ioapic_addr;
    ioapic[0] = (reg & 0xFF);
    ioapic[4] = data;
}

void init_apic() {
    madt_addr = acpi_find_sdt("APIC");
    lapic_addr = (uint64_t)madt_addr->lapic_addr;
    uint8_t* ptr = (uint8_t*)madt_addr + sizeof(madt_t);
    uint8_t* ptr2 = (uint8_t*)madt_addr + madt_addr->header.length;
    for (; ptr < ptr2; ptr += ptr[1]) {
        switch (ptr[0]) {
            case 1: ioapic_addr = (uint64_t)((madt_entry_type_1_t*)ptr)->ioapic_address; break;
            case 5: lapic_addr = (uint64_t)((madt_entry_type_5_t*)ptr)->lapic_address; break;
        }
    }
    lapic_addr = (uint64_t)videntity(lapic_base, (void*)lapic_addr, 1, PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE | PAGE_FLAG_CACHEDBLE);
    ioapic_addr = (uint64_t)videntity(ioapic_base, (void*)ioapic_addr, 1, PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE | PAGE_FLAG_CACHEDBLE);
    pic_disable();
    apic_enable();
    register_interrupt_handler(IRQ7, irq_spurious_handler);
}

void init_apic_ap() {
    apic_enable();
}

void read_ioapic_redir(uint8_t irq, ioapic_redirection_t* entry) {
    uint64_t* tmp = (uint64_t*)entry;
    *tmp = read_ioapic_reg(0x10 + irq) | (read_ioapic_reg(0x10 + irq + 1) << 32);
}

void write_ioapic_redir(uint8_t irq, ioapic_redirection_t* entry) {
    uint64_t tmp = *(uint64_t*)entry;
    write_ioapic_reg(0x10 + irq * 2, (uint32_t)(tmp & 0xFFFFFFFF));
    write_ioapic_reg(0x10 + irq * 2 + 1, (uint32_t)((tmp >> 32) & 0xFFFFFFFF));
}

uint64_t apic_get_cpuid() {
    if (lapic_addr) {
        return (uint64_t)read_lapic_reg(APIC_APICID) >> 24;
    }
    return (uint16_t)-1;
}

void apic_send_eoi() {
    write_lapic_reg(APIC_EOI, 0);
}